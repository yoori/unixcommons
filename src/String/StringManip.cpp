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

#include <String/StringManip.hpp>
#include <String/UTF8Handler.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>


namespace
{
  using String::AsciiStringManip::convert;

  namespace Base64
  {
    const char STD_ENCODE[64] =
    {
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
      'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
      'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
      'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
      'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
      'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
      'w', 'x', 'y', 'z', '0', '1', '2', '3',
      '4', '5', '6', '7', '8', '9', '+', '/'
    };

    const char MOD_ENCODE[64] =
    {
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
      'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
      'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
      'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
      'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
      'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
      'w', 'x', 'y', 'z', '0', '1', '2', '3',
      '4', '5', '6', '7', '8', '9', '-', '_'
    };

    const char PADDING = '.';

    inline
    char
    std_encode(unsigned char ch) throw ()
    {
      return STD_ENCODE[ch & 077];
    }

    inline
    char
    mod_encode(unsigned char ch) throw ()
    {
      return MOD_ENCODE[ch & 077];
    }

    const uint8_t DECODE[256] =
    {
      0100, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*000-007*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*010-017*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*020-027*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*030-037*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*040-047*/
      0177, 0177, 0177, 0076, 0177, 0076, 0100, 0077, /*050-057*/
      0064, 0065, 0066, 0067, 0070, 0071, 0072, 0073, /*060-067*/
      0074, 0075, 0177, 0177, 0177, 0177, 0177, 0177, /*070-077*/
      0177, 0000, 0001, 0002, 0003, 0004, 0005, 0006, /*100-107*/
      0007, 0010, 0011, 0012, 0013, 0014, 0015, 0016, /*110-117*/
      0017, 0020, 0021, 0022, 0023, 0024, 0025, 0026, /*120-127*/
      0027, 0030, 0031, 0177, 0177, 0177, 0177, 0077, /*130-137*/
      0177, 0032, 0033, 0034, 0035, 0036, 0037, 0040, /*140-147*/
      0041, 0042, 0043, 0044, 0045, 0046, 0047, 0050, /*150-157*/
      0051, 0052, 0053, 0054, 0055, 0056, 0057, 0060, /*160-167*/
      0061, 0062, 0063, 0177, 0100, 0177, 0177, 0177, /*170-177*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*200-207*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*210-217*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*220-227*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*230-237*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*240-247*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*250-257*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*260-267*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*270-277*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*300-307*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*310-317*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*320-327*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*330-337*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*340-347*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*350-357*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*360-367*/
      0177, 0177, 0177, 0177, 0177, 0177, 0177, 0177, /*370-377*/
    };

    template <typename Functor>
    void
    func_encode(std::string& dst, const void* src, size_t n,
      bool padding, Functor encode, const char PADDING, uint8_t fill)
      throw (eh::Exception)
    {
      if (n == 0)
      {
        dst.clear();
        return;
      }

      std::string dest;
      dest.reserve((n + 2) / 3 * 4);

      register const unsigned char* p =
        static_cast<const unsigned char*>(src);

      for (; n > 2; n -= 3, p += 3)
      {
        char buf[] = { encode(*p >> 2),
          encode(((*p << 4) & 060) | ((p[1] >> 4) & 017)),
          encode(((p[1] << 2) & 074) | ((p[2] >> 6) & 03)),
          encode(p[2] & 077) };
        dest.append(buf, sizeof(buf));
      }

      switch (n)
      {
      case 1:
        {
          register unsigned char c1 = *p;
          char buf[] = { encode(c1 >> 2),
            encode(((c1 << 4) & 060) | (fill & 0x0F)), PADDING,
            PADDING };
          dest.append(buf, padding ? sizeof(buf) : 2);
        }
        break;

      case 2:
        {
          register unsigned char c1 = *p;
          register unsigned char c2 = p[1];
          char buf[] = { encode(c1 >> 2),
            encode(((c1 << 4) & 060) | ((c2 >> 4) & 017)),
            encode(((c2 << 2) & 074) | (fill & 0x03)), PADDING };
          dest.append(buf, padding ? sizeof(buf) : 3);
        }
        break;

      default:
        break;
      }

      dst.swap(dest);
    }

    class Iterator
    {
    public:
      Iterator(const String::SubString& src) throw ();
      bool
      available() const throw ();
      uint8_t
      operator *() const
        throw (String::StringManip::InvalidFormatException);
      void
      operator ++() throw ();
      uint8_t
      skip_blanks() throw (String::StringManip::InvalidFormatException);
      void
      check_padding(const char* src) const
        throw (String::StringManip::InvalidFormatException);

    private:
      const char* ptr_;
      ssize_t length_;
    };

    const char PAD1[] = "......";
    const char PAD2[] = "||||||";
    const char PAD3[] = "======";

    inline
    Iterator::Iterator(const String::SubString& src) throw ()
      : ptr_(src.data()), length_(src.length())
    {
    }

    inline
    bool
    Iterator::available() const throw ()
    {
      return length_ > 0;
    }

    inline
    uint8_t
    Iterator::operator *() const
      throw (String::StringManip::InvalidFormatException)
    {
      return DECODE[length_ <= 0 ? 0 : static_cast<uint8_t>(*ptr_)];
    }

