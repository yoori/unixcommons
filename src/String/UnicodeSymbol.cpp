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



// UnicodeSymbol.cpp
// Class UnicodeSymbol implementation.

#include <iostream>

#include <String/AsciiStringManip.hpp>
#include <String/UnicodeSymbol.hpp>

#include <Generics/Rand.hpp>

#include <Stream/FlagsSaver.hpp>


namespace String
{
  //
  //  class UnicodeSymbol
  //

  const wchar_t UnicodeSymbol::MAX_CODE_UNIT;
  const wchar_t UnicodeSymbol::NULL_CODE_UNIT_;
  int UnicodeSymbol::output_format_index_ = std::ios_base::xalloc();

  UnicodeSymbol
  UnicodeSymbol::random() throw ()
  {
    wchar_t value;
    do
    {
      value = Generics::safe_integral_rand(21);
    }
    while (!check_validity_(value));
    return UnicodeSymbol(value);
  }

  UnicodeSymbol&
  UnicodeSymbol::operator =(wchar_t new_value)
    throw (RangeException, eh::Exception)
  {
    if (!check_validity_(new_value))
    {
      Stream::Error ostr;
      ostr << FNS << static_cast<unsigned long>(new_value) <<
        "is out of UTF8 range";
      throw RangeException(ostr);
    }
    code_unit_ = new_value;
    return *this;
  }

  //
  //  Stream i/o
  //

  /**
   *  Put UTF-8 byte sequence into stream
   */
  std::ostream&
  operator <<(std::ostream &os, const UnicodeSymbol& u) throw ()
  {
    std::ostream::sentry ok(os);
    if (ok)
    {
      try
      {
        bool binary_out = UnicodeSymbol::get_out(os);
        if (binary_out)
        {
          if (u.is_null())
          {
            return os;
          }
          os << u.c_str();
        }
        else
        {
          if (u.is_null())
          {
            os << "null";
            return os;
          }
          Stream::FlagsSaver flags_saver(os);
          os << std::hex << std::right;
          const unsigned char* p = u.c_ustr();
          os.width(2);
          os.fill('0');
          os << static_cast<int>(*p++);
          for (; *p != 0; ++p)
          {
            os << '.' << static_cast<int>(*p);
          }
        }
      }
      catch (...)
      {
        os.setstate(std::ios_base::failbit);
      }
    }
    return os;
  }

  /**
   *  Get UTF-8 byte sequence from stream, and put it into internal
   *  representation Unicode code unit.
   */
  std::istream&
  operator >>(std::istream &is, UnicodeSymbol &u) throw ()
  {
    std::istream::sentry ok(is);
    if (ok)
    {
      bool binary_out = UnicodeSymbol::get_out(is);
      wchar_t code_unit;
      std::string utf8_sequence;  // buffer for binary string
                                  // converted from text view.
      unsigned long utf8_sequence_length = 0;
      char c = 0;
      is.get(c);

      if (!binary_out)
      {
        // TEXTUAL REPRESENTATION, example: C2.B9, ...
        // 1. Space symbols (tabs, spaces) scroll out before significant
        // text begin.
        // 2. Read first byte of sequence and use it to understanding
        // how many bytes we will read, while end of UTF-8 sequence reached.

        for (; is && AsciiStringManip::SPACE(c); is.get(c))
        {
        }

        if (!AsciiStringManip::HEX_NUMBER(c))
        {
          is.setstate(std::ios_base::failbit);
          return is;
        }
        char byte_text_view[3];
        byte_text_view[2] = 0;

        byte_text_view[0] = c;
        is.get(c);
        if (!AsciiStringManip::HEX_NUMBER(c))
        {
          is.setstate(std::ios_base::failbit);
          return is;
        }
        byte_text_view[1] = c;

        utf8_sequence += strtol(byte_text_view, 0, 16);
        utf8_sequence_length =
          UTF8Handler::get_octet_count(utf8_sequence[0]);

        for (std::size_t end_sequence = utf8_sequence_length;
          is && (end_sequence > 1); --end_sequence)
        {
          is.get(c);
          if (c != '.')
          {
            is.setstate(std::ios_base::failbit);
            return is;
          }
          is.get(c);
          if (!AsciiStringManip::HEX_NUMBER(c))
          {
            is.setstate(std::ios_base::failbit);
            return is;
          }
          byte_text_view[0] = c;
          is.get(c);
          if (!AsciiStringManip::HEX_NUMBER(c))
          {
            is.setstate(std::ios_base::failbit);
            return is;
          }
          byte_text_view[1] = c;
          utf8_sequence += strtol(byte_text_view, 0, 16);
        }
      }
      else
      {
        // BINARY - RAW string
        // Much the same logic that in Text mode reading,
        // 1. Read byte
        // 2. Become clear how many bytes we should read until end of UTF-8
        // sequence reached, and do this.

        utf8_sequence_length = UTF8Handler::get_octet_count(c);
        std::size_t end_sequence = utf8_sequence_length;
        if (!utf8_sequence_length)  // ill-formed UTF-8 sequence
        {
          is.setstate(std::ios_base::failbit);
          return is;
        }

        while (is)
        {
          utf8_sequence += c;
          if (--end_sequence == 0)
          {
            break;
          }
          is.get(c);
        }
        if (!is)
        {
          return is;
        }
      }

      // realize is >> src;
      UTF8Handler::is_correct_utf8_sequence(utf8_sequence.c_str(),
        utf8_sequence_length);

      if (!UTF8Handler::utf8_char_to_wchar(
        utf8_sequence.c_str(), utf8_sequence_length, code_unit))
      {
        is.setstate(std::ios_base::failbit);
        return is;
      }

      u = code_unit;
    }
    return is;
  }
} // namespace String
