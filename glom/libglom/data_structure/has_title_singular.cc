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

#include <libglom/data_structure/has_title_singular.h>

namespace Glom
{


HasTitleSingular::HasTitleSingular()
{
}

bool HasTitleSingular::operator==(const HasTitleSingular& src) const
{
  const bool bResult = (m_title_singular == src.m_title_singular);

  return bResult;
}

bool HasTitleSingular::operator!=(const HasTitleSingular& src) const
{
  return !(operator==(src));
}


Glib::ustring HasTitleSingular::get_title_singular(const Glib::ustring& locale) const
{
  Glib::ustring result;
  if(m_title_singular)
    result = m_title_singular->get_title(locale);

  return result;
}

Glib::ustring HasTitleSingular::get_title_singular_original() const
{
  Glib::ustring result;
  if(m_title_singular)
    result = m_title_singular->get_title_original();

  return result;
}

Glib::ustring HasTitleSingular::get_title_singular_with_fallback(const Glib::ustring& locale) const
{
  const auto result = get_title_singular(locale);
  if(!result.empty())
    return result;

  //If it this is also a regular TranslatableItem (usually it is),
  //then try getting the regular title instead.
  if(const auto translatable = dynamic_cast<const TranslatableItem*>(this))
    return translatable->get_title_or_name(locale);

  return result;
}


void HasTitleSingular::set_title_singular(const Glib::ustring& title, const Glib::ustring& locale)
{
  if(!m_title_singular)
    m_title_singular = std::make_shared<TranslatableItem>();

  m_title_singular->set_title(title, locale);
}

} //namespace Glom
