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
 
#include <libglom/data_structure/system_prefs.h>

namespace Glom
{

SystemPrefs::SystemPrefs()
{
}

SystemPrefs::SystemPrefs(const SystemPrefs& src)
: m_name(src.m_name),
  m_org_name(src.m_org_name),
  m_org_address_street(src.m_org_address_street),
  m_org_address_street2(src.m_org_address_street2),
  m_org_address_town(src.m_org_address_town),
  m_org_address_county(src.m_org_address_county),
  m_org_address_country(src.m_org_address_country),
  m_org_address_postcode(src.m_org_address_postcode),
  m_org_logo(src.m_org_logo)
{
}

SystemPrefs& SystemPrefs::operator=(const SystemPrefs& src)
{
  m_name = src.m_name;
  m_org_name =src.m_org_name;
  m_org_address_street = src.m_org_address_street;
  m_org_address_street2 = src.m_org_address_street2;
  m_org_address_town = src.m_org_address_town;
  m_org_address_county = src.m_org_address_county;
  m_org_address_country = src.m_org_address_country;
  m_org_address_postcode = src.m_org_address_postcode;
  m_org_logo = src.m_org_logo;

  return *this;
}

bool SystemPrefs::operator==(const SystemPrefs& src) const
{
  return (m_name == src.m_name) &&
    (m_org_name == src.m_org_name) &&
    (m_org_address_street == src.m_org_address_street) &&
    (m_org_address_street2 == src.m_org_address_street2) &&
    (m_org_address_town == src.m_org_address_town) &&
    (m_org_address_county == src.m_org_address_county) &&
    (m_org_address_country == src.m_org_address_country) &&
    (m_org_address_postcode == src.m_org_address_postcode) &&
    (m_org_logo == src.m_org_logo);
}

bool SystemPrefs::operator!=(const SystemPrefs& src) const
{
  return !(operator==(src));
}

} //namespace Glom

