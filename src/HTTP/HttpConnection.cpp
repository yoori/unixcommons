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



// File   : HttpConnection.cpp
// Author : Pavel Gubin

#include <sstream>

#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <ace/INET_Addr.h>

#include <eh/Errno.hpp>

#include <String/StringManip.hpp>

#include <Generics/Time.hpp>

#include <Stream/SocketStream.hpp>

#include <HTTP/HttpConnection.hpp>


namespace
{
  void
  throw_exception(String::SubString function, const char* description)
    throw (HTTP::HTTP_Connection::Timeout,
      HTTP::HTTP_Connection::Exception)
  {
    if (errno == ETIME)
    {
      eh::throw_errno_exception<HTTP::HTTP_Connection::Timeout>(
        function, "(): ", description);
    }
    else
    {
      eh::throw_errno_exception<HTTP::HTTP_Connection::Exception>(
        function, "(): ", description);
    }
  }
}

namespace HTTP
{
  //
  // HTTP_Connection
  //

  HTTP_Connection::HTTP_Connection(const HTTPAddress& url, const char* proxy)
    throw (eh::Exception)
    : url_(url), proxy_port_(0)
  {
    if (proxy)
    {
      const char* port = strchr(proxy, ':');

      if (!port)
      {
        proxy_host_ = proxy;
        proxy_port_ = 3128;
      }
      else
      {
        proxy_host_.assign(proxy, port - proxy);
        proxy_port_ = atol(port + 1);
      }
    }
  }

  HTTP_Connection::~HTTP_Connection() throw ()
  {
    stream_.close();
  }

  void
  HTTP_Connection::connect(const Generics::Time* connect_timeout,
    const ACE_Addr& local_ip, ACE_Addr& addr) throw (eh::Exception, Exception)
  {
    ACE_INET_Addr& inet_addr = static_cast<ACE_INET_Addr&>(addr);

    if (inet_addr == ACE_INET_Addr() /*addr == ACE_Addr::sap_any*/)
    {
      inet_addr = proxy_host_.empty() ?
        ACE_INET_Addr(url_.port_number(), url_.host().str().c_str()) :
        ACE_INET_Addr(proxy_port_, proxy_host_.c_str());
    }

    ACE_Time_Value connect_timeout_ace(
      connect_timeout ? *connect_timeout : Generics::Time());
    ACE_SOCK_Connector connector;
    if (connector.connect(stream_, inet_addr,
      connect_timeout ? &connect_timeout_ace : 0, local_ip) == -1)
    {
      throw_exception(FNB, "Connection error");
    }
  }

  void
  HTTP_Connection::connect(const Generics::Time* connect_timeout,
    const ACE_Addr& local_ip) throw (eh::Exception, Exception)
  {
    ACE_INET_Addr addr;
    connect(connect_timeout, local_ip, addr);
  }


/// Execute HTTP request
/**
*   Forges HTTP request from arguments. Sends the request to the stream.
*   Calls parse_response if the response is expected.
*
*   @param[in] method: can be one of \b HM_Post or \b HM_Get
*   @param[in] params: request parameters
*   @param[in] headers: HTTP headers
*   @param[in] body
*   @param[in] need_response(true): if \b true then parse_response is called
*   @param[in] send_timeout(NULL): timeout values
*   @param[in] recv_timeout(NULL): timeout values
*   @param[out] bytes_sent(NULL): number of bytes sent to the stream
*   @param[out] bytes_rcvd(NULL): number of bytes received from the stream
*   @param response_latency(NULL)
*/

  unsigned int
  HTTP_Connection::process_request(
    HTTP_Method method, const HTTP::ParamList& params,
    HTTP::HeaderList& headers, HttpBody*& body, bool need_response,
    const Generics::Time* send_timeout, const Generics::Time* recv_timeout,
    unsigned int* bytes_sent, unsigned int* bytes_rcvd,
    Generics::Time* response_latency)
    throw (eh::Exception, Exception)
  {
    unsigned int status = 0;

    try
    {
      unsigned int sent = 0;

      std::ostringstream request;
      switch (method)
      {
      case HM_Post:
        request << "POST ";
        break;

      case HM_Get:
        request << "GET ";
        break;

      default:
        Stream::Error ostr;
        ostr << FNS << "Unknown method requested.";
        throw InvalidArgs(ostr);
      }

      request << (proxy_host_.empty() ? url_.path() : url_.url());
      std::string params_seq;
      if (proxy_host_.empty())
      {
        url_.query().assign_to(params_seq);
      }

      if (!params.empty())
      {
        std::ostringstream params_str;
        for (HTTP::ParamList::const_iterator it = params.begin();
          it != params.end(); ++it)
        {
          if (it != params.begin())
          {
            params_str << "&";
          }

          std::string name;
          String::StringManip::mime_url_encode(it->name, name);
          std::string value;
          String::StringManip::mime_url_encode(it->value, value);
          params_str << name << "=" << value;
        }

        if (!params_seq.empty())
        {
          params_seq += "&";
        }
        params_seq += params_str.str();
      }

      if (!params_seq.empty())
      {
        // Sometimes we must pass different parameters both in URI and body
        if (method == HM_Get || body)
        {
          if (!proxy_host_.empty() && url_.query()[0] != '\0')
          {
            request << params_seq.c_str();
          }
          else
          {
            request << "?" << params_seq.c_str();
          }
        }
        else
        {
          body = new HTTP_Connection::HttpBody();
          body->init(params_seq.c_str(), params_seq.length());
        }
      }

      request << " HTTP/1.0" << "\r\n";

      // headers

      bool add_host_hdr = true;
      for (HTTP::HeaderList::iterator it = headers.begin();
        it != headers.end(); ++it)
      {
        request << it->name << ": " << it->value << "\r\n";

        if (it->name == String::AsciiStringManip::Caseless("Host"))
        {
          add_host_hdr = false;
        }
      }

      unsigned int body_len = body ? body->total_size() : 0;

      request << "Content-Length: " << body_len << "\r\n";
      if (add_host_hdr)
      {
        request << "Host: " << url_.host();
        if (url_.port_number() != 80)
        {
          request << ":" << url_.port_number();
        }
        request << "\r\n";
      }
      request << "\r\n";

      const std::string& request_string = request.str();
      ssize_t request_len = request_string.length();
      ACE_Time_Value send_timeout_ace(
        send_timeout ? *send_timeout : Generics::Time());
      if (stream_.send_n(request_string.data(), request_len,
        send_timeout ? &send_timeout_ace : 0) != request_len)
      {
        throw_exception(FNB, "failed to send HTTP headers");
      }
      sent += request_len;

      ACE_Message_Block* body_ptr = body;
      while (body_ptr)
      {
        if (stream_.send_n(body_ptr->base(), body_ptr->size(),
          send_timeout ? &send_timeout_ace : 0) !=
            static_cast<ssize_t>(body_ptr->size()))
        {
          throw_exception(FNB, "failed to send data");
        }
        sent += body_ptr->size();
        body_ptr = body_ptr->cont();
      }

//      stream_.close_writer();

      if (bytes_sent)
      {
        *bytes_sent = sent;
      }

      if (need_response)
      {
        status = parse_response(headers, body, recv_timeout, bytes_rcvd,
          response_latency);
      }

      stream_.close();
    }
    catch (...)
    {
      stream_.close();
      throw;
    }

    return status;
  }

/// Process HTTP response
/**
*   Gets HTTP response from the stream. Parses the response.
*   @param[out]  bytes_rcvd: number of bytes received from the stream
*/

