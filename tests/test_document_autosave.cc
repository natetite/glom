/* Glom
 *
 * Copyright (C) 2010 Openismus GmbH
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
71 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <libglom/document/document.h>
#include <libglom/init.h>
#include <giomm/file.h>

Glib::ustring file_uri;

void cleanup()
{
  try
  {
    //TODO: Catch exceptions:
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(file_uri);
    file->remove();
  }
  catch(const Gio::Error& ex)
  {
    //It's OK if it's not found - we just want to make sure it doesn't exist.
    if(ex.code() == Gio::Error::NOT_FOUND)
      return;

    std::cerr << "Exception from Gio::File::remove(): " << ex.what();
    exit(EXIT_FAILURE);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "Exception from Gio::File::remove(): " << ex.what();
    exit(EXIT_FAILURE);
  }
}

int main()
{
  Glom::libglom_init();

  //For instance, /tmp/testfile.glom");
  const std::string temp_filename = "testglom";
  const std::string temp_filepath = Glib::build_filename(Glib::get_tmp_dir(), 
    temp_filename);
  file_uri = Glib::filename_to_uri(temp_filepath);
  
  //Make sure that the file does not exist yet:
  cleanup();

  const Glib::ustring test_title = "test_title";
  
  //Test manual saving:
  {
    Glom::Document document;
    document.set_file_uri(file_uri);
    document.set_hosting_mode(Glom::Document::HOSTING_MODE_POSTGRES_CENTRAL);
    document.set_database_title(test_title);
    const bool saved = document.save();
    g_assert(saved);
  }
  {
    Glom::Document document;
    document.set_file_uri(file_uri);
    int failure_code = 0;
    const bool test = document.load(failure_code);
    g_assert(test);
    
    g_assert( document.get_database_title() == test_title );
  }

  cleanup();

  //Test autosaving:
  {
    Glom::Document document;
    document.set_file_uri(file_uri);
    document.set_hosting_mode(Glom::Document::HOSTING_MODE_POSTGRES_CENTRAL);
    document.set_allow_autosave();
    document.set_database_title(test_title);
    g_assert( !document.get_modified() );
  }
  {
    Glom::Document document;
    document.set_file_uri(file_uri);
    int failure_code = 0;
    const bool test = document.load(failure_code);
    g_assert(test);
    
    g_assert( document.get_database_title() == test_title );
  }
  
  cleanup();
  
  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}