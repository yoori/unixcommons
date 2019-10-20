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



#include <algorithm>
#include <limits>


namespace String
{
  namespace StringManip
  {
    //
    // int_to_str function
    //

    namespace IntToStrHelper
    {
      template <typename Integer, const bool is_signed>
      struct IntToStrSign;

      template <typename Integer>
      struct IntToStrSign<Integer, false>
      {
        static
        size_t
        convert(Integer value, char* str) throw ();
      };

      template <typename Integer>
      size_t
      IntToStrSign<Integer, false>::convert(Integer value, char* str)
        throw ()
      {
        char* ptr = str;
        do
        {
          *ptr++ = '0' + value % 10;
        }
        while (value /= 10);
        size_t size = ptr - str;
        for (*ptr-- = '\0'; str < ptr; str++, ptr--)
        {
          std::swap(*str, *ptr);
        }
        return size;
      }

      template <typename Integer>
      struct IntToStrSign<Integer, true>
      {
        static
        size_t
        convert(Integer value, char* str) throw ();
      };

      template <typename Integer>
      size_t
      IntToStrSign<Integer, true>::convert(Integer value, char* str)
        throw ()
      {
        if (value < -std::numeric_limits<Integer>::max())
        {
          return 0;
        }
        if (value < 0)
        {
          *str = '-';
          return IntToStrSign<Integer, false>::convert(-value, str + 1) + 1;
        }
        return IntToStrSign<Integer, false>::convert(value, str);
      }
    }

    template <typename Integer>
    size_t
    int_to_str(Integer value, char* str, size_t size) throw ()
    {
      static_assert(std::numeric_limits<Integer>::is_integer,
        "Integer is not an integer type");

      if (size < std::numeric_limits<Integer>::digits10 + 3)
      {
        return 0;
      }

      return IntToStrHelper::IntToStrSign<Integer,
        std::numeric_limits<Integer>::is_signed>::convert(value, str);
    }

    template <typename Integer>
    bool
    str_to_int(const String::SubString& str, Integer& value) throw ()
    {
      const char* src = str.begin();
      const char* const END = str.end();
      if (src == END)
      {
        return false;
      }
      bool negative = false;
      switch (*src)
      {
      case '-':
        if (!std::numeric_limits<Integer>::is_signed)
        {
          return false;
        }
        negative = true;
      case '+':
        if (++src == END)
        {
          return false;
        }
      }
      value = 0;
      const Integer LIMIT = std::numeric_limits<Integer>::max() / 10;
      if (negative)
      {
        do
        {
          unsigned char ch = static_cast<unsigned char>(*src) -
            static_cast<unsigned char>('0');
          if (ch > 9 || value < -LIMIT || (value == -LIMIT &&
            ch > static_cast<unsigned char>(
              -(std::numeric_limits<Integer>::min() + LIMIT * 10))))
          {
            return false;
          }
          value = value * static_cast<Integer>(10) -
            static_cast<Integer>(ch);
        }
        while (++src != END);
      }
      else
      {
        do
        {
          unsigned char ch = static_cast<unsigned char>(*src) -
            static_cast<unsigned char>('0');
          if (ch > 9 || value > LIMIT || (value == LIMIT &&
            ch > static_cast<unsigned char>(
              std::numeric_limits<Integer>::max() - LIMIT * 10)))
          {
            return false;
          }
          value = value * static_cast<Integer>(10) +
            static_cast<Integer>(ch);
        }
        while (++src != END);
      }

      return true;
    }


    //
    // InverseCategory class
    //

    template <class Category>
    InverseCategory<Category>::InverseCategory() throw (eh::Exception)
      : Category()
    {
    }

    template <class Category>
    template <typename... T>
    InverseCategory<Category>::InverseCategory(T... args)
      throw (eh::Exception)
      : Category(std::forward<T>(args)...)
    {
    }

    template <class Category>
    template <typename Character>
    bool
    InverseCategory<Category>::is_owned(Character ch) const throw ()
    {
      return !Category::is_owned(ch);
    }

    template <class Category>
    template <typename Character>
    bool
    InverseCategory<Category>::operator ()(Character ch) const throw ()
    {
      return is_owned(ch);
    }

    template <class Category>
    const char*
    InverseCategory<Category>::find_owned(
      const char* begin, const char* end, unsigned long* octets) const
      throw ()
    {
      return Category::find_nonowned(begin, end, octets);
    }

    template <class Category>
    const char*
    InverseCategory<Category>::find_nonowned(
      const char* begin, const char* end, unsigned long* octets) const
      throw ()
    {
      return Category::find_owned(begin, end, octets);
    }

    template <class Category>
    const char*
    InverseCategory<Category>::rfind_owned(
      const char* begin, const char* end, unsigned long* octets) const
      throw ()
    {
      return Category::rfind_nonowned(begin, end, octets);
    }

    template <class Category>
    const char*
    InverseCategory<Category>::rfind_nonowned(
      const char* begin, const char* end, unsigned long* octets) const
      throw ()
    {
      return Category::rfind_owned(begin, end, octets);
    }


    //
    // IntToStr class
    //

    template <typename Integer>
    IntToStr::IntToStr(Integer value) throw ()
      : length_(int_to_str(value, buf_, sizeof(buf_)))
    {
    }

    inline
    SubString
    IntToStr::str() const throw ()
    {
      return SubString(buf_, length_);
    }

    inline
    IntToStr::operator SubString() const throw ()
    {
      return str();
    }
  }
}
