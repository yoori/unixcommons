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



// File   : UrlAddress.cpp
// Author : Karen Aroutiounov

#include <cassert>

#include <String/StringManip.hpp>
#include <String/UTF8Category.hpp>
#include <String/UTF8AllProperties.hpp>
#include <String/UnicodeNormalizer.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>

#include <HTTP/UrlAddress.hpp>


namespace
{
  using namespace HTTP;

  const String::AsciiStringManip::Caseless WWW("www.");

  // The following are taken from RFC 3986
  // Appendix A
  const String::AsciiStringManip::CharCategory UNRESERVED(
    String::AsciiStringManip::ALPHA_NUM,
    String::AsciiStringManip::CharCategory("-._~"));
  const String::AsciiStringManip::CharCategory SUB_DELIMS("!$&'()*+,;=");
  const String::AsciiStringManip::CharCategory PCHAR(
    UNRESERVED,
    SUB_DELIMS,
    String::AsciiStringManip::CharCategory(":@"));

  // These are not RFC-compliant symbols but apache works with them
  const String::AsciiStringManip::CharCategory NON_COMPLIANT("{}|^~[]`");

  // Url splitting
  const String::AsciiStringManip::CharCategory
    URL_PARSER_SCHEME_END(":/?#");
  typedef const String::AsciiStringManip::Char3Category<'/', '?', '#'>
    UrlParserAuthorityEnd;
  UrlParserAuthorityEnd URL_PARSER_AUTORITY_END{};
  typedef const String::AsciiStringManip::Char2Category<'?', '#'>
    UrlParserPathEnd;
  UrlParserPathEnd URL_PARSER_PATH_END{};
  typedef const String::AsciiStringManip::Char1Category<'#'>
    UrlParserQueryEnd;
  UrlParserQueryEnd URL_PARSER_QUERY_END{};

  // Part 3.1
  const String::AsciiStringManip::CharCategory& SCHEME_FIRST(
    String::AsciiStringManip::ALPHA);
  const String::AsciiStringManip::CharCategory SCHEME_NOT_FIRST(
    String::AsciiStringManip::ALPHA_NUM,
    String::AsciiStringManip::CharCategory("-+."));

  // Part 3.2.1
  const String::AsciiStringManip::CharCategory USER_INFO(
    UNRESERVED,
    SUB_DELIMS,
    String::AsciiStringManip::CharCategory(":"));

  // Part 3.2.2 is too wide for DNS, using special per-label checks
  const String::AsciiStringManip::CharCategory HOST(
    UNRESERVED,
    SUB_DELIMS);

  // Part 3.2.3
  const String::AsciiStringManip::CharCategory& PORT(
    String::AsciiStringManip::NUMBER);

  // Part 3.3 (Simplified)
  const String::AsciiStringManip::CharCategory PATH(
    NON_COMPLIANT,
    PCHAR,
    String::AsciiStringManip::CharCategory("/"));

  // Part 3.4
  const String::AsciiStringManip::CharCategory QUERY(
    NON_COMPLIANT,
    PCHAR,
    String::AsciiStringManip::CharCategory("/?"));

  // Part 3.5
  const String::AsciiStringManip::CharCategory FRAGMENT(
    NON_COMPLIANT,
    PCHAR,
    String::AsciiStringManip::CharCategory("/?"));


  const char SCHEME_SUFFIX = ':';
  const std::size_t SCHEME_SUFFIX_SIZE = 1;
  const char AUTHORITY_PREFIX[] = "//";
  const std::size_t AUTHORITY_PREFIX_SIZE = 2;
  const char USERINFO_SEPARATOR = '@';
  const std::size_t USERINFO_SEPARATOR_SIZE = 1;
  const char PORT_SEPARATOR = ':';
  const std::size_t PORT_SEPARATOR_SIZE = 1;
  const char QUERY_SEPARATOR = '?';
  const std::size_t QUERY_SEPARATOR_SIZE = 1;
  const char FRAGMENT_SEPARATOR = '#';
  const std::size_t FRAGMENT_SEPARATOR_SIZE = 1;
  // all address separators size, separators = "://@:?#"
  const std::size_t ALL_SEPS_SIZE = 7;

  const unsigned short DEFAULT_HTTP_PORT = 80;
  const unsigned short DEFAULT_HTTPS_PORT = 443;
  const String::SubString DEFAULT_PATH("/", 1);
  const char PATH_SEPARATOR = '/';
  const std::size_t PATH_SEPARATOR_SIZE = 1;
  const String::SubString SCHEME_AUTHORITY_MEDIATOR("://", 3);

  // RFC 1034
  const size_t MAX_HOSTNAME_LABEL_SIZE = 63;
  const size_t MAX_HOSTNAME_SIZE = 255;

  const char LABEL_SEPARATOR = '.';
  typedef const String::AsciiStringManip::Char1Category<LABEL_SEPARATOR>
    LabelSeparatorCategory;

  // 3.5 of RFC1034 and 2.1 of RFC1123.
  // Also non-standard underscore is included.
  const String::AsciiStringManip::CharCategory LABEL_FIRST_LAST(
    String::AsciiStringManip::ALPHA_NUM,
    String::AsciiStringManip::CharCategory("_"));
  const String::AsciiStringManip::CharCategory LABEL_MIDDLE(
    String::AsciiStringManip::ALPHA_NUM,
    String::AsciiStringManip::CharCategory("-_"));


  inline
  bool
  is_valid_chars(const String::SubString& str,
    const String::AsciiStringManip::CharCategory& category) throw ()
  {
    const char* const END = str.end();
    return category.find_nonowned(str.begin(), END) == END;
  }

  inline
  const char*
  find_invalid(const char* str, const char* end,
    const String::AsciiStringManip::CharCategory& category) throw ()
  {
    while ((str = category.find_nonowned(str, end)) && str != end)
    {
      if (*str != '%')
      {
        return str;
      }
      if (end - str < 3 ||
        !String::AsciiStringManip::HEX_NUMBER(str[1]) ||
        !String::AsciiStringManip::HEX_NUMBER(str[2]))
      {
        return str;
      }
      str += 3;
    }
    return end;
  }

