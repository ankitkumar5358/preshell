#ifndef EDITLINE_SHELL_HPP
#define EDITLINE_SHELL_HPP

// Copyright Abel Sinkovics (abel@sinkovics.hu)  2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <preshell/shell.hpp>

#include <vector>
#include <string>

class editline_shell : public preshell::shell
{
public:
  editline_shell(
    const preshell::config& config_,
    const std::vector<std::string>& macros_
  );
  virtual ~editline_shell();

  virtual void display_normal(const std::string& s_) const;
  virtual void display_info(const std::string& s_) const;
  virtual void display_error(const std::string& s_) const;

  void run();
private:
  static char* tab_generator(const char* text_, int state_);
  static char** tab_completion(const char* text_, int start_, int end_);

  static editline_shell* _instance;
};

#endif

