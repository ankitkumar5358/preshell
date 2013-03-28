#ifndef PRESHELL_CONFIG_HPP
#define PRESHELL_CONFIG_HPP

// Copyright Abel Sinkovics (abel@sinkovics.hu)  2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <vector>

namespace preshell
{
  struct config
  {
    std::vector<std::string> include_path;
    std::vector<std::string> sysinclude_path;

    config(); // gives the default config
  };
}

#endif

