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





#include <HTTP/HTTPCookie.hpp>

#define TRACE_COOKIE 0
#if TRACE_COOKIE == 1
#include <iostream>
#endif

namespace
{
  const String::AsciiStringManip::Caseless SECURE("secure");
  const String::AsciiStringManip::Caseless PATH("path");
  const String::AsciiStringManip::Caseless CDOMAIN("domain");
  const String::AsciiStringManip::Caseless EXPIRES("expires");
  const String::AsciiStringManip::Caseless SET_COOKIE("Set-Cookie");
  const String::AsciiStringManip::Caseless COOKIE("Cookie");

  const String::SubString DEFAULT_PATH("/");


  template <typename Type1, typename Type2>
  void
  assign(Type1& dst, const Type2& src) throw (eh::Exception)
  {
    dst = src;
  }

  void
  assign(std::string& dst, const String::SubString& src)
    throw (eh::Exception)
  {
    src.assign_to(dst);
  }

  template <typename InvalidArgument>
  Generics::Time
  get_expires(const String::SubString& value)
    throw (eh::Exception, Generics::Time::Exception, InvalidArgument)
  {
    std::string week_day;
    std::string date;
    std::string time;

    {
      Stream::Parser istr(value);
      istr >> week_day >> date >> time;
      if (!istr)
      {
        throw InvalidArgument(value);
      }
    }

    int day = 0;
    int month = 0;
    int year = 0;

    {
      String::StringManip::SplitMinus tokenizer(date);
      String::SubString token;
      if (tokenizer.get_token(token))
      {
        Stream::Parser parser(token);
        parser >> day;
        if (!parser || !parser.eof())
        {
          throw InvalidArgument(token);
        }
        if (tokenizer.get_token(token))
        {
          month = Generics::Time::month(token) + 1;
          if (tokenizer.get_token(token))
          {
            //@@TREF: some cookies have following data format:
            // 01-Oct-05
            Stream::Parser parser(token);
            parser >> year;
            if (!parser || !parser.eof())
            {
              throw InvalidArgument(token);
            }
            if (year < 100)
            {
              year += 2000;
            }
          }
        }
      }

      if (day < 1 || day > 31 || month < 1 || month > 12 ||
        year < 1900)
      {
        Stream::Error ostr;
        ostr << FNS << "invalid date " << date;
        throw InvalidArgument(ostr);
      }
    }

    int hours = 0;
    int minutes = 0;
    int seconds = 0;

    {
      String::StringManip::SplitColon tokenizer(time);
      String::SubString token;
      if (tokenizer.get_token(token))
      {
        Stream::Parser parser(token);
        parser >> hours;
        if (!parser || !parser.eof())
        {
          throw InvalidArgument(token.data(), token.size());
        }
        if (tokenizer.get_token(token))
        {
          Stream::Parser parser(token);
          parser >> minutes;
          if (!parser || !parser.eof())
          {
            throw InvalidArgument(token.data(), token.size());
          }
          if (tokenizer.get_token(token))
          {
            Stream::Parser parser(token);
            parser >> seconds;
            if (!parser || !parser.eof())
            {
              throw InvalidArgument(token);
            }
          }
        }
      }
    }

    return Generics::ExtendedTime(year, month, day,
      hours, minutes, seconds, 0);
  }

