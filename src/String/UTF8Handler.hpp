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





#ifndef STRING_UTF8_HANDLER_HPP
#define STRING_UTF8_HANDLER_HPP

#include <cstdint>

#include <Generics/ArrayAutoPtr.hpp>


namespace String
{
  /**
   * Old UTF8 functions, if there is a possibility, you should use improved
   * methods from later namespaces. See String, UnicodeProperty, etc.
   */
  namespace UTF8Handler
  {
    /**
     * Check as soon as possible byte sequence about UTF-8 well-formed
     * sequence or ill-formed.
     * @param src string that contain checking UTF-8 byte sequence.
     * @param octets_count return sequence size in bytes for well-formed
     * sequences and zero for ill-formed.
     * @return true - well-formed, false ill-formed.
     */
    bool
    is_correct_utf8_sequence(const char* src,
      unsigned long& octets_count) throw ();

    /**
     * Iteratively checks each symbol in the string to be UTF8-valid one.
     * @param str zero terminated string to check
     * @return pointer to invalid symbol or 0
     */
    const char*
    is_correct_utf8_string(const char* str) throw ();

    unsigned long
    get_octet_count(char ch) throw ();

    bool
    utf8_char_to_wchar(const char* src, unsigned long octets_count,
      wchar_t& dest) throw ();

    bool
    wchar_to_utf8(const wchar_t* src, char* dest_buff,
      unsigned long& dest_octets_count) throw ();

    bool
    wchar_to_utf8_char(wchar_t src, char* dest_buff,
      unsigned long& dest_octets_count) throw ();

    bool
    ulong_to_utf8_char(unsigned long ul4wc, char* dest_buff,
      unsigned long& dest_octets_count) throw ();

    bool
    distance_to_sequence_beginning(const char* src,
      unsigned long& octets_count, unsigned long& distance,
      const char* limit = 0) throw ();
  }
}

///////////////////////////////////////////////////////////////////////////
// Inlines implementation
///////////////////////////////////////////////////////////////////////////
namespace String
{
  namespace UTF8Handler
  {
    inline
    bool
    is_correct_utf8_sequence(const char* src, unsigned long& octets_count)
      throw ()
    {
      const unsigned char B1 = src[0];

      octets_count = 0;
      if (B1 <= 0x7F)
      {
        octets_count = 1;
        return true;
      }
      if (B1 < 0xC2 || B1 > 0xF4)
      {
        return false;
      }
      const unsigned char B2 = src[1];
      if (B1 <= 0xDF)
      {
        if ((B2 & 0xC0) == 0x80) // B2 in [80, BF]
        {
          octets_count = 2;
          return true;
        }
        return false;
      }

      switch (B1)
      {
      case 0xE0:
        if ((B2 & 0xE0) == 0xA0) // B2 in [A0, BF]
        {
          break;
        }
        return false;
      case 0xED:
        if ((B2 & 0xE0) == 0x80) // B2 in [80, 9F]
        {
          break;
        }
        return false;
      case 0xF0:
        if (B2 >= 0x90 && B2 <= 0xBF) // B2 in [90, 9F]
        {
          break;
        }
        return false;
      case 0xF4:
        if ((B2 & 0xF0) == 0x80)  // B2 in [80, 8F]
        {
          break;
        }
        return false;

      default:
        if (B1 <= 0xEF) // B1 in [E1..EC] and [EE..EF]
        {
          if ((B2 & 0xC0) == 0x80) // B2 in [80, BF]
          {
            break;
          }
          return false;
        }
        // here we can be if B1 in [F1..F3] only
        if ((B2 & 0xC0) == 0x80) // B2 in [80, BF]
        {
          break;
        }
        return false;
      }

      // if 3 bytes sequence
      if (B1 < 0xF0)
      {
        // if next_byte in [80, BF]
        if ((static_cast<unsigned char>(src[2]) & 0xC0) != 0x80)
        {
          return false;
        }
        octets_count = 3;
        return true;
      }
      // if next_byte in [80, BF]
      if ((static_cast<unsigned char>(src[3]) & 0xC0) != 0x80)
      {
        return false;
      }
      octets_count = 4;
      return true;
    }

