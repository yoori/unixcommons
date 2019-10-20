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



// @file String/AsciiStringManip.hpp
#ifndef ASCII_STRING_MANIP_HPP
#define ASCII_STRING_MANIP_HPP

#include <algorithm>
#include <functional>
#include <limits>
#include <climits>
#include <cassert>
#include <cstdint>

#include <String/SubString.hpp>


namespace String
{
  /**
   * Contain routines that manipulate only with ASCII encoded data.
   * Bytes range in 0-127.
   */
  namespace AsciiStringManip
  {
    extern const char HEX_DIGITS[]; // = "0123456789ABCDEF";

    /**
     * these functions don't use stdlib codepage and
     * affect only for (0-127) chars.
     */
    char
    to_lower(char ch) throw ()
      __attribute__((always_inline));

    char
    to_upper(char ch) throw ()
      __attribute__((always_inline));

    void
    to_lower(std::string& dest) throw ()
      __attribute__((always_inline));

    void
    to_upper(std::string& dest) throw ()
      __attribute__((always_inline));

    template <typename Iterator>
    void
    to_lower(Iterator first, Iterator last) throw (eh::Exception)
      __attribute__((always_inline));

    template <typename Iterator>
    void
    to_upper(Iterator first, Iterator last) throw (eh::Exception)
      __attribute__((always_inline));

    namespace Category
    {
      /**
       * Category inherits a predicate and allows to search strings
       * for belong/nonbelong chars
       */
      template <typename Predicate>
      struct Category :
        public std::unary_function<char, bool>,
        public Predicate
      {
      public:
        /**
         * Constructor
         * Calls default constructor of Predicate
         */
        Category() throw (eh::Exception);

        /**
         * Constructor
         * Calls constructor of Predicate with data
         * @param data data for constructor of Predicate
         */
        template <typename... T>
        explicit
        Category(T... data) throw (eh::Exception);

        /**
         * Checks if character is in the set
         * @param ch character to test
         * @return Presence of the character in the set
         */
        bool
        is_owned(char ch) const throw ()
          __attribute__((always_inline));

        /**
         * Checks category for emptiness
         * @return true if category has no symbol inside
         */
        bool
        empty() const throw ();

        /**
         * Finds the first character in the string which belongs to the set
         * @param str the string to search in
         * @return Pointer to found character or NULL if none
         */
        const char*
        find_owned(const char* str) const throw ()
          __attribute__((always_inline));

        /**
         * Finds the first character in the string which belongs to the set
         * @param begin beginning of the string to search in
         * @param end end of the string to search in
         * @param octets_length used for storing length of symbol in octets
         * @return Pointer to found character or end if none
         */
        const char*
        find_owned(const char* begin, const char* end,
          unsigned long* octets_length = 0) const throw ()
          __attribute__((always_inline));

        /**
         * Finds the first character in the string which doesn't belong
         * to the set
         * @param str the string to search in
         * @return Pointer to found character or NULL if none
         */
        const char*
        find_nonowned(const char* str) const throw ()
          __attribute__((always_inline));

        /**
         * Finds the first character in the string which does not belong
         * to the set
         * @param begin beginning of the string to search in
         * @param end end of the string to search in
         * @return Pointer to found character or end if none
         */
        const char*
        find_nonowned(const char* begin, const char* end) const throw ()
          __attribute__((always_inline));

        /**
         * Finds the last character in the string which belongs to the set
         * @param pos The pointer to char beyond the string to search in
         * @param start The pointer to begin of the string to search in.
         * Interval [start, pos) will be looked in.
         * @return Pointer to found character or original value of pos if
         * none.
         */
        const char*
        rfind_owned(const char* pos, const char* start) const throw ()
          __attribute__((always_inline));

        /**
         * Finds the last character in the string which does not belong
         * to the set
         * @param pos The pointer to char beyond  the string to search in
         * @param start The pointer to begin of the string to search in.
         * Interval [start, pos) will be looked in.
         * @return Pointer to found character or original value of pos if
         * none.
         */
        const char*
        rfind_nonowned(const char* pos, const char* start) const throw ()
          __attribute__((always_inline));
      };

      /**
       * Predicate for Category
       * Contains a table of symbols inside
       */
      class CharTable
      {
      public:
        /**
         * Default constructor.
         * Does not initialize the object.
         */
        CharTable()
          throw ()
          __attribute__((always_inline));