  template <typename Exception, typename InvalidArgument,
    typename CookieDef>
  void
  parse_value(std::list<CookieDef>& list,
    const HTTP::HTTPAddress& url_address, const String::SubString& HEADER,
    CookieDef& cookie, bool& control_info, bool& expired, bool keep_expired,
    String::SubString::ConstReverseIterator cookie_begin,
    String::SubString::ConstReverseIterator cookie_end,
    bool has_sep,
    String::SubString::ConstReverseIterator sep)
    throw (Exception, InvalidArgument, eh::Exception)
  {
    typedef std::list<CookieDef> Container;

    String::SubString name;
    String::SubString value;

    if (has_sep)
    {
      name.assign(cookie_begin.base(), sep.base() - 1);
      value.assign(sep.base(), cookie_end.base());
      String::StringManip::trim(name);
      String::StringManip::trim(value);
    }
    else
    {
      name.assign(cookie_begin.base(), cookie_end.base());
      String::StringManip::trim(name);
    }

#if TRACE_COOKIE == 1
    std::cerr << FNS << "'" <<
      String::SubString(itor.base(), cookie_end.base()) << "' => '" <<
      name << "' = '" << value << "'" << std::endl;
#endif

    if (name.empty())
    {
      return;
    }

    if (control_info)
    {
      if (name == SECURE)
      {
        cookie.secure = true;
      }
      else if (name == PATH)
      {
        assign(cookie.path, value);
      }
      else if (name == CDOMAIN)
      {
        assign(cookie.domain, value);
      }
      else if (name == EXPIRES)
      {
        try
        {
          cookie.expires = get_expires<InvalidArgument>(value);
          expired = cookie.expires < Generics::Time::get_time_of_day();
        }
        catch (const Generics::Time::Exception& ex)
        {
          Stream::Error ostr;
          ostr << FNS << "\tTime::Exception has been caught during "
            "parsing header:" << std::endl <<
            "\t\"Set-Cookie: " << HEADER << "\"" << std::endl <<
            "Description:" << std::endl << ex.what() << std::endl;

          throw InvalidArgument(ostr);
        }
      }
      else
      {
        if (cookie.path.empty())
        {
          assign(cookie.path, url_address.path());
        }
        else if (cookie.path == "\\")
        {
          assign(cookie.path, DEFAULT_PATH);
        }

        if (cookie.domain.empty() || cookie.domain == ".")
        {
          assign(cookie.domain, url_address.host());
        }

#if TRACE_COOKIE == 1
        std::cerr << FNS << std::endl <<
          "  cookie.secure '" << cookie.secure << "'\n"
          "  cookie.path '" << cookie.path << "'\n"
          "  cookie.domain '" << cookie.domain << "'\n"
          "  cookie.expires '" << HTTP::cookie_date(cookie.expires) <<
          "'\n";
        if (expired)
        {
          std::cerr << "  EXPIRED\n";
        }
#endif
        control_info = false;
      }
    }

    if (!control_info)
    {
      assign(cookie.name, name);
      assign(cookie.value, value);

#if TRACE_COOKIE == 1
      std::cerr << "    cookie." << name << " '" << value << "'\n";
#endif
      typename Container::iterator it;

      for (it = list.begin(); it != list.end(); ++it)
      {
        if (it->domain ==
          String::AsciiStringManip::Caseless(cookie.domain) &&
          it->path == cookie.path && it->name == cookie.name)
        {
          if (expired && !keep_expired)
          {
#if TRACE_COOKIE == 1
            std::cerr << "      erasing ...\n";
#endif
            list.erase(it);
          }
          else
          {
            *it = cookie;
          }
          break;
        }
      }

      if (it == list.end())
      {
        if (!expired || keep_expired)
        {
          list.push_front(cookie);
        }
#if TRACE_COOKIE == 1
        else
        {
          std::cerr << "      skipping ...\n";
        }
#endif
      }
    }
  }

  template <typename Exception, typename InvalidArgument,
    typename CookieDef, typename HeaderList>
  void
  load_from_headers(std::list<CookieDef>& list, const HeaderList& headers,
    const HTTP::HTTPAddress& url_address, bool keep_expired = false)
    throw (Exception, InvalidArgument, eh::Exception)
  {
    for (typename HeaderList::const_iterator it(headers.begin());
      it != headers.end(); ++it)
    {
      if (it->name != SET_COOKIE)
      {
        continue;
      }

      const String::SubString HEADER(it->value);

#if TRACE_COOKIE == 1
      std::cerr << FNS <<
        "header '" << HEADER << "', url '" << url_address.url() << "'\n";
#endif

      CookieDef cookie;

      bool control_info = true;
      bool expired = false;

      cookie.secure = url_address.secure();

      bool has_sep = false;
      String::SubString::ConstReverseIterator sep,
        cookie_end(HEADER.rbegin());
      for (String::SubString::ConstReverseIterator itor(cookie_end);;
        ++itor)
      {
        if (itor == HEADER.rend() || *itor == ';')
        {
          if (itor != HEADER.rbegin())
          {
            parse_value<Exception, InvalidArgument>(
              list, url_address, HEADER,
              cookie, control_info, expired, keep_expired,
              itor, cookie_end, has_sep, sep);
          }

          if (itor == HEADER.rend())
          {
            break;
          }

          cookie_end = itor + 1;
          has_sep = false;
        }
        else if (*itor == '=')
        {
          has_sep = true;
          sep = itor;
        }
      }
    }
  }