  bool
  is_valid_encoded(const String::SubString& encoded_str,
    const String::AsciiStringManip::CharCategory& category) throw ()
  {
    const char* const END = encoded_str.end();
    return find_invalid(encoded_str.begin(), END, category) == END;
  }

  /**
   * Create an error message.
   */
  bool
  make_invalid(std::string& error, const char* type,
    const String::SubString& url) throw (eh::Exception)
  {
    error = std::string("invalid ") + type + " '";
    url.append_to(error);
    error.push_back('\'');
    return false;
  }

  bool
  check_http_url_components(const String::SubString& url,
    const String::SubString& scheme, const String::SubString& host,
    std::string& error, bool strict) throw (eh::Exception)
  {
    if (scheme != HTTP_SCHEME && scheme != HTTPS_SCHEME &&
      (strict || !scheme.empty()))
    {
      return make_invalid(error, "unexpected protocol in url", url);
    }

    if (host.empty())
    {
      return make_invalid(error, "empty server name in url", url);
    }

    return true;
  }

  bool
  http_url_needs_prefix(const String::SubString& scheme,
    const String::SubString& host) throw (eh::Exception)
  {
    return host.empty() && scheme != HTTP_SCHEME && scheme != HTTPS_SCHEME;
  }

  void
  http_add_scheme(std::string& fixed_url, const String::SubString& url)
    throw (eh::Exception)
  {
    fixed_url.reserve(HTTP_SCHEME.str.size() +
      SCHEME_AUTHORITY_MEDIATOR.size() + url.size());
    HTTP_SCHEME.str.append_to(fixed_url);
    SCHEME_AUTHORITY_MEDIATOR.append_to(fixed_url);
    url.append_to(fixed_url);
  }

  bool
  http_fix_part(const String::SubString& part,
    const String::AsciiStringManip::CharCategory& checker,
    std::string& new_part) throw (eh::Exception)
  {
    const char* const END = part.end();
    const char* str = find_invalid(part.begin(), END,
      checker);
    if (str == END)
    {
      return false;
    }
    new_part.reserve(part.length() * 3);
    new_part.append(part.begin(), str);
    do
    {
      const char CH = *str;
      const char buf[] = { '%',
        String::AsciiStringManip::HEX_DIGITS[(CH >> 4) & 0x0F],
        String::AsciiStringManip::HEX_DIGITS[CH & 0x0F] };
      new_part.append(buf, sizeof(buf));
      const char* const OLD = str + 1;
      str = find_invalid(OLD, END, QUERY);
      new_part.append(OLD, str);
    }
    while (str != END);
    return true;
  }

  const String::AsciiStringManip::Caseless IDNA_PREFIX("xn--");
  const char IDNA_DELIMITER = '-';
  const String::AsciiStringManip::CharCategory IDNA_ALLOWED(
    String::AsciiStringManip::ALPHA_NUM,
    String::AsciiStringManip::CharCategory("-"));

  struct PartCheckInfo
  {
    PartCheckInfo(String::SubString& part,
      const String::AsciiStringManip::CharCategory& checker)
      throw (eh::Exception);

    String::SubString& part;
    const String::AsciiStringManip::CharCategory& CHECKER;
    std::string new_part;
  };

  PartCheckInfo::PartCheckInfo(String::SubString& part,
    const String::AsciiStringManip::CharCategory& checker)
    throw (eh::Exception)
    : part(part), CHECKER(checker)
  {
  }

  void
  unmime(const String::SubString& str,
    const String::AsciiStringManip::CharCategory& valid,
    std::string& result) throw (eh::Exception)
  {
    result.clear();
    result.reserve(str.length());
    for (String::SubString::ConstPointer itor(str.begin());
      itor != str.end(); ++itor)
    {
      if (*itor == '%')
      {
        ++itor;
        assert(itor != str.end());
        assert(String::AsciiStringManip::HEX_NUMBER(*itor));
        assert(str.end() - itor >= 2);
        ++itor;
        char ch = String::AsciiStringManip::hex_to_char(itor[-1], *itor);
        if (valid(ch))
        {
          result.push_back(String::AsciiStringManip::to_lower(ch));
        }
        else
        {
          char buf[3] = { '%',
            String::AsciiStringManip::to_lower(itor[-1]),
            String::AsciiStringManip::to_lower(*itor) };
          result.append(buf, 3);
        }
      }
      else
      {
        result.push_back(String::AsciiStringManip::to_lower(*itor));
      }
    }
  }

  void
  unmime_all(const String::SubString& src, std::string& dst)
    throw (eh::Exception)
  {
    dst.clear();
    dst.reserve(src.size());
    for (String::SubString::ConstPointer it = src.begin();
      it != src.end(); it++)
    {
      if (*it == '%' && src.end() - it >= 3 &&
        String::AsciiStringManip::HEX_NUMBER(it[1]) &&
        String::AsciiStringManip::HEX_NUMBER(it[2]))
      {
        dst.push_back(String::AsciiStringManip::hex_to_char(it[1], it[2]));
        it += 2;
      }
      else
      {
        dst.push_back(*it);
      }
    }
  }


  class IDNA0 : private Generics::Uncopyable
  {
  public:
    explicit
    IDNA0(std::string& ascii) throw ();

    ~IDNA0() throw ();

    void
    append(const String::SubString& label)
      throw (BrowserAddress::IDNAError);

  private:
    std::string& ascii_;
  };

  IDNA0::IDNA0(std::string& ascii) throw ()
    : ascii_(ascii)
  {
  }

  IDNA0::~IDNA0() throw ()
  {
    String::AsciiStringManip::to_lower(ascii_);
  }