    inline
    void
    Iterator::operator ++() throw ()
    {
      ++ptr_;
      --length_;
    }

    inline
    uint8_t
    Iterator::skip_blanks()
      throw (String::StringManip::InvalidFormatException)
    {
      register uint8_t ch;
      while ((ch = operator *()) == 0177)
      {
        operator ++();
      }
      return ch;
    }

    inline
    void
    Iterator::check_padding(const char* src) const
      throw (String::StringManip::InvalidFormatException)
    {
      ssize_t pad_size = static_cast<size_t>(src - ptr_) & 3;
      if ((length_ != pad_size && length_ != pad_size + 4) ||
        (strncmp(ptr_, PAD1, length_) && strncmp(ptr_, PAD2, length_) &&
          strncmp(ptr_, PAD3, length_)))
      {
        Stream::Error ostr;
        ostr << FNS << "invalid format. Padding characters expected.";
        throw String::StringManip::InvalidFormatException(ostr);
      }
    }
  }

  const String::AsciiStringManip::CharCategory MIME("A-Za-z0-9_*.,-");
  const String::AsciiStringManip::CharCategory NON_JS(
    "\\\"'/\n\r<>\xE2", true);
  const String::AsciiStringManip::CharCategory NON_JSON(
    "\\\"\n\r\x01-\x1F", true);

  const String::AsciiStringManip::CharCategory C_NON_CSV(",\"\n\r");
  const String::AsciiStringManip::CharCategory PC_NON_CSV(";\"\n\r");

  namespace JS
  {
    inline
    void
    add_unicode_symbol(std::string& dst, wchar_t dest)
      throw (eh::Exception)
    {
      char buf[] = { '\\', 'u',
        String::AsciiStringManip::HEX_DIGITS[(dest >> 12) & 0x0F],
        String::AsciiStringManip::HEX_DIGITS[(dest >> 8) & 0x0F],
        String::AsciiStringManip::HEX_DIGITS[(dest >> 4) & 0x0F],
        String::AsciiStringManip::HEX_DIGITS[dest & 0x0F] };
      dst.append(buf, sizeof(buf));
    }

    inline
    void
    add_wchar(const String::SubString& src, std::string& dst,
      wchar_t ch, bool strict)
      throw (eh::Exception, String::StringManip::InvalidFormatException)
    {
      char buf[16];
      unsigned long count;
      if (String::UTF8Handler::wchar_to_utf8_char(ch, buf, count))
      {
        dst.append(buf, count);
      }
      else
      {
        if (strict)
        {
          Stream::Error ostr;
          ostr << FNS << "invalid symbol in '" << src << "'";
          throw String::StringManip::InvalidFormatException(ostr);
        }
      }
    }

    inline
    void
    add_surrogate(const String::SubString& src, wchar_t surrogate,
      std::string& dest, bool strict)
      throw (eh::Exception, String::StringManip::InvalidFormatException)
    {
      if (strict)
      {
        Stream::Error ostr;
        ostr << FNS << "unpaired surrogate in '" << src << "'";
        throw String::StringManip::InvalidFormatException(ostr);
      }
      add_wchar(src, dest, surrogate, strict);
    }
  }

  namespace XmlEncode
  {
    bool
    init(const void* src, std::string& dst, unsigned long& units)
      throw (eh::Exception)
    {
      if (!src)
      {
        dst.erase();
        return true;
      }

      if (units == 0)
      {
        units = String::StringManip::XU_TEXT |
          String::StringManip::XU_ATTRIBUTE;
      }

      return false;
    }

    inline
    void
    special(std::string& dest, char symbol, unsigned long units)
      throw (eh::Exception)
    {
      switch (symbol)
      {
      case '<':
        dest.append("&lt;", 4);
        return;
      case '>':
        dest.append("&gt;", 4);
        return;
      case '&':
        dest.append("&amp;", 5);
        return;
      default:
        break;
      }

      if (units & String::StringManip::XU_ATTRIBUTE)
      {
        switch (symbol)
        {
        case '\'':
          dest.append("&apos;", 6);
          return;
        case '"':
          dest.append("&quot;", 6);
          return;
        default:
          break;
        }
      }

      dest.push_back(symbol);
    }

    void
    wchar_to_hex(std::string& dest, register unsigned ucs)
    {
      assert(ucs > 0 && ucs <= 0xFFFFFFFFu);
      char buf[11] = "&#x";
      register char* ptr = buf + 3;
      register unsigned mask = 0xF0000000u;
      register unsigned shift = 28;
      while (!(ucs & mask))
      {
        mask >>= 4;
        shift -= 4;
      }
      for (;;)
      {
        if (shift)
        {
          *ptr++ =
            String::AsciiStringManip::HEX_DIGITS[(ucs & mask) >> shift];
        }
        else
        {
          *ptr++ = String::AsciiStringManip::HEX_DIGITS[ucs & mask];
          break;
        }
        mask >>= 4;
        shift -= 4;
      }
      *ptr++ = ';';

      dest.append(buf, ptr - buf);
    }
  }

  namespace XmlDecode
  {
    const String::SubString APOS("apos", 4);
    const String::SubString QUOT("quot", 4);
    const String::SubString LT("lt", 2);
    const String::SubString GT("gt", 2);
    const String::SubString AMP("amp", 3);
  }

