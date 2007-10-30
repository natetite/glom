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

#include "canvas_layout_item.h"
#include <glom/utility_widgets/canvas/canvas_rect_movable.h>
#include <glom/utility_widgets/canvas/canvas_text_movable.h>
#include <glom/utility_widgets/canvas/canvas_image_movable.h>
#include <glom/utility_widgets/canvas/canvas_line_movable.h>
#include <glom/utility_widgets/canvas/canvas_group_movable.h>
#include <glom/libglom/data_structure/layout/layoutitem_button.h>
#include <glom/libglom/data_structure/layout/layoutitem_text.h>
#include <glom/libglom/data_structure/layout/layoutitem_image.h>
#include <glom/libglom/data_structure/layout/layoutitem_field.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <glom/libglom/data_structure/glomconversions.h>
#include <glom/utility_widgets/imageglom.h> //For ImageGlom::scale_keeping_ratio().
#include <glibmm/i18n.h>

namespace Glom
{

CanvasLayoutItem::CanvasLayoutItem()
{
}

CanvasLayoutItem::CanvasLayoutItem(const sharedptr<LayoutItem>& layout_item)
{
  set_layout_item(layout_item);
}

CanvasLayoutItem::~CanvasLayoutItem()
{
}

Glib::RefPtr<CanvasLayoutItem> CanvasLayoutItem::create()
{
  return Glib::RefPtr<CanvasLayoutItem>(new CanvasLayoutItem());
}

Glib::RefPtr<CanvasLayoutItem> CanvasLayoutItem::create(const sharedptr<LayoutItem>& layout_item)
{
  return Glib::RefPtr<CanvasLayoutItem>(new CanvasLayoutItem(layout_item));
}

sharedptr<LayoutItem> CanvasLayoutItem::get_layout_item()
{
  return m_layout_item;
}

void CanvasLayoutItem::check_and_apply_formatting(const Glib::RefPtr<CanvasTextMovable>& canvas_item, FieldFormatting& formatting)
{
  if(!canvas_item)
    return;

  Glib::ustring font = formatting.get_text_format_font();
  if(font.empty())
  {
    font = "Serif 9";

    //Set it in the input parameter,
    //so that this is the default:
    formatting.set_text_format_font(font);    
  }

  canvas_item->set_font_points(font);

  //TODO: Are these sensible properties? Maybe we need to use markup:
  const Glib::ustring fg = formatting.get_text_format_color_foreground();
  if(!fg.empty())
    canvas_item->property_stroke_color() = fg;

  const Glib::ustring bg = formatting.get_text_format_color_background();
  if(!bg.empty())
  canvas_item->property_fill_color() = bg;
}

void CanvasLayoutItem::set_layout_item(const sharedptr<LayoutItem>& item)
{
  //Add the new child:
  m_layout_item = item;

  Glib::RefPtr<CanvasItemMovable> child;
  Glib::RefPtr<Goocanvas::Item> child_item;
  sharedptr<LayoutItem_Text> text = sharedptr<LayoutItem_Text>::cast_dynamic(m_layout_item);
  if(text)
  {
    Glib::RefPtr<CanvasTextMovable> canvas_item = CanvasTextMovable::create();
    canvas_item->property_line_width() = 0;

    FieldFormatting& formatting = text->m_formatting;
    check_and_apply_formatting(canvas_item, formatting);

    std::cout << "DEBUG: adding text=" << text->get_text() << std::endl;
    canvas_item->set_text(text->get_text());
    child = canvas_item;
    child_item = canvas_item;
  }
  else
  {
    sharedptr<LayoutItem_Image> image = sharedptr<LayoutItem_Image>::cast_dynamic(m_layout_item);
    if(image)
    {
      Glib::RefPtr<CanvasRectMovable> canvas_item = CanvasRectMovable::create();

      canvas_item->property_fill_color() = "white"; //This makes the whole area clickable, not just the outline stroke.
      child = canvas_item;
      child_item = canvas_item;
    }
    else
    {
      sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(m_layout_item);
      if(field)
      {
        //Create an appropriate canvas item for the field type:
        if(field->get_glom_type() == Field::TYPE_IMAGE)
        {
          Glib::RefPtr<CanvasImageMovable> canvas_item = CanvasImageMovable::create();

          //Glib::ustring name = field->get_name();
          //  if(name.empty())
          //    name = _("Choose Field");
          
          //canvas_item->set_text(name);

          child = canvas_item;
          child_item = canvas_item;
        }
        else //text, numbers, date, time, boolean:
        {
          Glib::RefPtr<CanvasTextMovable> canvas_item = CanvasTextMovable::create();
          canvas_item->property_line_width() = 0;
         
          FieldFormatting& formatting = field->m_formatting;
          check_and_apply_formatting(canvas_item, formatting);

          Glib::ustring name = field->get_name();
            if(name.empty())
              name = _("Choose Field");
          
          canvas_item->set_text(name);

          child = canvas_item;
          child_item = canvas_item;
        }
      }
      else
      {
        std::cerr << "CanvasLayoutItem::set_layout_item(): Unhandled LayoutItem type." << std::endl;
      }
    }
  }

  if(child && child_item)
  {
    //child_item->property_pointer_events() = 
    //  (Goocanvas::PointerEvents)(Goocanvas::CANVAS_EVENTS_VISIBLE_FILL & GOO_CANVAS_EVENTS_VISIBLE_STROKE);
      
    //Set the position and dimensions:
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;
    item->get_print_layout_position(x, y, width, height);

    set_xy(x, y);
    set_width_height(width, height);
    std::cout << "CanvasLayoutItem::set_layout_item(): item x=" << x << std::endl;

    //Set the child (this removes the previous child):
    set_child(child);
  }
}

void CanvasLayoutItem::set_db_data(const Gnome::Gda::Value& value)
{
  sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(m_layout_item);
  if(!field)
    return;
    
  Glib::RefPtr<CanvasItemMovable> child = get_child();
  if(!child)
    return;
  
  const Field::glom_field_type field_type = field->get_glom_type();
  switch(field->get_glom_type())
  {
    case(Field::TYPE_TEXT):
    case(Field::TYPE_NUMERIC):
    case(Field::TYPE_BOOLEAN):
    case(Field::TYPE_TIME):
    case(Field::TYPE_DATE):
    {
      Glib::RefPtr<CanvasTextMovable> canvas_item = Glib::RefPtr<CanvasTextMovable>::cast_dynamic(child);
      if(!canvas_item)
        return;
        
      Glib::ustring text_value = Conversions::get_text_for_gda_value(field_type, value, field->get_formatting_used().m_numeric_format);

      //The Postgres summary functions return NULL when summarising NULL records, but 0 is more sensible:
      if(text_value.empty() && sharedptr<const LayoutItem_FieldSummary>::cast_dynamic(field) && (field_type == Field::TYPE_NUMERIC))
      {
        //Use get_text_for_gda_value() instead of "0" so we get the correct numerical formatting:
        Gnome::Gda::Value value = Conversions::parse_value(0);
        text_value = Conversions::get_text_for_gda_value(field_type, value, field->get_formatting_used().m_numeric_format);
      }
    
      canvas_item->set_text(text_value);
      break;
    }
    case(Field::TYPE_IMAGE):
    {
      Glib::RefPtr<CanvasImageMovable> canvas_item = Glib::RefPtr<CanvasImageMovable>::cast_dynamic(child);
      if(!canvas_item)
        return;
        
      //Get the height of the item (not of the pixbuf),
      //so we can scale the pixbuf:
      double width = 0;
      double height = 0;
      canvas_item->get_width_height(width, height);
      
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = Conversions::get_pixbuf_for_gda_value(value);
      
      //Scale the image down to fit the item:
       //(Just resetting the height and width of the canvas item would crop the image)
      if(pixbuf)
        pixbuf = ImageGlom::scale_keeping_ratio(pixbuf, (int)height, (int)width);
      
      canvas_item->property_pixbuf() = pixbuf;
      
     
     
      break;
    }
    default:
      std::cerr << "CanvasLayoutItem::set_db_data(): unhandled field type." << std::endl;
  }
        
}

} //namespace Glom