  void
  IDNA0::append(const String::SubString& label)
    throw (BrowserAddress::IDNAError)
  {
    if (IDNA_PREFIX.start(label))
    {
      throw HTTP::BrowserAddress::IDNAError(
        "Possibly IDNA label");
    }
  }

  class IDNA2008 : private Generics::Uncopyable
  {
  public:
    IDNA2008(std::string& ascii, std::string& unicode) throw ();

    void
    append(const String::WSubString& label)
      throw (eh::Exception, BrowserAddress::IDNAError);

  private:
    bool
    decode_(const String::WSubString& lab, std::string& alabel,
      std::wstring& decoded, String::WSubString& wlab, bool& unicode)
      throw (eh::Exception, BrowserAddress::IDNAError);
    bool
    encode_(const String::WSubString& wlabel, bool unicode)
      throw (eh::Exception, BrowserAddress::IDNAError);

    std::string& ascii_;
    std::string& unicode_;
  };

  IDNA2008::IDNA2008(std::string& ascii, std::string& unicode) throw ()
    : ascii_(ascii), unicode_(unicode)
  {
  }

  bool
  IDNA2008::decode_(const String::WSubString& lab, std::string& alabel,
    std::wstring& decoded, String::WSubString& wlab, bool& unicode)
    throw (eh::Exception, BrowserAddress::IDNAError)
  {
    unicode = false;
    alabel.reserve(lab.size() + 1);
    for (String::WSubString::SizeType i = 0; i < lab.size(); i++)
    {
      if (lab[i] >= 0x80)
      {
        unicode = true;
        break;
      }
      alabel.push_back(static_cast<char>(lab[i]));
    }

    if (unicode)
    {
      wlab = lab;
    }
    else
    {
      if (!is_valid_chars(alabel, HOST))
      {
        Stream::Error ostr;
        ostr << "Invalid input sequence in label '" << alabel << "'";
        throw BrowserAddress::IDNAError(ostr);
      }

      if (!IDNA_PREFIX.start(alabel) ||
        alabel.size() == IDNA_PREFIX.str.size() ||
        *IDNA_ALLOWED.find_nonowned(alabel.c_str()) ||
        !String::StringManip::punycode_decode(
          String::SubString(alabel).substr(IDNA_PREFIX.str.size()),
            decoded))
      {
        return true;
      }

      std::wstring normalized;
      if (!String::lower_and_normalize(decoded, normalized, false) ||
        normalized.empty())
      {
        return true;
      }

      decoded = std::move(normalized);
      wlab = decoded;
    }

    return false;
  }

  bool
  IDNA2008::encode_(const String::WSubString& wlabel, bool unicode)
    throw (eh::Exception, BrowserAddress::IDNAError)
  {
    if (wlabel[0] == IDNA_DELIMITER ||
      wlabel[wlabel.size() - 1] == IDNA_DELIMITER)
    {
      if (unicode)
      {
        throw BrowserAddress::IDNAError("extra hyphens");
      }
      return false;
    }

    std::string encoded;
    if (!String::StringManip::punycode_encode(wlabel, encoded))
    {
      if (unicode)
      {
        throw BrowserAddress::IDNAError("punycode failure");
      }
    }

    if (*IDNA_ALLOWED.find_nonowned(encoded.c_str()))
    {
      if (unicode)
      {
        throw BrowserAddress::IDNAError(
          "Invalid symbols in the encoded label");
      }
      return false;
    }

    if (encoded[encoded.size() - 1] == IDNA_DELIMITER)
    {
      if (unicode)
      {
        throw BrowserAddress::IDNAError(
          "Extra hyphens in the encoded label");
      }
      return false;
    }

    for (String::WSubString::ConstPointer itor(wlabel.begin());
      itor != wlabel.end(); ++itor)
    {
      char buf[16];
      unsigned long octets_count;
      if (!String::UTF8Handler::ulong_to_utf8_char(*itor, buf,
        octets_count))
      {
        if (unicode)
        {
          throw BrowserAddress::IDNAError("Invalid input sequence");
        }
        return false;
      }
      unicode_.append(buf, octets_count);
    }
    unicode_.push_back(LABEL_SEPARATOR);

    IDNA_PREFIX.str.append_to(ascii_);
    ascii_.append(encoded);
    ascii_.push_back(LABEL_SEPARATOR);

    return true;
  }

  void
  IDNA2008::append(const String::WSubString& label)
    throw (eh::Exception, BrowserAddress::IDNAError)
  {
    std::string alabel;
    std::wstring decoded;
    String::WSubString wlabel;
    bool unicode;

    do
    {
      if (decode_(label, alabel, decoded, wlabel, unicode))
      {
        break;
      }

      bool has_nonascii = false;
      for (String::WSubString::ConstPointer itor(wlabel.begin());
        itor != wlabel.end(); ++itor)
      {
        if (*itor >= 0x80)
        {
          has_nonascii = true;
          break;
        }
      }

      if (has_nonascii)
      {
        if (!encode_(wlabel, unicode))
        {
          break;
        }
      }
      else
      {
        std::string adecoded;
        adecoded.reserve(wlabel.size() + 1);
        for (String::WSubString::SizeType i = 0; i < wlabel.size(); i++)
        {
          adecoded.push_back(static_cast<char>(wlabel[i]));
        }
        if (!is_valid_chars(adecoded, HOST))
        {
          alabel.push_back(LABEL_SEPARATOR);
          ascii_.append(alabel);
          adecoded.push_back(LABEL_SEPARATOR);
          unicode_.append(adecoded);
        }
        else
        {
          adecoded.push_back(LABEL_SEPARATOR);
          ascii_.append(adecoded);
          unicode_.append(adecoded);
        }
      }

      return;
    }
    while (false);

    alabel.push_back(LABEL_SEPARATOR);
    ascii_.append(alabel);
    unicode_.append(alabel);
  }


  std::string
  convert_label(const String::WSubString& label)
  {
    std::string utf8;
    String::StringManip::wchar_to_utf8(label, utf8);
    return utf8;
  }

