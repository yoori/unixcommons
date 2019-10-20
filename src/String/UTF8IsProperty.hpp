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



// @file String/UTF8IsProperty.hpp
#ifndef UTF8_IS_PROPERTY_HPP
#define UTF8_IS_PROPERTY_HPP

#include <String/UTF8NArcTree.hpp>


namespace String
{
  /**
   * Check that UTF-8 encoded string contain space Unicode character,
   * buffer overrun IS NOT checked.
   * @param str input string
   * @return true for space Unicode symbols.
   */
  bool
  is_space(const char* str) throw ();

  /**
   * Check that UTF-8 encoded string contain digit Unicode character,
   * buffer overrun IS NOT checked.
   * @param str input string
   * @return true for digit Unicode symbols.
   */
  bool
  is_digit(const char* str) throw ();

  /**
   * Check that UTF-8 encoded string contain letter Unicode character,
   * buffer overrun IS NOT checked.
   * @param str input string
   * @return true for letter Unicode symbols.
   */
  bool
  is_letter(const char* str) throw ();

  /**
   * Check that UTF-8 encoded string contain lower letter Unicode character,
   * buffer overrun IS NOT checked.
   * @param str input string
   * @return true for lower letter Unicode symbols.
   */
  bool
  is_lower_letter(const char* str) throw ();

  /**
   * Check that UTF-8 encoded string contain title letter Unicode character,
   * buffer overrun IS NOT checked.
   * @param str input string
   * @return true for title letter Unicode symbols.
   */
  bool
  is_title_letter(const char* str) throw ();

  /**
   * Check that UTF-8 encoded string contain upper letter Unicode character,
   * buffer overrun IS NOT checked.
   * @param str input string
   * @return true for upper letter Unicode symbols.
   */
  bool
  is_upper_letter(const char* str) throw ();
} // namespace String

//////////////////////////////////////////////////////////////////////////
//        Implementation
//////////////////////////////////////////////////////////////////////////

namespace String
{
  namespace UnicodeProperty
  {
    extern const TreeStartNode SPACE_TREE;
    extern const TreeStartNode DIGIT_TREE;
    extern const TreeStartNode LETTER_TREE;
    extern const TreeStartNode LETTER_LOWER_TREE;
    extern const TreeStartNode LETTER_TITLE_TREE;
    extern const TreeStartNode LETTER_UPPER_TREE;
    extern const TreeStartNode BIDI_L_TREE;
    extern const TreeStartNode BIDI_RAL_TREE;
  } // namespace UnicodeProperty

  inline
  bool
  is_space(const char* str) throw ()
  {
    return UnicodeProperty::belong(UnicodeProperty::SPACE_TREE, str);
  }

  inline
  bool
  is_digit(const char* str) throw ()
  {
    return UnicodeProperty::belong(UnicodeProperty::DIGIT_TREE, str);
  }

  inline
  bool
  is_letter(const char* str) throw ()
  {
    return UnicodeProperty::belong(UnicodeProperty::LETTER_TREE, str);
  }

  inline
  bool
  is_lower_letter(const char* str) throw ()
  {
    return UnicodeProperty::belong(UnicodeProperty::LETTER_LOWER_TREE, str);
  }

  inline
  bool
  is_title_letter(const char* str) throw ()
  {
    return UnicodeProperty::belong(UnicodeProperty::LETTER_TITLE_TREE, str);
  }

  inline
  bool
  is_upper_letter(const char* str) throw ()
  {
    return UnicodeProperty::belong(UnicodeProperty::LETTER_UPPER_TREE, str);
  }
} // namespace String

#endif
