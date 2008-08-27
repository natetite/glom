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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
 
#include "layouttoolbarbutton.h"

namespace
{

Glib::ustring get_icon_path(const Glib::ustring& filename)
{
#ifdef G_OS_WIN32
  gchar* basepath = g_win32_get_package_installation_subdirectory(NULL, NULL, "share/glom/pixmaps");
  Glib::ustring result = Glib::build_filename(basepath, filename);
  g_free(basepath);
  return result;
#else
  return Glib::build_filename(GLOM_ICON_DIR, filename);
#endif
}

} //anonymous namespace


namespace Glom
{

LayoutToolbarButton::LayoutToolbarButton(const Glib::ustring& icon_name, LayoutWidgetBase::enumType type,
                                         const Glib::ustring& title, const Glib::ustring& tooltip)
: Gtk::ToolButton( *(Gtk::manage (new Gtk::Image(get_icon_path(icon_name)))) )
{
  m_type = type;
  g_object_set_data(G_OBJECT(gobj()), "glom-type", GINT_TO_POINTER(type));

  std::list<Gtk::TargetEntry> targetentries;
  targetentries.push_back(Gtk::TargetEntry(get_target()));

  drag_source_set(targetentries, Gdk::MODIFIER_MASK, 
                  Gdk::ACTION_COPY | Gdk::ACTION_MOVE);
  set_tooltip_text(tooltip);
  set_label(title);
}

LayoutToolbarButton::~LayoutToolbarButton()
{

}

void LayoutToolbarButton::on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>&, 
                                  Gtk::SelectionData& selection_data, guint, guint)
{
  selection_data.set(8, (guint8*)(&m_type), 4);
}

void LayoutToolbarButton::on_drag_begin(const Glib::RefPtr<Gdk::DragContext>& drag_context)
{
  drag_context->set_icon(dynamic_cast<Gtk::Image*>(get_icon_widget())->get_pixbuf(), 0, 0);
}

} // namespace Glom
