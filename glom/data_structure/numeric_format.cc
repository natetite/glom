/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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

#include "numeric_format.h"

NumericFormat::NumericFormat()
: m_use_thousands_separator(true), //A sensible default.
  m_decimal_places_restricted(false),
  m_decimal_places(2) //A sensible default.
{
}

NumericFormat::NumericFormat(const NumericFormat& src)
{
  operator=(src);
}

NumericFormat::~NumericFormat()
{
}

NumericFormat& NumericFormat::operator=(const NumericFormat& src)
{
  m_use_thousands_separator = src.m_use_thousands_separator;
  m_decimal_places_restricted = src.m_decimal_places_restricted;
  m_decimal_places = src.m_decimal_places;

  return *this;
}

bool NumericFormat::operator==(const NumericFormat& src) const
{
  bool result = (m_use_thousands_separator == src.m_use_thousands_separator);
  result = result && (m_decimal_places_restricted == src.m_decimal_places_restricted);
  result = result && (m_decimal_places == src.m_decimal_places);

  return result;
}

bool NumericFormat::operator!=(const NumericFormat& src) const
{
  return !(operator==(src));
}