    inline
    const char*
    is_correct_utf8_string(const char* str) throw ()
    {
      for (unsigned long octets_count; *str; str += octets_count)
      {
        if (!is_correct_utf8_sequence(str, octets_count))
        {
          return str;
        }
      }
      return 0;
    }

    /**
     * Index into the table below with the first byte of a UTF-8 sequence to
     * get the number of trailing bytes that are supposed to follow it.
     * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
     * left as-is for anyone who may want to do such conversion, which was
     * allowed in earlier algorithms.
     */
    static const unsigned char TRAILING_BYTES_FOR_UTF8[256] =
    {
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //32
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //64
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //96
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, //128
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //160 - illegal octets
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //192 - window
      0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, //224
      3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,5,5,5,5,6,6,0,0  //256
    };

    inline
    unsigned long
    get_octet_count(char ch) throw ()
    {
      return TRAILING_BYTES_FOR_UTF8[static_cast<uint8_t>(ch)];
    }

    static const unsigned char FIRST_BYTE_MASKS[7] =
      { 0, 0x7F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };

    inline
    bool
    utf8_char_to_wchar(const char* src, unsigned long octets_count,
      wchar_t& dest) throw ()
    {
      bool result = src && (octets_count && octets_count <= 6);

      dest = 0;

      if (result)
      {
        const unsigned char CH = static_cast<unsigned char>(*src);
        dest = CH & FIRST_BYTE_MASKS[octets_count];
        ++src;

        for (unsigned int i = 0; i < octets_count - 1; ++i)
        {
          const unsigned char CH = static_cast<unsigned char>(*src);
          if ((CH & 0xC0) != 0x80)
          {
            result = false;
            break;
          }

          dest = (dest << 6) | (CH & 0x3F);
          ++src;
        }
      }

      return result;
    }

    inline
    bool
    wchar_to_utf8_char(wchar_t src, char* dest_buff,
      unsigned long& dest_octets_count) throw ()
    {
      bool result = true;
      unsigned long ul4wc = static_cast<unsigned long>(src);

      if (ul4wc < 0x80)
      {
        dest_octets_count = 1;
        *dest_buff     = ul4wc         & 0x0000007F;
      }
      else if (ul4wc < 0x800)
      {
        dest_octets_count = 2;
        *dest_buff     = ((ul4wc >> 6) & 0x0000001F) | 0x000000C0;
        *(++dest_buff) = (ul4wc        & 0x0000003F) | 0x00000080;
      }
      else if (ul4wc < 0x10000)
      {
        dest_octets_count = 3;
        *dest_buff     = ((ul4wc >> 12) & 0x0000000F) | 0x000000E0;
        *(++dest_buff) = ((ul4wc >> 6)  & 0x0000003F) | 0x00000080;
        *(++dest_buff) = (ul4wc         & 0x0000003F) | 0x00000080;
      }
      else if (ul4wc < 0x200000)
      {
        dest_octets_count = 4;
        *dest_buff     = ((ul4wc >> 18) & 0x00000007) | 0x000000F0;
        *(++dest_buff) = ((ul4wc >> 12) & 0x0000003F) | 0x00000080;
        *(++dest_buff) = ((ul4wc >> 6)  & 0x0000003F) | 0x00000080;
        *(++dest_buff) = (ul4wc         & 0x0000003F) | 0x00000080;
      }
#if 0
      else if (ul4wc < 0x4000000)
      {
        dest_octets_count = 5;
        *dest_buff     = ((ul4wc >> 24) & 0x00000003) | 0x000000F8;
        *(++dest_buff) = ((ul4wc >> 18) & 0x0000003F) | 0x00000080;
        *(++dest_buff) = ((ul4wc >> 12) & 0x0000003F) | 0x00000080;
        *(++dest_buff) = ((ul4wc >> 6)  & 0x0000003F) | 0x00000080;
        *(++dest_buff) = (ul4wc         & 0x0000003F) | 0x00000080;
      }
      else if (ul4wc < 0x80000000)
      {
        dest_octets_count = 6;
        *dest_buff     = ((ul4wc >> 30) & 0x00000001) | 0x000000FC;
        *(++dest_buff) = ((ul4wc >> 24) & 0x0000003F) | 0x00000080;
        *(++dest_buff) = ((ul4wc >> 18) & 0x0000003F) | 0x00000080;
        *(++dest_buff) = ((ul4wc >> 12) & 0x0000003F) | 0x00000080;
        *(++dest_buff) = ((ul4wc >> 6)  & 0x0000003F) | 0x00000080;
        *(++dest_buff) = (ul4wc         & 0x0000003F) | 0x00000080;
      }
#endif
      else
      {
        result = false;
      }

      return result;
    }

