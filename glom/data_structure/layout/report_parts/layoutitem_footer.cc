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
 
#include "layoutitem_footer.h"
#include <glibmm/i18n.h>

LayoutItem_Footer::LayoutItem_Footer()
{
}

LayoutItem_Footer::LayoutItem_Footer(const LayoutItem_Footer& src)
: LayoutGroup(src)
{
}

LayoutItem_Footer::~LayoutItem_Footer()
{
}

LayoutItem* LayoutItem_Footer::clone() const
{
  return new LayoutItem_Footer(*this);
}


LayoutItem_Footer& LayoutItem_Footer::operator=(const LayoutItem_Footer& src)
{
  LayoutGroup::operator=(src);

  return *this;
}

Glib::ustring LayoutItem_Footer::get_part_type_name() const
{
  return _("Footer");
}

Glib::ustring LayoutItem_Footer::get_report_part_id() const
{
  return "footer";
}