  const String::SubString&
  convert_label(const String::SubString& label)
  {
    return label;
  }

  template <typename SubStringT, typename Dest>
  void
  idna_label_convert(const String::SubString& host,
    const SubStringT& normalized, Dest&& dst)
    throw (eh::Exception, BrowserAddress::IDNAError)
  {
    typename SubStringT::SizeType last = 0, pos;
    SubStringT label;
    for (;;)
    {
      if (last >= normalized.size())
      {
        break;
      }
      pos = normalized.find(LABEL_SEPARATOR, last);
      if (pos == SubStringT::NPOS)
      {
        pos = normalized.size();
      }
      label = normalized.substr(last, pos - last);
      last = pos + 1;

      if (label.empty())
      {
        Stream::Error ostr;
        ostr << FNS << "Empty label in '" << host << "'";
        throw BrowserAddress::IDNAError(ostr);
      }

      if (label.size() > MAX_HOSTNAME_LABEL_SIZE)
      {
        Stream::Error ostr;
        ostr << FNS << "Label '" << convert_label(label) <<
          "' in '" << host << "' is too large";
        throw BrowserAddress::IDNAError(ostr);
      }

      try
      {
        dst.append(label);
      }
      catch (const BrowserAddress::IDNAError& ex)
      {
        Stream::Error ostr;
        ostr << FNS << "Problem with label '" <<
          convert_label(label) << "' in '" << host <<
          "': " << ex.what();
        throw BrowserAddress::IDNAError(ostr);
      }
    }
  }

  void
  idna_normalize_host(const String::SubString& host, std::string& ascii,
    std::string& unicode)
    throw (eh::Exception, BrowserAddress::IDNAError)
  {
    if (host.empty())
    {
      throw BrowserAddress::IDNAError("Host name is empty");
    }

    if (host.size() >= MAX_HOSTNAME_SIZE)
    {
      Stream::Error ostr;
      ostr << FNS << "Host name '" << host << "' is too large";
      throw BrowserAddress::IDNAError(ostr);
    }

    bool has_unicode = false;
    wchar_t whost[MAX_HOSTNAME_SIZE];
    size_t whost_size = 0;
    for (String::SubString::SizeType i = 0; i < host.size();)
    {
      unsigned long octet_count =
        String::UTF8Handler::get_octet_count(host[i]);
      wchar_t wch;
      if (!String::UTF8Handler::utf8_char_to_wchar(&host[i],
        octet_count, wch))
      {
        Stream::Error ostr;
        ostr << FNS << "Invalid input sequence in host '" << host << "'";
        throw BrowserAddress::IDNAError(ostr);
      }
      if (octet_count > 1)
      {
        has_unicode = true;
      }
      whost[whost_size++] = wch;
      i += octet_count;
    }

    ascii.clear();
    unicode.clear();

    if (!has_unicode)
    {
      try
      {
        host.assign_to(ascii);
        idna_label_convert(host, host, IDNA0(ascii));
        unicode = ascii;
        return;
      }
      catch (const BrowserAddress::IDNAError&)
      {
        // We have 'xn--' prefix in a label,
        // additional processing is required
        has_unicode = true;
        ascii.clear();
      }
    }

    std::wstring normalized;
    if (!String::lower_and_normalize(
      String::WSubString(whost, whost_size), normalized, true))
    {
      Stream::Error ostr;
      ostr << FNS << "Normalization of host name '" << host << "' failed";
      throw BrowserAddress::IDNAError(ostr);
    }
    if (normalized.empty())
    {
      Stream::Error ostr;
      ostr << FNS << "Empty host name '" << host <<
        "' after normalization";
      throw BrowserAddress::IDNAError(ostr);
    }

    bool last_is_sep =
      normalized[normalized.size() - 1] == LABEL_SEPARATOR;

    ascii.reserve(normalized.size() * 4 + 1);
    unicode.reserve(normalized.size() * 4 + 1);

    idna_label_convert(host, String::WSubString(normalized),
      IDNA2008(ascii, unicode));

    if (ascii.size() >= MAX_HOSTNAME_SIZE)
    {
      Stream::Error ostr;
      ostr << FNS << "Resulted host name '" << ascii << "' is too large";
      throw BrowserAddress::IDNAError(ostr);
    }

    if (!last_is_sep)
    {
      ascii.resize(ascii.size() - 1);
      unicode.resize(unicode.size() - 1);
    }
  }

  bool
  idna_host_normalize(const String::SubString& url,
    const String::SubString& host, std::string& ascii, std::string& unicode,
    std::string& error)
    throw (eh::Exception)
  {
    try
    {
      idna_normalize_host(host, ascii, unicode);
    }
    catch (const BrowserAddress::IDNAError& ex)
    {
      error = ex.what();
      error.append(" in url '");
      url.append_to(error);
      error.push_back('\'');
      return false;
    }

    return true;
  }
}

namespace HTTP
{
  const String::AsciiStringManip::Caseless HTTP_SCHEME("http");
  const String::AsciiStringManip::Caseless HTTPS_SCHEME("https");

  const String::AsciiStringManip::Caseless HTTP_PREFIX("http:");
  const String::AsciiStringManip::Caseless HTTPS_PREFIX("https:");

  const String::AsciiStringManip::Caseless HTTP_BEGIN("http://");
  const String::AsciiStringManip::Caseless HTTPS_BEGIN("https://");

  //
  // UrlParts class
  //

  UrlParts::UrlParts(const String::SubString& scheme,
    const String::SubString& userinfo, const String::SubString& host,
    const String::SubString& port, const String::SubString& path,
    const String::SubString& query, const String::SubString& fragment)
    throw ()
    : has_scheme(!scheme.empty()), scheme(scheme),
      has_userinfo(!userinfo.empty()), userinfo(userinfo),
      has_host(!host.empty()), host(host),
      has_port(!port.empty()), port(port),
      has_path(!path.empty()), path(path),
      has_query(!query.empty()), query(query),
      has_fragment(!fragment.empty()), fragment(fragment)
  {
  }

