/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "box_data_list.h"
#include "../data_structure/glomconversions.h"
#include "dialog_layout_list.h"
#include <sstream> //For stringstream
#include <libintl.h>

Box_Data_List::Box_Data_List()
: m_has_one_or_more_records(false)
{
  m_layout_name = "list";

  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_data_layout_list"); //TODO: Use a generic layout dialog?
  if(refXml)
  {
    Dialog_Layout_List* dialog = 0;
    refXml->get_widget_derived("window_data_layout_list", dialog);
    if(dialog)
    {
      m_pDialogLayout = dialog;
      m_pDialogLayout->signal_hide().connect( sigc::mem_fun(*this, &Box_Data::on_dialog_layout_hide) );
    }
  }

  m_strHint = gettext("When you change the data in a field the database is updated immediately.\n Click [Add] or enter data into the last row to add a new record.\n Leave automatic ID fields empty - they will be filled for you.\nOnly the first 100 records are shown.");

  pack_start(m_AddDel);
  m_AddDel.set_auto_add(false); //We want to add the row ourselves when the user clicks the Add button, because the default behaviour there is not suitable.
  m_AddDel.set_rules_hint(); //Use alternating row colors when the theme does that.

  //Connect signals:
  m_AddDel.signal_user_requested_add().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_requested_add)); //Only emitted when m_AddDel.set_auto_add(false) is used.
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_requested_edit));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_requested_delete));
  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_added));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_changed));
  m_AddDel.signal_user_reordered_columns().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_reordered_columns));

  m_AddDel.signal_user_requested_layout().connect(sigc::mem_fun(*this, &Box_Data_List::on_adddel_user_requested_layout));


  //Groups are not very helpful for a list view:
  //m_pDialogLayout->set_show_groups(false);

  m_AddDel.show();
}

Box_Data_List::~Box_Data_List()
{
}

void Box_Data_List::fill_from_database()
{
  Bakery::BusyCursor(*get_app_window());

  try
  {
    sharedptr<SharedConnection> sharedconnection = connect_to_server();

    Box_DB_Table::fill_from_database();

    m_AddDel.remove_all();

    //Field Names:
    fill_column_titles();

    //if(sharedconnection)
    //{
      //Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

    Glib::ustring strWhereClausePiece;
    if(!m_strWhereClause.empty())
      strWhereClausePiece = " WHERE " + m_strWhereClause;

    m_Fields = get_fields_to_show();

    //Add extra possibly-non-visible columns that we need:
    type_vecLayoutFields fieldsToGet = m_Fields;

    LayoutItem_Field layout_item;
    Field field_key = m_AddDel.get_key_field();
    layout_item.set_name(field_key.get_name());
    layout_item.m_field = field_key;
    fieldsToGet.push_back(layout_item);
    const int index_primary_key = fieldsToGet.size() - 1;

    if(!fieldsToGet.empty())
    {
      const Glib::ustring query = build_sql_select_with_where_clause(m_strTableName, fieldsToGet, m_strWhereClause);

      Glib::RefPtr<Gnome::Gda::DataModel> result = Query_execute(query);
      if(!result)
        handle_error();
      else
      {
        //Field contents:
        m_AddDel.remove_all();

        //each row:
        guint rows_count = result->get_n_rows();
        if(rows_count)
          m_has_one_or_more_records = true; //Save it for later.

        if(rows_count > 100)
          rows_count = 100; //Don't get more than 100. TODO: Get other rows dynamically.

        for(guint result_row = 0; result_row < (rows_count); result_row++)
        {
          Gnome::Gda::Value key = result->get_value_at(index_primary_key, result_row);
          //It is usually an integer.
          if(GlomConversions::value_is_empty(key))
            g_warning("Box_Data_List::fill_from_database(): primary key value is empty");
          else
          {
            Gnome::Gda::Value value_primary_key = result->get_value_at(index_primary_key, result_row);

            Gtk::TreeModel::iterator tree_iter = m_AddDel.add_item(value_primary_key);

            type_vecLayoutFields::const_iterator iterFields = fieldsToGet.begin();

            //each field:
            //We use cols_count -1 because we have an extra field for the primary_key.
            guint cols_count = result->get_n_columns() - 1;
            for(guint uiCol = 0; uiCol < cols_count; uiCol++)
            {
              const Gnome::Gda::Value value = result->get_value_at(uiCol, result_row);
              const LayoutItem_Field& layout_item = *iterFields;

              //g_warning("list fill: field_name=%s", iterFields->get_name().c_str());
              //g_warning("  value_as_string=%s", value.to_string().c_str());

              m_AddDel.set_value(tree_iter, layout_item, value);
               //g_warning("addedel size=%d", m_AddDel.get_count());

              ++iterFields;
            }
          }
        }
      }
    } //If !fieldsToGet.empty()


    //Select first record:
    Glib::RefPtr<Gtk::TreeModel> refModel = m_AddDel.get_model();
    if(refModel)
      m_AddDel.select_item(refModel->children().begin());

    fill_end();

  }
  catch(std::exception& ex)
  {
    handle_error(ex);
  }
}

