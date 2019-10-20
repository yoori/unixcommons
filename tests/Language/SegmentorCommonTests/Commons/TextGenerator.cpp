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




#include "TextGenerator.hpp"
#include <String/UTF8Handler.hpp>
#include <String/UTF8Category.hpp>
#include <Generics/Rand.hpp>
#include <iostream>

namespace SegmentorTestCommons
{

 namespace
 {
   /**
    * Start octets for utf8 sequence
    */
   static const unsigned char uf8_char_begins[] = {0x00, 0x00, 
                                                   0xc0, 0xe0, 
                                                   0xf0, 0xf8, 
                                                   0xfc, 0xfe};
 }
  
  ////// class Utf8Generator
  
  size_t
  Utf8Generator::gen_rand_utf8_sequence(char* buf,
                                        size_t max_sequence_len,
                                        bool valid_only)
    throw()
  {
    size_t i = 0;
    while (i + (valid_only? 4: 6) < max_sequence_len)
    {
      unsigned long length = 0;
      uint32_t unicode_val = 0;
      if (valid_only)
      {
        switch (Generics::safe_rand(0, 3))
        {
        case 0:
          {
            unicode_val = Generics::safe_rand(0, B_TOP_1BYTE);
            break;
          }
        case 1:
          {
            unicode_val = Generics::safe_rand(B_TOP_1BYTE + 1, B_TOP_2BYTES);
            break;
          }
        case 2:
          {
            do
            {
              unicode_val = Generics::safe_rand(B_TOP_2BYTES + 1, B_TOP_3BYTES);
            }
            while (unicode_val >= B_GAP_3BYTES_BOTTOM &&
                   unicode_val <= B_GAP_3BYTES_TOP);
            break;
          }
        default:
          {
            unicode_val = Generics::safe_rand(B_TOP_3BYTES + 1,
              B_GAP_4BYTES_BOTTOM - 1);
          }
        }//switch (Generics::safe_rand(0, 3))
      }
      else
      {
        unicode_val = Generics::safe_rand(0, B_TOP_6BYTES);
      }

      if (!String::UTF8Handler::ulong_to_utf8_char(unicode_val, buf + i, length))
      {
        std::cerr << "Utf8Generator::gen_rand_utf8_sequence: "
                  << unicode_val << " is incorrect unicode value."
                  << std::endl;
      }

      i += length;
    } //while (i + (valid_only? 4: 6) < max_sequence_len)

    return i;
  }

  ////// class AsciiGenerator

  void
  AsciiGenerator::gen_rand_ascii_sequence(char* buf, size_t buf_len)
    throw ()
  {
    for (size_t i = 0; i < buf_len; ++i)
    {
      buf[i] = Generics::safe_rand(0, 256);
    }
  }

  ////// String dumper

  void
  hex_dump (std::ostream &os, const char* str, size_t size)
    throw ()
  {
    if(str == 0 || size == 0)
    {
      return;
    }
    os << std::hex << std::right;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(str);
    const unsigned char* e = p + size;
    os.width(2);
    os.fill('0');
    os << static_cast<int>(*p++);
    for (; p != e; ++p)
    {
      os << '.' << static_cast<int>(*p);
    }
  }

  ////// class Utf8CharWalker

  void
  Utf8CharWalker::setup_ (size_t octets)
    throw ()
  {
    octets_ = octets; 
    if (octets_ > 4)
    {
      octets_ = 4;
    }
    char str[octets_];
    str[0] = static_cast<char>(uf8_char_begins[octets_]);
    for (size_t i = 1; i < octets_; ++i)
    {
      str[i] = 0x80;
    }
    str[octets_] = 0x00;
    sym_ = String::UnicodeSymbol(str);
  }
  
  const char* 
  Utf8CharWalker::next()
    throw ()
  {
    if (String::UnicodeSymbol::MAX_CODE_UNIT > sym_)
    {
      ++sym_;
      size_t octets = sym_.length();
      const char* ret = sym_.c_str();
      if (octets != octets_)
      {// we are moved to next size char
        octets_ = octets;
        return 0;
      }
      return ret;
    }
    else
    {// we are under overflow
      octets_ = 5;
      return 0;
    }
  }

  ////// class PseudoUtf8CharWalker

  void
  PseudoUtf8CharWalker::setup_(size_t octets)
    throw ()
  {
    octets_ = octets; 
    if (octets_ > 6)
    {
      octets_ = 6;
    }
    data_[0] = uf8_char_begins[octets_];
    for (size_t i = 1; i < octets_; ++i)
    {
      data_[i] = 0x80;
    }
    data_[octets_] = 0x00;    
  }

  const char* 
  PseudoUtf8CharWalker::next ()
    throw ()
  {
    if (octets_ > 6)
    {
      return 0;
    }
    for (unsigned char* last = data_ + octets_ - 1; 
         last != data_; *(last--) = 0x80)
    {
      if ((*last += 1) <= 0xbf)
      {
        return reinterpret_cast<const char*>(data_);
      }
    }
    unsigned char overflow = octets_ > 1 ? uf8_char_begins[octets_+1] : 0x80;
    if ((data_[0] += 1) < overflow)
    {
      return reinterpret_cast<const char*>(data_);
    }
    setup_(octets_+1);
    return 0;
  }
} //namespace SegmentorTestCommons
