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




#ifndef LANGUAGE_SEGMENTOR_COMMON_TESTS_COMMONS_TEXT_GENERATOR_HPP
#define LANGUAGE_SEGMENTOR_COMMON_TESTS_COMMONS_TEXT_GENERATOR_HPP

#include <cstddef>
#include <String/UnicodeSymbol.hpp>
#include <Generics/Uncopyable.hpp>

namespace SegmentorTestCommons
{

  /**
   * class Utf8Generator
   * perfomance test
   */
  class Utf8Generator
  {
  public:
    
    /**
     * Generate random UTF8 chars sequence in supplied buffer
     * @param buf is buffer for writing utf8 sequence into it
     * @param max_sequence_len is buffer length
     * @return utf8 sequence length
     */
    static
    size_t
    gen_rand_utf8_sequence(char* buf, size_t max_sequence_len,
                           bool valid_only = true) throw ();
    
    /**
     * Borders for unicode with different UTF8 char length
     */
    enum Borders
    {
      B_TOP_1BYTE = 0x0000007F,
      B_TOP_2BYTES = 0x000007FF,
      B_GAP_3BYTES_BOTTOM = 0x0000D800,
      B_GAP_3BYTES_TOP = 0x0000DFFF,
      B_TOP_3BYTES = 0x0000FFFF,
      B_GAP_4BYTES_BOTTOM = 0x00110000,
      B_TOP_4BYTES = 0x001FFFFF,
      B_TOP_5BYTES = 0x03FFFFFF,
      B_TOP_6BYTES = 0x7FFFFFFF
    };

  };

  /**
   * class AsciiGenerator
   */
  class AsciiGenerator
  {
  public:
    
    /**
     * Generate random ascii string
     * @param buf is buffer for writing ascii sequence into it
     * @param buf_len is buffer length
     */
    static
    void
    gen_rand_ascii_sequence(char* buf, size_t buf_len) throw ();
  };

  /**
   * Dump hex representation of string 
   * @param os is output stream
   * @param str is sting to dump
   * @param size is this size
   */
  void
  hex_dump (std::ostream &os, const char* str, size_t size) throw ();

  /**
   * Provide iteration over valid utf8 chars and not more 4 octets for char
   */
  class Utf8CharWalker : private Generics::Uncopyable
                         //TODO: if you need copy constructor - realize it
  {
  protected:

    String::UnicodeSymbol sym_;
    size_t octets_;

    /**
     * Value constructor
     * create walker for octets size char but always not more 4 octets
     * @param octets number of octets of this new utf8 char
     */
    void
    setup_(size_t octets) throw (); 

  public:

    /**
     * Default constructor.
     * create walker for octets size char but always not more 4 octets
     * @param octets number of octets of this new utf8 char
     */
    explicit Utf8CharWalker(size_t octets) throw ();

    /**
     * Accessor for number of octets.
     * @return number of octets of this char
     * special value 5 is overflow number of octets
     */
    size_t
    octets() const throw ();

    /**
     * Interpret this char as C string.
     * @return C string interpretation of this char.
     */
    operator const char*() const throw ();

    /**
     * Accessor to indexed octet of this char.
     * @param index is index of requested octet
     * @return indexed octet of this char.
     */
    unsigned char
    operator [](size_t index) const throw ();

    /**
     * Unicode code of this char.
     * @return unicode code of this char.
     */
    unsigned long
    code() const throw ();

    /**
     * Move to next char.
     * @return 0 in case - number of octets changed,
     * or if we are under overflow.
     * in all other cases return C string interpretation of this char.
     */
    const char*
    next() throw ();

    /**
     * Dump hex representation of symbol
     * @param os is output stream
     */
    void
    dump (std::ostream &os) const throw ();
  };

  /**
   * Provide iteration over all possible (may be invalid) utf8 chars
   * but not more 6 octets for char
   */
  class PseudoUtf8CharWalker : private Generics::Uncopyable
                               //TODO: if you need copy constructor - realize it
  {
  protected:

    unsigned char data_[8];
    size_t        octets_;

    /**
     * Value constructor
     * create walker for octets size char but always not more 6 octets
     * @param octets number of octets of this new pseudo utf8 char
     */
    void
    setup_(size_t octets) throw (); 

  public:

    /**
     * Default constructor.
     * create walker for octets size char but always not more 6 octets
     * @param octets number of octets of this new pseudo utf8 char
     */
    explicit PseudoUtf8CharWalker(size_t octets) throw ();

    /**
     * Accessor for number of octets.
     * @return number of octets of this char
     * special value 7 is overflow number of octets
     */
    size_t
    octets() const throw ();

    /**
     * Interpret this char as C string.
     * @return C string interpretation of this char.
     */
    operator const char*() const throw ();

    /**
     * Accessor to indexed octet of this char.
     * @param index is index of requested octet
     * @return indexed octet of this char.
     */
    unsigned char
    operator [](size_t index) const throw ();

    /**
     * Unicode code of this char.
     * @return unicode code of this char.
     */
    unsigned long
    code() const throw ();

    /**
     * Move to next char.
     * @return 0 in case - number of octets changed,
     * or if we are under overflow.
     * in all other cases return C string interpretation of this char.
     */
    const char*
    next() throw ();

    /**
     * Dump hex representation of symbol
     * @param os is output stream
     */
    void
    dump (std::ostream &os) const throw ();

  };
} //namespace SegmentorTestCommons

////// implementation inlines

namespace SegmentorTestCommons
{
  inline
  Utf8CharWalker::Utf8CharWalker(size_t octets)
    throw ()
  {
    setup_(octets);
  }

  inline
  size_t
  Utf8CharWalker::octets() const
    throw ()
  { 
    return octets_;
  }

  inline
  Utf8CharWalker::operator const char*() const
    throw ()
  { 
    return sym_.c_str();
  }

  inline
  unsigned char
  Utf8CharWalker::operator [](size_t index) const
    throw ()
  { 
    return sym_.c_ustr()[index];
  }

  inline
  unsigned long 
  Utf8CharWalker::code() const
    throw ()
  { 
    return static_cast<wchar_t>(sym_);
  }

  inline
  void
  Utf8CharWalker::dump (std::ostream &os) const
    throw ()
  {
    SegmentorTestCommons::hex_dump(os, sym_.c_str(), octets_);
  }

  inline
  PseudoUtf8CharWalker::PseudoUtf8CharWalker(size_t octets)
    throw ()
  {
    setup_(octets);
  }

  inline
  size_t
  PseudoUtf8CharWalker::octets() const
    throw ()
  { 
    return octets_;
  }

  inline
  PseudoUtf8CharWalker::operator const char*() const
    throw ()
  {
    return reinterpret_cast<const char*>(data_);
  }

  inline
  unsigned char
  PseudoUtf8CharWalker::operator [](size_t index) const
    throw ()
  { 
    return data_[index];
  }

  inline
  unsigned long 
  PseudoUtf8CharWalker::code() const
    throw ()
  {
    unsigned long octets_count = octets_;
    wchar_t dest;
    return
      String::UTF8Handler::utf8_char_to_wchar(*this, octets_count, dest) ? 
      dest : ~wchar_t(0);
  }

  inline
  void
  PseudoUtf8CharWalker::dump (std::ostream &os) const
    throw ()
  {
    SegmentorTestCommons::hex_dump(os, 
      reinterpret_cast<const char*>(data_),
      octets_);
  }

} //namespace SegmentorTestCommons

#endif //LANGUAGE_SEGMENTOR_COMMON_TESTS_COMMONS_TEXT_GENERATOR_HPP
