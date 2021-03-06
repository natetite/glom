/*
 * Copyright 2000-2002 Murray Cumming
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <libglom/document/bakery/document_xml.h>
#include <libglom/xml_utils.h>
#include <iostream>

namespace GlomBakery
{


Document_XML::Document_XML()
: m_dom_document(nullptr),
  m_write_formatted(false)
{
}

Document_XML::~Document_XML()
{
  //m_dom_document is owned by m_DOM_Document;
}

bool Document_XML::load_after(int& failure_code)
{
  //Initialize the output parameter:
  failure_code = 0;

  const auto bTest = type_base::load_after(failure_code);
  if(!bTest)
    return false; //Failed.

  try
  {
    //Link the parser to the XML text that was loaded:
    //m_dom_parser.setDoValidation(true);

    if(m_strContents.empty())
      std::cerr << G_STRFUNC << ": parsing empty document.\n";

    m_dom_parser.parse_memory(m_strContents);
    m_dom_document = m_dom_parser.get_document();
    if(!m_dom_document) return false;

    return true; //Success.
  }
  catch(const std::exception& ex)
  {
    std::cerr << G_STRFUNC << ": XML Parser error: \n" << ex.what() << std::endl;

    return false; //Failed.
  }
}


bool Document_XML::save_before()
{
  if(get_modified())
  {
    //Write XML to string:
    m_strContents.erase();

    Util_DOM_Write(m_strContents);
  }

  //Write the file even if nothing was modified,
  //to make sure that we write empty documents that have only default values,
  //when save() is explicitly called.
  //
  //Save the XML string:
  return type_base::save_before();
}

Glib::ustring Document_XML::get_xml() const
{
  //Write XML to string:
  Glib::ustring strXML;
  Util_DOM_Write(strXML);
  return strXML;
}

void Document_XML::Util_DOM_Write(Glib::ustring& refstrXML) const
{
  try
  {
    if(m_write_formatted)
      refstrXML = m_dom_document->write_to_string_formatted();
    else
      refstrXML = m_dom_document->write_to_string();
  }
  catch(xmlpp::exception& ex)
  {
    std::cerr << G_STRFUNC << ": exception caught: " << ex.what() << std::endl;
  }
}

void Document_XML::set_dtd_name(const std::string& strVal)
{
  m_dtd_name = strVal;
}

std::string Document_XML::get_dtd_name() const
{
  return m_dtd_name;
}

void Document_XML::set_dtd_root_node_name(const Glib::ustring& strVal, const Glib::ustring& xmlns)
{
  m_root_node_name = strVal;
  m_root_xmlns = xmlns;
}

Glib::ustring Document_XML::get_dtd_root_node_name() const
{
  return m_root_node_name;
}


const xmlpp::Element* Document_XML::get_node_document() const
{
  //Call the non-const overload:
  return const_cast<Document_XML*>(this)->get_node_document();
}

xmlpp::Element* Document_XML::get_node_document()
{
  if(!m_dom_document)
    m_dom_document = m_dom_parser.get_document();

  //Make sure that it has the DTD declaration:
  //TODO: Put this in a better place, where it's more guaranteed to always be set?
  //TODO: Add API to specify the PUBLIC URI, if the document should write this:
  //SYSTEM (local) DTDs do not seem very useful.
  //- means non-registered, which is commonly used.
  //m_dom_document->set_internal_subset(m_root_node_name, "-//glom/" + m_dtd_name, m_dtd_name);

  auto nodeRoot = m_dom_document->get_root_node();
  if(!nodeRoot)
  {
    //Add it if it isn't there already:
    nodeRoot = m_dom_document->create_root_node(m_root_node_name, m_root_xmlns);
  }

  //Make sure that it has the root name name and xmlns:
  nodeRoot->set_namespace_declaration(m_root_xmlns);

  return nodeRoot;
}

void Document_XML::add_indenting_white_space_to_node(xmlpp::Node* node, const Glib::ustring& start_indent)
{
  if(!node)
    node = get_node_document();

  //Remove any previous indenting:
  {
  for(const auto& child : node->get_children())
  {
    if(!child)
      continue;

    if(auto text = dynamic_cast<xmlpp::ContentNode*>(child))
    {
      if(text->is_white_space())
        node->remove_node(text);
    }
  }
  }

  //All indents have a newline,
  //and we add spaces each time we recurse:
  auto indent = start_indent;
  if(indent.empty())
    indent = "\n  ";
  else
    indent += "  ";

  //Add indenting text items:
  bool had_children = false;
  auto node_as_element = dynamic_cast<xmlpp::Element*>(node);
  for(const auto& child : node_as_element->get_children())
  {
    if(!child)
      continue;

    if(auto text = dynamic_cast<xmlpp::ContentNode*>(child))
    {
      if(!text->is_white_space())
        continue; //Don't change content items.
    }

    //Add a text item for the indenting, before the child:
    //std::cout << "  Adding indent after node=" << child->get_name() << ": START" << indent << "END\n";
    node_as_element->add_child_text_before(child, indent);
    had_children = true;

    //Recurse into the children:
    add_indenting_white_space_to_node(child, indent);
  }

  //If it has children then add an indent before the closing tag:
  if(had_children)
    node_as_element->add_child_text(start_indent);
}


} //namespace GlomBakery.