  //
  // ExtendedUrlParts class
  //

  void
  ExtendedUrlParts::clear() throw ()
  {
    has_scheme = false;
    scheme.clear();
    has_userinfo = false;
    userinfo.clear();
    has_host = false;
    host.clear();
    has_port = false;
    port.clear();
    has_path = false;
    path.clear();
    has_query = false;
    query.clear();
    has_fragment = false;
    fragment.clear();
    authority.clear();
  }

  void
  ExtendedUrlParts::split_url(const String::SubString& url)
    throw (eh::Exception)
  {
    clear();

    // Split URL into scheme, authority, path, query and fragment
    do
    {
      const char* const END = url.end();
      const char* cur = url.begin();
      const char* end;

      end = URL_PARSER_SCHEME_END.find_owned(cur, END);
      if (end == END)
      {
        has_path = true;
        path = url;
        break;
      }

      if (*end == ':')
      {
        has_scheme = true;
        scheme = String::SubString(cur, end);
        cur = end + 1;
        if (cur == END)
        {
          break;
        }
      }

      if (*cur != '?' && *cur != '#')
      {
        if (*cur == '/' && cur + 1 != END && cur[1] == '/')
        {
          cur += 2;
          if (cur == END)
          {
            break;
          }
          end = URL_PARSER_AUTORITY_END.find_owned(cur, END);
          authority = String::SubString(cur, end);
          if (end == END)
          {
            break;
          }
          cur = end;
        }
        end = URL_PARSER_PATH_END.find_owned(cur, END);
        has_path = true;
        path = String::SubString(cur, end);
        if (end == END)
        {
          break;
        }
        cur = end;
      }

      if (*cur == '?')
      {
        cur++;
        end = URL_PARSER_QUERY_END.find_owned(cur, END);
        has_query = true;
        query = String::SubString(cur, end);
        if (end == END)
        {
          break;
        }
        cur = end;
      }

      assert(*cur == '#');

      has_fragment = true;
      fragment = String::SubString(cur + 1, END);
    }
    while (false);

    // Split authority into userinfo, host and port
    if (!authority.empty())
    {
      String::SubString::SizeType host_begin =
        authority.find(USERINFO_SEPARATOR);
      if (host_begin != String::SubString::NPOS)
      {
        if (host_begin != 0)
        {
          has_userinfo = true;
          userinfo.assign(authority, 0, host_begin);
        }
        host_begin += USERINFO_SEPARATOR_SIZE;
      }
      else
      {
        host_begin = 0;
      }

      String::SubString::SizeType host_end =
        authority.rfind(PORT_SEPARATOR);
      if (host_end != String::SubString::NPOS && host_begin &&
        host_end < host_begin)
      {
        host_end = String::SubString::NPOS;
      }
      if (host_end != String::SubString::NPOS)
      {
        if (host_end != authority.length() - PORT_SEPARATOR_SIZE)
        {
          has_port = true;
          port.assign(authority, host_end + PORT_SEPARATOR_SIZE,
            authority.length() - host_end);
        }
      }
      else
      {
        host_end = authority.length();
      }
      has_host = true;
      host.assign(authority, host_begin, host_end - host_begin);
    }
  }

  //
  // URLPartsChecker class
  //

  bool
  URLPartsChecker::operator ()(const String::SubString& url,
    const UrlParts& parts, std::string& error) throw (eh::Exception)
  {
    // Check scheme
    if (parts.scheme.empty() ? parts.has_scheme :
      !SCHEME_FIRST.is_owned(parts.scheme[0]) ||
        !is_valid_chars(parts.scheme.substr(1), SCHEME_NOT_FIRST))
    {
      return make_invalid(error, "scheme in url", url);
    }

    // Check userinfo
    if (!parts.userinfo.empty())
    {
      if (!is_valid_encoded(parts.userinfo, USER_INFO))
      {
        return make_invalid(error, "userinfo in url", url);
      }
    }

    // Check host
    if (!parts.host.empty())
    {
      if (parts.host.size() > MAX_HOSTNAME_SIZE ||
        !is_valid_chars(parts.host, HOST))
      {
        return make_invalid(error, "host in url", url);
      }
      if (parts.host.size())
      {
        String::StringManip::Splitter<LabelSeparatorCategory, true> labels(
          parts.host[parts.host.size() - 1] == LABEL_SEPARATOR ?
            parts.host.substr(0, parts.host.size() - 1) : parts.host);
        String::SubString label;
        while (labels.get_token(label))
        {
          if (!label.size() || label.size() > MAX_HOSTNAME_LABEL_SIZE)
          {
            return make_invalid(error,
              "length of host's label in url", url);
          }
          if (!LABEL_FIRST_LAST(label[0]) ||
            (label.size() > 1 && !LABEL_FIRST_LAST(*(label.end() - 1))) ||
            (label.size() > 2 &&
              !is_valid_chars(label.substr(1, label.size() - 2),
                LABEL_MIDDLE)))
          {
            return make_invalid(error,
              "characters in host's label in url", url);
          }
        }
      }
    }
    else
    {
      if (!parts.userinfo.empty() || !parts.port.empty())
      {
        return make_invalid(error, "empty host in url", url);
      }
    }

    // Check port
    if (!parts.port.empty())
    {
      if (!is_valid_chars(parts.port, PORT))
      {
        return make_invalid(error, "port in url", url);
      }
    }

    // Check path
    if (!parts.path.empty())
    {
      // Simplified check
      if ((!parts.host.empty() ? parts.path[0] != PATH_SEPARATOR :
        parts.path[0] == PATH_SEPARATOR && parts.path.size() > 1 &&
        parts.path[1] == PATH_SEPARATOR) ||
        !is_valid_encoded(parts.path, PATH))
      {
        return make_invalid(error, "path in url", url);
      }
    }

    // Check query
    if (!parts.query.empty())
    {
      if (!is_valid_encoded(parts.query, QUERY))
      {
        return make_invalid(error, "query in url", url);
      }
    }

    // Check fragment
    if (!parts.fragment.empty())
    {
      if (!is_valid_encoded(parts.fragment, FRAGMENT))
      {
        return make_invalid(error, "fragment in url", url);
      }
    }

    return true;
  }