void Box_Data_List::on_adddel_user_requested_add()
{
  Gtk::TreeModel::iterator iter = m_AddDel.get_item_placeholder();
  if(iter)
  {
    //Start editing in the primary key or the first cell if the primary key is auto-incremented (because there is no point in editing an auto-generated value)..
    guint index_primary_key = 0;
    bool bPresent = get_field_primary_key_index(index_primary_key); //If there is no primary key then the default of 0 is OK.
    guint index_field_to_edit = 0;
    if(bPresent)
    {
      index_field_to_edit = index_primary_key;

      Field fieldPrimaryKey = m_Fields[index_primary_key].m_field;
      if(fieldPrimaryKey.get_field_info().get_auto_increment())
      {
        //Start editing in the first cell that is not the primary key:
        if(index_primary_key == 0)
        {
          index_field_to_edit += 1;
        }
        else
          index_field_to_edit = 0;
      }
    }

    if(index_field_to_edit < m_Fields.size())
    {
      guint treemodel_column = 0;
      bool test = get_field_column_index(m_Fields[index_field_to_edit].get_name(), treemodel_column);
      if(test)
        m_AddDel.select_item(iter, treemodel_column, true /* start_editing */);
    }
    else
    {
      //The only keys are non-editable, so just add a row:
      on_adddel_user_added(iter);
      m_AddDel.select_item(iter); //without start_editing.
      //g_warning("Box_Data_List::on_adddel_user_requested_add(): index_field_to_edit does not exist: %d", index_field_to_edit);
    }
  }
}

void Box_Data_List::on_adddel_user_requested_edit(const Gtk::TreeModel::iterator& row)
{
  Gnome::Gda::Value primary_key_value = m_AddDel.get_value_key(row); //The primary key is in the key.

  signal_user_requested_details().emit(primary_key_value);
}

void Box_Data_List::on_adddel_user_requested_layout()
{
  show_layout_dialog();
}

void Box_Data_List::on_adddel_user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator&  /* rowEnd TODO */)
{
  if(rowStart)
  {
    //Ask the user for confirmation:
    Gtk::MessageDialog dialog(gettext("<b>Delete record</b>"), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
    dialog.set_secondary_text(gettext("Are you sure that you would like to delete this record? The data in this record will then be permanently lost."));
    dialog.set_transient_for(*get_app_window());
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::DELETE, Gtk::RESPONSE_OK);
    
    int response = dialog.run();
    if(response == Gtk::RESPONSE_OK)
    {
      record_delete( get_primary_key_value(rowStart) );

      //Remove the row:
      m_AddDel.remove_item(rowStart);
    }
  }
}

void Box_Data_List::on_adddel_user_added(const Gtk::TreeModel::iterator& row)
{
  Gnome::Gda::Value primary_key_value;

  Field field_primary_key = m_AddDel.get_key_field();

  if(field_primary_key.get_field_info().get_auto_increment())
  {
    //Auto-increment is awkward (we can't get the last-generated ID) with postgres, so we auto-generate it ourselves;
    const Glib::ustring& strPrimaryKeyName = field_primary_key.get_name();
    primary_key_value = generate_next_auto_increment(m_strTableName, strPrimaryKeyName);  //TODO: return a Gnome::Gda::Value of an appropriate type.
  }
  else
  {
    primary_key_value = get_primary_key_value(row);
  }

  sharedptr<SharedConnection> sharedconnection = connect_to_server(); //Keep it alive while we need the data_model.
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = record_new(true /* use entered field data*/, primary_key_value);
    if(data_model)
    {
      //Save the primary key value for later use:
      m_AddDel.set_value_key(row, primary_key_value);

      //Show the primary key in the row, if the primary key is visible:
      const Gnome::Gda::FieldAttributes fieldInfo = field_primary_key.get_field_info();
      //If it's an auto-increment, then get the value and show it:
      if(fieldInfo.get_auto_increment())
      {
        LayoutItem_Field layout_item;
        layout_item.set_name(field_primary_key.get_name());
        layout_item.m_field = field_primary_key;
        m_AddDel.set_value(row, layout_item, primary_key_value);
      }

      //Allow derived class to respond to record addition.
      on_record_added(primary_key_value);
    }
    else
      handle_error();
  }
  else
  {
    //Add Record failed.
    //Replace with correct values:
    fill_from_database();
  }
}

