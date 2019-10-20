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





#ifndef HTTP_HTTPCOOKIE_HPP
#define HTTP_HTTPCOOKIE_HPP

#include <sstream>
#include <list>

#include <String/StringManip.hpp>

#include <Generics/Time.hpp>

#include <HTTP/HttpMisc.hpp>
#include <HTTP/UrlAddress.hpp>


namespace HTTP
{
  struct Cookie
  {
    Cookie() throw ();

    Cookie(const String::SubString& name, const String::SubString& value)
      throw ();

    String::SubString name;
    String::SubString value;
  };

  struct CookieDef : public Cookie
  {
    CookieDef() throw ();

    CookieDef(const String::SubString& nam, const String::SubString& val,
      const String::SubString& dmn, const String::SubString& pth,
      const Generics::Time& exp, bool sec) throw ();

    String::SubString domain;
    String::SubString path;
    Generics::Time expires;
    bool secure;
  };

  struct PersistentCookieDef
  {
    PersistentCookieDef() throw ();

    PersistentCookieDef(const String::SubString& nam,
      const String::SubString& val, const String::SubString& dmn,
      const String::SubString& pth, const Generics::Time& exp, bool sec)
      throw (eh::Exception);

    operator CookieDef() const throw ();

    std::string name;
    std::string value;
    std::string domain;
    std::string path;
    Generics::Time expires;
    bool secure;
  };

  class CookieList : protected std::list<Cookie>
  {
  public:
    DECLARE_EXCEPTION(Exception, HTTP::Exception);
    DECLARE_EXCEPTION(InvalidArgument, Exception);

    typedef std::list<Cookie> Parent;

    using Parent::iterator;
    using Parent::const_iterator;
    using Parent::begin;
    using Parent::end;
    using Parent::cbegin;
    using Parent::cend;
    using Parent::empty;
    using Parent::size;
    using Parent::clear;
    using Parent::push_back;
    using Parent::emplace_back;

    virtual
    ~CookieList() throw ();

    void
    load_from_headers(const SubHeaderList& headers,
      bool replace_duplicate = false)
      throw (Exception, InvalidArgument, eh::Exception);

    void
    load_from_headers(const HeaderList& headers,
      bool replace_duplicate = false)
      throw (Exception, InvalidArgument, eh::Exception);

    std::string
    cookie_header() throw (eh::Exception);

  private:
    template <typename HeaderList>
    void
    load_from_headers_(const HeaderList& headers,
      bool replace_duplicate)
      throw (Exception, InvalidArgument, eh::Exception);
  };

  class CookieDefList : protected std::list<CookieDef>
  {
  public:
    DECLARE_EXCEPTION(Exception, HTTP::Exception);
    DECLARE_EXCEPTION(InvalidArgument, Exception);

    typedef std::list<CookieDef> Parent;

    using Parent::iterator;
    using Parent::const_iterator;
    using Parent::begin;
    using Parent::end;
    using Parent::cbegin;
    using Parent::cend;
    using Parent::empty;
    using Parent::size;
    using Parent::clear;
    using Parent::push_back;
    using Parent::emplace_back;

    explicit
    CookieDefList(bool keep_expired = false) throw (eh::Exception);

    virtual
    ~CookieDefList() throw ();

    void
    load_from_headers(const SubHeaderList& headers,
      const HTTPAddress& url_address)
      throw (Exception, InvalidArgument, eh::Exception);
    void
    load_from_headers(const HeaderList& headers,
      const HTTPAddress& url_address)
      throw (Exception, InvalidArgument, eh::Exception);

    void
    set_cookie_header(HeaderList& headers)
      throw (eh::Exception);

    /**
     * Set cookie headers without any checks
     */
    void
    set_cookie_header_plain(HeaderList& headers)
      throw (eh::Exception);

    std::string
    cookie_header(const HTTPAddress& url_address)
      throw (eh::Exception);

  protected:
    void
    expire_(bool sesion_cookies = false) throw (eh::Exception);

    bool keep_expired_;
  };

  class ClientCookieFacility : protected std::list<PersistentCookieDef>
  {
  public:
    DECLARE_EXCEPTION(Exception, HTTP::Exception);
    DECLARE_EXCEPTION(InvalidArgument, Exception);