  template <typename CookieDef>
  void
  expire(std::list<CookieDef>& list, bool session_cookies)
    throw (eh::Exception)
  {
    typedef std::list<CookieDef> Container;

    Generics::Time cur_time(Generics::Time::get_time_of_day());

    for (typename Container::iterator it = list.begin(); it != list.end();)
    {
      if (it->expires == Generics::Time::ZERO ? session_cookies :
        it->expires < cur_time)
      {
        it = list.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  template <typename CookieDef>
  std::string
  cookie_header(std::list<CookieDef>& list,
    const HTTP::HTTPAddress& url_address)
    throw (eh::Exception)
  {
    typedef std::list<CookieDef> Container;

    Stream::Dynamic result(4096);
    bool is_empty = true;

    for (typename Container::const_iterator it = list.begin();
      it != list.end(); ++it)
    {
      size_t domain_len = it->domain.size();
      String::SubString host(url_address.host());

      if ((it->secure && !url_address.secure()) || domain_len > host.size())
      {
        continue;
      }

      if (host.substr(host.size() - domain_len, domain_len) !=
        String::AsciiStringManip::Caseless(it->domain))
      {
        continue;
      }

      if (url_address.path().substr(0, it->path.size()) != it->path)
      {
        continue;
      }

      if (!is_empty)
      {
        result << "; ";
      }

      result << it->name << "=" << it->value;

      is_empty = false;
    }

    return result.str().str();
  }

  template <typename CookieDef>
  void
  set_cookie_header_plain(std::list<CookieDef>& list,
    HTTP::HeaderList& headers)
    throw (eh::Exception)
  {
    typedef std::list<CookieDef> Container;

    for (typename Container::const_iterator it = list.begin();
      it != list.end(); ++it)
    {
      std::string header;
      HTTP::cookie_header_plain(*it, header);
      headers.emplace_back(SET_COOKIE.str.str(), header);
    }
  }

  template <typename CookieDef>
  void
  set_cookie_header(std::list<CookieDef>& list, HTTP::HeaderList& headers)
    throw (eh::Exception)
  {
    typedef std::list<HTTP::CookieDef> Container;

    Container cookies(list.begin(), list.end());

    while (!cookies.empty())
    {
      Stream::Dynamic header(4096);

      HTTP::CookieDef cookie = *cookies.begin();

      bool first = true;

      for (Container::iterator it = cookies.begin(); it != cookies.end();)
      {
        if (String::AsciiStringManip::Caseless(it->domain) ==
          cookie.domain && it->path == cookie.path &&
          it->expires == cookie.expires && it->secure == cookie.secure)
        {
          if (first)
          {
            first = false;
          }
          else
          {
            header << "; ";
          }

          header << it->name << "=" << it->value;
          it = cookies.erase(it);
        }
        else
        {
          ++it;
        }
      }

      if (cookie.expires != Generics::Time::ZERO)
      {
        header << "; expires=" << HTTP::cookie_date(cookie.expires);
      }

      if (!cookie.domain.empty())
      {
        header << "; domain=" << cookie.domain;
      }

      if (!cookie.path.empty())
      {
        header << "; path=" << cookie.path;
      }

      if (cookie.secure)
      {
        header << "; secure";
      }

      headers.emplace_back(SET_COOKIE.str.str(), header.str().str());
    }
  }

  struct CookieSep
  {
    const char*
    find_owned(const char* begin, const char* end,
      unsigned long* octets_length) throw ()
    {
      for (const char* next; begin != end; begin = next)
      {
        next = begin + 1;
        if (*begin != ';' && *begin != ',')
        {
          continue;
        }
        if (next == end)
        {
          break;
        }
        if (*next == ' ')
        {
          *octets_length = 2;
          return begin;
        }
      }
      return end;
    }
  };
}

namespace HTTP
{
  //
  // CookieList class
  //

  CookieList::~CookieList() throw ()
  {
  }

  template <typename HeaderList>
  void
  CookieList::load_from_headers_(const HeaderList& headers,
    bool replace_duplicate)
    throw (InvalidArgument, Exception, eh::Exception)
  {
    for (typename HeaderList::const_iterator it(headers.begin());
      it != headers.end(); ++it)
    {
      if (it->name != COOKIE)
      {
        continue;
      }

      String::SubString header(it->value);
      String::StringManip::trim(header);

#if TRACE_COOKIE == 1
      std::cerr << FNS << "header '" << header << "'\n";
#endif

      String::StringManip::Splitter<CookieSep, true>
        cookie_tokenizer(header);
      String::SubString cookie;
      String::SubString token;
      while (cookie_tokenizer.get_token(cookie))
      {
        if (cookie.empty())
        {
          continue;
        }

        String::SubString name;
        String::SubString value;

        String::SubString::SizeType pos = cookie.find('=');

        if (pos == String::SubString::NPOS)
        {
          Stream::Error ostr;
          ostr << FNS << "invalid cookie format '" << token << "'";
          throw InvalidArgument(ostr);
        }

        name = cookie.substr(0, pos);
        String::StringManip::trim(name);

        if (name.empty())
        {
          Stream::Error ostr;
          ostr << FNS << "empty cookie name '" << cookie << "'";
          throw InvalidArgument(ostr);
        }

        value = cookie.substr(pos + 1);
        String::StringManip::trim(value);

#if TRACE_COOKIE == 1
        std::cerr << FNS << std::endl <<
          "  cookie.name '" << name << "'\n"
          "  cookie.value '" << value << "'\n";
#endif

        CookieList::iterator it = replace_duplicate ? end() : begin();
        for (; it != end(); ++it)
        {
          if (name == it->name)
          {
            it->value = value;
            break;
          }
        }

        if (it == end())
        {
          Cookie cookie;
          cookie.name = name;
          cookie.value = value;
          push_back(std::move(cookie));
        }
      }
    }
  }

  void
  CookieList::load_from_headers(const SubHeaderList& headers,
    bool replace_duplicate)
    throw (InvalidArgument, Exception, eh::Exception)
  {
    load_from_headers_(headers, replace_duplicate);
  }

  void
  CookieList::load_from_headers(const HeaderList& headers,
    bool replace_duplicate)
    throw (InvalidArgument, Exception, eh::Exception)
  {
    load_from_headers_(headers, replace_duplicate);
  }

  std::string
  CookieList::cookie_header() throw (eh::Exception)
  {
    Stream::Dynamic result(4096);
    bool is_empty = true;

    for (CookieList::iterator it = begin(); it != end(); ++it)
    {
      if (!is_empty)
      {
        result << "; ";
      }

      result << it->name << "=" << it->value;

      is_empty = false;
    }

    return result.str().str();
  }


  //
  // CookieDefList class
  //

  CookieDefList::CookieDefList(bool keep_expired) throw (eh::Exception)
    : keep_expired_(keep_expired)
  {
  }

  CookieDefList::~CookieDefList() throw ()
  {
  }

  void
  CookieDefList::load_from_headers(const SubHeaderList& headers,
    const HTTP::HTTPAddress& url_address)
    throw (InvalidArgument, Exception, eh::Exception)
  {
    ::load_from_headers<Exception, InvalidArgument>(*this, headers,
      url_address, keep_expired_);
  }

  void
  CookieDefList::load_from_headers(const HeaderList& headers,
    const HTTP::HTTPAddress& url_address)
    throw (InvalidArgument, Exception, eh::Exception)
  {
    ::load_from_headers<Exception, InvalidArgument>(*this, headers,
      url_address, keep_expired_);
  }

  std::string
  CookieDefList::cookie_header(const HTTPAddress& url_address)
    throw (eh::Exception)
  {
    expire_();
    return ::cookie_header(*this, url_address);
  }

  void
  CookieDefList::set_cookie_header_plain(HeaderList& headers)
    throw (eh::Exception)
  {
    ::set_cookie_header_plain(*this, headers);
  }

  void
  CookieDefList::set_cookie_header(HeaderList& headers)
    throw (eh::Exception)
  {
    ::set_cookie_header(*this, headers);
  }

  void
  CookieDefList::expire_(bool session_cookies) throw (eh::Exception)
  {
    if (keep_expired_)
    {
      return;
    }

    ::expire(*this, session_cookies);
  }


  //
  // ClientCookieFacility class
  //

  ClientCookieFacility::~ClientCookieFacility() throw ()
  {
  }

  void
  ClientCookieFacility::load_from_headers(const SubHeaderList& headers,
    const HTTP::HTTPAddress& url_address)
    throw (InvalidArgument, Exception, eh::Exception)
  {
    ::load_from_headers<Exception, InvalidArgument>(*this, headers,
      url_address);
  }

  void
  ClientCookieFacility::load_from_headers(const HeaderList& headers,
    const HTTP::HTTPAddress& url_address)
    throw (InvalidArgument, Exception, eh::Exception)
  {
    ::load_from_headers<Exception, InvalidArgument>(*this, headers,
      url_address);
  }

  std::string
  ClientCookieFacility::cookie_header(const HTTPAddress& url_address)
    throw (eh::Exception)
  {
    expire_();
    return ::cookie_header(*this, url_address);
  }

  void
  ClientCookieFacility::set_cookie_header_plain(HeaderList& headers)
    throw (eh::Exception)
  {
    ::set_cookie_header_plain(*this, headers);
  }

  void
  ClientCookieFacility::set_cookie_header(HeaderList& headers)
    throw (eh::Exception)
  {
    ::set_cookie_header(*this, headers);
  }

  void
  ClientCookieFacility::expire_(bool session_cookies) throw (eh::Exception)
  {
    ::expire(*this, session_cookies);
  }
}