void Box_Data_List::on_adddel_user_reordered_columns()
{
  Document_Glom* pDoc = dynamic_cast<Document_Glom*>(get_document());
  if(pDoc)
  {
    LayoutGroup group;
    group.set_name("toplevel");
    
    AddDel::type_vecStrings vec_field_names = m_AddDel.get_columns_order();

    guint index = 0;
    for(AddDel::type_vecStrings::iterator iter = vec_field_names.begin(); iter != vec_field_names.end(); ++iter)
    {
      LayoutItem_Field layout_item;
      layout_item.set_name(*iter);
      layout_item.m_sequence = index;
      group.add_item(layout_item, index); 
      ++index;
    }

    Document_Glom::type_mapLayoutGroupSequence mapGroups;
    mapGroups[1] = group;

    pDoc->set_data_layout_groups("list", m_strTableName, mapGroups);  
  }
}

void Box_Data_List::on_adddel_user_changed(const Gtk::TreeModel::iterator& row, guint col)
{
  const Gnome::Gda::Value parent_primary_key_value = get_primary_key_value(row);
  if(!GlomConversions::value_is_empty(parent_primary_key_value)) //If the record's primary key is filled in:
  {
    //Just update the record:
    try
    {
      LayoutItem_Field layout_field = m_AddDel.get_column_field(col);

      Glib::ustring table_name = m_strTableName;
      Field primary_key_field;
      Gnome::Gda::Value primary_key_value;

      if(!layout_field.get_has_relationship_name())
      {
        table_name = m_strTableName;
        primary_key_field = m_AddDel.get_key_field();
        primary_key_value = parent_primary_key_value;
      }
      else
      {
        //If it's a related field then discover the actual table that it's in,
        //plus how to identify the record in that table.
        const Glib::ustring relationship_name = layout_field.get_relationship_name();

        Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());

        Relationship relationship;
        bool test = document->get_relationship(m_strTableName, relationship_name, relationship);
        if(test)
        {
          table_name = relationship.get_to_table();
          const Glib::ustring to_field_name = relationship.get_to_field();
          //Get the key field in the other table (the table that we will change)
          bool test = get_fields_for_table_one_field(table_name, to_field_name, primary_key_field); //TODO_Performance.
          if(test)
          {
            //Get the value of the corresponding key in the current table (that identifies the record in the table that we will change)
            LayoutItem_Field layout_item;
            layout_item.set_name(relationship.get_from_field());

            primary_key_value = get_entered_field_data(layout_item);
            //TODO: If the primary_key is empty, or not yet valid, should we create a new record. It probably has to be anoption in the relationship.
          }
          else
          {
            g_warning("Box_Data_Details::on_flowtable_field_edited(): key not found for edited related field.");
          }
        }
      }


      const Gnome::Gda::Value field_value = m_AddDel.get_value(row, layout_field);
      const Field& field = layout_field.m_field;
      const Glib::ustring strFieldName = layout_field.get_name();

      Glib::ustring strQuery = "UPDATE " + table_name;
      strQuery += " SET " +  /* table_name + "." + postgres does not seem to like the table name here */ strFieldName + " = " + field.sql(field_value);
      strQuery += " WHERE " + table_name + "." + primary_key_field.get_name() + " = " + primary_key_field.sql(primary_key_value);
      bool bTest = Query_execute(strQuery);
      if(!bTest)
      {
        //Update failed.
        fill_from_database(); //Replace with correct values.
      }
      else
      {
        //Get-and-set values for lookup fields, if this field triggers those relationships:
        do_lookups(row, layout_field, field_value, primary_key_field, primary_key_value);

        //Update related fields, if this field is used in the relationship:
        refresh_related_fields(row, layout_field, field_value, primary_key_field, primary_key_value);
      }
    }
    catch(const std::exception& ex)
    {
      handle_error(ex);
    }
  }
  else
  {
    //This record probably doesn't exist yet.
    //Add new record, which will generate the primary key:

    if(m_AddDel.get_key_field().get_field_info().get_auto_increment())
    {
      on_adddel_user_added(row);

       const Gnome::Gda::Value primaryKeyValue = get_primary_key_value(row); //TODO_Value
       if(!(GlomConversions::value_is_empty(primaryKeyValue))) //If the Add succeeeded:
         on_adddel_user_changed(row, col); //Change this field in the new record.
    }
  }

}

