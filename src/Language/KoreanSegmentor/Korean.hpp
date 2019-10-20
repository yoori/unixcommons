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



#ifndef LANGUAGE_KOREAN_SEGMENTOR_KOREAN_HPP
#define LANGUAGE_KOREAN_SEGMENTOR_KOREAN_HPP

#include <String/StringManip.hpp>
#include <String/UTF8Handler.hpp>
#include <String/UTF8Category.hpp>


namespace Language
{
  namespace Segmentor
  {
    namespace Korean
    {
      typedef const String::StringManip::InverseCategory<
        String::Utf8Category> NotHangul;
      extern NotHangul NOT_HANGUL;
    }
  }
}

#endif
