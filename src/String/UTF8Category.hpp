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



// @file String/UTF8Category.hpp
#ifndef STRING_UTF8_CATEGORY_HPP
#define STRING_UTF8_CATEGORY_HPP

#include <String/UTF8NArcTree.hpp>

#include <Generics/CompressedSet.hpp>
#include <Generics/Uncopyable.hpp>


namespace String
{
  /**
   * It includes the concepts of Set and Category UTF-8.
   */
  namespace Utf8Set
  {
    typedef uint32_t Utf8Char;
    typedef Generics::CompressedSet<Utf8Char> Utf8Chars;

    /**
     * Read UTF-8 byte sequence for well-formed sequence
     * into 4-bytes integral type Utf8Char. Action like union.
     * @param symbol Pointer to input string, you should control
     * rest of the string, to avoid buffer overrun - 4 bytes must be
     * accessible.
     * @param poctets Optional parameter to return number of bytes
     * into read symbol
     * @return UTF-8 sequence putted into integral type. Zero if errors or
     * termination zero occurred.
     */
    Utf8Char
    get_char(const char* symbol, unsigned long* poctets = 0)
      throw (eh::Exception);

    /**
     * Convert UTF-8 sequence to Utf8Char and add into Utf8Chars
     * @param chars Set to add UTF-8 encoded symbol
     * @param symbol UTF-8 encoded input data string
     */
    void
    add_symbol(Utf8Chars& chars, const char* symbol)
      throw (eh::Exception);

    /**
     * Add some UTF-8 encoded symbols into Utf8Chars
     * @param chars Set to add UTF-8 encoded symbols
     * @param first Pointer to begin of UTF-8 encoded input data
     * @param last Pointer to end of UTF-8 encoded input data
     */
    void
    add_symbols(Utf8Chars& chars, const char* first, const char* last)
      throw (eh::Exception);
  };

  class Utf8Category : private Generics::Uncopyable
  {
  public:
    DECLARE_EXCEPTION(InvalidArgument, eh::DescriptiveException);

    /**
     * Constructor
     * @param symbols List of symbols in the set. '-' may be used for ranges
     * To specify dash character it should be the first or the last in the
     * passed string or within a range
     * @param check_zero If nul character should be included in the set
     */
    explicit
    Utf8Category(const char* symbols, bool check_zero = false)
      throw (eh::Exception, InvalidArgument);

    /**
     * Constructor
     * @param chars set of symbols to have in the category
     */
    explicit
    Utf8Category(const Utf8Set::Utf8Chars& chars) throw (eh::Exception);

    /**
     * Constructor
     * @param tree static definitions of tree stored UTF8 category
     */
    explicit
    constexpr
    Utf8Category(UnicodeProperty::TreeStartNode& tree)
      throw (eh::Exception);

    /**
     * Destructor
     */
    ~Utf8Category() throw ();

    /**
     * Swaps this and passed category content in a safe way
     * @param category another object to swap content with
     */
    void
    swap(Utf8Category& category) throw ();

    /**
     * Checks if a symbol is in the set
     * @param str pointer to UTF-8 symbol
     * @return Presence of the symbol in the set
     */
    bool
    is_owned(const char* str) const throw ();

    /**
     * Functor-compatible way to call is_owned
     * @param str The same as in is_owned
     * @return The same as in is_owned
     */
    bool
    operator ()(const char* str) const throw ();

    /**
     * Finds the first symbol in the string which belongs to the set
     * @param str string to search in
     * @param octets length of found symbol
     * @return Pointer to found symbol or NULL if none.
     * NULL is returned when it is impossible to determine length of some
     * pseudo-UTF-8 symbol
     */
    const char*
    find_owned(const char* str, unsigned long* octets = 0) const throw ();

    /**
     * Finds the first symbol in the string which belongs to the set
     * @param begin beginning of the string to search in
     * @param end end of the string to search in
     * @param octets length of found symbol
     * @return Pointer to found symbol or end if none
     * NULL is returned when it is impossible to determine length of some
     * pseudo-UTF-8 symbol
     */
    const char*
    find_owned(const char* begin, const char* end,
      unsigned long* octets = 0) const throw ();