void Box_Data_List::refresh_related_fields(const Gtk::TreeModel::iterator& row, const LayoutItem_Field& field_changed, const Gnome::Gda::Value& /* field_value */, const Field& primary_key, const Gnome::Gda::Value& primary_key_value)
{
  if(field_changed.get_has_relationship_name())
    return; //TODO: Handle these too.

  //Get values for lookup fields, if this field triggers those relationships:
  //TODO_performance: There is a LOT of iterating and copying here.
  const Glib::ustring strFieldName = field_changed.get_name();
  type_vecLayoutFields fieldsToGet = get_related_fields(strFieldName);

  if(!fieldsToGet.empty())
  {
    const Glib::ustring query = build_sql_select(m_strTableName, fieldsToGet, primary_key, primary_key_value);

    Glib::RefPtr<Gnome::Gda::DataModel> result = Query_execute(query);
    if(!result)
      handle_error();
    else
    {
      //Field contents:
      if(result->get_n_rows())
      {
        type_vecLayoutFields::const_iterator iterFields = fieldsToGet.begin();

        guint cols_count = result->get_n_columns();
        for(guint uiCol = 0; uiCol < cols_count; uiCol++)
        {
          const Gnome::Gda::Value value = result->get_value_at(uiCol, 0 /* row */);
          const LayoutItem_Field& layout_item = *iterFields;

          //g_warning("list fill: field_name=%s", iterFields->get_name().c_str());
          //g_warning("  value_as_string=%s", value.to_string().c_str());

          m_AddDel.set_value(row, layout_item, value);
            //g_warning("addedel size=%d", m_AddDel.get_count());

          ++iterFields;
        }
      }
    }
  }
}

void Box_Data_List::do_lookups(const Gtk::TreeModel::iterator& row, const LayoutItem_Field& field_changed, const Gnome::Gda::Value& field_value, const Field& primary_key, const Gnome::Gda::Value& primary_key_value)
{
   if(field_changed.get_has_relationship_name())
    return; //TODO: Handle these too.

   //Get values for lookup fields, if this field triggers those relationships:
   //TODO_performance: There is a LOT of iterating and copying here.
   const Glib::ustring strFieldName = field_changed.get_name();
   type_list_lookups lookups = get_lookup_fields(strFieldName);
   for(type_list_lookups::const_iterator iter = lookups.begin(); iter != lookups.end(); ++iter)
   {
     const LayoutItem_Field& layout_Item = iter->first;

     const Relationship relationship = iter->second;
     const Field& field_lookup = layout_Item.m_field;

     Field field_source;
     bool test = get_fields_for_table_one_field(relationship.get_to_table(), field_lookup.get_lookup_field(), field_source);
     if(test)
     {
       Gnome::Gda::Value value = get_lookup_value(iter->second /* relationship */,  field_source /* the field to look in to get the value */, field_value /* Value of to and from fields */);

       //Add it to the view:
       m_AddDel.set_value(row, layout_Item, value);

       //Add it to the database (even if it is not shown in the view)
       Glib::ustring strQuery = "UPDATE " + m_strTableName;
       strQuery += " SET " + field_lookup.get_name() + " = " + field_lookup.sql(value);
       strQuery += " WHERE " + primary_key.get_name() + " = " + primary_key.sql(primary_key_value);
       Query_execute(strQuery);  //TODO: Handle errors

       //TODO: Handle lookups triggered by these fields (recursively)? TODO: Check for infinitely looping lookups.
     }
   }

}

void Box_Data_List::on_details_nav_first()
{
  m_AddDel.select_item(m_AddDel.get_model()->children().begin());

  signal_user_requested_details().emit(m_AddDel.get_value_key_selected());
}

void Box_Data_List::on_details_nav_previous()
{
  Gtk::TreeModel::iterator iter = m_AddDel.get_item_selected();
  if(iter)
  {
    //Don't try to select a negative record number.
    if(!m_AddDel.get_is_first_row(iter))
    {
      iter--;

      m_AddDel.select_item(iter);
      signal_user_requested_details().emit(m_AddDel.get_value_key_selected());
    }
  }
}

