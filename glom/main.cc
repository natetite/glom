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

#include <config.h> //For VERSION.

//We use Python for calculated fields.
#include <Python.h> //Include it before anything else to avoid "_POSIX_C_SOURCE redefined".

//#include <gnome.h>
#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <giomm.h>

// For postgres availability checks:
#ifdef GLOM_ENABLE_POSTGRESQL
#include <libglom/connectionpool_backends/postgres.h>
#ifndef GLOM_ENABLE_CLIENT_ONLY
#include <libglom/connectionpool_backends/postgres_self.h>
#endif //GLOM_ENABLE_CLIENT_ONLY
#endif //GLOM_ENABLE_POSTGRESQL

// For sanity checks:
#include <libglom/data_structure/glomconversions.h> // For GLOM_IMAGE_FORMAT

#ifndef GLOM_ENABLE_CLIENT_ONLY
#include <gtksourceviewmm/init.h>
#include <goocanvasmm/init.h>
#endif // !GLOM_ENABLE_CLIENT_ONLY
#include <glibmm/i18n.h>

#ifdef GLOM_ENABLE_MAEMO
#include <hildonmm/init.h>
#include <hildonmm/note.h>
#endif

#include <glom/application.h>
#include <glom/glade_utils.h>
#include <glom/utils_ui.h>

#ifndef G_OS_WIN32
#include <fontconfig/fontconfig.h> //For cleanup.
#else
#define SAVE_DATADIR DATADIR
#undef DATADIR
#include <winsock2.h>
#define DATADIR SAVE_DATADIR
#endif

namespace Glom
{

class OptionGroup : public Glib::OptionGroup
{ 
public:
  OptionGroup();

