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



#include <HTTP/HttpConnection.hpp>
#include <HTTP/HttpAsync.hpp>


namespace HTTP
{
  //
  // ResponseCallback class
  //

  ResponseCallback::~ResponseCallback() throw ()
  {
  }

  void
  ResponseCallback::quick_on_response(const ResponseInformation& data)
    throw ()
  {
    on_response(data);
  }

  void
  ResponseCallback::quick_on_error(const String::SubString& description,
    const RequestInformation& data) throw ()
  {
    on_error(description, data);
  }


  //
  // RequestInformation class
  //

  RequestInformation::~RequestInformation() throw ()
  {
  }


  //
  // ResponseInformation class
  //

  void
  ResponseInformation::find_headers(const char* name,
    HeaderList& headers) const throw (eh::Exception)
  {
    String::AsciiStringManip::Caseless header_name(name);
    const HeaderList& rheaders = response_headers();

    for (HeaderList::const_iterator itor(rheaders.begin());
      itor != rheaders.end(); ++itor)
    {
      if (header_name == itor->name)
      {
        headers.push_back(*itor);
      }
    }
  }


  //
  // HttpInterface class
  //

  HttpInterface::~HttpInterface() throw ()
  {
  }


  //
  // HttpActiveInterface class
  //

  HttpActiveInterface::~HttpActiveInterface() throw ()
  {
  }


  const char*
  method_name(HttpMethod method) throw ()
  {
    switch (method)
    {
    case HM_POST:
      return "Post";
    case HM_GET:
      return "Get";
    default:
      break;
    }
    return "Unknown";
  }

  namespace
  {
    class HttpConnectionWrapper :
      public HttpInterface,
      public ReferenceCounting::AtomicImpl
    {
    public:
      HttpConnectionWrapper(const Generics::Time* connect_timeout,
        const Generics::Time* send_timeout,
        const Generics::Time* recv_timeout)
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
      ~HttpConnectionWrapper() throw ();

    private:
      void
      do_request_(HttpMethod method,
        HTTP_Connection::HTTP_Method vx_method,
        const char* request, ResponseCallback* callback,
        const String::SubString& body, const HttpServer& peer,
        const HeaderList& headers)
        throw (eh::Exception, Exception);

      std::unique_ptr<Generics::Time> connect_timeout_;
      std::unique_ptr<Generics::Time> send_timeout_;
      std::unique_ptr<Generics::Time> recv_timeout_;
    };

    class Response : public ResponseInformation
    {
    public:
      Response(HttpMethod method, const char* request,
        const HeaderList& headers) throw ();

      void
      response(int response_code, const HeaderList& response_headers,
        const String::SubString& response_body, ResponseCallback* callback)
        throw ();

      virtual
      HttpMethod
      method() const throw ();

      virtual
      const char*
      http_request() const throw ();

      virtual
      const HeaderList&
      headers() const throw ();

      virtual
      int
      response_code() const throw ();

      virtual
      const HeaderList&
      response_headers() const throw ();

      virtual
      String::SubString
      body() const throw ();

    private:
      HttpMethod method_;
      const char* request_;
      const HeaderList& headers_;

      int response_code_;
      const HeaderList* response_headers_;
      const String::SubString* response_body_;
    };


    //
    // Response class
    //

    Response::Response(HttpMethod method, const char* request,
      const HeaderList& headers) throw ()
      : method_(method), request_(request), headers_(headers),
        response_code_(0), response_body_(0)
    {
    }

    void
    Response::response(int response_code, const HeaderList& response_headers,
      const String::SubString& response_body, ResponseCallback* callback)
      throw ()
    {
      if (callback)
      {
        response_code_ = response_code;
        response_headers_ = &response_headers;
        response_body_ = &response_body;
        callback->on_response(*this);
      }
    }

    HttpMethod
    Response::method() const throw ()
    {
      return method_;
    }

    const char*
    Response::http_request() const throw ()
    {
      return request_;
    }

    const HeaderList&
    Response::headers() const throw ()
    {
      return headers_;
    }

    int
    Response::response_code() const throw ()
    {
      return response_code_;
    }

    const HeaderList&
    Response::response_headers() const throw ()
    {
      return *response_headers_;
    }

