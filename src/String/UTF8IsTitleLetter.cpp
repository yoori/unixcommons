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



//////////////////////////////////////////////////////////////////////////
// Static N-arc tree definition for is_letter_title property

// @file String/UTF8IsTitleLetter.cpp
#include <String/UTF8IsProperty.hpp>


namespace String
{
  namespace UnicodeProperty
  {
    static TreeNode NODE_E1 =
    {
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x1000FF00FF00FF00ull,
      0x1000000000001000ull,
    };

    TreeStartNode LETTER_TITLE_TREE =
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0004000000000920ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0x0000000000000000ull,
      0x0000000000000000ull, 0x0000000000000000ull, 0, NODE_E1, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0,
    };
  } // namespace UnicodeProperty
} // namespace String