    inline
    bool
    ulong_to_utf8_char(unsigned long ul4wc, char* dest_buff,
      unsigned long& dest_octets_count) throw ()
    {
      bool result(true);
      if (ul4wc < 0x80)
      {
        dest_octets_count = 1;
        *dest_buff     = ul4wc         & 0x0000007F;
      }
      else if (ul4wc < 0x800)
      {
        dest_octets_count = 2;
        *dest_buff     = ((ul4wc >> 6) & 0x0000001F) | 0x000000C0;
        *(++dest_buff) = (ul4wc        & 0x0000003F) | 0x00000080;
      }
      else if (ul4wc < 0x10000)
      {
        dest_octets_count = 3;
        *dest_buff     = ((ul4wc >> 12) & 0x0000000F) | 0x000000E0;
        *(++dest_buff) = ((ul4wc >> 6)  & 0x0000003F) | 0x00000080;
        *(++dest_buff) = (ul4wc         & 0x0000003F) | 0x00000080;
      }
      else if (ul4wc < 0x200000)
      {
        dest_octets_count = 4;
        *dest_buff     = ((ul4wc >> 18) & 0x00000007) | 0x000000F0;
        *(++dest_buff) = ((ul4wc >> 12) & 0x0000003F) | 0x00000080;
        *(++dest_buff) = ((ul4wc >> 6)  & 0x0000003F) | 0x00000080;
        *(++dest_buff) = (ul4wc         & 0x0000003F) | 0x00000080;
      }
      else if (ul4wc < 0x4000000)
      {
        dest_octets_count = 5;
        *dest_buff     = ((ul4wc >> 24) & 0x00000003) | 0x000000F8;
        *(++dest_buff) = ((ul4wc >> 18) & 0x0000003F) | 0x00000080;
        *(++dest_buff) = ((ul4wc >> 12) & 0x0000003F) | 0x00000080;
        *(++dest_buff) = ((ul4wc >> 6)  & 0x0000003F) | 0x00000080;
        *(++dest_buff) = (ul4wc         & 0x0000003F) | 0x00000080;
      }
      else if (ul4wc < 0x80000000)
      {
        dest_octets_count = 6;
        *dest_buff     = ((ul4wc >> 30) & 0x00000001) | 0x000000FC;
        *(++dest_buff) = ((ul4wc >> 24) & 0x0000003F) | 0x00000080;
        *(++dest_buff) = ((ul4wc >> 18) & 0x0000003F) | 0x00000080;
        *(++dest_buff) = ((ul4wc >> 12) & 0x0000003F) | 0x00000080;
        *(++dest_buff) = ((ul4wc >> 6)  & 0x0000003F) | 0x00000080;
        *(++dest_buff) = (ul4wc         & 0x0000003F) | 0x00000080;
      }
      else
      {
        result = false;
      }

      return result;
    }

    inline
    bool
    distance_to_sequence_beginning(const char* src,
      unsigned long& octets_count, unsigned long& distance,
      const char* limit) throw ()
    {
      if (!(static_cast<unsigned char>(*src) & 0x80))
      {
        octets_count = 1;
        distance = 0;
        return true;
      }

      const char* p = src;

      while ((static_cast<unsigned char>(*p) & 0xC0) == 0x80)
      {
        if (p == limit)
        {
          return false;
        }

        --p;
      }

      distance = src - p;
      octets_count = get_octet_count(*p);
      if (!octets_count || octets_count <= distance)
      {
        return false;
      }

      return true;
    }
  }
}

#endif