    String::SubString
    Response::body() const throw ()
    {
      return *response_body_;
    }


    //
    // HttpConnectionWrapper class
    //

    HttpConnectionWrapper::HttpConnectionWrapper(
      const Generics::Time* connect_timeout,
      const Generics::Time* send_timeout,
      const Generics::Time* recv_timeout)
      throw (eh::Exception)
      : connect_timeout_(connect_timeout ?
          new Generics::Time(*connect_timeout) : 0),
        send_timeout_(send_timeout ? new Generics::Time(*send_timeout) : 0),
        recv_timeout_(recv_timeout ? new Generics::Time(*recv_timeout) : 0)
    {
    }

    HttpConnectionWrapper::~HttpConnectionWrapper() throw ()
    {
    }

    void
    HttpConnectionWrapper::add_get_request(const char* http_request,
      ResponseCallback* callback, const HttpServer& peer,
      const HeaderList& headers)
      throw (eh::Exception, Exception)
    {
      do_request_(HM_GET, HTTP_Connection::HM_Get, http_request,
        callback, String::SubString(), peer, headers);
    }

    void
    HttpConnectionWrapper::add_post_request(const char* http_request,
      ResponseCallback* callback,
      const String::SubString& body, const HttpServer& peer,
      const HeaderList& headers)
      throw (eh::Exception, Exception)
    {
      do_request_(HM_POST, HTTP_Connection::HM_Post, http_request,
        callback, body, peer, headers);
    }

    void
    HttpConnectionWrapper::do_request_(HttpMethod method,
      HTTP_Connection::HTTP_Method vx_method, const char* request,
      ResponseCallback* callback,
      const String::SubString& body, const HttpServer& peer,
      const HeaderList& headers)
      throw (eh::Exception, Exception)
    {
      if (!request)
      {
        Stream::Error ostr;
        ostr << FNS << "request is NULL";
        throw Exception(ostr);
      }

      Response response(method, request, headers);

      HTTP_Connection::HttpBody* http_body =
        new HTTP_Connection::HttpBody();
      if (!body.empty())
      {
        http_body->init(body.data(), body.size());
      }

      try
      {
        HeaderList response_headers;
        std::copy(headers.begin(), headers.end(),
          std::back_inserter(response_headers));

        std::string proxy;
        if (!peer.first.empty())
        {
          Stream::Stack<1024> ostr;
          ostr << peer.first << ":" << peer.second;
          ostr.str().assign_to(proxy);
        }

        HTTP_Connection connection(HTTPAddress(String::SubString(request)),
          peer.first.empty() ? 0 : proxy.c_str());

        connection.connect(connect_timeout_.get());

        int status = connection.process_request(vx_method, ParamList(),
          response_headers, http_body, callback != 0, send_timeout_.get(),
          recv_timeout_.get());

        std::vector<char> response_body;
        if (http_body)
        {
          response_body.reserve(http_body->total_size());
          for (HTTP_Connection::HttpBody* block = http_body; block;
            block = block->cont())
          {
            response_body.insert(response_body.end(), block->base(),
              block->base() + block->size());
          }
        }

        response.response(status, response_headers,
          http_body ? String::SubString(&response_body.front(),
            response_body.size()) : String::SubString(),
          callback);
      }
      catch (const HTTP_Connection::StatusException& ex)
      {
        const char* data = ex.what();
        response.response(ex.status, HeaderList(),
          String::SubString(data, data ? strlen(data) + 1 : 0),
          callback);
      }
      catch (const eh::Exception& ex)
      {
        if (callback)
        {
          callback->on_error(String::SubString(ex.what()), response);
        }
      }
      catch (...)
      {
        if (callback)
        {
          Stream::Error ostr;
          ostr << FNS << "Unknown exception";
          callback->on_error(ostr.str(), response);
        }
      }

      if (http_body)
      {
        http_body->release();
      }
    }
  }

  HttpInterface*
  CreateSyncHttp(const Generics::Time* connect_timeout,
    const Generics::Time* send_timeout,
    const Generics::Time* recv_timeout)
    throw (eh::Exception)
  {
    return new HttpConnectionWrapper(connect_timeout, send_timeout,
      recv_timeout);
  }
}