        /**
         * Constructor
         * @param str List of characters in the set. '-' may be used for
         * ranges. To specify dash character it should be the first or the
         * last in the passed string or within a range
         * @param check_zero If nul character should be included in the set
         */
        explicit
        CharTable(const char* str, bool check_zero = false)
          throw ();

        /**
         * Constructor
         * Created object is a union of passed ones
         * @param first first object to unite
         * @param second second object to unite
         */
        CharTable(const CharTable& first, const CharTable& second)
          throw ();

        /**
         * Constructor
         * Created object is a union of passed ones
         * @param first first object to unite
         * @param second second object to unite
         * @param third third object to unite
         */
        CharTable(const CharTable& first, const CharTable& second,
          const CharTable& third)
          throw ();

        /**
         * Constructor
         * Created object is a predicate result
         * @param predicate predicate for initialization
         */
        template <typename Predicate>
        explicit
        CharTable(Predicate predicate) throw ();

        bool
        operator ()(char ch) const throw ()
          __attribute__((always_inline));

      private:
        bool table_[256];
      };

      /**
       * Predicate for Category
       * Checks the one symbol
       */
      template <const char SYMBOL>
      struct Char1
      {
        /**
         * Checks if ch equals to SYMBOL
         * @return if ch equals to SYMBOL
         */
        bool
        operator ()(char ch) const throw ()
          __attribute__((always_inline));
      };

      /**
       * Predicate for Category
       * Checks the two symbols
       */
      template <const char SYMBOL1, const char SYMBOL2>
      struct Char2
      {
        /**
         * Checks if ch equals to SYMBOL1 or SYMBOL2
         * @return if ch equals to SYMBOL1 or SYMBOL2
         */
        bool
        operator ()(char ch) const throw ()
          __attribute__((always_inline));
      };

      /**
       * Predicate for Category
       * Checks the three symbols
       */
      template <const char SYMBOL1, const char SYMBOL2, const char SYMBOL3>
      struct Char3
      {
        /**
         * Checks if ch equals to SYMBOL1, SYMBOL2 or SYMBOL3
         * @return if ch equals to SYMBOL1, SYMBOL2 or SYMBOL3
         */
        bool
        operator ()(char ch) const throw ()
          __attribute__((always_inline));
      };
    }

    // Quick access classes for different Categories
    typedef Category::Category<Category::CharTable> CharCategory;
    template <const char SYMBOL>
    struct Char1Category :
      public Category::Category<Category::Char1<SYMBOL> >
    {
    };
    template <const char SYMBOL1, const char SYMBOL2>
    struct Char2Category :
      public Category::Category<Category::Char2<SYMBOL1, SYMBOL2> >
    {
    };
    template <const char SYMBOL1, const char SYMBOL2, const char SYMBOL3>
    struct Char3Category :
      public Category::Category<Category::Char3<SYMBOL1, SYMBOL2, SYMBOL3> >
    {
    };

    /// Small and capital Latin letters.
    extern const CharCategory ALPHA;
    /// Arabic numerals
    extern const CharCategory NUMBER;
    /// Arabic numerals and Latin letters
    extern const CharCategory ALPHA_NUM;
    /// Numerals used in octal notation
    extern const CharCategory OCTAL_NUMBER;
    /// Numerals and letters used in hexadecimal notation
    extern const CharCategory HEX_NUMBER;
    /// Space characters
    extern const CharCategory SPACE;
    /// Symbols are used in regular expressions.
    extern const CharCategory REGEX_META;

    typedef const Char1Category<':'> SepColon;
    typedef const Char1Category<','> SepComma;
    typedef const Char1Category<'.'> SepPeriod;
    typedef const Char1Category<'-'> SepMinus;
    typedef const Char1Category<';'> SepSemCol;
    typedef const Char1Category<'&'> SepAmp;
    typedef const Char1Category<' '> SepSpace;
    typedef const Char1Category<'='> SepEq;
    typedef const Char1Category<'/'> SepSlash;
    typedef const Char1Category<'#'> SepHash;
    typedef const Char1Category<'|'> SepBar;
    typedef const Char1Category<'\n'> SepNL;
    typedef const Char1Category<'\t'> SepTab;
    typedef const Char1Category<'_'> SepUnderscore;

