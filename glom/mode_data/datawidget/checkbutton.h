/* Glom
 *
 * Copyright (C) 2008 Johannes Schmid
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

#ifndef GLOM_MODE_DATA_CHECK_BUTTON_H
#define GLOM_MODE_DATA_CHECK_BUTTON_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <gtkmm/checkbutton.h>
#include <libglom/data_structure/field.h>
#include <glom/utility_widgets/layoutwidgetfield.h>
#include <gtkmm/builder.h>

namespace Glom
{

class AppWindow;

namespace DataWidgetChildren
{

class CheckButton
: public Gtk::CheckButton,
  public LayoutWidgetField
{
public:
  explicit CheckButton(const Glib::ustring& title = Glib::ustring());

  void set_value(const Gnome::Gda::Value& value) override;
  Gnome::Gda::Value get_value() const override;

private:
#ifndef GLOM_ENABLE_CLIENT_ONLY
  bool on_button_press_event(GdkEventButton *event) override;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  AppWindow* get_appwindow() const override;
};

} //namespace DataWidetChildren
} //namespace Glom

#endif // GLOM_MODE_DATA_CHECK_BUTTON_H
