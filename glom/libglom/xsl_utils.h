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

#ifndef GLOM_XSL_UTILS_H
#define GLOM_XSL_UTILS_H

#include <libglom/data_structure/field.h>
#include <libglom/data_structure/numeric_format.h>

#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libxml++/libxml++.h>


namespace Glom
{

///field, ascending
typedef std::pair< sharedptr<const LayoutItem_Field>, bool> type_pair_sort_field;
typedef std::list<type_pair_sort_field> type_sort_clause;

namespace GlomXslUtils
{

/**
 * @result the filepath of the generated HTML file.
 */
std::string transform(const xmlpp::Document& xml_document, const std::string& xsl_file_path);

} //namespace GlomXslUtils

} //namespace Glom

#endif //GLOM_XSL_UTILS_H