  //These int instances should live as long as the OptionGroup to which they are added, 
  //and as long as the OptionContext to which those OptionGroups are added.
  std::string m_arg_filename;
  bool m_arg_version;
  bool m_arg_debug_sql;
};

OptionGroup::OptionGroup()
: Glib::OptionGroup("Glom", _("Glom options"), _("Command-line options for glom")),
  m_arg_version(false),
  m_arg_debug_sql(false)
{
  Glib::OptionEntry entry;
  entry.set_long_name("file");
  entry.set_short_name('f');
  entry.set_description(_("The Filename"));
  add_entry_filename(entry, m_arg_filename);

  Glib::OptionEntry entry_version;
  entry_version.set_long_name("version");
  entry_version.set_short_name('V');
  entry_version.set_description(_("The version of this application."));
  add_entry(entry_version, m_arg_version);

  Glib::OptionEntry entry_debug_sql;
  entry_version.set_long_name("debug_sql");
  entry_version.set_description(_("Show the generated SQL queries on stdout, for debugging."));
  add_entry(entry_version, m_arg_debug_sql);
}

#ifdef GLOM_ENABLE_POSTGRESQL
bool check_user_is_not_root()
{
  Glib::ustring message;
#ifdef G_OS_WIN32
  try
  {
    if(pgwin32_is_admin())
    {
      message = _("You seem to be running Glom as a user with administrator privileges. Glom may not be run with such privileges for security reasons.\nPlease login to your system as a normal user.");
    }
  }
  catch(const std::runtime_error& ex)
  {
    message = ex.what();
  }
#else
  //std::cout << "ConnectionPool::check_user_is_not_root(): geteuid()=" << geteuid() << ", getgid()=" << getgid() << std::endl;

  //This is very linux-specific. We should ifdef this out for other platforms.
  if(geteuid() == 0)
  {
    //Warn the user:
    message = _("You seem to be running Glom as root. Glom may not be run as root.\nPlease login to your system as a normal user.");
  }
#endif

  if(!message.empty())
  {
#ifndef GLOM_ENABLE_MAEMO
    Gtk::MessageDialog dialog(Utils::bold_message(_("Running As Root")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true /* modal */);
    dialog.set_secondary_text(message);
    dialog.run();
#else
    Hildon::Note note(Hildon::NOTE_TYPE_INFORMATION, message);
    note.run();
#endif

    return false; /* Is root. Bad. */
  }

  return true; /* Not root. It's OK. */
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
// Message to packagers:
// If your Glom package does not depend on PostgreSQL, for some reason, 
// then your distro-specific patch should uncomment this #define.
// and implement ConnectionPool::install_posgres().
// But please, just make your Glom package depend on PostgreSQL instead, 
// because this is silly.
//
//#define DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED 1

/** Check whether PostgreSQL is really available for self-hosting,
 * in case the distro package has incorrect dependencies.
 *
 * @results True if everything is OK.
 */
bool check_postgres_is_available_with_warning()
{
  const std::string binpath = Glom::ConnectionPoolBackends::PostgresSelfHosted::get_path_to_postgres_executable("postgres");

  // TODO: At least on Windows we should probably also check for initdb and
  // pg_ctl. Perhaps it would also be a good idea to access these files as
  // long as glom runs so they cannot be (re)moved.
  if(!binpath.empty())
  {
    const Glib::ustring uri_binpath = Glib::filename_to_uri(binpath);
    if(Utils::file_exists(uri_binpath))
      return true;
  }

  #ifdef DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED

  //Show message to the user about the broken installation:
  //This is a packaging bug, but it would probably annoy packagers to mention that in the dialog:
  Gtk::MessageDialog dialog(Utils::bold_message(_("Incomplete Glom Installation")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_NONE, true /* modal */);
  dialog.set_secondary_text(_("Your installation of Glom is not complete, because PostgreSQL is not available on your system. PostgreSQL is needed for self-hosting of Glom databases.\n\nYou may now install PostgreSQL to complete the Glom installation."));
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(_("Install PostgreSQL"), Gtk::RESPONSE_OK);
  const int response = dialog.run();
  if(response != Gtk::RESPONSE_OK)
    return false; //Failure. Glom should now quit.
  else
    return install_postgres(&dialog);

  #else  //DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED

  //Show message to the user about the broken installation:
  Gtk::MessageDialog dialog(Utils::bold_message(_("Incomplete Glom Installation")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true /* modal */);
  dialog.set_secondary_text(_("Your installation of Glom is not complete, because PostgreSQL is not available on your system. PostgreSQL is needed for self-hosting of Glom databases.\n\nPlease report this bug to your vendor, or your system administrator so it can be corrected."));
  dialog.run();
  return false;

  #endif //DISTRO_SPECIFIC_POSTGRES_INSTALL_IMPLEMENTED
}
#endif //GLOM_ENABLE_CLIENT_ONLY

#endif //GLOM_ENABLE_POSTGRESQL

} //namespace Glom

#ifndef G_OS_WIN32
extern "C" void __libc_freeres(void);
#endif

int 
main(int argc, char* argv[])
{
#ifndef G_OS_WIN32
  //Force some cleanup at exit,
  //to help valgrind to detect memory leaks:
  atexit(__libc_freeres);
#else
  WSADATA data;
  int errcode = WSAStartup(MAKEWORD(2, 0), &data);
  if(errcode != 0)
  {
    std::cerr << "Failed to initialize WinSock: " << errcode << std::endl;
    return -1;
  }

  gchar* installation_dir_c = g_win32_get_package_installation_directory_of_module(NULL);
  const std::string installation_dir(installation_dir_c);
  g_free(installation_dir_c);
#endif

  // TODO: I am not sure why, but this does not work. PYTHONPATH is set
  // correctly according to getenv(), but python still does not look in it.
  // For now, the installer installs all the python stuff directly into the 
  // application directory, although I would like to move this to a python/
  // subdirectory.
#if 0
#ifdef G_OS_WIN32
  // Set PYTHONPATH to point to python/ because that's where the installer
  // installs all the python modules into.
  std::string python_path = Glib::build_filename(installation_dir, "python");
  std::string current_path = Glib::getenv("PYTHONPATH");
  if(current_path.empty()) current_path = python_path;
  else current_path += (std::string(";") + python_path); // PATH-like variables are separated by ; on Windows because : is a valid character in paths.
  std::cout << "Setting " << current_path << ":" << std::endl;
  std::cout << Glib::setenv("PYTHONPATH", current_path) << std::endl;
  std::cout << getenv("PYTHONPATH") << std::endl;
#endif
#endif

#ifdef G_OS_WIN32
  // Add glom's bin directory to PATH so that g_spawn* finds the
  // gspawn-win32-helper.exe helper program. The installer installs it there.
  Glib::setenv("PATH", Glib::getenv("PATH") + ";" + Glib::build_filename(installation_dir, "bin"));
#endif

#ifdef G_OS_WIN32
  // Load translations relative to glom.exe on Windows
  bindtextdomain(GETTEXT_PACKAGE, Glib::build_filename(installation_dir, "share/locale").c_str());
#else
  //Make this application use the current locale for _() translation:
  bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);  //LOCALEDIR is defined in the Makefile.am
#endif
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  g_thread_init(NULL); //So we can use GMutex.

  Gnome::Gda::init();
#ifdef GLOM_ENABLE_MAEMO
  Hildon::init();
#endif

  Glib::OptionContext context;

  Glom::OptionGroup group;
  context.set_main_group(group);
  //We use python for calculated-fields:
  Py_Initialize();
  PySys_SetArgv(argc, argv);
  Gtk::Main mainInstance(argc, argv, context);

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
#else
  std::auto_ptr<Glib::Error> error;
#endif // GLIBMM_EXCEPTIONS_ENABLED
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    context.parse(argc, argv);
#else
    context.parse(argc, argv, error);
#endif // GLIBMM_EXCEPTIONS_ENABLED
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  catch(const Glib::OptionError& ex)
#else
  if(error.get() != NULL)
#endif
  {
#ifndef GLIBMM_EXCEPTIONS_ENABLED
    const Glib::OptionError* exptr = dynamic_cast<Glib::OptionError*>(error.get());
    if(exptr)
    {
      const Glib::OptionError& ex = *exptr;
#endif // !GLIBMM_EXCEPTIONS_ENABLED
      std::cout << _("Error while parsing command-line options: ") << std::endl << ex.what() << std::endl;
      std::cout << _("Use --help to see a list of available command-line options.") << std::endl;
      return 0;
#ifndef GLIBMM_EXCEPTIONS_ENABLED
    }
    const Glib::Error& ex = *error.get();
#else
  }
  catch(const Glib::Error& ex)
  {
#endif
    std::cout << "Error: " << ex.what() << std::endl;
    return 0;
  }


  if(group.m_arg_version)
  {
    std::cout << VERSION << std::endl;
    return 0;
  }

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
#endif
  {
#ifndef GLOM_ENABLE_CLIENT_ONLY
    gtksourceview::init();
    Goocanvas::init(PACKAGE, VERSION, argc, argv ) ;
#endif //!GLOM_ENABLE_CLIENT_ONLY

    //Get command-line parameters, if any:
    Glib::ustring input_uri = group.m_arg_filename;

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
      if(file)
        input_uri = file->get_uri(); 
      //std::cout << "URI = " << input_uri << std::endl;
    }

    //debugging:
    //input_uri = "file:///home/murrayc/cvs/gnome212/glom/examples/example_smallbusiness.glom";

#ifdef GLOM_ENABLE_POSTGRESQL

    //Check that the libgda postgres provider is really available:
    bool install_complete = Glom::ConnectionPoolBackends::Postgres::check_postgres_gda_client_is_available();
    if(!install_complete)
    {
      /* The Postgres provider was not found, so warn the user: */
      const Glib::ustring message = _("Your installation of Glom is not complete, because the PostgreSQL libgda provider is not available on your system. This provider is needed to access Postgres database servers.\n\nPlease report this bug to your vendor, or your system administrator so it can be corrected.");
      
      #ifndef GLOM_ENABLE_MAEMO
      Gtk::MessageDialog dialog(Glom::Utils::bold_message(_("Incomplete Glom Installation")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true /* modal */);
      dialog.set_secondary_text(message);
      dialog.run();
      #else
      Hildon::Note note(Hildon::NOTE_TYPE_INFORMATION, message);
      note.run();
      #endif
      
      return -1; //There is no point in going further because Glom would not be able to connect to any Postgres servers.
    }

    #ifndef GLOM_ENABLE_CLIENT_ONLY
    //Check that the postgres executable is really available:
    if(!Glom::check_postgres_is_available_with_warning())
      return -1; //There is no point in going further because the most useful Glom functionality will not work without Postgres. Only a very cut-down Glom client would be useful without self-hosting.
    #endif // !GLOM_ENABLE_CLIENT_ONLY
      
    // Postgres can't be started as root. initdb complains.
    // So just prevent this in general. It is safer anyway.
    if(!Glom::check_user_is_not_root())
      return -1;
#endif //GLOM_ENABLE_POSTGRESQL


    // Some more sanity checking:
    // These print errors to the stdout if they fail.
    // In future we might refuse to start if they fail.
    const bool test1 = Glom::Conversions::sanity_check_date_text_representation_uses_4_digit_years();
    const bool test2 = Glom::Conversions::sanity_check_date_parsing();
    if(!test1 || !test2)
    {
      std::cerr << "Glom: ERROR: Date parsing sanity checks failed. Glom will not display dates correctly or interperet entered dates correctly. This needs attention from a translator. Please file a bug. See http://www.glom.org." << std::endl;
    }


#ifdef GLIBMM_EXCEPTIONS_ENABLED
    // Main app
    Glib::RefPtr<Gtk::Builder> refXml = Gtk::Builder::create_from_file(Glom::Utils::get_glade_file_path("glom.glade"), "window_main");
#else
    std::auto_ptr<Glib::Error> error;
    Glib::RefPtr<Gtk::Builder> refXml = Gtk::Builder::create_from_file(Glom::Utils::get_glade_file_path("glom.glade"), "window_main", "", error);
    if(error.get())
    {
      std::cerr << "Glom: exception: \n  " << error->what() << std::endl;
      return -1;
    }
#endif


    Glom::App_Glom* pApp_Glom = 0;
    refXml->get_widget_derived("window_main", pApp_Glom);

    pApp_Glom->set_command_line_args(argc, argv);
    pApp_Glom->set_show_sql_debug(group.m_arg_debug_sql);

    bool test = pApp_Glom->init(input_uri); //Sets it up and shows it.
    if(test) //The user could cancel the offer of a new or existing database.
      Gtk::Main::run(*pApp_Glom); //Quit when the window is closed.

    //Cleanup:
    delete pApp_Glom;
    pApp_Glom = 0;
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  catch(const Glib::Exception& ex)
  {
    //If this happens then comment out the try/catch, and let the debugger show the call stack.
    std::cerr << "Glom: exception: \n  " << ex.what() << std::endl;
  }
  catch(const std::exception& ex)
  {
    //If this happens then comment out the try/catch, and let the debugger show the call stack.
    std::cerr << "Glom: exception: \n  " << ex.what() << std::endl;
  }
#endif // GLIBMM_EXCEPTIONS_ENABLED

  //We use python for calculated-fields:
  Py_Finalize();

  //Clean up singletons:
  Glom::ConnectionPool::delete_instance();

  //These fail, probably because of previous things that are causing leaks:
  //cairo_debug_reset_static_data(); //This crashes with _cairo_hash_table_destroy: Assertion `hash_table->live_entries == 0' failed.
  //FcFini(); //This crashes with "FcCacheFini: Assertion `fcCacheChains[i] == ((void *)0)' failed."
#ifdef G_OS_WIN32
  WSACleanup();
#endif

  return 0;
}


