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



#ifndef HTTP_HTTP_HPP
#define HTTP_HTTP_HPP

#include <HTTP/HttpAsync.hpp>
#include <HTTP/HttpAsyncPolicies.hpp>
#include <HTTP/HttpSync.hpp>
#include <HTTP/HttpClient.hpp>


namespace HTTP
{
  /**
   * Checks separate HTTP header for RFC compliance
   * @param name header name
   * @param value header value
   * @return whether or not header is RFC compliant
   */
  bool
  check_header(const char* name, const char* value) throw ();

  /**
   * Checks headers for RFC compliance
   * @param headers list of headers
   * @return true only if every header is RFC compliant
   */
  bool
  check_headers(const HeaderList& headers) throw ();
}

#endif
