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



#ifndef HTTP_HTTPASYNC_HPP
#define HTTP_HTTPASYNC_HPP

#include <Generics/ActiveObject.hpp>
#include <Generics/Time.hpp>

#include <HTTP/HttpMisc.hpp>


namespace HTTP
{
  enum HttpMethod
  {
    HM_POST,
    HM_GET
  };

  const char*
  method_name(HttpMethod method) throw ();


  // host name and port
  typedef std::pair<std::string, int> HttpServer;


  /**
   * Class describing http-request
   */
  class RequestInformation
  {
  public:
    /**
     * Destructor
     */
    virtual
    ~RequestInformation() throw ();


    /**
     * Request method
     * @return request method
     */
    virtual
    HttpMethod
    method() const throw () = 0;

    /**
     * Request URI
     * @return request URI string
     */
    virtual
    const char*
    http_request() const throw () = 0;

    /**
     * List of provided request headers
     * @return list of request headers
     */
    virtual
    const HeaderList&
    headers() const throw () = 0;
  };

  /**
   * Class describing http-request and http-response
   */
  class ResponseInformation : public RequestInformation
  {
  public:
    /**
     * Response code
     * @return response code
     */
    virtual
    int
    response_code() const throw () = 0;

    /**
     * Response headers
     * @return response headers
     */
    virtual
    const HeaderList&
    response_headers() const throw () = 0;

    /**
     * Searches for specific header in the response
     * @param name header name
     * @param headers all headers with the specified name to place to
     */
    void
    find_headers(const char* name, HeaderList& headers) const
      throw (eh::Exception);

    /**
     * Response body
     * @return response body data
     */
    virtual
    String::SubString
    body() const throw () = 0;
  };


  /**
   * Callback class allowing to inform about successes and failures
   * for HTTP requests
   */
  class ResponseCallback : public virtual ReferenceCounting::Interface
  {
  public:
    /**
     * Called when request succeeded
     * @param data response
     */
    virtual
    void
    on_response(const ResponseInformation& data) throw () = 0;

    /**
     * Called when request succeeded and it is not possible to call on_response
     * Should return control ASAP
     * @param data response
     */
    virtual
    void
    quick_on_response(const ResponseInformation& data) throw ();

    /**
     * Called when request failed
     * @param description error message
     * @param data request
     */
    virtual
    void
    on_error(
      const String::SubString& description,
      const RequestInformation& data)
      throw () = 0;

    /**
     * Called when request failed and it is not possible to call on_error
     * Should return control ASAP
     * @param description error message
     * @param data request
     */
    virtual
    void
    quick_on_error(
      const String::SubString& description,
      const RequestInformation& data)
      throw ();

  protected:
    /**
     * Destructor
     */
    virtual
    ~ResponseCallback() throw ();
  };
  typedef ReferenceCounting::QualPtr<ResponseCallback> ResponseCallback_var;


  /**
   * General HTTP interface
   */
  class HttpInterface : public virtual ReferenceCounting::Interface
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    /**
     * Adds GET request for execution
     * Order of execution is unspecified
     * @param http_request request URI
     * @param callback callback to call for the request
     * @param peer http server address
     * @param headers list of additional headers for request
     */
    virtual
    void
    add_get_request(const char* http_request,
      ResponseCallback* callback = 0,
      const HttpServer& peer = HttpServer(),
      const HeaderList& headers = HeaderList())
      throw (eh::Exception, Exception) = 0;

    /**
     * Adds POST request for execution
     * Order of execution is unspecified
     * @param http_request request URI
     * @param callback callback to call for the request
     * @param body request data to post
     * @param peer http server address
     * @param headers list of additional headers for request
     */
    virtual
    void
    add_post_request(const char* http_request,
      ResponseCallback* callback = 0,
      const String::SubString& body = String::SubString(),
      const HttpServer& peer = HttpServer(),
      const HeaderList& headers = HeaderList())
      throw (eh::Exception, Exception) = 0;

  protected:
    /**
     * Destructor
     */
    virtual
    ~HttpInterface() throw ();
  };
  typedef ReferenceCounting::QualPtr<HttpInterface>
    HttpInterface_var;


  /**
   * Http interface allowing activity control
   */
  class HttpActiveInterface :
    public HttpInterface,
    public Generics::ActiveObject
  {
  public:
    typedef HttpInterface::Exception Exception;
    typedef Generics::ActiveObject::Exception ActiveObjectException;

  protected:
    /**
     * Destructor
     */
    virtual
    ~HttpActiveInterface() throw ();
  };
  typedef ReferenceCounting::QualPtr<HttpActiveInterface>
    HttpActiveInterface_var;


  /**
   * Helper function for VxInter::HTTP_Connection wrapper creation
   */
  HttpInterface*
  CreateSyncHttp(const Generics::Time* connect_timeout = 0,
    const Generics::Time* send_timeout = 0,
    const Generics::Time* recv_timeout = 0)
    throw (eh::Exception);
}

#endif