  unsigned int
  HTTP_Connection::parse_response(HTTP::HeaderList& headers,
    HttpBody*& body, const Generics::Time* recv_timeout,
    unsigned int* bytes_rcvd, Generics::Time* responce_latency)
    throw (eh::Exception, Exception)
  {
    headers.clear();
    if (body)
    {
      body->release();
      body = 0;
    }

    Generics::Time resp_latency;
    Generics::Timer timer;
    timer.start();

    std::string str;
    Stream::SocketInStream in(stream_, recv_timeout);

    unsigned int status_code = 0;

    try
    {
      in >> str; // skip HTTP version

      timer.stop();
      resp_latency = timer.elapsed_time();

      if (responce_latency)
      {
        *responce_latency = resp_latency;
      }

      timer.start();

      in >> status_code; // read status code

      std::getline(in, str, '\r'); // read reason phrase

      if (status_code < 200 || status_code >= 400)
      {
        Stream::Error ostr;
        ostr << FNS << "status code " << status_code << ", reason " << str;
        throw StatusException(ostr, status_code);
      }

      if (in.get() != '\n')
      {
        Stream::Error ostr;
        ostr << FNS << "invalid response format";
        throw Exception(ostr);
      }

      ssize_t body_len = -1;

      // reading headers
      for (;;)
      {
        std::getline(in, str, '\r');
        if (in.get() != '\n')
        {
          Stream::Error ostr;
          ostr << FNS << "invalid response format";
          throw Exception(ostr);
        }

        if (str.empty())
        {
          // got empty line
          break;
        }

        std::string::size_type colon_pos = str.find(':');
        if (colon_pos == std::string::npos)
        {
          Stream::Error ostr;
          ostr << FNS << "cannot find colon in header";
          throw Exception(ostr);
        }

        const char* const STR_START = str.c_str();
        const char* name_start = STR_START;
        while (std::isspace(*name_start))
        {
          ++name_start;
        }
        std::string name(name_start, STR_START + colon_pos);

        const char* value_start = STR_START + colon_pos + 1;
        while (std::isspace(*value_start))
        {
          ++value_start;
        }
        std::string value(value_start);

        headers.emplace_back(name, value);

        if (name == String::AsciiStringManip::Caseless("Content-Length"))
        {
          Stream::Parser istr(value.data(), value.size());
          istr >> body_len;
        }
      }

      HttpBody* cur_block = body;
      const ssize_t BUFFER_SIZE = 1000;
      if (body_len > 0 || body_len == -1)
      {
        while (in.good() && body_len)
        {
          ssize_t to_read = body_len > 0 && body_len < BUFFER_SIZE ?
            body_len : BUFFER_SIZE;
          Generics::ArrayChar buf(to_read);
          size_t n = in.rdbuf()->sgetn(buf.get(), to_read);
          if (!n)
          {
            break;
          }
          HttpBody* new_block = new HttpBody(buf.get(), to_read);
          buf.release();
          new_block->size(n);
          new_block->clr_flags(ACE_Message_Block::DONT_DELETE);
          if (cur_block)
          {
            cur_block->cont(new_block);
          }
          else
          {
            body = new_block;
          }
          cur_block = new_block;
          if (body_len > 0)
          {
            body_len -= n;
          }
        }

        if (body_len > 0)
        {
          Stream::Error ostr;
          ostr << FNS << "unexpected EOF";
          throw Exception(ostr);
        }
      }
    }
    catch (const Exception& ex)
    {
      if (in.fail())
      {
        timer.stop();

        Generics::Time tv = timer.elapsed_time();

        tv += resp_latency;

        Stream::Error ostr;
        ostr << FNS << "reading response failed ( time: " << tv.tv_sec <<
          " ). Exception:" << ex.what();
        throw Exception(ostr);
      }

      throw;
    }

    if (bytes_rcvd)
    {
      *bytes_rcvd = in.bytes_received();
    }

    return status_code;
  }
}