    /**
     * Finds the first symbol in the string which doesn't belong to the set
     * @param str string to search in
     * @param octets length of found symbol
     * @return Pointer to found symbol or NULL if none
     * NULL is returned when it is impossible to determine length of some
     * pseudo-UTF-8 symbol
     */
    const char*
    find_nonowned(const char* str, unsigned long* octets = 0) const
      throw ();

    /**
     * Finds the first symbol in the string which does not belong to the set
     * @param begin beginning of the string to search in
     * @param end end of the string to search in
     * @param octets length of found symbol
     * @return Pointer to found character or end if none
     * NULL is returned when it is impossible to determine length of some
     * pseudo-UTF-8 symbol
     */
    const char*
    find_nonowned(const char* begin, const char* end,
      unsigned long* octets = 0) const throw ();

    /**
     * Finds the last symbol in the string which belongs to the set
     * @param pos The pointer to char beyond the string to search in
     * @param start The pointer to begin of string to search in. Interval
     * [start, pos) will be looked in.
     * @param octets The length of found symbol
     * @return Pointer to found symbol or original value of pos if none.
     * NULL is returned when it is impossible to determine length of some
     * pseudo-UTF-8 symbol (found incorrect UTF-8 byte sequence)
     */
    const char*
    rfind_owned(const char* pos, const char* start,
      unsigned long* octets = 0) const throw ();

    /**
     * Finds the last symbol in the string which does not belong to the set
     * @param pos The pointer to char beyond the string to search in
     * @param start The pointer to begin of string to search in. Interval
     * [start, pos) will be looked in.
     * @param octets The length of found symbol
     * @return Pointer to found symbol or original value of pos if not found.
     * NULL is returned when it is impossible to determine length of some
     * pseudo-UTF-8 symbol (found incorrect UTF-8 byte sequence)
     */
    const char*
    rfind_nonowned(const char* pos, const char* start,
      unsigned long* octets = 0) const throw ();

  protected:
    /**
     * Get internal container
     * @return pointer to internal N-arc tree structures that represent
     * Utf8Category.
     */
    const UnicodeProperty::TreeStartNode&
    get_container_() const throw ();

  private:
    void
    clear_() throw ();

    void
    clear_(const UnicodeProperty::Node* node, unsigned long depth) throw ();

    void
    init_(const Utf8Set::Utf8Chars& chars) throw (eh::Exception);

    void
    init_interval_(const Utf8Set::Utf8Chars& chars,
      UnicodeProperty::Node& node,
      Utf8Set::Utf8Char prefix, unsigned long depth_left)
      throw (eh::Exception);

    Utf8Set::Utf8Chars::CheckStatus
    check_interval_(const Utf8Set::Utf8Chars& chars,
      Utf8Set::Utf8Char prefix, unsigned long depth_left)
      throw (eh::Exception);

    UnicodeProperty::TreeStartNode nodes_;
    bool need_cleaning_;
  };

  /// Set of spacing characters in Unicode
  extern const Utf8Category UNICODE_SPACES;
  /// Numerals Unicode symbols
  extern const Utf8Category UNICODE_DIGITS;
  /// Letters from different languages in Unicode
  extern const Utf8Category UNICODE_LETTERS;
  /// Lower letters from different languages in Unicode
  extern const Utf8Category UNICODE_LOWER_LETTERS;
  /// Title letters from different languages in Unicode
  extern const Utf8Category UNICODE_TITLE_LETTERS;
  /// Upper letters from different languages in Unicode
  extern const Utf8Category UNICODE_UPPER_LETTERS;
}

//
// Implementation
//

namespace String
{
  inline
  constexpr
  Utf8Category::Utf8Category(UnicodeProperty::TreeStartNode& tree)
    throw (eh::Exception)
    : nodes_(tree), need_cleaning_(false)
  {
  }

  inline
  bool
  Utf8Category::is_owned(const char* str) const throw ()
  {
    return UnicodeProperty::belong(get_container_(), str);
  }

  inline
  bool
  Utf8Category::operator ()(const char* str) const throw ()
  {
    return is_owned(str);
  }

  inline
  const UnicodeProperty::TreeStartNode&
  Utf8Category::get_container_() const throw ()
  {
    return nodes_;
  }
}

#endif