  //
  // URLChecker class
  //

  bool
  URLChecker::operator ()(const String::SubString& url)
    throw (eh::Exception)
  {
    ExtendedUrlParts parts;
    parts.split_url(url);
    std::string error;
    return URLPartsChecker::operator ()(url, parts, error);
  }

  //
  // URLAddress class
  //

  URLAddress::URLAddress() throw ()
  {
  }

  URLAddress::URLAddress(const String::SubString& value)
    throw (eh::Exception, InvalidURL)
  {
    url(value);
  }

  URLAddress::URLAddress(const String::SubString& scheme,
    const String::SubString& userinfo, const String::SubString& host,
    const String::SubString& port, const String::SubString& path,
    const String::SubString& query, const String::SubString& fragment)
    throw (eh::Exception, InvalidURL)
  {
    UrlParts parts(scheme, userinfo, host, port, path, query, fragment);
    assign_url_parts_(parts, true);
  }

  URLAddress::URLAddress(const URLAddress& another) throw (eh::Exception)
  {
    assign_url_parts_(another.parts_, false);
  }

  URLAddress&
  URLAddress::operator =(const URLAddress& another) throw (eh::Exception)
  {
    assign_url_parts_(another.parts_, false);
    return *this;
  }

  void
  URLAddress::url_without_check_(const String::SubString& value)
    throw (eh::Exception)
  {
    url_.clear();
    parts_.clear();

    if (value.empty())
    {
      return;
    }

    value.assign_to(url_);

    parts_.split_url(url_);
  }

  void
  URLAddress::specific_checks_()
    throw (InvalidURL, Exception, eh::Exception)
  {
  }

  void
  URLAddress::url(const String::SubString& value)
    throw (eh::Exception, Exception, InvalidURL)
  {
    assign_(value);
  }

  void
  URLAddress::assign_(const String::SubString& value)
    throw (eh::Exception, Exception, InvalidURL)
  {
    url_without_check_(value);

    specific_checks_();

    std::string error;
    URLPartsChecker checker;
    if (!checker(url_, parts_, error))
    {
      Stream::Error ostr;
      ostr << FNS << error;
      throw InvalidURL(ostr);
    }
  }

  void
  URLAddress::assign_url_parts_(const UrlParts& parts, bool check)
    throw (eh::Exception, InvalidURL)
  {
    // Assemble url
    std::string new_url;
    new_url.reserve(parts.scheme.size() + parts.userinfo.size() +
      parts.host.size() + parts.port.size() + parts.path.size() +
      parts.fragment.size() + ALL_SEPS_SIZE);
    if (!parts.scheme.empty())
    {
      parts.scheme.assign_to(new_url);
      new_url += SCHEME_SUFFIX;
    }
    bool has_authority =
      parts.has_userinfo || !parts.host.empty() || parts.has_port;
    size_t authority_size = 0;
    if (has_authority)
    {
      new_url += AUTHORITY_PREFIX;
      authority_size = new_url.size();
      // Append authority
      if (parts.has_userinfo)
      {
        parts.userinfo.append_to(new_url);
        new_url += USERINFO_SEPARATOR;
      }
      parts.host.append_to(new_url);
      if (parts.has_port)
      {
        new_url += PORT_SEPARATOR;
        parts.port.append_to(new_url);
      }
      authority_size = new_url.size() - authority_size;
    }
    parts.path.append_to(new_url);
    if (parts.has_query)
    {
      new_url += QUERY_SEPARATOR;
      parts.query.append_to(new_url);
    }
    if (parts.has_fragment)
    {
      new_url += FRAGMENT_SEPARATOR;
      parts.fragment.append_to(new_url);
    }

    // Check components
    if (check)
    {
      std::string error;
      URLPartsChecker checker;
      if (!checker(new_url, parts, error))
      {
        Stream::Error ostr;
        ostr << FNS << error;
        throw InvalidURL(ostr);
      }
    }
    new_url.swap(url_);

    // adjust parts to new url string
    parts_.clear();

    String::SubString::Pointer ptr = url_.data();

    if (!parts.scheme.empty())
    {
      parts_.has_scheme = true;
      parts_.scheme.assign(ptr, parts.scheme.size());
      ptr += parts.scheme.size() + SCHEME_SUFFIX_SIZE;
    }

    if (has_authority)
    {
      ptr += AUTHORITY_PREFIX_SIZE;
      parts_.authority.assign(ptr, authority_size);
      ptr += parts_.authority.size();
      String::SubString::SizeType host_begin = parts.userinfo.size();
      if (parts.has_userinfo)
      {
        parts_.has_userinfo = true;
        parts_.userinfo = parts_.authority.substr(0, parts.userinfo.size());
        host_begin += USERINFO_SEPARATOR_SIZE;
      }
      if (!parts.host.empty())
      {
        parts_.has_host = true;
        parts_.host =
          parts_.authority.substr(host_begin, parts.host.size());
      }
      if (parts.has_port)
      {
        parts_.has_port = true;
        parts_.port = parts_.authority.substr(
          parts_.authority.size() - parts.port.size(), parts.port.size());
      }
    }

    parts_.has_path = true;
    parts_.path.assign(ptr, parts.path.size());
    ptr += parts.path.size();

    if (parts.has_query)
    {
      parts_.has_query = true;
      ptr += QUERY_SEPARATOR_SIZE;
      parts_.query.assign(ptr, parts.query.size());
      ptr += parts.query.size();
    }

    if (parts.has_fragment)
    {
      parts_.has_fragment = true;
      ptr += FRAGMENT_SEPARATOR_SIZE;
      parts_.fragment.assign(ptr, parts.fragment.size());
    }
  }

