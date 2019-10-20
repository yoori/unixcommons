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





#ifndef APACHE_ADAPTERS_HPP
#define APACHE_ADAPTERS_HPP

#include <ace/Init_ACE.h>

#include <Stream/BinaryStream.hpp>

#include <HTTP/HttpMisc.hpp>

#include <Apache/Module.hpp>


namespace Apache
{
  extern const char REMOTE_HOST_HEADER[];

  struct Protocol
  {
    Protocol(const char* protocol_name) throw (eh::Exception);

    const char* name;
  };
  typedef std::list<Protocol> ProtocolList;

  class ApacheInputStream : public Stream::BinaryInputStream
  {
  public:
    ApacheInputStream(request_rec* r) throw ();

    virtual
    Stream::BinaryInputStream&
    read(char_type* s, streamsize n) throw (eh::Exception);

    void
    has_body(bool val) throw ();

  private:
    request_rec* request_;
    bool has_body_;
  };

  class ApacheOutputStream : public Stream::BinaryOutputStream
  {
  public:
    ApacheOutputStream(request_rec* r) throw ();

    virtual
    Stream::BinaryOutputStream&
    write(const char_type* s, streamsize n) throw (eh::Exception);

  private:
    request_rec* request_;
  };

  class HttpRequest
  {
  public:
    class Exception : public Apache::Exception
    {
    public:
      template <typename T>
      explicit
      Exception(const T& description, int error_code = DECLINED)
        throw ();
      virtual
      ~Exception() throw ();

      int
      error_code() const throw ();

    protected:
      Exception() throw ();

    protected:
      int error_code_;
    };

  public:
    static
    void
    parse_params(const String::SubString& str, HTTP::ParamList& params)
      throw (eh::Exception);

  public:
    HttpRequest(request_rec* r) throw (Exception, eh::Exception);

    int
    method() const throw ();
    const char*
    uri() const throw ();
    const char*
    args() const throw ();
    const HTTP::ParamList&
    params() const throw ();
    const HTTP::SubHeaderList&
    headers() const throw ();
    String::SubString
    body() const throw ();
    ApacheInputStream&
    get_input_stream() const throw ();
    const ProtocolList&
    input_protocols() const throw();
    bool
    secure() const throw();

    String::SubString
    server_name() const throw();

    void
    set_params(HTTP::ParamList&& params) throw ();

    /**
     * HEAD request, as opposed to GET
     * @return true if HEAD request
     */
    bool
    header_only() const throw ();

  protected:
    static
    std::istream&
    get_token_(std::istream& istr, std::string& dst, char delim = 'n')
      throw (eh::Exception);

  private:
    request_rec* r_;
    std::string body_;
    HTTP::ParamList params_;
    HTTP::SubHeaderList headers_;
    mutable ApacheInputStream input_stream_;
    ProtocolList input_protocols_;
    bool secure_;
  };

  class HttpResponse
  {
  public:
    HttpResponse(request_rec* r) throw ();

    void
    add_header(const char* name, const char* value)
      throw (eh::Exception);

    void
    set_content_type(const char* value)
      throw (eh::Exception);

    void
    add_cookie(const char* value)
      throw (eh::Exception);

    ApacheOutputStream&
    get_output_stream() throw ();

  private:
    request_rec* r_;
    ApacheOutputStream output_stream_;
  };

  template <typename T, typename HttpResponse = Apache::HttpResponse>
  class QuickNoParamsHandlerAdapter : public QuickHandlerHook<T>
  {
  public:
    QuickNoParamsHandlerAdapter(int where) throw ();

    virtual
    bool
    will_handle(const char*) throw ();
    virtual
    int
    handle_request_noparams(HttpRequest& request, HttpResponse& response)
      throw (eh::Exception) = 0;

    virtual
    int
    quick_handler(request_rec* r, int lookup_uri) throw ();
  };

  template <typename T, typename HttpResponse = Apache::HttpResponse>
  class QuickHandlerAdapter : public QuickNoParamsHandlerAdapter<T, HttpResponse>
  {
  public:
    QuickHandlerAdapter(int where) throw ();

    virtual
    int
    handle_request_noparams(HttpRequest& request, HttpResponse& response)
      throw (eh::Exception);

    virtual
    int
    handle_request(const HttpRequest& request, HttpResponse& response)
      throw () = 0;
  };

  template <typename T>
  class ChildLifecycleAdapter : public ChildInitHook<T>
  {
  public:
    ChildLifecycleAdapter(int where) throw ();

    virtual
    void
    init() throw ();
    virtual
    void
    shutdown() throw ();

    virtual
    void
    child_init(apr_pool_t* p, server_rec* s) throw ();

  protected:
    static
    apr_status_t
    child_cleanup_s(void* data) throw ();
  };
}

/////////////////////////////////////////////////////////////////////////////
// Inlines
/////////////////////////////////////////////////////////////////////////////

#if __GNUC__ == 4 && __GNUC_MINOR__ == 8
static int* aplog_module_index = 0;
#endif

namespace Apache
{
  //
  // Protocol class
  //

  inline
  Protocol::Protocol(const char* protocol_name) throw (eh::Exception)
    : name(protocol_name)
  {
  }


  //
  // ApacheInputStream
  //

  inline
  ApacheInputStream::ApacheInputStream(request_rec* r) throw ()
    : request_(r), has_body_(false)
  {
  }

  inline
  void
  ApacheInputStream::has_body(bool val) throw ()
  {
    has_body_ = val;
  }


  //
  // ApacheOutputStream
  //

  inline
  ApacheOutputStream::ApacheOutputStream(request_rec* r) throw ()
    : request_(r)
  {
  }


