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

#include "entryglom.h"
#include "../data_structure/glomconversions.h"
#include <gtkmm/messagedialog.h>
#include "../dialog_invalid_data.h"
#include "../data_structure/glomconversions.h"
#include "../application.h"
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl

EntryGlom::EntryGlom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /* refGlade */)
: Gtk::Entry(cobject),
  m_glom_type(Field::TYPE_TEXT)
{
  setup_menu();
}

EntryGlom::EntryGlom(Field::glom_field_type glom_type)
: m_glom_type(glom_type)
{
  setup_menu();
}

EntryGlom::~EntryGlom()
{
}

void EntryGlom::set_glom_type(Field::glom_field_type glom_type)
{
  m_glom_type = glom_type;
}

void EntryGlom::check_for_change()
{
  Glib::ustring new_text = get_text();
  if(new_text != m_old_text)
  {
    //Validate the input:
    bool success = false;

    const LayoutItem_Field* layout_item = dynamic_cast<const LayoutItem_Field*>(get_layout_item());
    Gnome::Gda::Value value = GlomConversions::parse_value(m_glom_type, get_text(), layout_item->m_numeric_format, success);

    if(success)
    {
      //Actually show the canonical text:
      set_value(value);
      m_signal_edited.emit(); //The text was edited, so tell the client code.
    }
    else
    {
      //Tell the user and offer to revert or try again:
      bool revert = glom_show_dialog_invalid_date(m_glom_type);
      if(revert)
      {
        set_text(m_old_text);
      }
      else
        grab_focus(); //Force the user back into the same field, so that the field can be checked again and eventually corrected or reverted.
    }
  }
}

EntryGlom::type_signal_edited EntryGlom::signal_edited()
{
  return m_signal_edited;
}

bool EntryGlom::on_focus_out_event(GdkEventFocus* event)
{
  bool result = Gtk::Entry::on_focus_out_event(event);

  //The user has finished editing.
  check_for_change();

  //Call base class:
  return result;
}

void EntryGlom::on_activate()
{ 
  //Call base class:
  Gtk::Entry::on_activate();

  //The user has finished editing.
  check_for_change();
}

void EntryGlom::on_changed()
{
  //The text is being edited, but the user has not finished yet.

  //Call base class:
  Gtk::Entry::on_changed();
}

void EntryGlom::on_insert_text(const Glib::ustring& text, int* position)
{
  Gtk::Entry::on_insert_text(text, position);
}

void EntryGlom::set_value(const Gnome::Gda::Value& value)
{
  const LayoutItem_Field* layout_item = dynamic_cast<const LayoutItem_Field*>(get_layout_item());
  if(layout_item)
    set_text(GlomConversions::get_text_for_gda_value(m_glom_type, value, layout_item->m_numeric_format));
}

void EntryGlom::set_text(const Glib::ustring& text)
{
  m_old_text = text;

  //Call base class:
  Gtk::Entry::set_text(text);
}

Gnome::Gda::Value EntryGlom::get_value() const
{
  bool success = false;

  const LayoutItem_Field* layout_item = dynamic_cast<const LayoutItem_Field*>(get_layout_item());
  return GlomConversions::parse_value(m_glom_type, get_text(), layout_item->m_numeric_format, success);
}

EntryGlom::type_signal_user_requested_layout EntryGlom::signal_user_requested_layout()
{
  return m_signal_user_requested_layout;
}

EntryGlom::type_signal_user_requested_layout_properties EntryGlom::signal_user_requested_layout_properties()
{
  return m_signal_user_requested_layout_properties;
}

void EntryGlom::setup_menu()
{
  m_refActionGroup->add(m_refContextLayout,
    sigc::mem_fun(*this, &EntryGlom::on_menupopup_activate_layout) );

  m_refActionGroup->add(m_refContextLayoutProperties,
    sigc::mem_fun(*this, &EntryGlom::on_menupopup_activate_layout_properties) );

  m_refActionGroup->add(m_refContextAddField,
    sigc::bind( sigc::mem_fun(*this, &EntryGlom::on_menupopup_add_item), TreeStore_Layout::TYPE_FIELD ) );

  m_refActionGroup->add(m_refContextAddRelatedRecords,
    sigc::bind( sigc::mem_fun(*this, &EntryGlom::on_menupopup_add_item), TreeStore_Layout::TYPE_PORTAL ) );

  m_refActionGroup->add(m_refContextAddGroup,
    sigc::bind( sigc::mem_fun(*this, &EntryGlom::on_menupopup_add_item), TreeStore_Layout::TYPE_GROUP ) );

  //TODO: This does not work until this widget is in a container in the window:s
  App_Glom* pApp = get_application();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextLayoutProperties); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextAddField);
    pApp->add_developer_action(m_refContextAddRelatedRecords);
    pApp->add_developer_action(m_refContextAddGroup);

    pApp->update_userlevel_ui(); //Update our action's sensitivity. 
  }

  m_refUIManager = Gtk::UIManager::create();

  m_refUIManager->insert_action_group(m_refActionGroup);

  //TODO: add_accel_group(m_refUIManager->get_accel_group());

  try
  {
    Glib::ustring ui_info = 
        "<ui>"
        "  <popup name='ContextMenu'>"
        "    <menuitem action='ContextLayout'/>"
        "    <menuitem action='ContextLayoutProperties'/>"
        "    <menuitem action='ContextAddField'/>"
        "    <menuitem action='ContextAddRelatedRecords'/>"
        "    <menuitem action='ContextAddGroup'/>"
        "  </popup>"
        "</ui>";

    m_refUIManager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }

  //Get the menu:
  m_pMenuPopup = dynamic_cast<Gtk::Menu*>( m_refUIManager->get_widget("/ContextMenu") ); 
  if(!m_pMenuPopup)
    g_warning("menu not found");


  if(pApp)
    m_refContextLayout->set_sensitive(pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER);
}

bool EntryGlom::on_button_press_event(GdkEventButton *event)
{
  //Enable/Disable items.
  //We did this earlier, but get_application is more likely to work now:
  App_Glom* pApp = get_application();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextAddField);
    pApp->add_developer_action(m_refContextAddRelatedRecords);
    pApp->add_developer_action(m_refContextAddGroup);

    pApp->update_userlevel_ui(); //Update our action's sensitivity. 

    //Only show this popup in developer mode, so operators still see the default GtkEntry context menu.
    //TODO: It would be better to add it somehow to the standard context menu.
    if(pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
    {
      GdkModifierType mods;
      gdk_window_get_pointer( Gtk::Widget::gobj()->window, 0, 0, &mods );
      if(mods & GDK_BUTTON3_MASK)
      {
        //Give user choices of actions on this item:
        m_pMenuPopup->popup(event->button, event->time);
        return true; //We handled this event.
      }
    }

  }

  return Gtk::Entry::on_button_press_event(event);
}

void EntryGlom::on_menupopup_add_item(TreeStore_Layout::enumType item)
{
  signal_layout_item_added().emit(item);
}

void EntryGlom::on_menupopup_activate_layout()
{
  //finish_editing();

  //Ask the parent widget to show the layout dialog:
  signal_user_requested_layout().emit();
}

void EntryGlom::on_menupopup_activate_layout_properties()
{
  //finish_editing();

  //Ask the parent widget to show the layout dialog:
  signal_user_requested_layout_properties().emit();
}

App_Glom* EntryGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<App_Glom*>(pWindow);
}