  URLAddress*
  URLAddress::create_address(const String::SubString& url)
    throw (InvalidURL, Exception, eh::Exception)
  {
    if (url.empty())
    {
      Stream::Error ostr;
      ostr << FNS << "url is empty";
      throw InvalidURL(ostr);
    }
    if (HTTP_PREFIX.start(url) || HTTPS_PREFIX.start(url))
    {
      return new HTTPAddress(url);
    }
    Stream::Error ostr;
    ostr << FNS << "unsupported protocol in url " << url;
    throw InvalidURL(ostr);
  }

  //
  // HTTPChecker class
  //

  bool
  HTTPChecker::operator ()(const String::SubString& url, std::string* error,
    bool strict) throw (eh::Exception)
  {
    ExtendedUrlParts parts;

    std::string error_message;
    std::string& error_ref = error ? *error : error_message;

    if (url.empty())
    {
      error_ref = "url is null";
      return false;
    }

    parts.split_url(url);

    std::string fixed_url;
    if (!strict && http_url_needs_prefix(parts.scheme, parts.host))
    {
      http_add_scheme(fixed_url, url);
      parts.split_url(fixed_url);
    }
    if (!process_parts_(url, parts, error_ref, strict) ||
      !check_http_url_components(
        url, parts.scheme, parts.host, error_ref, strict))
    {
      return false;
    }

    return URLPartsChecker::operator ()(url, parts, error_ref);
  }

  bool
  HTTPChecker::process_parts_(const String::SubString& /*url*/,
    ExtendedUrlParts& parts, std::string& /*error*/, bool strict)
    throw (eh::Exception)
  {
    if (!strict)
    {
      parts.has_userinfo = false;
      parts.userinfo.clear();
      parts.has_path = false;
      parts.path.clear();
      parts.has_query = false;
      parts.query.clear();
      parts.has_fragment = false;
      parts.fragment.clear();
    }

    return true;
  }

  //
  // HTTPAddress class
  //

  HTTPAddress::HTTPAddress(const String::SubString& url)
    throw (InvalidURL, eh::Exception)
    : URLAddress(), strict_(true), port_number_(0), secure_(false),
      default_port_(true)
  {
    if (!url.empty())
    {
      assign_(url);
    }
  }

  HTTPAddress::HTTPAddress(const String::SubString& url, bool strict_url)
    throw (InvalidURL, eh::Exception)
    : URLAddress(), strict_(strict_url), port_number_(0), secure_(false),
      default_port_(true)
  {
    if (!url.empty())
    {
      assign_(url);
    }
  }

  HTTPAddress::HTTPAddress(const String::SubString& host,
    const String::SubString& path, const String::SubString& query,
    const String::SubString& fragment, unsigned short port, bool secure,
    const String::SubString& userinfo) throw (InvalidURL, eh::Exception)
    : URLAddress(), strict_(false), port_number_(0), secure_(false),
      default_port_(true)
  {
    set_(secure, userinfo, host, port ? port : get_default_port_(secure),
      path, query, fragment);
  }

  int
  HTTPAddress::get_default_port_(bool secure) throw ()
  {
    return secure ? DEFAULT_HTTPS_PORT : DEFAULT_HTTP_PORT;
  }

  void
  HTTPAddress::set_(bool secure, const String::SubString& userinfo,
    const String::SubString& host, unsigned short port,
    const String::SubString& path, const String::SubString& query,
    const String::SubString& fragment) throw (eh::Exception)
  {
    if (host.empty())
    {
      default_port_ = true;
      port_number_ = 0;
      return;
    }

    secure_ = secure;
    port_number_ = port;
    default_port_ = port_number_ == get_default_port_(secure_);

    char port_buffer[32];

    UrlParts parts(secure ? HTTPS_SCHEME.str : HTTP_SCHEME.str, userinfo,
      host, default_port_ ? String::SubString() : String::SubString(
        port_buffer, snprintf(port_buffer, sizeof(port_buffer), "%hu",
          port_number_)),
      path.empty() ? DEFAULT_PATH : String::SubString(path), query,
      fragment);

    assign_url_parts_(parts, true);
  }

  void
  HTTPAddress::assign_(const String::SubString& http_url)
    throw (InvalidURL, Exception, eh::Exception)
  {
    if (http_url.empty())
    {
      Stream::Error ostr;
      ostr << FNS << "url is nil";
      throw InvalidURL(ostr);
    }

    URLAddress::assign_(http_url);

    secure_ = scheme() == HTTPS_SCHEME;

    const String::SubString& port_str = port();
    if (!port_str.empty())
    {
      if (!String::StringManip::str_to_int(port_str, port_number_))
      {
        Stream::Error ostr;
        ostr << FNS << "invalid port value=" << port_str;
        throw InvalidURL(ostr);
      }
      default_port_ = false;
    }
    else
    {
      port_number_ = get_default_port_(secure_);
      default_port_ = true;
    }

    if (path().empty())
    {
      parts_.path = DEFAULT_PATH;
    }
  }

  void
  HTTPAddress::specific_checks_()
    throw (InvalidURL, Exception, eh::Exception)
  {
    if (!strict_ && http_url_needs_prefix(scheme(), host()))
    {
      std::string fixed_url;
      http_add_scheme(fixed_url, url_);
      url_without_check_(fixed_url);
    }

    bool rebuild = additional_checks_();

    {
      std::string error;
      if (!check_http_url_components(
        url(), scheme(), host(), error, strict_))
      {
        Stream::Error ostr;
        ostr << FNS << error;
        throw InvalidURL(ostr);
      }
    }

    UrlParts new_parts(parts_);

    PartCheckInfo parts[] =
    {
      PartCheckInfo(new_parts.userinfo, USER_INFO),
      PartCheckInfo(new_parts.path, PATH),
      PartCheckInfo(new_parts.query, QUERY),
      PartCheckInfo(new_parts.fragment, FRAGMENT),
    };

    if (!strict_)
    {
      for (PartCheckInfo* part = parts;
        part != parts + sizeof(parts) / sizeof(*parts); part++)
      {
        if (http_fix_part(part->part, part->CHECKER, part->new_part))
        {
          part->part = part->new_part;
          rebuild = true;
        }
      }
    }

    if (rebuild)
    {
      assign_url_parts_(new_parts, false);
    }
  }

