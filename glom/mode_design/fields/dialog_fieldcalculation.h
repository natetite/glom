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

#ifndef DIALOG_FIELDCALCULATION_H
#define DIALOG_FIELDCALCULATION_H

#include <gtkmm.h>
#include <libglademm.h>
#include "../../data_structure/field.h"
#include "../../base_db.h"

class Dialog_FieldCalculation
 : public Gtk::Dialog
{
public:
  Dialog_FieldCalculation(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_FieldCalculation();

  virtual void set_field(const Field& field, const Glib::ustring& table_name);
  virtual Field get_field() const;

protected:
  virtual void on_button_test();

  Gtk::TextView* m_text_view;
  Gtk::Button* m_button_test;
  
  Field m_field;
  Glib::ustring m_table_name;
};

#endif //DIALOG_FIELDCALCULATION_H