  namespace Punycode
  {
    const wchar_t DECODE[] =
    {
      26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 36, 36, 36, 36,
      36, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
      13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 36, 36,
      36, 36, 36, 36,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
      11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    };

    const char ENCODE[] = "abcdefghijklmnopqrstuvwxyz0123456789";

    const wchar_t MAX_WCHAR_T = std::numeric_limits<wchar_t>::max();
    // assert(MAX_WCHAR_T >= 1 << 26)

    wchar_t
    adapt(wchar_t delta, wchar_t numpoints, bool firsttime) throw ()
    {
      delta = firsttime ? delta / 700 : delta >> 1;
      delta += delta / numpoints;

      wchar_t k = 0;
      for (; delta > 455; k += 36)
      {
        delta /= 35;
      }

      return k + 36 * delta / (delta + 38);
    }
  }
}

namespace String
{
  namespace StringManip
  {
    void
    base64_encode(std::string& dst, const void* src, size_t n,
      bool padding) throw (eh::Exception)
    {
      Base64::func_encode(dst, src, n, padding, Base64::std_encode, '=', 0);
    }

    void
    base64mod_encode(std::string& dst, const void* src, size_t n,
      bool padding, uint8_t fill) throw (eh::Exception)
    {
      Base64::func_encode(dst, src, n, padding, Base64::mod_encode,
        Base64::PADDING, fill);
    }

    void
    base64mod_decode(std::string& dest, const SubString& src,
      bool padding, uint8_t* fill)
      throw (InvalidFormatException, eh::Exception)
    {
      Base64::Iterator p(src);

      std::string dst;
      dst.reserve(src.size() * 3 / 4);

      while (p.available())
      {
        register uint8_t c1, c2, c3, c4;

        c1 = p.skip_blanks();
        if (c1 == 0100)
        {
          break;
        }
        ++p;

        c2 = p.skip_blanks();
        if (c2 == 0100)
        {
          Stream::Error ostr;
          ostr << FNS << "invalid format. Unexpected symbol.";
          throw InvalidFormatException(ostr);
        }
        ++p;

        c3 = p.skip_blanks();
        if (c3 == 0100)
        {
          uint8_t left = c2 & 0x0F;
          if (fill)
          {
            *fill = left;
          }
          else
          {
            if (left)
            {
              Stream::Error ostr;
              ostr << FNS << "invalid format. Unexpected end of data.";
              throw InvalidFormatException(ostr);
            }
          }
          dst.push_back(convert(c1 << 2 | c2 >> 4));
          break;
        }
        ++p;

        c4 = p.skip_blanks();

        dst.push_back(convert(c1 << 2 | c2 >> 4));
        dst.push_back(convert(c2 << 4 | c3 >> 2));
        if (c4 == 0100)
        {
          uint8_t left = c3 & 0x03;
          if (fill)
          {
            *fill = left;
          }
          else
          {
            if (left)
            {
              Stream::Error ostr;
              ostr << FNS << "invalid format. Unexpected end of data.";
              throw StringManip::InvalidFormatException(ostr);
            }
          }
          break;
        }
        dst.push_back(convert(c3 << 6 | c4));
        ++p;
      }
      if (padding || p.available())
      {
        p.check_padding(src.data());
      }

      dest.swap(dst);
    }

    void
    mime_url_encode(const SubString& src, std::string& dst)
      throw (eh::Exception)
    {
      std::string dest;
      dest.reserve(src.length() * 3);

      const char* cur = src.begin();
      const char* const END = src.end();

      for (;;)
      {
        const char* ptr = MIME.find_nonowned(cur, END);

        if (ptr != cur)
        {
          dest.append(cur, ptr);
        }

        if (ptr == END)
        {
          break;
        }

        cur = ptr + 1;

        register uint8_t ch = static_cast<uint8_t>(*ptr);

        if (ch == ' ')
        {
          dest.push_back('+');
          continue;
        }

        char buf[] = { '%', AsciiStringManip::HEX_DIGITS[(ch >> 4) & 0x0F],
          AsciiStringManip::HEX_DIGITS[ch & 0x0F] };
        dest.append(buf, sizeof(buf));
      }

      dst.swap(dest);
    }

    void
    mime_url_decode(const SubString& src, std::string& dest,
      bool strict) throw (eh::Exception, InvalidFormatException)
    {
      size_t size = 0, length = src.size();
      std::string dst;
      dst.resize(length);
      for (const char* cur = src.data(); length--; cur++, size++)
      {
        switch (char ch = *cur)
        {
        case '+':
          dst[size] = ' ';
          break;

        case '%':
          if (length < 2 || !AsciiStringManip::HEX_NUMBER(cur[1]) ||
            !AsciiStringManip::HEX_NUMBER(cur[2]))
          {
            if (strict)
            {
              Stream::Error ostr;
              ostr << FNS << "broken encoding in '" << src << "'";
              throw InvalidFormatException(ostr);
            }
          }
          else
          {
            dst[size] = AsciiStringManip::hex_to_char(cur[1], cur[2]);
            cur += 2;
            length -= 2;
            break;
          }
          // FALLTHROUGH

        default:
          dst[size] = ch;
          break;
        }
      }

      dst.resize(size);
      dst.swap(dest);
    }