  bool
  HTTPAddress::additional_checks_()
    throw (InvalidURL, Exception, eh::Exception)
  {
    return false;
  }

  const std::string&
  HTTPAddress::get_view(unsigned long flags, std::string& str) const
    throw (eh::Exception)
  {
    str.clear();
    str.reserve(url_.size() + 36);

    if (flags & VW_PROTOCOL)
    {
      if (parts_.has_scheme)
      {
        (secure_ ? HTTPS_PREFIX : HTTP_PREFIX).str.append_to(str);
      }
      str += AUTHORITY_PREFIX;
    }
    if (flags & VW_HOSTNAME)
    {
      if ((flags & VW_HOSTNAME_WWW) == VW_HOSTNAME_WWW &&
        !WWW.start(host()))
      {
        WWW.str.append_to(str);
      }
      host().append_to(str);
    }
    if ((flags & VW_PORT) || (!default_port_ && (flags & VW_NDEF_PORT)))
    {
      char port[8] = ":";
      snprintf(port + 1, sizeof(port) - 1, "%hu", port_number_);
      str += port;
    }
    if (flags & VW_PATH)
    {
      const String::SubString& path_ref = path();
      size_t path_len = path_ref.size();
      if ((flags & VW_STRIP_PATH) == VW_STRIP_PATH &&
        path_ref[path_len - 1] == PATH_SEPARATOR)
      {
        path_len -= PATH_SEPARATOR_SIZE;
      }
      str.append(path_ref.data(), path_len);
    }
    if (flags & VW_QUERY && parts_.has_query)
    {
      str += QUERY_SEPARATOR;
      query().append_to(str);
    }
    if (flags & VW_FRAGMENT && parts_.has_fragment)
    {
      str += FRAGMENT_SEPARATOR;
      fragment().append_to(str);
    }
    return str;
  }



  //
  // BrowserChecker class
  //

  bool
  BrowserChecker::process_parts_(const String::SubString& url,
    ExtendedUrlParts& parts, std::string& error, bool strict)
    throw (eh::Exception)
  {
    if (!HTTPChecker::process_parts_(url, parts, error, strict))
    {
      return false;
    }

    std::string unicode;
    if (!idna_host_normalize(url, parts.host, encoded_host_, unicode,
      error))
    {
      return false;
    }
    parts.host = encoded_host_;
    return true;
  }

  bool
  BrowserChecker::operator ()(const String::SubString& url,
    std::string* error) throw (eh::Exception)
  {
    return HTTPChecker::operator()(url, error, false);
  }


  //
  // BrowserAddress
  //

  BrowserAddress::BrowserAddress(const String::SubString& url)
    throw (InvalidURL, eh::Exception)
    : HTTPAddress(String::SubString(), false)
  {
    if (!url.empty())
    {
      assign_(url);
    }
  }

  BrowserAddress::BrowserAddress(const String::SubString& host,
    const String::SubString& path, const String::SubString& query,
    const String::SubString& fragment, unsigned short port, bool secure,
    const String::SubString& userinfo) throw (InvalidURL, eh::Exception)
    : HTTPAddress()
  {
    process_host_(host);
    set_(secure, userinfo, encoded_host_,
      port ? port : HTTPAddress::get_default_port_(secure),
      path, query, fragment);
  }

  void
  BrowserAddress::process_host_(const String::SubString& host)
    throw (InvalidURL, eh::Exception)
  {
    std::string error;
    if (!idna_host_normalize(url_, host, encoded_host_,
      decoded_host_, error))
    {
      Stream::Error ostr;
      ostr << FNS << error;
      throw InvalidURL(ostr);
    }
    parts_.host = encoded_host_;
  }

  bool
  BrowserAddress::additional_checks_()
    throw (InvalidURL, Exception, eh::Exception)
  {
    process_host_(parts_.host);
    return true;
  }


  //
  // Functions
  //

  std::string
  normalize_http_address(const String::SubString& url) throw (eh::Exception)
  {
    std::string norm;

    BrowserAddress address(url);
    if (!address.secure() && address.port_number() == DEFAULT_HTTP_PORT)
    {
      HTTP_PREFIX.str.assign_to(norm);
      norm.append(AUTHORITY_PREFIX, AUTHORITY_PREFIX_SIZE);
      std::string tmp;
      address.host().assign_to(tmp);
      String::AsciiStringManip::to_lower(tmp);
      norm.append(tmp);
      unmime(address.path(), PATH, tmp);
      norm.append(tmp);
      if (!address.query().empty())
      {
        unmime(address.query(), QUERY, tmp);
        norm.push_back(QUERY_SEPARATOR);
        norm.append(tmp);
      }
    }

    return norm;
  }

  std::string
  keywords_from_http_address(const String::SubString& url)
    throw (eh::Exception)
  {
    ExtendedUrlParts parts;

    parts.split_url(url);

    std::string fixed_url;
    if (http_url_needs_prefix(parts.scheme, parts.host))
    {
      http_add_scheme(fixed_url, url);
      parts.split_url(fixed_url);
    }

    std::string keywords;

    std::string tmp, tmp2;
    idna_normalize_host(parts.host, tmp, keywords);
    unmime_all(parts.path, tmp);
    keywords.append(tmp);
    if (!parts.query.empty())
    {
      unmime_all(parts.query, tmp);
      keywords.push_back(QUERY_SEPARATOR);
      keywords.append(tmp);

      if(!tmp.empty())
      {
        unmime_all(tmp, tmp2);
        keywords.push_back(QUERY_SEPARATOR);
        keywords.append(tmp2);
      }
    }

    return keywords;
  }
}
