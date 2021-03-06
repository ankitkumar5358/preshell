#ifndef PRESHELL_SHELL_HPP
#define PRESHELL_SHELL_HPP

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

#include <preshell/config.hpp>
#include <preshell/context.hpp>
#include <preshell/result.hpp>
#include <preshell/indenter.hpp>

#include <vector>
#include <list>
#include <string>

namespace preshell
{
  class shell
  {
  public:
    shell(
      const preshell::config& config_,
      const std::vector<std::string>& macros_
    );
    virtual ~shell();

    virtual void add_history(const std::string& s_) = 0;
  
    virtual void display_normal(const std::string& s_) const = 0;
    virtual void display_info(const std::string& s_) const = 0;
    virtual void display_error(const std::string& s_) const = 0;

    virtual unsigned int width() const = 0;
  
    void display_splash() const;
    void line_available(const std::string& s_);
    std::string prompt() const;
  
    const preshell::config& config() const;
    const preshell::context& context() const;
  
    void cancel_operation();

    // Precondition: line_available has not been called
    void display_initialisation_diagnostic() const;
  
    static const std::vector<std::string> directives;

    const std::list<std::string>& history() const;

    void replay_history();
  private:
    indenter _info_indenter;
    indenter _error_indenter;
    preshell::config _config;
    preshell::result_ptr _context;
    std::string _buffer;
    std::string _prev_line;
    std::list<std::string> _history;
    preshell::context _initial_context;

    void precompile_input(const std::string& s_);

    void display_output_if_available() const;
    void display_info_if_available() const;
    void display_error_if_available() const;

    template <bool EnableOutput>
    void process_line(const std::string& s_);
  };
}

#endif

