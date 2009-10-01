/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
 * Copyright (C) 2009 Openismus GmbH
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

#ifndef GLOM_IMPORT_CSV_H
#define GLOM_IMPORT_CSV_H

#include "base_db.h"

#include <memory>
#include <giomm/asyncresult.h>
#include <giomm/file.h>
#include <giomm/inputstream.h>
#include <gtkmm/liststore.h>

namespace Glom
{

// We use the low-level Glib::IConv routines to progressively convert the
// input data in an idle handler.

/** Parses .csv (comma-separated values) text files.
 * See http://en.wikipedia.org/wiki/Comma-separated_values for the file format.
 *
 * set_file_and_start_parsing() to start parsing.
 * The data can then be read via get_date(), with get_rows_count() and get_cols_count().
 * 
 * The signals offer feedback while the parsing is happening.
 */
class CsvParser
{
public:

  typedef std::vector<Glib::ustring> type_row_strings;
  typedef std::vector<type_row_strings> type_rows;

  //TODO: Avoid having to specify an initial encoding.
  explicit CsvParser(const std::string& encoding_charset);

  ~CsvParser();

  enum State {
    STATE_NONE, 
    STATE_PARSING,  /**< Parsing is in progress. */
    STATE_ENCODING_ERROR, /**< An error happened while parsing. */
    STATE_PARSED /**< Finished parsing. */
  };

  /// Get the current state of the parser.
  State get_state() const;

/*
  /// Get the number of rows parsed so far.
  guint get_rows_count() const;
*/

  bool get_rows_empty() const;

/*
  /// Get the number of columns of data in this row.
  guint get_cols_count(guint row_number) const;
*/

  // The nasty reference return is for performance.
  const Glib::ustring& get_data(guint row, guint col);

  const type_row_strings fetch_next_row();

  void reset_row_index();

  // Signals:
  typedef sigc::signal<void, const Glib::ustring&> type_signal_file_read_error;

  /** This signal will be emitted if the parser encounters an error while trying to open the file for reading.
   */
  type_signal_file_read_error signal_file_read_error() const;


  typedef sigc::signal<void, const Glib::ustring&> type_signal_have_display_name;

  /** This signal will be emitted when the parser has discovered the 
   * display name for the file. This does not require any parsing of the contents, 
   * but it is asynchronous, so CsvParser signals this as a convenience.
   */
  type_signal_have_display_name signal_have_display_name() const;


  typedef sigc::signal<void> type_signal_encoding_error;

  /** This signal will be emitted when the parser encounters an error while parsing.
   * TODO: How do we discover what the error is?
   */
  type_signal_encoding_error signal_encoding_error() const;


  typedef sigc::signal<void, type_row_strings, unsigned int> type_signal_line_scanned;

  /** This signal will be emitted each time the parser has scanned a line. TODO: Do we mean row instead of line? - A row contain a newline.
   */
  type_signal_line_scanned signal_line_scanned() const;


  typedef sigc::signal<void> type_signal_finished_parsing;

  /** This signal will be emitted when the parser successfully finished to parse a file.
   */
  type_signal_finished_parsing signal_finished_parsing() const;


  typedef sigc::signal<void> type_signal_state_changed;

  /** This signal will be emitted when the state changes.
   */
  type_signal_state_changed signal_state_changed() const;


  /// Make parser object reusable.
  void clear();

  /** Change the encoding used when reading the file.
   * This stop parsing. Call set_file_and_start_parsing() to restart the parser  
   * with the specified encoding. 
   * See the FileEncoding namespace.
   */
  void set_encoding(const Glib::ustring& encoding_charset);

  void set_file_and_start_parsing(const std::string& uri);

private:
  // In order to not make the UI feel sluggish during larger imports we parse
  // on chunk at a time in the idle handler.
  bool on_idle_parse();

  void begin_parse();

  static const gunichar DELIMITER = ',';
  static const gunichar QUOTE = '\"';

  static bool next_char_is_quote(const Glib::ustring::const_iterator& iter, const Glib::ustring::const_iterator& end);

  void do_line_scanned(const Glib::ustring& current_line, guint line_number);

  //TODO: Document this:
  static Glib::ustring::const_iterator advance_field(const Glib::ustring::const_iterator& iter, const Glib::ustring::const_iterator& end, Glib::ustring& field);

  void on_file_read(const Glib::RefPtr<Gio::AsyncResult>& result);
  void copy_buffer_and_continue_reading(gssize size);
  void on_buffer_read(const Glib::RefPtr<Gio::AsyncResult>& result);
  void on_file_query_info(const Glib::RefPtr<Gio::AsyncResult>& result);

  void set_state(State state);

  // The raw data in the original encoding. We keep this so we can convert
  // from the user-selected encoding to UTF-8 every time the user changes
  // the encoding.
  std::vector<char> m_raw;

  std::string m_encoding;
  std::vector<char>::size_type m_input_position;
  std::string m_current_line;

  sigc::connection m_idle_connection;
  unsigned int m_line_number;

  State m_state;

  Glib::RefPtr<Gio::FileInputStream> m_stream;

  // Parsed data:
  type_rows m_rows;

  guint m_row_index;

  type_signal_file_read_error m_signal_file_read_error;
  type_signal_have_display_name m_signal_have_display_name;
  type_signal_encoding_error m_signal_encoding_error;
  type_signal_line_scanned m_signal_line_scanned;
  type_signal_finished_parsing m_finished_parsing;
  type_signal_state_changed m_signal_state_changed;

  Glib::RefPtr<Gio::File> m_file;

  struct Buffer
  {
    char buf[1024];
  };
  std::auto_ptr<Buffer> m_buffer;
};

} //namespace Glom

#endif //GLOM_IMPORT_CSV_H