    typedef std::list<PersistentCookieDef> Parent;

    using Parent::clear;
    using Parent::push_back;
    using Parent::emplace_back;

    virtual
    ~ClientCookieFacility() throw ();

    void
    load_from_headers(const SubHeaderList& headers,
      const HTTPAddress& url_address)
      throw (Exception, InvalidArgument, eh::Exception);
    void
    load_from_headers(const HeaderList& headers,
      const HTTPAddress& url_address)
      throw (Exception, InvalidArgument, eh::Exception);

    void
    set_cookie_header(HeaderList& headers)
      throw (eh::Exception);

    /**
     * Set cookie headers without any checks
     */
    void
    set_cookie_header_plain(HeaderList& headers)
      throw (eh::Exception);

    std::string
    cookie_header(const HTTPAddress& url_address)
      throw (eh::Exception);

    void
    end_session() throw (eh::Exception);

  protected:
    void
    expire_(bool sesion_cookies = false) throw (eh::Exception);
  };

  std::string
  cookie_date(const Generics::Time& time, bool show_usec = false)
    throw (eh::Exception);

  template <typename CookieDef>
  void
  cookie_header_plain(const CookieDef& cookie, std::string& dst)
    throw (eh::Exception);
} // namespace HTTP

///////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////

namespace HTTP
{
  //
  // Cookie struct
  //

  inline
  Cookie::Cookie(const String::SubString& nam,
    const String::SubString& val) throw ()
    : name(nam), value(val)
  {
  }

  inline
  Cookie::Cookie() throw ()
  {
  }


  //
  // CookieDef struct
  //

  inline
  CookieDef::CookieDef() throw ()
    : expires(Generics::Time::ZERO), secure(false)
  {
  }

  inline
  CookieDef::CookieDef(const String::SubString& nam,
    const String::SubString& val, const String::SubString& dmn,
    const String::SubString& pth, const Generics::Time& exp, bool sec)
    throw ()
    : Cookie(nam, val), domain(dmn), path(pth), expires(exp), secure(sec)
  {
  }


  //
  // PersistentCookieDef struct
  //

  inline
  PersistentCookieDef::PersistentCookieDef() throw ()
    : expires(Generics::Time::ZERO), secure(false)
  {
  }

  inline
  PersistentCookieDef::PersistentCookieDef(const String::SubString& nam,
    const String::SubString& val, const String::SubString& dmn,
    const String::SubString& pth, const Generics::Time& exp, bool sec)
    throw (eh::Exception)
    : name(nam.str()), value(val.str()), domain(dmn.str()), path(pth.str()),
      expires(exp), secure(sec)
  {
  }

  inline
  PersistentCookieDef::operator CookieDef() const throw ()
  {
    return CookieDef(name, value, domain, path, expires, secure);
  }

  //
  // ClientCookieFacility class
  //

  inline
  void
  ClientCookieFacility::end_session() throw (eh::Exception)
  {
    expire_(true);
  }


  //

  inline
  std::string
  cookie_date(const Generics::Time& tim, bool show_usec)
    throw (eh::Exception)
  {
    const Generics::ExtendedTime& time = tim.get_gm_time();
    char out[128];
    snprintf(out, sizeof(out), "%s, %i-%s-%i %02i:%02i:%02i",
      Generics::Time::week_day(time.tm_wday), time.tm_mday,
      Generics::Time::month(time.tm_mon), time.tm_year + 1900,
      time.tm_hour, time.tm_min, time.tm_sec);

    if (show_usec && time.tm_usec)
    {
      size_t length = strlen(out);
      snprintf(out + length, sizeof(out) - length, ".%06i GMT",
        time.tm_usec);
    }
    else
    {
      String::StringManip::strlcat(out, " GMT", sizeof(out));
    }

    return out;
  }

  template <typename CookieDef>
  void
  cookie_header_plain(const CookieDef& cookie, std::string& dst)
    throw (eh::Exception)
  {
    std::ostringstream header;

    header << cookie.name << "=" << cookie.value;

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

    header.str().swap(dst);
  }
} // namespace HTTP

#endif