    void
    mime_url_decode(std::string& text)
      throw (eh::Exception, InvalidFormatException)
    {
      register std::string::size_type size = text.size();
      if (!size)
      {
        return;
      }

      register char* dest = &text[0];
      register const char* src = dest;

      while (size-- > 0)
      {
        switch (*src)
        {
        case '+':
          *dest++ = ' ';
          src++;
          break;

        case '%':
          if (size < 2 || !AsciiStringManip::HEX_NUMBER(src[1]) ||
            !AsciiStringManip::HEX_NUMBER(src[2]))
          {
            Stream::Error ostr;
            ostr << FNS << "broken encoding in '" << src << "'";
            throw InvalidFormatException(ostr);
          }

          *dest++ = AsciiStringManip::hex_to_char(src[1], src[2]);
          src += 3;
          size -= 2;
          break;

        default:
          *dest++ = *src++;
          break;
        }
      }

      text.resize(dest - &text[0]);
    }

    bool
    punycode_encode(const WSubString& input, std::string& output)
      throw (eh::Exception)
    {
      output.clear();
      output.reserve(input.size() * 4);

      for (WSubString::SizeType i = 0; i < input.size(); i++)
      {
        const wchar_t WCH = input[i];
        if (WCH < 0)
        {
          return false;
        }
        if (WCH < 0x80)
        {
          output.push_back(AsciiStringManip::to_lower(
            static_cast<char>(WCH)));
        }
      }

      WSubString::SizeType handled = output.size();

      if (!output.empty())
      {
        output.push_back('-');
      }
  
      bool first = true;
      for (wchar_t not_less_than = 0x80, delta = 0, bias = 72;
        handled < input.size(); delta++, not_less_than++)
      {
        {
          wchar_t least_found = Punycode::MAX_WCHAR_T;
          for (WSubString::SizeType j = 0; j < input.size(); j++)
          {
            if (input[j] >= not_less_than && input[j] < least_found)
            {
              least_found = input[j];
            }
          }

          delta += (least_found - not_less_than) *
            static_cast<wchar_t>(handled + 1);
          not_less_than = least_found;
        }

        for (WSubString::SizeType j = 0; j < input.size(); j++)
        {
          if (input[j] < not_less_than)
          {
            delta++;
          }
          else if (input[j] == not_less_than)
          {
            {
              wchar_t q = delta;
              for (wchar_t k = 36; ; k += 36)
              {
                wchar_t t = k <= bias ? 1 : k >= bias + 26 ? 26 : k - bias;
                if (q < t)
                {
                  break;
                }
                output.push_back(Punycode::ENCODE[t + (q - t) % (36 - t)]);
                q = (q - t) / (36 - t);
              }
              output.push_back(Punycode::ENCODE[q]);
            }

            bias = Punycode::adapt(delta, ++handled, first);
            first = false;
            delta = 0;
          }
        }
      }

      return true;
    }

    bool
    punycode_decode(const SubString& input, std::wstring& output)
      throw (eh::Exception)
    {
      output.clear();
      output.reserve(input.size());

      SubString::SizeType in = input.rfind('-');
      if (in == SubString::NPOS)
      {
        in = 0;
      }
      else
      {
        for (SubString::SizeType j = 0; j < in; j++)
        {
          if (input[j] & 0x80)
          {
            return false;
          }
          output.push_back(input[j]);
        }
        in++;
      }

      wchar_t decoded = 0x80, bias = 72;
      for (std::wstring::size_type out = 0; in < input.size();
        output[out++] = decoded)
      {
        std::wstring::size_type oldout = out;
        for (wchar_t w = 1, k = 36; ; k += 36)
        {
          if (in >= input.size())
          {
            return false;
          }
          wchar_t digit = input[in++];
          digit -= 48;
          digit = digit >= 0 && digit <= 74 ? Punycode::DECODE[digit] : 36;
          if (digit >= 36)
          {
            return false;
          }
          out += digit * w;
          wchar_t t = k <= bias ? 1 : k >= bias + 26 ? 26 : k - bias;
          if (digit < t)
          {
            break;
          }
          w *= 36 - t;
        }

        output.push_back('\0');
        bias = Punycode::adapt(out - oldout, output.size(), oldout == 0);

        decoded += out / output.size();
        out %= output.size();

        memmove(&output[out + 1], &output[out],
          (output.size() - out) * sizeof(wchar_t));
      }

      return true;
    }

    std::string&
    csv_encode(const char* src, std::string& dst, char separator)
      throw (eh::Exception)
    {
      if (!src)
      {
        dst.clear();
        return dst;
      }
      // fields that contain separator, double-quotes,
      //     or line-breaks must be quoted.
      // a quote within a field must be escaped with
      //    an additional quote immediately preceding the literal quote.
      char non_csv_chars[] = {separator , '"', '\n', '\r', 0};
      const AsciiStringManip::CharCategory& non_csv =
        separator == ',' ? C_NON_CSV :
        separator == ';' ? PC_NON_CSV :
        AsciiStringManip::CharCategory(
          static_cast<const char*>(non_csv_chars));
      if (!non_csv.find_owned(src))
      {
        dst = src;
        return dst;
      }
      std::string dest;
      dest.reserve(1024);
      dest.push_back('"');

      const char* ptr = src;
      for (; *ptr; ++ptr)
      {
        if (*ptr == '"')
        {
          dest.append(src, ptr);
          dest.append(2, '"');
          src = ptr + 1;
        }
      }
      if (*src)
      {
        dest.append(src, ptr);
      }

      dest.push_back('"');
      dst.swap(dest);
      return dst;
    }

