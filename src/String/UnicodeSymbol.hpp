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



// @file String/UnicodeSymbol.hpp
#ifndef STRING_UNICODE_SYMBOL_HPP
#define STRING_UNICODE_SYMBOL_HPP

#include <istream>
#include <ostream>

#include <String/UTF8Handler.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>


namespace String
{
  /**
   * Container for Unicode symbol that belong UTF-8 subset.
   * Representation with get next well-formed utf8 octet ability.
   * Internal stored Unicode unit code.
   * Legal range [U+0000, U+10FFFF].
   */

  class UnicodeSymbol
  {
  public:
    /**
     * Exception raised when Unicode security recommendations
     * isn't satisfied.
     * See: Unicode Technical Report 36, p 3.6
     * http://www.unicode.org/reports/tr36/#UTF-8_Exploit
     */
    DECLARE_EXCEPTION(RangeException, eh::DescriptiveException);

    /**
     * Default constructor
     * Construct UnicodeSymbol that doesn't contain any valid
     * Unicode code unit. (Null state)
     */
    UnicodeSymbol() throw ();

    /**
     * Copy constructor
     * @param symbol source object for construction
     */
    UnicodeSymbol(const UnicodeSymbol& symbol) throw ();

    /**
     * Constructor
     * @param byte_sequence should be UTF-8 well-formed byte sequence.
     * Translate it into Unicode code unit and construct
     * corresponding UnicodeSymbol.
     */
    UnicodeSymbol(const char* byte_sequence) throw (RangeException);

    /**
     * Constructor
     * @param code_unit construct from Unicode unit code
     * corresponding UnicodeSymbol.
     */
    UnicodeSymbol(wchar_t code_unit) throw (RangeException);

    /**
     * Prefix increment operator.
     * Maximum value U+10FFFF never overflow. Illegal code units over jump.
     * Raise exception when call on Null UnicodeSymbol.
     */
    UnicodeSymbol&
    operator ++() throw (RangeException);

    /**
     * Prefix decrement operator.
     * Illegal code units over jump.
     * Raise exception when call on Null UnicodeSymbol.
     */
    UnicodeSymbol&
    operator --() throw (RangeException);

    /**
     * This type conversion operator allow comparison of
     * UnicodeSymbol objects.
     */
    operator wchar_t() const throw ();

    /**
     * Assignment operator.
     * @param new_value new Unicode code point value.
     * @return object with new code point.
     */
    UnicodeSymbol&
    operator =(wchar_t new_value)
      throw (RangeException, eh::Exception);

    /**
     * @return length of corresponding UTF-8 byte sequence.
     */
    size_t
    length() const throw (RangeException);

    /**
     * @return true if isn't stored any valid code unit.
     */
    bool
    is_null() const throw ();

    /**
     * After call UnicodeSymbol doesn't contain any legal
     * Unicode code unit.
     */
    void
    set_null() throw ();

    /**
     * Convert UnicodeSymbol into UTF-8 byte sequence and
     * @return pointer to internal buffer with UTF-8 byte
     * sequence.
     */
    const char*
    c_str() const throw (RangeException);

    /**
     * The same that c_str, but return unsigned char* pointer. Need it
     * to avoid reinterpret_cast and more usability code.
     * @return pointer to internal buffer with UTF-8 byte
     * sequence.
     */
    const unsigned char*
    c_ustr() const throw (RangeException);

    /**
     * Manipulator for std streams input/output
     * <<(>>) UnicodeSymbol::binary, switch
     * mode into raw UTF-8 byte sequences put/get.
     * Sample out U+005A after binary put: Z
     * @param iosbase stream to manipulate
     * @return manipulated stream
     */
    static
    std::ios_base&
    binary(std::ios_base& iosbase) throw ();

    /**
     * Manipulator for std streams input/output
     * <<(>>) UnicodeSymbol::nobinary, switch to
     * text mode for UTF-8 byte sequences put/get.
     * Sample out U+0080 after nobinary put: c2.80
     * Text mode is by default.
     * @param iosbase stream to manipulate
     * @return manipulated stream
     */
    static
    std::ios_base&
    nobinary(std::ios_base& iosbase) throw ();

    /**
     * @return true if set binary output mode for UnicodeSymbol's
     * in iosbase stream. false is text mode established.
     */
    static
    bool
    get_out(std::ios_base& iosbase) throw ();

    /**
     * @param iosbase stream to set UnicodeSymbols in/out mode.
     * @param binary_output_mode true - binary mode, false - text mode.
     */
    static
    void
    set_out(std::ios_base& iosbase, bool binary_output_mode) throw ();

    /**
     * Random well-formed UnicodeSymbol generator.
     * @return any well-formed UnicodeSymbol.
     */
    static
    UnicodeSymbol
    random() throw ();

  private:
    /**
     * Checks if a symbol in the allowed Unicode range
     * @param value code unit to check
     * @return if a simbol suits the standard
     */
    static
    bool
    check_validity_(wchar_t value) throw ();

  public:
    /// Maximum legal Unicode code unit value
    static const wchar_t MAX_CODE_UNIT = 0x10FFFF;
  private:
    static const wchar_t NULL_CODE_UNIT_ = ~L'\0';

    static int output_format_index_;

    /// Value of Unicode Symbol.
    wchar_t code_unit_;

    /// Internal buffer for UTF-8 byte sequence conversion.
    mutable char as_text_[5];
  };

  /**
   * Put UTF-8 byte sequence into stream
   */
  std::ostream&
  operator <<(std::ostream &os, const UnicodeSymbol& u)
    throw ();

