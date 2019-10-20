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



#include <HTTP/HttpClient.hpp>


namespace HTTP
{
  namespace
  {
    const char* const COOKIE = "Cookie";
    const char* const SET_COOKIE = "Set-Cookie";


    class Callback :
      public ResponseCallback,
      public ReferenceCounting::AtomicImpl
    {
    public:
      Callback(ResponseCallback* callback, CookiePoolPtr* cookie) throw ();

      virtual
      void
      on_response(const ResponseInformation& data) throw ();

      virtual
      void
      on_error(
        const String::SubString& description,
        const RequestInformation& data)
        throw ();

    protected:
      virtual
      ~Callback() throw ();

    private:
      ResponseCallback_var callback_;
      CookiePool_var cookie_;
    };

    class CookieClient :
      public HttpInterface,
      public ReferenceCounting::AtomicImpl
    {
    public:
      CookieClient(HttpInterface* pool, CookiePoolPtr* cookie)
        throw (eh::Exception);

      virtual
      void
      add_get_request(const char* http_request,
        ResponseCallback* callback = 0,
        const HttpServer& peer = HttpServer(),
        const HeaderList& headers = HeaderList())
        throw (eh::Exception, Exception);

      virtual
      void
      add_post_request(const char* http_request,
        ResponseCallback* callback = 0,
        const String::SubString& body = String::SubString(),
        const HttpServer& peer = HttpServer(),
        const HeaderList& headers = HeaderList())
        throw (eh::Exception, Exception);

    protected:
      virtual
      ~CookieClient() throw ();

    private:
      void
      add_cookies(const char* url, HeaderList& headers)
        throw (eh::Exception);

      HttpInterface_var pool_;
      CookiePool_var cookie_;
    };


    //
    // Callback class
    //

    Callback::Callback(ResponseCallback* callback, CookiePoolPtr* cookie)
      throw ()
      : callback_(ReferenceCounting::add_ref(callback)),
        cookie_(ReferenceCounting::add_ref(cookie))
    {
    }

    Callback::~Callback() throw ()
    {
    }

    void
    Callback::on_response(const ResponseInformation& data) throw ()
    {
      try
      {
        HTTP::HeaderList headers;
        data.find_headers(SET_COOKIE, headers);
        if (!headers.empty())
        {
          (*cookie_)->load_from_headers(headers,
            HTTPAddress(String::SubString(data.http_request())));
        }
      }
      catch (...)
      {
      }

      callback_->on_response(data);
    }

    void
    Callback::on_error(const String::SubString& description,
      const RequestInformation& data) throw ()
    {
      callback_->on_error(description, data);
    }


    //
    // CookieClient class
    //

    CookieClient::CookieClient(HttpInterface* pool, CookiePoolPtr* cookie)
      throw (eh::Exception)
      : pool_(ReferenceCounting::add_ref(pool)),
        cookie_(ReferenceCounting::add_ref(cookie))
    {
    }

    void
    CookieClient::add_get_request(const char* http_request,
      ResponseCallback* callback, const HttpServer& peer,
      const HeaderList& headers)
      throw (eh::Exception, Exception)
    {
      HeaderList new_headers(headers);
      add_cookies(http_request, new_headers);
      ResponseCallback_var response_callback(
        new Callback(callback, cookie_));
      pool_->add_get_request(http_request, response_callback,
        peer, new_headers);
    }

    void
    CookieClient::add_post_request(const char* http_request,
      ResponseCallback* callback,
      const String::SubString& body, const HttpServer& peer,
      const HeaderList& headers)
      throw (eh::Exception, Exception)
    {
      HeaderList new_headers(headers);
      add_cookies(http_request, new_headers);
      ResponseCallback_var response_callback(
        new Callback(callback, cookie_));
      pool_->add_post_request(http_request, response_callback,
        body, peer, new_headers);
    }

    CookieClient::~CookieClient() throw ()
    {
    }

    void
    CookieClient::add_cookies(const char* url, HeaderList& headers)
      throw (eh::Exception)
    {
      std::string cookie((*cookie_)->cookie_header(HTTPAddress(
        String::SubString(url))));
      if (!cookie.empty())
      {
        headers.emplace_back(COOKIE, cookie);
      }
    }
  }


  //
  //
  //

  HttpInterface*
  CreateCookieClient(HttpInterface* pool, CookiePoolPtr* cookie)
    throw (eh::Exception)
  {
    return new CookieClient(pool, cookie);
  }
}