    Generics::ArrayWChar
    utf8_to_wchar(const SubString& src)
      throw (eh::Exception, InvalidFormatException)
    {
      size_t size = src.size();
      Generics::ArrayWChar dst(size + 1);

      int pos = 0;
      for (const char* cur = src.data(); size;)
      {
        unsigned long length = UTF8Handler::get_octet_count(*cur);
        if (size < length)
        {
          Stream::Error ostr;
          ostr << FNS << "Incomplete octet sequence in UTF-8 string '" <<
            src << "'";
          throw InvalidFormatException(ostr);
        }

        wchar_t wch;
        if (!length || !UTF8Handler::utf8_char_to_wchar(cur, length, wch))
        {
          Stream::Error ostr;
          ostr << FNS << "Invalid octet in UTF-8 string '" << src << "'";
          throw InvalidFormatException(ostr);
        }

        dst[pos++] = wch;
        cur += length;
        size -= length;
      }

      dst[pos] = L'\0';

      return dst;
    }

    void
    xml_encode(const wchar_t* src, std::string& dst, unsigned long units)
      throw (eh::Exception)
    {
      if (XmlEncode::init(src, dst, units))
      {
        return;
      }

      std::string dest;
      dest.reserve(dst.size() * 12);

      for (register wchar_t current; (current = *src) != L'\0'; src++)
      {
        const register char l_byte =
          convert(static_cast<unsigned char>(current));

        if ((current >> 8) == 0 && l_byte >= 0x20 && l_byte <= 0x7E)
        {
          XmlEncode::special(dest, l_byte, units);
        }
        else
        {
          if (units & XU_PRESERVE_UTF8)
          {
            wchar_to_utf8(current, dest);
          }
          else
          {
            XmlEncode::wchar_to_hex(dest, static_cast<unsigned>(current));
          }
        }
      }

      dest.swap(dst);
    }

    void
    xml_encode(const char* src, std::string& dst, unsigned long units)
      throw (InvalidFormatException, eh::Exception)
    {
      if (XmlEncode::init(src, dst, units))
      {
        return;
      }

      std::string dest;
      dest.reserve(dst.size() * 6);

      unsigned long octets_count;
      for (register char current; (current = *src) != '\0';
        src += octets_count)
      {
        if (!UTF8Handler::is_correct_utf8_sequence(src, octets_count))
        {
          Stream::Error ostr;
          ostr << FNS << "Invalid source UTF-8 string: '" << src << "'";
          throw InvalidFormatException(ostr);
        }

        if (octets_count == 1 && current >= 0x20 && current <= 0x7E)
        {
          XmlEncode::special(dest, current, units);
        }
        else
        {
          if (units & XU_PRESERVE_UTF8)
          {
            dest.append(src, octets_count);
          }
          else
          {
            wchar_t ucs;
            UTF8Handler::utf8_char_to_wchar(src, octets_count, ucs);
            XmlEncode::wchar_to_hex(dest, static_cast<unsigned>(ucs));
          }
        }
      }

      dest.swap(dst);
    }

    void
    xml_decode(const SubString& src, std::string& dest)
      throw (InvalidFormatException, eh::Exception)
    {
      std::string dst;
      dst.reserve(src.size());

      for (SubString::SizeType cur = 0; ; cur++)
      {
        SubString::SizeType found = src.find('&', cur);
        if (found == SubString::NPOS)
        {
          src.substr(cur).append_to(dst);
          break;
        }
        if (found != cur)
        {
          src.substr(cur, found - cur).append_to(dst);
        }

        cur = src.find(';', ++found);
        if (cur == SubString::NPOS)
        {
          Stream::Error ostr;
          ostr << FNS << "absent semicolon";
          throw InvalidFormatException(ostr);
        }
        if (cur == found)
        {
          Stream::Error ostr;
          ostr << FNS << "empty sequence";
          throw InvalidFormatException(ostr);
        }

        SubString tag(src.substr(found, cur - found));
        switch (tag[0])
        {
        case '#':
          if (src[++found] == 'x')
          {
            if (++found == cur)
            {
              break;
            }
            while (src[found] == '0')
            {
              if (++found == cur)
              {
                break;
              }
            }
            if (found == cur)
            {
              dst.push_back('\0');
              continue;
            }
            size_t length = cur - found;
            if (length > 6)
            {
              Stream::Error ostr;
              ostr << FNS << "invalid char code '" << tag << "'";
              throw InvalidFormatException(ostr);
            }
            wchar_t value = 0;
            for (; found != cur; found++)
            {
              if (!AsciiStringManip::HEX_NUMBER(src[found]))
              {
                Stream::Error ostr;
                ostr << FNS << "invalid char code '" << tag << "'";
                throw InvalidFormatException(ostr);
              }
              value = (value << 4) |
                AsciiStringManip::hex_to_int(src[found]);
            }
            char buf[7];
            unsigned long octets_count;
            if (!UTF8Handler::wchar_to_utf8_char(value, buf, octets_count))
            {
              Stream::Error ostr;
              ostr << FNS << "invalid char code '" << tag << "'";
              throw InvalidFormatException(ostr);
            }
            dst.append(buf, octets_count);
            continue;
          }
          break;

        case 'a':
          if (tag == XmlDecode::AMP)
          {
            dst.push_back('&');
            continue;
          }
          if (tag == XmlDecode::APOS)
          {
            dst.push_back('\'');
            continue;
          }
          break;

        case 'g':
          if (tag == XmlDecode::GT)
          {
            dst.push_back('>');
            continue;
          }
          break;

        case 'l':
          if (tag == XmlDecode::LT)
          {
            dst.push_back('<');
            continue;
          }
          break;

        case 'q':
          if (tag == XmlDecode::QUOT)
          {
            dst.push_back('"');
            continue;
          }
          break;

        default:
          break;
        }

        Stream::Error ostr;
        ostr << FNS << "unknown sequence '" << tag << "'";
        throw InvalidFormatException(ostr);
      }
      dest.swap(dst);
    }

