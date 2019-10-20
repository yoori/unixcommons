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



#ifndef PRIVACYFILTER_FILTER_HPP
#define PRIVACYFILTER_FILTER_HPP

#include <String/SubString.hpp>


namespace PrivacyFilter
{
  /**
   * Shows whether or not something should be filtered.
   * @return filtering status
   */
  bool
  filter() throw ();

  /**
   * Filters messages depending on found correct key.
   * @param original_message message to return if key is found
   * @param replace_message message to return if key is not found
   * @return original_message if key is found or replace_message otherwise
   */
  const char*
  filter(const char* original_message, const char* replace_message = "")
    throw ();

  /**
   * Filters messages depending on found correct key.
   * @param original_message message to return if key is found
   * @param replace_message message to return if key is not found
   * @return original_message if key is found or replace_message otherwise
   */
  const String::SubString&
  filter(const String::SubString& original_message,
    const String::SubString& replace_message)
    throw ();
}

#endif
