/* 
 * This file is part of the UnixCommons distribution (https://github.com/yoori/unixcommons).
 * UnixCommons contains help classes and functions for Unix Server application writing
 *
 * Copyright (c) 2012 Yuri Kuznecov <yuri.kuznecov@gmail.com>.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */



#include <String/AsciiStringManip.hpp>

#include <Generics/BoolFunctors.hpp>

#include <HTTP/Http.hpp>


namespace
{
  // RFC 2616
  const String::AsciiStringManip::CharCategory LWS(" \t");
  const String::AsciiStringManip::CharCategory SEPARATORS(
    "()<>@,;:\\\"/[]?={} \t");

  const String::AsciiStringManip::CharCategory NAME(
    Generics::and1(
      String::AsciiStringManip::CharCategory(" -\x7E"),
      Generics::not1(SEPARATORS)));

  const String::AsciiStringManip::CharCategory VALUE("\t -\x7E");
}

namespace HTTP
{
  bool
  check_header(const char* name, const char* value) throw ()
  {
    if (!name || !value)
    {
      return false;
    }

    // Check name
    if (*NAME.find_nonowned(name))
    {
      return false;
    }

    // Check value
    for (const char* ptr = value; *(ptr = VALUE.find_nonowned(ptr));)
    {
      if (*ptr == '\r' && ptr[1] == '\n' && LWS(ptr[2]))
      {
        ptr += 2;
        continue;
      }
      return false;
    }
    return true;
  }

  bool
  check_headers(const HeaderList& headers) throw ()
  {
    for (HeaderList::const_iterator itor(headers.begin());
      itor != headers.end(); ++itor)
    {
      if (!check_header(itor->name.c_str(), itor->value.c_str()))
      {
        return false;
      }
    }
    return true;
  }
}