    void
    js_unicode_encode(const char* src, std::string& dest)
      throw (InvalidFormatException, eh::Exception)
    {
      if (!*src)
      {
        dest.erase();
        return;
      }

      std::string dst;

      for (;;)
      {
        // '\0' is non owned
        const char* ptr = AsciiStringManip::ALPHA_NUM.find_nonowned(src);

        if (ptr != src)
        {
          dst.append(src, ptr);
        }

        if (!*ptr)
        {
          break;
        }

        unsigned long octets_count;
        wchar_t dest;
        if (!(octets_count = UTF8Handler::get_octet_count(*ptr)))
        {
          Stream::Error ostr;
          ostr << FNS << "found non-unicode symbol " << ptr;
          throw InvalidFormatException(ostr);
        }

        UTF8Handler::utf8_char_to_wchar(ptr, octets_count, dest);
        if (dest < 0x10000)
        {
          JS::add_unicode_symbol(dst, dest);
        }
        else
        {
          JS::add_unicode_symbol(dst, 0xD7C0 + (dest >> 10));
          JS::add_unicode_symbol(dst, 0xDC00 + (dest & 0x3FF));
        }
        src = ptr + octets_count;
      }

      dest.swap(dst);
    }

    void
    js_unicode_decode(const SubString& src, std::string& dest,
      bool strict, char special)
      throw (InvalidFormatException, eh::Exception)
    {
      dest.reserve(src.size());

      wchar_t last_surrogate = 0;
      for (size_t i = 0; i < src.size(); i++)
      {
        if (src[i] == special)
        {
          if (src.size() - i >= 5 && src[i + 1] == 'u')
          {
            wchar_t ch = 0;
            for (int j = 2; j < 6; j++)
            {
              ch <<= 4;
              ch |= AsciiStringManip::hex_to_int(src[i + j]);
            }
            i += 5;
            if (last_surrogate)
            {
              if (ch >= 0xDC00 && ch < 0xE000)
              {
                ch = ((last_surrogate - 0xD7C0) << 10) | (ch - 0xDC00);
              }
              else
              {
                JS::add_surrogate(src, last_surrogate, dest, strict);
              }
              last_surrogate = 0;
            }
            else
            {
              if (ch >= 0xD700 && ch < 0xDC00)
              {
                last_surrogate = ch;
                continue;
              }
              else if (ch >= 0xDC00 && ch < 0xE000)
              {
                JS::add_surrogate(src, ch, dest, strict);
                continue;
              }
            }

            JS::add_wchar(src, dest, ch, strict);
            continue;
          }
          else
          {
            if (strict)
            {
              Stream::Error ostr;
              ostr << FNS << "broken encoding in '" << src << "'";
              throw InvalidFormatException(ostr);
            }
          }
        }

        if (last_surrogate)
        {
          JS::add_surrogate(src, last_surrogate, dest, strict);
          last_surrogate = 0;
        }

        dest.push_back(src[i]);
      }

      if (last_surrogate)
      {
        JS::add_surrogate(src, last_surrogate, dest, strict);
      }
    }

