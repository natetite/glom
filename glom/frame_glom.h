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

#ifndef FRAME_GLOM_H
#define FRAME_GLOM_H

#include <gtkmm/frame.h>
#include "bakery/View/View_Composite.h"
#include "document/document_glom.h"

#include "dialog_glom.h"

#include "navigation/box_databases.h"
#include "navigation/box_tables.h"

#include "mode_data/notebook_data.h"
#include "mode_find/notebook_find.h"

#include "mode_design/dialog_fields.h"
#include "mode_design/dialog_relationships.h"

class Frame_Glom :
  public PlaceHolder,
  public Bakery::View_Composite<Document_Glom>
{
public: 
  Frame_Glom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Frame_Glom();

  virtual void on_Box_Databases_selected(Glib::ustring strName);
  virtual void on_Box_Tables_selected(Glib::ustring strName);

  virtual void on_menu_UserLevel_Developer(Glib::RefPtr<Gtk::RadioAction> action);
  virtual void on_menu_UserLevel_Operator(Glib::RefPtr<Gtk::RadioAction> action);
      
  virtual void on_menu_Mode_Data();
  virtual void on_menu_Mode_Find();

  virtual void on_menu_Navigate_Database();
  virtual void do_menu_Navigate_Database(bool bUseList = true);
  virtual void on_menu_Navigate_Table();
  virtual void do_menu_Navigate_Table(bool open_default = false);

  virtual void on_menu_developer_fields();
  virtual void on_menu_developer_relationships();
  virtual void on_menu_developer_users();     
  virtual void on_menu_developer_layout();

  virtual void on_developer_dialog_hide();

  virtual void set_document(Document_Glom* pDocument); //View override
  virtual void load_from_document(); //View override

  enum enumModes
  {
    MODE_None, //at the start.
    MODE_Data,
    MODE_Find
  };
  enumModes m_Mode;
  enumModes m_Mode_Previous; // see comments in set_mode_widget().

protected:

  //virtual void set_document(Document_Glom* pDocument); //override
     
  void show_table(const Glib::ustring& strTableName);
  void show_table_title();
  void update_table_in_document_from_database();
  
  virtual void set_mode_widget(Gtk::Widget& widget); //e.g. show the design mode notebook.
  virtual bool set_mode(enumModes mode); //bool indicates that there was a change.

  virtual Gtk::Window* get_app_window();
  virtual const Gtk::Window* get_app_window() const;
  
  virtual void show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message);
  virtual void alert_no_table();
  
  //Signal handlers:
  virtual void on_Notebook_Find(Glib::ustring strWhereClause);
  virtual void on_userlevel_changed(AppState::userlevels userlevel);

  //Member data:
  Glib::ustring m_strTableName;

  //Child widgets:
  Gtk::Label* m_pLabel_Table;
  Gtk::Label* m_pLabel_Mode;
  Gtk::Label* m_pLabel_UserLevel;
    
  PlaceHolder* m_pBox_Mode; //Contains e.g. design mode notebook.
  
  Box_DataBases* m_pBox_Databases;
  Box_Tables* m_pBox_Tables;

  Notebook_Data m_Notebook_Data;
  Notebook_Find m_Notebook_Find;

  //Navigation:
  Dialog_Glom* m_pDialog_Databases;
  Dialog_Glom* m_pDialog_Tables;

  //Developer:
  Dialog_Fields* m_pDialog_Fields;
  Dialog_Relationships* m_pDialog_Relationships;
};

#endif //FRAME_GLOM_H
