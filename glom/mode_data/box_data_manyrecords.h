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

#ifndef GLOM_MODE_DATA_BOX_DATA_MANY_RECORDS_H
#define GLOM_MODE_DATA_BOX_DATA_MANY_RECORDS_H

#include "config.h" // GLOM_ENABLE_CLIENT_ONLY

#include "box_data.h"
#include <glom/mode_data/db_adddel/db_adddel_withbuttons.h>

namespace Glom
{

class Box_Data_ManyRecords : public Box_Data
{
public:
  Box_Data_ManyRecords();

  void refresh_data_from_database_blank();

  ///Highlight and scroll to the specified record, with primary key value @primary_key_value.
  virtual void set_primary_key_value_selected(const Gnome::Gda::Value& primary_key_value);

  //Primary Key value:
  typedef sigc::signal<void(const Gnome::Gda::Value&)> type_signal_user_requested_details;
  type_signal_user_requested_details signal_user_requested_details();

  typedef sigc::signal<void()> type_signal_record_selection_changed;
  type_signal_record_selection_changed signal_record_selection_changed();
protected:
  void print_layout() override;
  void print_layout_group(xmlpp::Element* node_parent, const std::shared_ptr<const LayoutGroup>& group);

  type_signal_user_requested_details m_signal_user_requested_details;
  type_signal_record_selection_changed m_signal_record_selection_changed;
};

} //namespace Glom

#endif // GLOM_MODE_DATA_BOX_DATA_MANY_RECORDS_H