  //
  // HttpRequest::Exception
  //

  template <typename T>
  HttpRequest::Exception::Exception(const T& description,
    int error_code) throw ()
    : Apache::Exception(description), error_code_(error_code)
  {
  }

  inline
  HttpRequest::Exception::~Exception() throw ()
  {
  }

  inline
  int
  HttpRequest::Exception::error_code() const throw ()
  {
    return error_code_;
  }


  //
  // HttpRequest
  //

  inline
  int
  HttpRequest::method() const throw ()
  {
    return r_->method_number;
  }

  inline
  const char*
  HttpRequest::uri() const throw ()
  {
    return r_->uri;
  }

  inline
  const char*
  HttpRequest::args() const throw ()
  {
    return r_->args;
  }

  inline
  const HTTP::ParamList&
  HttpRequest::params() const throw ()
  {
    return params_;
  }

  inline
  const HTTP::SubHeaderList&
  HttpRequest::headers() const throw ()
  {
    return headers_;
  }

  inline
  String::SubString
  HttpRequest::body() const throw ()
  {
    return body_;
  }

  inline
  ApacheInputStream&
  HttpRequest::get_input_stream() const throw ()
  {
    return input_stream_;
  }

  inline
  const ProtocolList&
  HttpRequest::input_protocols() const throw()
  {
    return input_protocols_;
  }

  inline
  bool
  HttpRequest::secure() const throw()
  {
    return secure_;
  }

  inline
  String::SubString
  HttpRequest::server_name() const throw()
  {
    return r_->hostname ? String::SubString(r_->hostname) : String::SubString();
  }

  inline
  bool
  HttpRequest::header_only() const throw ()
  {
    return r_->header_only;
  }

  inline
  void
  HttpRequest::set_params(HTTP::ParamList&& params) throw ()
  {
    params_ = std::move(params);
  }


  //
  // HttpResponse
  //

  inline
  HttpResponse::HttpResponse(request_rec* r) throw ()
    : r_(r), output_stream_(r)
  {
  }

  inline
  ApacheOutputStream&
  HttpResponse::get_output_stream() throw ()
  {
    return output_stream_;
  }


  //
  // QuickNoParamsHandlerAdapter
  //

  template <typename T, typename HttpResponse>
  QuickNoParamsHandlerAdapter<T, HttpResponse>::
    QuickNoParamsHandlerAdapter(int where) throw ()
    : QuickHandlerHook<T>(where)
  {
  }

  template <typename T, typename HttpResponse>
  bool
  QuickNoParamsHandlerAdapter<T, HttpResponse>::will_handle(const char*)
    throw ()
  {
    return true;
  }

  template <typename T, typename HttpResponse>
  int
  QuickNoParamsHandlerAdapter<T, HttpResponse>::quick_handler(
    request_rec* r, int) throw ()
  {
    switch (r->method_number)
    {
    case M_GET:
    case M_POST:
    case M_PUT:
      break;

    default:
      return DECLINED;
    }

    if (!will_handle(r->uri))
    {
      return DECLINED;
    }

    try
    {
      HttpRequest request(r);
      HttpResponse response(r);

      // handle request
      int result = handle_request_noparams(request, response);

      if (result != OK && (result >= 400 || result < 200))
      {
        return result;
      }
      return result;
    }
    catch (const HttpRequest::Exception& e)
    {
      ap_log_error(APLOG_MARK, APLOG_WARNING, 0, r->server, e.what());

      return e.error_code();
    }
    catch (const eh::Exception& e)
    {
      ap_log_error(APLOG_MARK, APLOG_WARNING, 0, r->server, e.what());

      return HTTP_INTERNAL_SERVER_ERROR;
    }
  }

  //
  // QuickHandlerAdapter
  //

  template <typename T, typename HttpResponse>
  QuickHandlerAdapter<T, HttpResponse>::QuickHandlerAdapter(int where)
    throw ()
    : QuickNoParamsHandlerAdapter<T, HttpResponse>(where)
  {
  }

  template <typename T, typename HttpResponse>
  int
  QuickHandlerAdapter<T, HttpResponse>::handle_request_noparams(
    HttpRequest& request, HttpResponse& response)
    throw (eh::Exception)
  {
    HTTP::ParamList params;

    // read parameters
    if (request.args())
    {
      HttpRequest::parse_params(String::SubString(request.args()), params);
    }

    if (!request.body().empty())
    {
      HttpRequest::parse_params(request.body(), params);
    }

    request.set_params(std::move(params));

    return handle_request(request, response);
  }

  //
  // ChildLifecycleAdapter
  //

  template <typename T>
  ChildLifecycleAdapter<T>::ChildLifecycleAdapter(int where) throw ()
    : ChildInitHook<T>(where)
  {
  }

  template <typename T>
  void
  ChildLifecycleAdapter<T>::init() throw ()
  {
  }

  template <typename T>
  void
  ChildLifecycleAdapter<T>::shutdown() throw ()
  {
  }

  template <typename T>
  void
  ChildLifecycleAdapter<T>::child_init(apr_pool_t* p, server_rec*)
    throw ()
  {
    apr_pool_cleanup_register(p, this, child_cleanup_s, child_cleanup_s);
    init();
  }

  template <typename T>
  apr_status_t
  ChildLifecycleAdapter<T>::child_cleanup_s(void* data) throw ()
  {
    ChildLifecycleAdapter* obj =
      static_cast<ChildLifecycleAdapter*>(data);
    obj->shutdown();

    T::instance.reset();

    ACE::fini();

    return APR_SUCCESS;
  }
} // namespace Apache

#endif
