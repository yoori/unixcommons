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



// File   : HttpConnection.hpp
// Author : Pavel Gubin

#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include <list>

#include <ace/SOCK_Stream.h>
#include <ace/Message_Block.h>

#include <Generics/Time.hpp>

#include <HTTP/UrlAddress.hpp>
#include <HTTP/Http.hpp>


namespace HTTP
{
  class HTTP_Connection
  {
  public:
    enum HTTP_Method
    {
      HM_Post,
      HM_Get
    };

  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(InvalidArgs, Exception);
    DECLARE_EXCEPTION(Timeout, Exception);

    class StatusException : public Exception
    {
    public:
      unsigned int status;

      StatusException(const char* description, unsigned int status)
        throw ();
      StatusException(const std::string& description, unsigned int status)
        throw ();
      StatusException(const Stream::Error& ostr, unsigned int status)
        throw ();
    };

    typedef ACE_Message_Block HttpBody;

  public:
    explicit
    HTTP_Connection(const HTTPAddress& url,
      const char* proxy = 0) throw (eh::Exception);
    virtual
    ~HTTP_Connection() throw ();

    ACE_SOCK_Stream&
    stream() throw ();

    void
    connect(const Generics::Time* connect_timeout = 0,
      const ACE_Addr& local_ip = ACE_Addr::sap_any)
      throw (eh::Exception, Exception);

    void
    connect(const Generics::Time* connect_timeout,
      const ACE_Addr& local_ip, ACE_Addr& addr)
      throw (eh::Exception, Exception);


    unsigned int
    process_request(HTTP_Method method,
      const HTTP::ParamList& params, HTTP::HeaderList& headers,
      HttpBody*& body, bool need_response = true,
      const Generics::Time* send_timeout = 0,
      const Generics::Time* recv_timeout = 0,
      unsigned int* bytes_sent = 0, unsigned int* bytes_rcvd = 0,
      Generics::Time* response_latency = 0)
      throw (eh::Exception, Exception);

  protected:
    unsigned int
    parse_response(HTTP::HeaderList& headers, HttpBody*& body,
      const Generics::Time* recv_timeout = 0,
      unsigned int* bytes_rcvd = 0,
      Generics::Time* responce_latency = 0)
      throw (eh::Exception, Exception);

  protected:
    HTTPAddress url_;
    ACE_SOCK_Stream stream_;
    std::string proxy_host_;
    unsigned short proxy_port_;
  };
} // namespace HTTP

namespace HTTP
{
  //
  // HTTP_Connection::StatusException class
  //

  inline
  HTTP_Connection::StatusException::StatusException(const char* description,
    unsigned int status) throw ()
    : Exception(description), status(status)
  {
  }

  inline
  HTTP_Connection::StatusException::StatusException(
    const std::string& description, unsigned int status) throw ()
    : Exception(description), status(status)
  {
  }

  inline
  HTTP_Connection::StatusException::StatusException(
    const Stream::Error& ostr, unsigned int status) throw ()
    : Exception(ostr), status(status)
  {
  }


  //
  // HTTP_Connection class
  //

  inline
  ACE_SOCK_Stream&
  HTTP_Connection::stream() throw ()
  {
    return stream_;
  }
}

#endif
