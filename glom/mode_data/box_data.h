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

#ifndef BOX_DATA_H
#define BOX_DATA_H

#include "../box_db_table.h"
#include "dialog_layout.h"

/**
  *@author Murray Cumming
  */

class Box_Data : public Box_DB_Table
{
public: 
  Box_Data();
  virtual ~Box_Data();

  virtual Glib::ustring get_WhereClause() const;

  virtual void set_unstored_data(bool bVal);
  virtual bool get_unstored_data() const;

  virtual bool confirm_discard_unstored_data() const;

  virtual void show_layout_dialog();
  
  //Signals:

  //signal_find: Used by _Find sub-classes.
  //Should be a MI class, derived by those sub-classes. TODO.
  //where_clause.
  sigc::signal<void, Glib::ustring> signal_find;

protected:
  virtual void fill_from_database(); //override.

  type_vecFields get_fields_to_show() const;
  type_vecFields get_table_fields_to_show(const Glib::ustring& table_name) const;

  typedef Document_Glom::type_vecRelationships type_vecRelationships;
  type_vecRelationships get_relationships_triggered_by(const Glib::ustring& field_name) const;

  type_vecFields get_lookup_fields(const Glib::ustring& relationship_name) const;

  static Glib::ustring value_to_string(const Gnome::Gda::Value& value);
  
  virtual bool record_new_from_entered(); //New record with all entered field values.
  guint generate_next_auto_increment(const Glib::ustring& table_name, const Glib::ustring field_name);

  //Signal handlers:
  virtual void on_Button_Find(); //only used by _Find sub-classes. Should be MI.
  virtual void on_dialog_layout_hide();
 
  Gtk::Button m_Button_Find; //only used by _Find sub-classes. Should be MI.
  Gtk::Label m_Label_FindStatus;

  bool m_bUnstoredData;

  Dialog_Layout* m_pDialogLayout;
  Glib::ustring m_layout_name;
};

#endif
