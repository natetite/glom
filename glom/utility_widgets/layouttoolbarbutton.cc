/* Glom
 *
 * Copyright (C) 2007, 2008 Openismus GmbH
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

#include "layouttoolbarbutton.h"
#include <glom/utils_ui.h>
#include <libglom/utils.h>
#include <iostream>

namespace Glom
{

LayoutToolbarButton::LayoutToolbarButton(const std::string& icon_name, LayoutWidgetBase::enumType type,
                                         const Glib::ustring& title, const Glib::ustring& tooltip)
: Gtk::ToolButton()
{
  auto image = Gtk::manage (new Gtk::Image());

  const auto resource_path = UiUtils::get_icon_path(icon_name);
  if(!Utils::get_resource_exists(resource_path))
  {
    std::cerr << G_STRFUNC << ": icon resource not found: " << resource_path << std::endl;
  }

  image->set_from_resource(resource_path);
  set_icon_widget(*image);

  m_type = type;
  g_object_set_data(G_OBJECT(gobj()), "glom-type", GINT_TO_POINTER(type));

  std::vector<Gtk::TargetEntry> targetentries;
  targetentries.emplace_back(Gtk::TargetEntry(get_target()));

  drag_source_set(targetentries, Gdk::MODIFIER_MASK,
                  Gdk::ACTION_COPY | Gdk::ACTION_MOVE);
  set_tooltip_text(tooltip);
  set_label(title);
}

} // namespace Glom