  /**
   * Get UTF-8 byte sequence from stream, and put it into internal
   * representation Unicode code unit.
   */
  std::istream&
  operator >>(std::istream &is, UnicodeSymbol &u)
    throw ();
} // namespace String

//////////////////////////////////////////////////////////////////////////
//  Implementations inline
//////////////////////////////////////////////////////////////////////////

namespace String
{
  //
  //  class UnicodeSymbol
  //

  inline
  bool
  UnicodeSymbol::check_validity_(wchar_t value) throw ()
  {
    // Surrogates are not symbols
    return (value >= 0 && value <= 0xD7FF) ||
      (value >= 0xE000 && value <= MAX_CODE_UNIT);
  }

  inline
  UnicodeSymbol::UnicodeSymbol() throw ()
    : code_unit_(NULL_CODE_UNIT_)
  {
  }

  inline
  UnicodeSymbol::UnicodeSymbol(const UnicodeSymbol& symbol) throw ()
    : code_unit_(symbol.code_unit_)
  {
  }

  inline
  UnicodeSymbol::UnicodeSymbol(wchar_t code_unit) throw (RangeException)
    : code_unit_(code_unit)
  {
    if (!check_validity_(code_unit))
    {
      Stream::Error ost;
      ost << FNS << std::hex << std::uppercase <<
        static_cast<unsigned long>(code_unit_) << "): out of range";
      throw RangeException(ost);
    }
  }

  inline
  UnicodeSymbol::UnicodeSymbol(const char* byte_sequence)
    throw (RangeException)
  {
    unsigned long count;
    wchar_t wchar;
    if (UTF8Handler::is_correct_utf8_sequence(byte_sequence, count) &&
      UTF8Handler::utf8_char_to_wchar(byte_sequence, count, wchar))
    {
      code_unit_ = wchar;
    }
    else
    {
      Stream::Error ostr;
      ostr << FNS << "out of range";
      throw RangeException(ostr);
    }
  }

  inline
  bool
  UnicodeSymbol::get_out(std::ios_base& iosbase) throw ()
  {
    return iosbase.iword(output_format_index_) != 0;
  }

  inline
  void
  UnicodeSymbol::set_out(std::ios_base& iosbase, bool binary_output_mode)
    throw ()
  {
    iosbase.iword(output_format_index_) = binary_output_mode;
  }

  inline
  std::ios_base&
  UnicodeSymbol::binary(std::ios_base& iosbase) throw ()
  {
    set_out(iosbase, true);
    return iosbase;
  }

  inline
  std::ios_base&
  UnicodeSymbol::nobinary(std::ios_base& iosbase) throw ()
  {
    set_out(iosbase, false);
    return iosbase;
  }

  inline
  size_t
  UnicodeSymbol::length() const throw (RangeException)
  {
    if (code_unit_ <= 0x7F)
    {
      return 1;
    }
    if (code_unit_ <= 0x07FF)
    {
      return 2;
    }
    if (code_unit_ <= 0xFFFF)
    {
      return 3;
    }
    if (code_unit_ <= 0x10FFFF)
    {
      return 4;
    }
  // Unicode Security recommend never go out 4-bytes UTF8 sequences.
    Stream::Error ostr;
    ostr << FNS << "Security warning, UTF-8 length overflow";
    throw RangeException(ostr);
  }

  inline
  bool
  UnicodeSymbol::is_null() const throw ()
  {
    return code_unit_ == NULL_CODE_UNIT_;
  }

  inline
  void
  UnicodeSymbol::set_null() throw ()
  {
    code_unit_ = NULL_CODE_UNIT_;
  }

  inline
  const char*
  UnicodeSymbol::c_str() const throw (RangeException)
  {
    if (is_null())
    {
      Stream::Error ostr;
      ostr << FNS << "out of range";
      throw RangeException(ostr);
    }
    else
    {
      unsigned long octets_count = 0;
      UTF8Handler::wchar_to_utf8_char(code_unit_,
        as_text_, octets_count);
      as_text_[octets_count] = 0;
    }
    return as_text_;
  }

  inline
  const unsigned char*
  UnicodeSymbol::c_ustr() const throw (RangeException)
  {
    return reinterpret_cast<const unsigned char*>(c_str());
  }

  inline
  UnicodeSymbol&
  UnicodeSymbol::operator ++() throw (RangeException)
  {
    if (!check_validity_(code_unit_) || is_null())
    {
      Stream::Error ost;
      ost << FNS << code_unit_ << "out of range";
      throw RangeException(ost);
    }

    if (code_unit_ <= MAX_CODE_UNIT)
    {
      if (code_unit_ == 0xD7FF)
      {
        code_unit_ = 0xE000;
        return *this;
      }
      ++code_unit_;
    }
    return *this;
  }

  inline
  UnicodeSymbol&
  UnicodeSymbol::operator --() throw (RangeException)
  {
    if (!check_validity_(code_unit_) || code_unit_ == 0 || is_null())
    {
      Stream::Error ostr;
      ostr << FNS << "out of range";
      throw RangeException(ostr);
    }
    if (code_unit_ > 0)
    {
      if (code_unit_ == 0xE000)
      {
        code_unit_ = 0xD7FF;
        return *this;
      }
      --code_unit_;
    }
    return *this;
  }

  inline
  UnicodeSymbol::operator wchar_t() const throw ()
  {
    return code_unit_;
  }
} // namespace String

#endif
