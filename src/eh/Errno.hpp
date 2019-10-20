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



#ifndef EH_ERRNO_HPP
#define EH_ERRNO_HPP

#include <errno.h>

#include <String/StringManip.hpp>


namespace eh
{
  //
  // Errno throw helper
  //

  namespace ErrnoHelper
  {
    // There are two versions of strerror_r functions with
    // different behavior
    inline
    const char*
    error_message(const char* /*buf*/, const char* function_result)
    {
      return function_result;
    }

    inline
    const char*
    error_message(const char* buf, int /*function_result*/)
    {
      return buf;
    }

    template <typename... Args>
    void
    compose_safe(char* string, size_t string_size, int error, Args... args)
      throw ()
    {
      char error_buf[128];
      char buf[128];

      String::StringManip::int_to_str(error, error_buf, sizeof(error_buf));
      String::StringManip::concat(string, string_size, args...,
        ": errno = ", error_buf, ": ",
        error_message(buf, strerror_r(error, buf, sizeof(buf))));
    }
  }

  template <typename SomeException, typename... Args>
  void
  throw_errno_exception(int error, Args... args) throw (SomeException)
  {
    char string[sizeof(SomeException)];
    ErrnoHelper::compose_safe(string, sizeof(string), error, args...);
    throw SomeException(string);
  }

  template <typename SomeException, typename... Args>
  void
  throw_errno_exception(Args... args) throw (SomeException)
  {
    throw_errno_exception<SomeException>(errno, args...);
  }
}

#endif