    /**
     * Finds and replaces all sequences of chars from CharCategory
     * to replace string.
     * @param dest result put here.
     * @param str source string
     * @param replacement string that replace sequences of.
     * @param to_replace all sequences of chars from this category, will
     * be replaced by replace.
     */
    void
    flatten(std::string& dest, const String::SubString& str,
      const SubString& replacement = SubString(" ", 1),
      const CharCategory& to_replace = SPACE)
      throw (eh::Exception);

    /**
     * Helper object to compare ASCII strings caselessly.
     */
    struct Caseless
    {
    public:
      /**
       * Constructor.
       * Calls strlen().
       * @param str string to store for comparison
       */
      explicit
      Caseless(const char* str) throw ();
      /**
       * Constructor
       * @param str string to store for comparison
       */
      explicit
      Caseless(const SubString& str) throw ();

      /**
       * Compares the stored string with the given one.
       * @param str string to compare with
       * @return return an integer less than, equal to,
       * or greater than zero, if the stored string is found,
       * respectively, to be less than, to match, or be
       * greater than str
       */
      int
      compare(const SubString& str) const throw ();

      /**
       * Checks SubStrings on equality ignoring letters case.
       * @param str string to compare with
       * @return true if lengths of string are equal and
       * the content of stored string equals to str
       * ignoring case, false if not.
       */
      bool
      equal(const SubString& str) const throw ();

      /**
       * Checks if str begins from the stored string.
       * @param str string to check
       * @return true if str size is not less than the stored string
       * size and those first letters are equal ignoring case.
       */
      bool
      start(const SubString& str) const throw ();

      SubString str;
    };

    /**
     * Converts unsigned char to char
     * @param ch unsigned char value
     * @return the corresponding char value
     */
    char
    convert(unsigned char ch) throw ()
      __attribute__((always_inline));

    /**
     * Converts [0-9a-fA-F] to the corresponding numeric value
     * @param ch hex digit
     * @return corresponding number
     */
    unsigned char
    hex_to_int(char ch) throw ()
      __attribute__((always_inline));

    /**
     * Converts to hex digits in char
     * @param major major hex digit
     * @param minor minor hex digit
     * @return corresponding char value
     */
    char
    hex_to_char(char major, char minor) throw ()
      __attribute__((always_inline));

    /**
     * Converts sizeof(Integer) * 2 hex digits into integer
     * @param data hex digits
     * @param value corresponding value
     */
    template <typename Integer>
    void
    hex_to_integer(const char* data, Integer& value) throw ()
      __attribute__((always_inline));

    /**
     * Converts hex string into data
     * @param data hex string
     * @param buf resulted data
     */
    void
    hex_to_buf(const SubString& data, char* buf) throw ()
      __attribute__((always_inline));
  }
}

namespace String
{
  namespace AsciiStringManip
  {
    /**
     * Contain names of tables, need for effective data manipulation
     */
    namespace Tables
    {
      /// Table convert ASCII (0-127) data to lower latin letters
      extern const char ASCII_TOLOWER_TABLE[256];
      /// Table convert ASCII (0-127) data to upper latin letters
      extern const char ASCII_TOUPPER_TABLE[256];
    }

    inline
    void
    flatten(std::string& dest, const String::SubString& str,
      const SubString& replacement, const CharCategory& to_replace)
      throw (eh::Exception)
    {
      const char* const REPL_DATA = replacement.data();
      const size_t REPL_SIZE = replacement.size();
      const char* const REPL_END = REPL_DATA + REPL_SIZE;
      dest.resize(str.size() * (REPL_SIZE ? REPL_SIZE : 1));
      char* out = &dest[0];
      const char* current;

      for (const char* first = str.begin(), * const LAST = str.end();
        first != LAST;)
      {
        current = to_replace.find_owned(first, LAST);
        // last if haven't spaces
        // copy text before space
        out = std::copy(first, current, out);

        if (current == LAST)
        {
          break;
        }
        out = std::copy(REPL_DATA, REPL_END, out);
        first = to_replace.find_nonowned(current, LAST);
      }
      dest.resize(out - &dest[0]);
    }

    inline
    char
    to_lower(char ch) throw ()
    {
      return Tables::ASCII_TOLOWER_TABLE[static_cast<uint8_t>(ch)];
    }

