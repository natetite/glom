/* Glom
 *
 * Copyright (C) 2011 Openismus GmbH
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

// For instance:
// glom_create_from_example /opt/gnome30/share/doc/glom/examples/example_music_collection.glom --output-path=/home/murrayc/ --output-name="something.glom"

#include "config.h"

#include <libglom/document/document.h>
#include <libglom/connectionpool.h>
#include <libglom/connectionpool_backends/postgres_self.h>
#include <libglom/init.h>
#include <libglom/privs.h>
#include <libglom/db_utils.h>
#include <libglom/utils.h>
#include <giomm/file.h>
#include <iostream>

#include <glibmm/i18n.h>

class GlomCreateOptionGroup : public Glib::OptionGroup
{
public:
  GlomCreateOptionGroup();

  //These instances should live as long as the OptionGroup to which they are added,
  //and as long as the OptionContext to which those OptionGroups are added.
  std::string m_arg_filename_input;
  std::string m_arg_filepath_dir_output;
  std::string m_arg_filepath_name_output;
  bool m_arg_version;
};

GlomCreateOptionGroup::GlomCreateOptionGroup()
: Glib::OptionGroup("glom_create_from_example", _("Glom options"), _("Command-line options")),
  m_arg_version(false)
{
  Glib::OptionEntry entry;
  entry.set_long_name("input");
  entry.set_short_name('i');
  entry.set_description(_("The example .glom file to open."));
  add_entry_filename(entry, m_arg_filename_input);
  
  Glib::OptionEntry entry2;
  entry2.set_long_name("output-path");
  entry2.set_short_name('o');
  entry2.set_description(_("The directory in which to save the created .glom file, or sub-directory if necessary, such as /home/someuser/"));
  add_entry_filename(entry2, m_arg_filepath_dir_output);
  
  Glib::OptionEntry entry3;
  entry3.set_long_name("output-name");
  entry3.set_short_name('n');
  entry3.set_description(_("The name for the created .glom file, such as something.glom"));
  add_entry_filename(entry3, m_arg_filepath_name_output);

  Glib::OptionEntry entry_version;
  entry_version.set_long_name("version");
  entry_version.set_short_name('V');
  entry_version.set_description(_("The version of this application."));
  add_entry(entry_version, m_arg_version);
}

static void on_initialize_progress()
{
  std::cout << "Database initialization progress" << std::endl;
}

static void on_startup_progress()
{
  std::cout << "Database startup progress" << std::endl;
}

static void on_recreate_progress()
{
  std::cout << "Database re-creation progress" << std::endl;
}

static void on_cleanup_progress()
{
  std::cout << "Database cleanup progress" << std::endl;
}

/** Delete a directory, if it exists, and its contents.
 * Unlike g_file_delete(), this does not fail if the directory is not empty.
 */
static bool delete_directory(const Glib::RefPtr<Gio::File>& directory)
{
  if(!(directory->query_exists()))
    return true;

  //(Recursively) Delete any child files and directories,
  //so we can delete this directory.
  Glib::RefPtr<Gio::FileEnumerator> enumerator = directory->enumerate_children();

  Glib::RefPtr<Gio::FileInfo> info = enumerator->next_file();
  while(info)
  {
    Glib::RefPtr<Gio::File> child = directory->get_child(info->get_name());
    bool removed_child = false;
    if(child->query_file_type() == Gio::FILE_TYPE_DIRECTORY)
      removed_child = delete_directory(child);
    else
      removed_child = child->remove();

    if(!removed_child)
       return false;

    info = enumerator->next_file();
  }

  //Delete the actual directory:
  if(!directory->remove())
    return false;

  return true;
}

/** Delete a directory, if it exists, and its contents.
 * Unlike g_file_delete(), this does not fail if the directory is not empty.
 */
static bool delete_directory(const std::string& uri)
{
  if(uri.empty())
    return true;

  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
  return delete_directory(file);
}

std::string filepath_dir;

static Glib::ustring convert_filepath_to_uri(const std::string& filepath)
{
  try
  {
    return Glib::filename_to_uri(filepath);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": Could not convert filepath to URI: " << filepath << std::endl;
    return Glib::ustring();
  }
}

static void cleanup()
{
  Glom::ConnectionPool* connection_pool = Glom::ConnectionPool::get_instance();

  const bool stopped = connection_pool->cleanup( sigc::ptr_fun(&on_cleanup_progress) );
  g_assert(stopped);

  //Make sure the directory is removed at the end,
  {
    const Glib::ustring uri = convert_filepath_to_uri(filepath_dir);
    delete_directory(uri);
  }
}