    std::string
    json_escape(const SubString& src) throw (eh::Exception)
    {
      static const SubString REPL[] =
      {
        SubString("\\u0000", 6),
        SubString("\\u0001", 6),
        SubString("\\u0002", 6),
        SubString("\\u0003", 6),
        SubString("\\u0004", 6),
        SubString("\\u0005", 6),
        SubString("\\u0006", 6),
        SubString("\\u0007", 6),
        SubString("\\b", 2),
        SubString("\\t", 2),
        SubString("\\n", 2),
        SubString("\\u000B", 6),
        SubString("\\f", 2),
        SubString("\\r", 2),
        SubString("\\u000E", 6),
        SubString("\\u000F", 6),
        SubString("\\u0010", 6),
        SubString("\\u0011", 6),
        SubString("\\u0012", 6),
        SubString("\\u0013", 6),
        SubString("\\u0014", 6),
        SubString("\\u0015", 6),
        SubString("\\u0016", 6),
        SubString("\\u0017", 6),
        SubString("\\u0018", 6),
        SubString("\\u0019", 6),
        SubString("\\u001A", 6),
        SubString("\\u001B", 6),
        SubString("\\u001C", 6),
        SubString("\\u001D", 6),
        SubString("\\u001E", 6),
        SubString("\\u001F", 6),
        SubString(),
        SubString(),
        SubString("\\\"", 2)
      };

      std::string dest;
      dest.reserve(src.size() * 6);

      const char* cur = src.begin();
      const char* const END = src.end();

      for (;;)
      {
        const char* ptr = NON_JSON.find_owned(cur, END);

        if (ptr != cur)
        {
          dest.append(cur, ptr);
        }

        if (ptr == END)
        {
          break;
        }

        cur = ptr + 1;

        register char ch = *ptr;

        if (ch == '\\')
        {
          dest.append("\\\\", 2);
        }
        else
        {
          REPL[static_cast<uint8_t>(ch)].append_to(dest);
        }
      }

      return dest;
    }

    void
    js_encode(const char* src, std::string& dest) throw (eh::Exception)
    {
      std::string dst;
      const char* from = src;

      for (;;)
      {
        // '\0' is owned
        const char* ptr = NON_JS.find_owned(from);

        if (ptr != src)
        {
          dst.append(src, ptr);
          src = ptr;
        }

        register uint8_t ch = *ptr;

        if (!ch)
        {
          break;
        }

        if (ch == 0xE2)
        {
          if (ptr[1] == '\x80')
          {
            const char* buf = "\\u2028";
            switch (ptr[2])
            {
            case '\xA9':
              buf = "\\u2029";
              // FALLTHROUGN
            case '\xA8':
              dst.append(buf, 6);
              from = src += 3;
              continue;
            default:
              break;
            }
          }
          from = ptr + 1;
          continue;
        }

        from = ++src;

        char buf[] = { '\\', 'x',
          AsciiStringManip::HEX_DIGITS[(ch >> 4) & 0x0F],
          AsciiStringManip::HEX_DIGITS[ch & 0x0F] };
        dst.append(buf, sizeof(buf));
      }

      dest = std::move(dst);
    }

    void
    wchar_to_utf8(const wchar_t* src, std::string& utf8_res)
      throw (eh::Exception)
    {
      utf8_res.clear();

      if (!src)
      {
        return;
      }

      for (register wchar_t current; (current = *src++) != L'\0';
        wchar_to_utf8(current, utf8_res))
      {
      }
    }

    void
    wchar_to_utf8(const String::WSubString& src, std::string& utf8_res)
      throw (eh::Exception)
    {
      utf8_res.clear();
      utf8_res.reserve(src.size() * 4);

      for (String::WSubString::ConstPointer itor(src.begin());
        itor != src.end(); ++itor)
      {
        wchar_to_utf8(*itor, utf8_res);
      }
    }

    bool
    utf8_substr(const String::SubString& src, size_t max_octets,
      String::SubString& dst) throw ()
    {
      size_t length = 0;
      for (size_t octets; max_octets && length < src.size();
        max_octets -= octets)
      {
        octets = UTF8Handler::get_octet_count(src[length]);
        if (octets > max_octets)
        {
          break;
        }
        if (!octets || length + octets > src.size())
        {
          return false;
        }
        length++;
        for (size_t left = octets; --left;)
        {
          if ((src[length++] & 0xC0) != 0x80)
          {
            return false;
          }
        }
      }
      dst = src.substr(0, length);
      return true;
    }

    void
    trim(SubString& str, const AsciiStringManip::CharCategory& trim_set)
      throw ()
    {
      register const char* end = str.end();
      register const char* begin =
        trim_set.find_nonowned(str.begin(), end);
      if (begin != end)
      {
        // We have at least one non-space character at str.begin
        while (trim_set(*--end));
        ++end;
      }
      str.assign(begin, end - begin);
    }

    String::SubString
    trim_ret(SubString str, const AsciiStringManip::CharCategory& trim_set)
      throw ()
    {
      trim(str, trim_set);
      return str;
    }

    void
    trim(const SubString& str, std::string& dest,
      const AsciiStringManip::CharCategory& trim_set) throw (eh::Exception)
    {
      trim_ret(str, trim_set).str().swap(dest);
    }

    bool
    flatten(std::string& dest, const String::SubString& str,
      const SubString& replacement, const Utf8Category& to_replace)
      throw (eh::Exception)
    {
      const char* const REPLACEMENT_DATA = replacement.data();
      const size_t REPLACEMENT_SIZE = replacement.size();
      const char* const REPLACEMENT_END =
        REPLACEMENT_DATA + REPLACEMENT_SIZE;
      dest.resize(str.size() * (REPLACEMENT_SIZE ? REPLACEMENT_SIZE : 1));
      char* out = &dest[0];
      const char* current;

      for (const char* first = str.begin(), * const LAST = str.end();
        first != LAST;)
      {
        current = to_replace.find_owned(first, LAST);
        if (current == 0)
        {
          return false;
        }
        // last if haven't spaces
        // copy text before space
        out = std::copy(first, current, out);
        if (current == LAST)
        {
          break;
        }
        out = std::copy(REPLACEMENT_DATA, REPLACEMENT_END, out);
        first = to_replace.find_nonowned(current, LAST);
        if (!first)
        {
          return false;
        }
      }
      dest.resize(out - &dest[0]);
      return true;
    }