    inline
    char
    to_upper(char ch) throw ()
    {
      return Tables::ASCII_TOUPPER_TABLE[static_cast<uint8_t>(ch)];
    }

    template <typename Iterator>
    inline
    void
    to_lower(Iterator first, Iterator last) throw (eh::Exception)
    {
      for (; first != last; ++first)
      {
        *first =
          Tables::ASCII_TOLOWER_TABLE[static_cast<uint8_t>(*first)];
      }
    }

    template <typename Iterator>
    inline
    void
    to_upper(Iterator first, Iterator last) throw (eh::Exception)
    {
      for (; first != last; ++first)
      {
        *first =
          Tables::ASCII_TOUPPER_TABLE[static_cast<uint8_t>(*first)];
      }
    }

    inline
    void
    to_lower(std::string& dest) throw ()
    {
      if (!dest.empty())
      {
        char* data = &dest[0];
        to_lower(data, data + dest.size());
      }
    }

    inline
    void
    to_upper(std::string& dest) throw ()
    {
      if (!dest.empty())
      {
        char* data = &dest[0];
        to_upper(data, data + dest.size());
      }
    }

    inline
    Caseless::Caseless(const char* str) throw ()
      : str(str)
    {
    }

    inline
    Caseless::Caseless(const SubString& str) throw ()
      : str(str)
    {
    }

    inline
    int
    Caseless::compare(const SubString& str) const throw ()
    {
      const char* str1 = this->str.data();
      const char* str2 = str.data();
      size_t len1 = this->str.size();
      size_t len2 = str.size();
      size_t len = std::min(len1, len2);
      if (str1 != str2 && len)
      {
        while (len--)
        {
          int result = static_cast<int>(Tables::ASCII_TOLOWER_TABLE[
            static_cast<uint8_t>(*str1++)]) -
            Tables::ASCII_TOLOWER_TABLE[static_cast<uint8_t>(*str2++)];
          if (result)
          {
            return result;
          }
        }
      }

      return len1 != len2 ? len1 < len2 ? -1 : 1 : 0;
    }

    inline
    bool
    Caseless::equal(const SubString& str) const throw ()
    {
      const char* str1 = str.data();
      const char* str2 = this->str.data();
      size_t len = str.size();
      if (len != this->str.size())
      {
        return false;
      }
      if (str1 == str2 || !len)
      {
        return true;
      }

      while (Tables::ASCII_TOLOWER_TABLE[static_cast<uint8_t>(*str1++)] ==
        Tables::ASCII_TOLOWER_TABLE[static_cast<uint8_t>(*str2++)])
      {
        if (!--len)
        {
          return true;
        }
      }

      return false;
    }

    inline
    bool
    Caseless::start(const SubString& str) const throw ()
    {
      return equal(str.substr(0, this->str.size()));
    }

    inline
    bool
    operator ==(const SubString& str, const Caseless& cl)
      throw ()
    {
      return cl.equal(str);
    }

    inline
    bool
    operator ==(const Caseless& cl, const SubString& str)
      throw ()
    {
      return str == cl;
    }

    inline
    bool
    operator !=(const SubString& str, const Caseless& cl)
      throw ()
    {
      return !(str == cl);
    }

    inline
    bool
    operator !=(const Caseless& cl, const SubString& str)
      throw ()
    {
      return str != cl;
    }

    namespace Category
    {
      //
      // Category class
      //

      template <typename Predicate>
      Category<Predicate>::Category() throw (eh::Exception)
      {
      }

      template <typename Predicate>
      template <typename... T>
      Category<Predicate>::Category(T... data) throw (eh::Exception)
        : Predicate(std::forward<T>(data)...)
      {
      }

      template <typename Predicate>
      inline
      bool
      Category<Predicate>::is_owned(char ch) const throw ()
      {
        return Predicate::operator ()(ch);
      }

      template <typename Predicate>
      bool
      Category<Predicate>::empty() const throw ()
      {
        for (register char ch = CHAR_MIN; ; ch++)
        {
          if (is_owned(ch))
          {
            return false;
          }
          if (ch == CHAR_MAX)
          {
            break;
          }
        }
        return true;
      }

      template <typename Predicate>
      inline
      const char*
      Category<Predicate>::find_owned(const char* str) const throw ()
      {
        for (register char ch; (ch = *str) != '\0'; str++)
        {
          if (is_owned(ch))
          {
            return str;
          }
        }
        return is_owned('\0') ? str : 0;
      }