int main(int argc, char* argv[])
{
  Glom::libglom_init();
  
  Glib::OptionContext context;
  GlomCreateOptionGroup group;
  context.set_main_group(group);
  
  try
  {
    context.parse(argc, argv);
  }
  catch(const Glib::OptionError& ex)
  {
      std::cout << _("Error while parsing command-line options: ") << std::endl << ex.what() << std::endl;
      std::cout << _("Use --help to see a list of available command-line options.") << std::endl;
      return 0;
  }
  catch(const Glib::Error& ex)
  {
    std::cout << "Error: " << ex.what() << std::endl;
    return 0;
  }

  if(group.m_arg_version)
  {
    std::cout << PACKAGE_STRING << std::endl;
    return 0;
  }

  // Get a URI for a test file:
  Glib::ustring input_uri = group.m_arg_filename_input;

  // The GOption documentation says that options without names will be returned to the application as "rest arguments".
  // I guess this means they will be left in the argv. Murray.
  if(input_uri.empty() && (argc > 1))
  {
     const char* pch = argv[1];
     if(pch)
       input_uri = pch;
  }
  
  if(!input_uri.empty())
  {
    //Get a URI (file://something) from the filepath:
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_commandline_arg(input_uri);

    if(!file->query_exists())
    {
      std::cerr << _("Glom: The file does not exist.") << std::endl;
      std::cerr << "uri: " << input_uri << std::endl;

      std::cerr << std::endl << context.get_help() << std::endl;
      return EXIT_FAILURE;
    }

    const Gio::FileType file_type = file->query_file_type();
    if(file_type == Gio::FILE_TYPE_DIRECTORY)
    {
      std::cerr << _("Glom: The file path is a directory instead of a file.") << std::endl;
      std::cerr << std::endl << context.get_help() << std::endl;
      return EXIT_FAILURE;
    }

    input_uri = file->get_uri();

  }
  
  if(input_uri.empty())
  {
    std::cerr << "Please specify a glom example file." << std::endl;
    std::cerr << std::endl << context.get_help() << std::endl;
    return EXIT_FAILURE;
  }


  //Check the output directory path: 
  if(group.m_arg_filepath_dir_output.empty())
  {
    std::cerr << "Please specify an output directory path." << std::endl;
    std::cerr << std::endl << context.get_help() << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    //Get a URI (file://something) from the filepath:
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_commandline_arg(group.m_arg_filepath_dir_output);

    if(!file->query_exists())
    {
      std::cerr << _("Glom: The output directory does not exist.") << std::endl;
      std::cerr << "uri: " << group.m_arg_filepath_dir_output << std::endl;

      std::cerr << std::endl << context.get_help() << std::endl;
      return EXIT_FAILURE;
    }

    const Gio::FileType file_type = file->query_file_type();
    if(file_type != Gio::FILE_TYPE_DIRECTORY)
    {
      std::cerr << _("Glom: The output path is not a directory.") << std::endl;
      std::cerr << std::endl << context.get_help() << std::endl;
      return EXIT_FAILURE;
    }
  }
  
  //Check the output name path: 
  if(group.m_arg_filepath_name_output.empty())
  {
    std::cerr << "Please specify an output name." << std::endl;
    std::cerr << std::endl << context.get_help() << std::endl;
    return EXIT_FAILURE;
  }


  // Load the document:
  Glom::Document document;
  document.set_file_uri(input_uri);
  int failure_code = 0;
  const bool test = document.load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!test)
  {
    std::cerr << "Document::load() failed with failure_code=" << failure_code << std::endl;
    return EXIT_FAILURE;
  }

  g_assert(document.get_is_example_file());;

  Glom::ConnectionPool* connection_pool = Glom::ConnectionPool::get_instance();

  //Save a copy, specifying the path to file in a directory:
  filepath_dir = 
    Glom::Utils::get_file_path_without_extension(
      Glib::build_filename(
        group.m_arg_filepath_dir_output,
        group.m_arg_filepath_name_output));
  const std::string filepath =
    Glib::build_filename(filepath_dir, group.m_arg_filepath_name_output);

  //Make sure that the file does not exist yet:
  {
    const Glib::ustring uri = convert_filepath_to_uri(filepath_dir);
    if(uri.empty())
      return EXIT_FAILURE;
        
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_commandline_arg(uri);
    if(file->query_exists())
    {
      std::cerr << "The output path already exists: " << filepath_dir << std::endl;
      return EXIT_FAILURE;
    }
  }

  //Save the example as a real file:
  const Glib::ustring file_uri = convert_filepath_to_uri(filepath);
  if(file_uri.empty())
    return EXIT_FAILURE;

  document.set_file_uri(file_uri);

  document.set_hosting_mode(Glom::Document::HOSTING_MODE_POSTGRES_SELF);
  document.set_is_example_file(false);
  document.set_network_shared(false);
  const bool saved = document.save();
  g_assert(saved);

  //Specify the backend and backend-specific details to be used by the connectionpool.
  connection_pool->setup_from_document(&document);

  //We must specify a default username and password:
  Glib::ustring password;
  const Glib::ustring user = Glom::Privs::get_default_developer_user_name(password);
  connection_pool->set_user(user);
  connection_pool->set_password(password);

  //Create the self-hosting files:
  const Glom::ConnectionPool::InitErrors initialized_errors =
    connection_pool->initialize( sigc::ptr_fun(&on_initialize_progress) );
  g_assert(initialized_errors == Glom::ConnectionPool::Backend::INITERROR_NONE);

  //Start self-hosting:
  //TODO: Let this happen automatically on first connection?
  const Glom::ConnectionPool::StartupErrors started = connection_pool->startup( sigc::ptr_fun(&on_startup_progress) );
  if(started != Glom::ConnectionPool::Backend::STARTUPERROR_NONE)
  {
    std::cerr << "connection_pool->startup(): result=" << started << std::endl;
    cleanup();
  }
  g_assert(started == Glom::ConnectionPool::Backend::STARTUPERROR_NONE);

  const bool recreated = Glom::DbUtils::recreate_database_from_document(&document, sigc::ptr_fun(&on_recreate_progress) );
  if(!recreated)
    cleanup();
  g_assert(recreated);
  

  //Tell the user where the file is:
  std::string output_path_used;
  try
  {
    output_path_used = Glib::filename_from_uri(document.get_file_uri());
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": Could not convert URI to output filepath: " << document.get_file_uri() << std::endl;
  }
   
  std::cout << "Glom file created at: " << output_path_used << std::endl;


  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}