void Box_Data_List::on_details_nav_next()
{
  Gtk::TreeModel::iterator iter = m_AddDel.get_item_selected();
  if(iter)
  {
    //Don't go past the last record:
    if( !m_AddDel.get_is_last_row(iter) )
    {
      iter++;    
      m_AddDel.select_item(iter);

      signal_user_requested_details().emit(m_AddDel.get_value_key_selected());
    }
  }
}

void Box_Data_List::on_details_nav_last()
{
  Gtk::TreeModel::iterator iter = m_AddDel.get_last_row();
  if(iter)
  {
    m_AddDel.select_item(iter);
    signal_user_requested_details().emit(m_AddDel.get_value_key_selected());
  }
}

void Box_Data_List::on_Details_record_deleted(Gnome::Gda::Value primary_key_value)
{
  //Find out which row is affected:
  Gtk::TreeModel::iterator iter = m_AddDel.get_row(primary_key_value);
  if(iter)
  {
    //Remove the row:
    Gtk::TreeModel::iterator iterNext = iter;
    iterNext++;

    m_AddDel.remove_item(iter);

    //Show Details for the next one:
    if(iterNext != m_AddDel.get_model()->children().end())
    {
      //Next record moves up one:
      on_adddel_user_requested_edit(iterNext);
    }
    else
    {
      //Just show the last one:
      on_details_nav_last();
    }
  }
  else
  {
    //Just update everything and go the first record.
    //This shouldn't happen.
    fill_from_database();
    on_details_nav_first();
  }
}

Gnome::Gda::Value Box_Data_List::get_primary_key_value(const Gtk::TreeModel::iterator& row)
{
  return m_AddDel.get_value_key(row);
}

Gnome::Gda::Value Box_Data_List::get_primary_key_value_selected()
{
  return Gnome::Gda::Value(m_AddDel.get_value_key_selected());
}

Gnome::Gda::Value Box_Data_List::get_entered_field_data(const LayoutItem_Field& field) const
{
  return m_AddDel.get_value_selected(field);
}

void Box_Data_List::set_entered_field_data(const LayoutItem_Field& field, const Gnome::Gda::Value& value)
{
  return m_AddDel.set_value_selected(field, value);
}

guint Box_Data_List::get_records_count() const
{
  return m_AddDel.get_count();
}

void Box_Data_List::fill_column_titles()
{
  const Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
  if(pDoc)
  {
    //Field Names:
    m_AddDel.remove_all_columns();
    //m_AddDel.set_columns_count(m_Fields.size());

    Field field_primary_key;
    bool test = get_field_primary_key_for_table(m_strTableName, field_primary_key);
    if(test)
        m_AddDel.set_key_field(field_primary_key);


    type_vecLayoutFields listFieldsToShow = get_fields_to_show();

    //Add a column for each table field:
    for(type_vecLayoutFields::const_iterator iter =  listFieldsToShow.begin(); iter != listFieldsToShow.end(); ++iter)
    {
      m_AddDel.add_column(*iter);
    }
  }

}

void Box_Data_List::on_record_added(const Gnome::Gda::Value& /* strPrimaryKey */)
{
  //Overridden by Box_Data_List_Related.
  //m_AddDel.add_item(strPrimaryKey); //Add blank row.
}

Box_Data_List::type_signal_user_requested_details Box_Data_List::signal_user_requested_details()
{
  return m_signal_user_requested_details;
}

bool Box_Data_List::get_field_column_index(const Glib::ustring& field_name, guint& index) const
{
  //Initialize output parameter:
  index = 0;

  //Get the index of the field with this name:
  guint i = 0;
  for(type_vecLayoutFields::const_iterator iter = m_Fields.begin(); iter != m_Fields.end(); ++iter)
  {
    if(iter->get_name() == field_name)
    {
      return m_AddDel.get_model_column_index(i, index); //Add the extra model columns to get the model column index from the field column index
    }

    ++i;
  }

  return false; //failure.
}

bool Box_Data_List::get_field_primary_key(Field& field) const
{
  field = m_AddDel.get_key_field();
  return true;
}

bool Box_Data_List::get_field_primary_key_index(guint& field_column) const
{
  return Box_Data::get_field_primary_key_index(m_Fields, field_column);
}