      template <typename Predicate>
      inline
      const char*
      Category<Predicate>::find_owned(const char* str, const char* end,
        unsigned long* octets_length) const throw ()
      {
        for (; str != end; ++str)
        {
          if (is_owned(*str))
          {
            if (octets_length)
            {
              *octets_length = 1;
            }

            return str;
          }
        }
        return end;
      }

      template <typename Predicate>
      inline
      const char*
      Category<Predicate>::find_nonowned(const char* str) const
        throw ()
      {
        for (register char ch; (ch = *str) != '\0'; str++)
        {
          if (!is_owned(ch))
          {
            return str;
          }
        }
        return !is_owned('\0') ? str : 0;
      }

      template <typename Predicate>
      inline
      const char*
      Category<Predicate>::find_nonowned(const char* str,
        const char* end) const
        throw ()
      {
        for (; str != end; ++str)
        {
          if (!is_owned(*str))
          {
            return str;
          }
        }
        return end;
      }

      template <typename Predicate>
      inline
      const char*
      Category<Predicate>::rfind_owned(const char* pos,
        const char* start) const
        throw ()
      {
        const char* const NOT_FOUND = pos;
        while (start != pos)
        {
          if (is_owned(*--pos))
          {
            return pos;
          }
        }
        return NOT_FOUND;
      }

      template <typename Predicate>
      inline
      const char*
      Category<Predicate>::rfind_nonowned(const char* pos,
        const char* start) const
        throw ()
      {
        const char* const NOT_FOUND = pos;
        while (start != pos)
        {
          if (!is_owned(*--pos))
          {
            return pos;
          }
        }
        return NOT_FOUND;
      }


      //
      // CharTable class
      //

      inline
      CharTable::CharTable() throw ()
      {
      }

      template <typename Predicate>
      CharTable::CharTable(Predicate predicate) throw ()
      {
        for (register int i = 0; i < 256; i++)
        {
          table_[i] = predicate(i);
        }
      }

      inline
      bool
      CharTable::operator ()(char ch) const throw ()
      {
        return table_[static_cast<uint8_t>(ch)];
      }


      //
      // Char1 class
      //

      template <const char SYMBOL>
      inline
      bool
      Char1<SYMBOL>::operator ()(char ch) const throw ()
      {
        return ch == SYMBOL;
      }


      //
      // Char2 class
      //

      template <const char SYMBOL1, const char SYMBOL2>
      inline
      bool
      Char2<SYMBOL1, SYMBOL2>::operator ()(char ch) const throw ()
      {
        return ch == SYMBOL1 || ch == SYMBOL2;
      }


      //
      // Char3 class
      //

      template <const char SYMBOL1, const char SYMBOL2, const char SYMBOL3>
      inline
      bool
      Char3<SYMBOL1, SYMBOL2, SYMBOL3>::operator ()(char ch) const throw ()
      {
        return ch == SYMBOL1 || ch == SYMBOL2 || ch == SYMBOL3;
      }
    }

    inline
    char
    convert(unsigned char ch) throw ()
    {
      return static_cast<const char&>(ch);
    }

    inline
    unsigned char
    hex_to_int(char ch) throw ()
    {
      return ch <= '9' ? ch - '0' : (ch & 0x0F) + 9;
    }

    inline
    char
    hex_to_char(char major, char minor) throw ()
    {
      return convert((hex_to_int(major) << 4) | hex_to_int(minor));
    }

    template <typename Integer>
    inline
    void
    hex_to_integer(const char* data, Integer& value) throw ()
    {
      assert(!std::numeric_limits<Integer>::is_signed);
      value = 0;
      for (size_t i = 0; i < sizeof(Integer) * 8; i += 8)
      {
        value |= static_cast<Integer>(hex_to_int(*data++)) << (i + 4);
        value |= static_cast<Integer>(hex_to_int(*data++)) << i;
      }
    }

    inline
    void
    hex_to_buf(const SubString& data, char* buf) throw ()
    {
      assert(!(data.size() & 1));
      for (size_t i = 0; i < data.size(); i += 2)
      {
        *buf++ = hex_to_char(data[i], data[i + 1]);
      }
    }
  }
}

#endif
