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



#include "Korean.hpp"

namespace Language
{
  namespace Segmentor
  {
    namespace Korean
    {
      const char HANGUL_RANGES[] =
        "\xE1\x84\x80-\xE1\x87\xB9" // U+1100-U+11F9
        "\xE3\x80\xAE-\xE3\x80\xAF" // U+302E-U+302F
        "\xE3\x84\xB1-\xE3\x86\x8E" // U+3131-U+318E
        "\xE3\x88\x80-\xE3\x88\x9E" // U+3200-U+321E
        "\xE3\x89\xA0-\xE3\x89\xBF" // U+3260-U+327F
        "\xEA\xB0\x80-\xED\x9E\xA3" // U+AC00-U+D7A3
        "\xEF\xBE\xA0-\xEF\xBF\x9C" // U+FFA0-U+FFDC
        ;

      NotHangul NOT_HANGUL(HANGUL_RANGES);
    }
  }
}