    //
    // mark function
    //

    void
    mark(const char* src, std::string& dst,
      const AsciiStringManip::CharCategory& predicate, char marker)
      throw (eh::Exception)
    {
      if (!src)
      {
        dst.clear();
        return;
      }
      std::string dest;

      for (;;)
      {
        const char* ptr = predicate.find_owned(src);
        if (!ptr)
        {
          dest.append(src);
          break;
        }

        register char ch = *ptr;

        if (ptr != src)
        {
          dest.append(src, ptr);
        }

        dest.push_back(marker);
        dest.push_back(ch);
        if (!ch)  // this check need for predicates that contain '\0'
        {
          break;
        }
        src = ptr + 1;
      }
      dst.swap(dest);
    }

    void
    replace(const String::SubString& str, std::string& dst,
      const String::SubString& to_find, const String::SubString& to_replace)
      throw (eh::Exception)
    {
      std::string dest;

      if (to_find.empty())
      {
        str.assign_to(dest);
        dst.swap(dest);
        return;
      }

      dest.reserve(to_find.size() < to_replace.length() ?
        (str.size() / to_find.size() + 1) * to_replace.length() :
        str.size());

      for (String::SubString::SizeType last = 0;;)
      {
        String::SubString::SizeType pos = str.find(to_find, last);
        if (pos != last)
        {
          str.substr(last, pos - last).append_to(dest);
        }
        if (pos == String::SubString::NPOS)
        {
          break;
        }
        to_replace.append_to(dest);
        last = pos + to_find.size();
      }

      dst.swap(dest);
    }

    std::string
    hex_encode(const unsigned char* data, size_t size,
      bool skip_leading_zeroes) throw (eh::Exception)
    {
      if (!size)
      {
        return std::string();
      }

      if (skip_leading_zeroes)
      {
        while (!*data)
        {
          data++;
          if (!--size)
          {
            return std::string(1, '0');
          }
        }
      }

      std::string result;
      result.reserve(size * 2);
      if (skip_leading_zeroes && !((*data) & 0xF0))
      {
        result.push_back(AsciiStringManip::HEX_DIGITS[*data]);
        data++;
        size--;
      }
      for (; size--; data++)
      {
        char buf[2] =
          {
            AsciiStringManip::HEX_DIGITS[(*data) >> 4],
            AsciiStringManip::HEX_DIGITS[(*data) & 0xF]
          };
        result.append(buf, 2);
      }
      return result;
    }

    size_t
    hex_decode(SubString src, Generics::ArrayByte& dst,
      bool allow_odd_string)
      throw (eh::Exception, InvalidFormatException)
    {
      bool odd_string = src.size() & 1;
      if (odd_string && !allow_odd_string)
      {
        Stream::Error ostr;
        ostr << FNS << "odd length of hex string";
        throw InvalidFormatException(ostr);
      }
      size_t size = (src.size() + 1) / 2;
      dst.reset(size);
      char* data = reinterpret_cast<char*>(dst.get());
      if (odd_string)
      {
        *data++ = AsciiStringManip::hex_to_int(src[0]);
        src = src.substr(1);
      }
      AsciiStringManip::hex_to_buf(src, data);
      return size;
    }

    //
    // Translit class
    //

    Translit::Translit(const char* invalid, const char* valid)
      throw ()
    {
      for (int i = 0; i < 256; i++)
      {
        replacement_[i] = convert(i);
      }

      if (!invalid || !valid)
      {
        return;
      }

      size_t valid_length = strlen(valid);
      if (!valid_length)
      {
        return;
      }

      for (int i = 0; invalid[i]; i++)
      {
        replacement_[static_cast<uint8_t>(invalid[i])] =
          valid[i % valid_length];
      }
    }

    void
    Translit::translit(const char* src, std::string& dst) const
      throw (eh::Exception)
    {
      std::string dest;

      for (; *src; src++)
      {
        dest.push_back(replacement_[static_cast<uint8_t>(*src)]);
      }

      dst.swap(dest);
    }

    void
    Translit::translit(const SubString& src, std::string& dst) const
      throw (eh::Exception)
    {
      std::string dest;

      dest.reserve(src.size());

      for (SubString::ConstPointer itor(src.begin());
        itor != src.end(); ++itor)
      {
        dest.push_back(replacement_[static_cast<uint8_t>(*itor)]);
      }

      dst.swap(dest);
    }

    void
    Translit::translit(char* str) const
      throw ()
    {
      for (; *str; str++)
      {
        *str = replacement_[static_cast<uint8_t>(*str)];
      }
    }

    void
    Translit::translit(char* str, size_t size) const
      throw ()
    {
      for (; size-- > 0; str++)
      {
        *str = replacement_[static_cast<uint8_t>(*str)];
      }
    }
  }
}
