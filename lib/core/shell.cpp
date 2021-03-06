// Preshell - Interactive C/C++ preprocessor shell
// Copyright (C) 2013, Abel Sinkovics (abel@sinkovics.hu)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <preshell/shell.hpp>
#include <preshell/preshell.hpp>
#include <preshell/version.hpp>
#include <preshell/indenter.hpp>
#include <preshell/util.hpp>

#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <boost/preprocessor/stringize.hpp>

using namespace preshell;

namespace
{
  const char clang_compatibility_macros[] =
    "#ifndef __has_builtin\n"
    "#  define __has_builtin(x) 0\n"
    "#endif\n"

    "#ifndef __has_feature\n"
    "#  define __has_feature(x) 0\n"
    "#endif\n"

    "#ifndef __has_extension\n"
    "#  define __has_extension __has_feature\n"
    "#endif\n"

    "#ifndef __has_attribute\n"
    "#  define __has_attribute(x) 0\n"
    "#endif\n"

    "#ifndef __has_include\n"
    "#  define __has_include(x) 0\n"
    "#endif\n"

    "#ifndef __has_include_next\n"
    "#  define __has_include_next(x) 0\n"
    "#endif\n"

    "#ifndef __has_warning\n"
    "#  define __has_warning(x) 0\n"
    "#endif\n"
    ;
}

shell::shell(
  const preshell::config& config_,
  const std::vector<std::string>& macros_
) :
  _info_indenter(
    boost::bind(&shell::width, static_cast<shell*>(this)),
    boost::bind(&shell::display_info, static_cast<shell*>(this), _1)
  ),
  _error_indenter(
    boost::bind(&shell::width, static_cast<shell*>(this)),
    boost::bind(&shell::display_error, static_cast<shell*>(this), _1)
  ),
  _config(config_),
  _context(
    new result(
      context::initial(
        config_,
        macros_,
        _info_indenter,
        _error_indenter,
        _history
      )
    )
  ),
  _initial_context(_context->pp_context)
{
  indenter ignore(&always<80>, throw_away);

  _context =
    precompile(
      clang_compatibility_macros,
      _context->pp_context,
      _config,
      ignore,
      _history
    );

  precompile_input(clang_compatibility_macros);

  if (!_config.builtin_macro_definitions.empty())
  {
    _context =
      precompile(
        remove_protected_macro_definitions(_config.builtin_macro_definitions),
        _context->pp_context,
        _config,
        ignore,
        _history
      );
  }

  _context->pp_context.line = 1;
}

shell::~shell() {}

const std::vector<std::string> shell::directives =
  boost::assign::list_of<std::string>
    ("#define")
    ("#elif")
    ("#else")
    ("#endif")
    ("#error")
    ("#if")
    ("#ifdef")
    ("#ifndef")
    ("#include")
    ("#line")
    ("#pragma")
    ("#undef")

    ("_Pragma(\"") // Supported by Wave
  ;

void shell::cancel_operation()
{
  preshell::cancel();
}

const preshell::config& shell::config() const
{
  return _config;
}

const preshell::context& shell::context() const
{
  return _context->pp_context;
}

void shell::display_splash() const
{
  using boost::bind;

  const std::string version_desc =
    #include "version_desc.hpp"
  ;

  indenter
    ind(bind(&shell::width, this), bind(&shell::display_normal, this, _1));

  ind
    .raw("/*")
    .left_align("Preprocessor shell " + preshell::version(), " * ");

  if (!version_desc.empty())
  {
    ind.left_align(version_desc, " * ");
  }

  ind
    .raw(" * ")
    .left_align(
      "Preshell Copyright (C) 2013 Abel Sinkovics (abel@sinkovics.hu)",
      " * "
    )
    .left_align(
      "This program comes with ABSOLUTELY NO WARRANTY. This is free software,"
      " and you are welcome to redistribute it under certain conditions;"
      " for details visit <http://www.gnu.org/licenses/>.",
      " * "
    )
    .raw(" * ")
    .left_align("Based on", " * ")
    .left_align(
      preshell::wave_version(),
      " *              ",
      " *   Boost.Wave "
    )
    .left_align(
      preshell::readline_version(),
      " *              ",
      " *   Readline   "
    )
    .raw(" * ")
    .left_align("Getting help: #pragma wave preshell_help", " * ")
    .raw(" */")
    .flush();
}

void shell::line_available(const std::string& s_)
{
  if (!s_.empty())
  {
    if (s_ != _prev_line)
    {
      add_history(s_);
      _prev_line = s_;
    }
    if (!is_pragma_usage(s_))
    {
      _history.push_back(s_);
    }
  }

  process_line<true>(s_);
}

std::string shell::prompt() const
{
  return
    _buffer == "" ?
      (_context->pp_context.tokens_skipped() ? "#if 0 ...> " : "> ") :
      ">>>> ";
}

void shell::precompile_input(const std::string& s_)
{
  _context =
    precompile(s_, _context->pp_context, _config, _info_indenter, _history);
}

void shell::display_output_if_available() const
{
  if (_context->output != "")
  {
    display_normal(_context->output);
  }
}

void shell::display_info_if_available() const
{
  if (_context->info != "")
  {
    display_info(_context->info);
  }
}

void shell::display_error_if_available() const
{
  if (_context->error != "")
  {
    display_error(_context->error);
  }
}

void shell::display_initialisation_diagnostic() const
{
  display_error_if_available();
}

const std::list<std::string>& shell::history() const
{
  return _history;
}

void shell::replay_history()
{
  _context->pp_context = _initial_context;
  BOOST_FOREACH(const std::string& s, _history)
  {
    process_line<false>(s);
  }
}

template <bool EnableOutput>
void shell::process_line(const std::string& s_)
{
  _buffer += (_buffer == "" ? "" : "\n") + s_;

  if (!preshell::continuation_needed(_buffer, _config))
  {
    assert(!_context->replay_history);

    _context =
      precompile(
        _buffer,
        _context->pp_context,
        _config,
        _info_indenter,
        _history
      );
    _buffer = "";

    if (EnableOutput)
    {
      display_output_if_available();
      display_info_if_available();
      display_error_if_available();
    }

    if (_context->replay_history)
    {
      _context->replay_history = false;
      replay_history();
    }
  }
}

