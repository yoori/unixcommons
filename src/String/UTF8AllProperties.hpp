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



// @file String/UTF8AllProperties.hpp
#ifndef STRING_UTF8_ALL_PROPERTIES_HPP
#define STRING_UTF8_ALL_PROPERTIES_HPP

#include <String/UTF8NArcTree.hpp>


namespace String
{
  /**
   * Object of AllProperties
   */
  class AllProperties
  {
  public:
    /**
     * Constructor get value encoded properties traits
     */
    AllProperties(uint8_t value) throw ();

    /**
     * @return true if object store is_space info
     */
    bool
    is_space() const throw ();

    /**
     * @return true if object store is_digit info
     */
    bool
    is_digit() const throw ();

    /**
     * @return true if object store is_letter info
     */
    bool
    is_letter() const throw ();

    /**
     * @return true if object store is_lower_letter info
     */
    bool
    is_lower_letter() const throw ();

    /**
     * @return true if object store is_title_letter info
     */
    bool
    is_title_letter() const throw ();

    /**
     * @return true if object store is_upper_letter info
     */
    bool
    is_upper_letter() const throw ();

  protected:
    uint8_t cumulative_value_;
  };

  /**
   * Function compute Unicode properties traits -
   * value of all properties in one mask value
   * @param str input data with checking character, UTF-8 encoded
   * @return Value contain bits mask with information about
   * Unicode properties for UTF-8 byte sequence from str
   */
  AllProperties
  all_properties(const char* str) throw ();

  namespace UnicodeProperty
  {
    /**
     * Enum use to return information about all properties for
     * some code unit
     */
    enum CODE_UNIT_PROPERTY
    {
      CUP_SPACE = 0x01,
      CUP_DIGIT = 0x02,
      CUP_LETTER = 0x04,
      CUP_LOWER_LETTER = 0x08,
      CUP_TITLE_LETTER = 0x10,
      CUP_UPPER_LETTER = 0x20,
    };

    typedef const uint8_t AllTreeLeaf[64];
    typedef const void* const AllTreeStartNode[128];
    typedef const void* const AllTreeNode[64];

    extern AllTreeStartNode ALL_PROPERTIES_TREE;
    extern const uint8_t ALL_PROPERTIES_READY_VALUES[0x80];
  } // namespace UnicodeProperty
} // namespace String

//
// INLINES
//

namespace String
{
  inline
  AllProperties::AllProperties(uint8_t value) throw ()
    : cumulative_value_(value)
  {
  }

  inline
  bool
  AllProperties::is_space() const throw ()
  {
    return cumulative_value_ & UnicodeProperty::CUP_SPACE;
  }

  inline
  bool
  AllProperties::is_digit() const throw ()
  {
    return cumulative_value_ & UnicodeProperty::CUP_DIGIT;
  }

  inline
  bool
  AllProperties::is_letter() const throw ()
  {
    return cumulative_value_ & UnicodeProperty::CUP_LETTER;
  }

  inline
  bool
  AllProperties::is_lower_letter() const throw ()
  {
    return cumulative_value_ & UnicodeProperty::CUP_LOWER_LETTER;
  }

  inline
  bool
  AllProperties::is_title_letter() const throw ()
  {
    return cumulative_value_ & UnicodeProperty::CUP_TITLE_LETTER;
  }

  inline
  bool
  AllProperties::is_upper_letter() const throw ()
  {
    return cumulative_value_ & UnicodeProperty::CUP_UPPER_LETTER;
  }

  inline
  AllProperties
  all_properties(const char* str) throw ()
  {
    if (static_cast<uint8_t>(*str) < 0x80)
    {
      return UnicodeProperty::ALL_PROPERTIES_READY_VALUES[
        static_cast<uint8_t>(*str)];
    }
    const void* current_tree = UnicodeProperty::ALL_PROPERTIES_TREE[
      static_cast<uint8_t>(*str) - 0x80];
    for (unsigned long depth = UTF8Handler::get_octet_count(*str);;)
    {
      if (current_tree == &UnicodeProperty::TREE_STOP)
      {
        return UnicodeProperty::CUP_LETTER;
      }
      if (!current_tree)
      {
        return 0;
      }
      if (--depth < 2)
      {
        break;
      }
      current_tree = (*static_cast<const UnicodeProperty::AllTreeNode*>
        (current_tree))[static_cast<uint8_t>(*++str) & 0x3F];
    }
    return (*static_cast<UnicodeProperty::AllTreeLeaf*>(current_tree))
      [static_cast<uint8_t>(*++str) & 0x3F];
  }
}

#endif // STRING_UTF8_ALL_PROPERTIES_HPP
