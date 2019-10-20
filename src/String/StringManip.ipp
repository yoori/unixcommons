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



#include <cstdarg>
#include <cassert>
#include <cstring>


namespace String
{
  namespace StringManip
  {
    inline
    size_t
    strlcpy(register char* dst, register const char* src,
      register size_t size) throw ()
    {
      register const char* const saved_src = src;

      if (size)
      {
        while (--size)
        {
          if (!(*dst++ = *src++))
          {
            return src - saved_src - 1;
          }
        }
        *dst = '\0';
      }

      while (*src++);

      return src - saved_src - 1;
    }

    inline
    size_t
    strlcat(register char* dst, register const char* src,
      register size_t size) throw ()
    {
      register const char* const saved_src = src;

      register size_t dst_len = size;
      while (size-- && *dst)
      {
        dst++;
      }
      size++;
      dst_len -= size;

      if (size)
      {
        while (--size)
        {
          if (!(*dst++ = *src++))
          {
            return src - saved_src + dst_len - 1;
          }
        }
        *dst = '\0';
      }

      while (*src++);

      return src - saved_src + dst_len - 1;
    }


    inline
    constexpr
    size_t
    base64mod_encoded_size(size_t original_size, bool padding) throw ()
    {
      return padding ? (original_size + 2) / 3 * 4 :
        original_size / 3 * 4 +
          (original_size % 3 ? original_size % 3 + 1 : 0);
    }

    inline
    constexpr
    size_t
    base64mod_max_decoded_size(size_t original_size) throw ()
    {
      return (original_size + 3) / 4 * 3;
    }

    inline
    constexpr
    size_t
    base64mod_fill_size(size_t original_size) throw ()
    {
      return (8 >> (original_size % 3)) & 6;
    }

    inline
    size_t
    append(char* buffer, size_t size, const char* str) throw ()
    {
      size_t length = strlcpy(buffer, str, size);
      return length < size ? length : size;
    }

    inline
    size_t
    append(char* buffer, size_t size, char* str) throw ()
    {
      return append(buffer, size, const_cast<const char*>(str));
    }

    inline
    size_t
    append(char* buffer, size_t size, const SubString& str) throw ()
    {
      //assert(size);
      if (str.size() >= size)
      {
        String::CharTraits<char>::copy(buffer, str.data(), size - 1);
        buffer[size - 1] = '\0';
        return size;
      }

      String::CharTraits<char>::copy(buffer, str.data(), str.size());
      return str.size();
    }

    inline
    size_t
    append(char* buffer, size_t size, const std::string& str) throw ()
    {
      return append(buffer, size, SubString(str));
    }

    template <typename Integer>
    size_t
    append(char* buffer, size_t size, Integer integer) throw ()
    {
      size_t res = int_to_str(integer, buffer, size);
      if (!res)
      {
        //assert(size);
        *buffer = '\0';
        return size;
      }
      return res - 1;
    }

    inline
    void
    concat(char* buffer, size_t size)
      throw ()
    {
      assert(size);
      *buffer = '\0';
    }

    template <typename First, typename... Args>
    void
    concat(char* buffer, size_t size, First f, Args... args)
      throw ()
    {
      size_t length = append(buffer, size, f);
      if (length < size)
      {
        concat(buffer + length, size - length, args...);
      }
    }
  }


  //
  // Fmt class
  //

  inline
  Fmt::Fmt(const char* fmt, ...) throw ()
  {
    int n;
    std::va_list ap;
    va_start(ap, fmt);
    n = vasprintf(&text_, fmt, ap);
    va_end(ap);

    if (n == -1)
    {
      text_ = 0;
    }
  }

  inline
  Fmt::~Fmt() throw ()
  {
    if (text_)
    {
      ::free(text_);
    }
  }

  inline
  Fmt::operator const char*() const throw ()
  {
    if (text_)
    {
      return text_;
    }
    else
    {
      return "";
    }
  }


  namespace StringManip
  {
    inline
    void
    wchar_to_utf8(wchar_t src, std::string& str)
      throw (eh::Exception)
    {
      // switch to exactly-four byte unicode
      uint32_t ucs4_char = static_cast<uint32_t>(src);

      if (ucs4_char < 0x00000080)
      {
        str.push_back(static_cast<char>(ucs4_char & 0x7F));
        return;
      }

      if (ucs4_char < 0x00010000)
      {
        if (ucs4_char < 0x00000800)
        {
          char buf[] =
          {
            static_cast<char>(((ucs4_char >> 6) & 0x1F) | 0xC0),
            static_cast<char>((ucs4_char & 0x3F) | 0x80)
          };
          str.append(buf, sizeof(buf));
        }
        else /* if (ucs4_char < 0x00010000) */
        {
          char buf[] =
            {
              static_cast<char>(((ucs4_char >> 12) & 0x0F) | 0xE0),
              static_cast<char>(((ucs4_char >> 6) & 0x3F) | 0x80),
              static_cast<char>((ucs4_char & 0x3F) | 0x80)
            };
          str.append(buf, sizeof(buf));
        }
      }
      else
      {
        if (ucs4_char < 0x00200000)
        {
          char buf[] =
            {
              static_cast<char>(((ucs4_char >> 18) & 0x07) | 0xF0),
              static_cast<char>(((ucs4_char >> 12) & 0x3F) | 0x80),
              static_cast<char>(((ucs4_char >> 6) & 0x3F) | 0x80),
              static_cast<char>((ucs4_char & 0x3F) | 0x80)
            };
          str.append(buf, sizeof(buf));
        }
        else
        {
          if (ucs4_char < 0x04000000)
          {
            char buf[] =
              {
                static_cast<char>(((ucs4_char >> 24) & 0x03) | 0xF8),
                static_cast<char>(((ucs4_char >> 18) & 0x3F) | 0x80),
                static_cast<char>(((ucs4_char >> 12) & 0x3F) | 0x80),
                static_cast<char>(((ucs4_char >> 6) & 0x3F) | 0x80),
                static_cast<char>((ucs4_char & 0x3F) | 0x80)
              };
            str.append(buf, sizeof(buf));
          }
          else /* if (ucs4_char < 0x80000000) */
          {
            char buf[] =
              {
                static_cast<char>(((ucs4_char >> 30) & 0x01) | 0xFC),
                static_cast<char>(((ucs4_char >> 24) & 0x3F) | 0x80),
                static_cast<char>(((ucs4_char >> 18) & 0x3F) | 0x80),
                static_cast<char>(((ucs4_char >> 12) & 0x3F) | 0x80),
                static_cast<char>(((ucs4_char >> 6) & 0x3F) | 0x80),
                static_cast<char>((ucs4_char & 0x3F) | 0x80)
              };
            str.append(buf, sizeof(buf));
          }
        }
      }
    }


    inline
    const char*
    base_name(const char* path) throw ()
    {
      const char* file = strrchr(path, '/');
      return file ? file + 1 : path;
    }
  }
}
