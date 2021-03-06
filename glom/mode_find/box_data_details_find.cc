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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include <libglom/libglom_config.h> // For GLOM_ENABLE_CLIENT_ONLY

#include "box_data_details_find.h"
#include <glibmm/i18n.h>

namespace Glom
{

Box_Data_Details_Find::Box_Data_Details_Find()
: Box_Data_Details(false)
{
  //Instead of nav buttons:
  m_hbox_buttons.pack_end(m_Button_Find, Gtk::PACK_SHRINK);

  //A signal handler is connected in the Box_Data base class.
  m_Button_Find.set_can_default();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Hide this because it is useless for Find mode:
  show_layout_toolbar(false);
#endif

  m_FlowTable.set_find_mode();

  show_all_children();
}

bool Box_Data_Details_Find::init_db_details(const Glib::ustring& table_name, const Glib::ustring& layout_platform)
{
  FoundSet found_set;
  found_set.m_table_name = table_name;
  return Box_Data_Details::init_db_details(found_set, layout_platform, Gnome::Gda::Value());
}

bool Box_Data_Details_Find::fill_from_database()
{
  BusyCursor busy_cursor(get_app_window());

  const auto result = Base_DB_Table_Data::fill_from_database();
  if(!result)
    return result;

  m_FieldsShown = get_fields_to_show();

  create_layout(); //TODO: Only do this when the layout has changed.

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Hide this because it is useless for Find mode:
  //Actually we already hide this in the constructor, but somehow it is visible again by now.
  show_layout_toolbar(false);
#endif

  return result;
}

void Box_Data_Details_Find::on_flowtable_field_edited(const std::weak_ptr<const LayoutItem_Field>& /* id */, const Gnome::Gda::Value& /* value */)
{
  //Don't do anything.
  //This just blocks the method in the base class.
}

Gtk::Widget* Box_Data_Details_Find::get_default_button() //override
{
  return &m_Button_Find;
}

} //namespace Glom
