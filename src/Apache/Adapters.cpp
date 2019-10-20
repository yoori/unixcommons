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





#include <String/StringManip.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>

#include <Apache/Adapters.hpp>


namespace
{
  const String::AsciiStringManip::CharCategory NONTOKEN(
    "\x01-\x31()<>@,;:\\\"/[]?={} \t", true);

  const String::AsciiStringManip::Caseless FORMURL(
    "application/x-www-form-urlencoded");

  const String::AsciiStringManip::Caseless SECURE_PROTOCOL_NAME(
    "ssl/tls filter");
}

namespace Apache
{
  const char REMOTE_HOST_HEADER[] = ".RemoteHost";

  //
  // ApacheInputStream
  //

  Stream::BinaryInputStream&
  ApacheInputStream::read(char_type* s, streamsize n)
    throw (eh::Exception)
  {
    if (!has_body_)
    {
      setstate(std::ios_base::badbit | std::ios_base::failbit);
    }

    if (n > 0)
    {
      int len = ap_get_client_block(request_, s, n);
      if (len == -1)
      {
        setstate(std::ios_base::badbit | std::ios_base::failbit);
      }
      else if (len == 0)
      {
        setstate(std::ios_base::eofbit | std::ios_base::failbit);
        gcount_ = 0;
      }
      else
      {
        gcount_ = len;
      }
    }

    return *this;
  }


  //
  // ApacheOutputStream
  //

  Stream::BinaryOutputStream&
  ApacheOutputStream::write(const char_type* s, streamsize n)
    throw (eh::Exception)
  {
    int len = ap_rwrite(s, n, request_);
    if (len <= 0 && n != 0)
    {
      setstate(std::ios_base::badbit | std::ios_base::failbit);
    }
    else if (len != 0)
    {
      setstate(std::ios_base::eofbit);
    }

    return *this;
  }


  //
  // HttpRequest
  //
  HttpRequest::HttpRequest(request_rec* r) throw (Exception, eh::Exception)
    : r_(r), input_stream_(r), secure_(false)
  {
    bool has_req_body = false;

    // prepare to read request body
    if (r->method_number == M_POST || r->method_number == M_PUT)
    {
      int err_code = ap_setup_client_block(r, REQUEST_CHUNKED_DECHUNK);
      if (err_code != OK)
      {
        Stream::Error ostr;
        ostr << FNS << "ap_setup_client_block failed.";
        throw Exception(ostr, err_code);
      }

      if (ap_should_client_block(r))
      {
        has_req_body = true;
        input_stream_.has_body(true);
      }
    }


    // read headers
    bool params_in_body = false;
    const apr_array_header_t* header_arr = apr_table_elts(r->headers_in);
    apr_table_entry_t* header_elt =
      reinterpret_cast<apr_table_entry_t*>(header_arr->elts);
    for (int i = 0; i < header_arr->nelts; i++)
    {
      String::SubString key(header_elt[i].key);
      String::SubString val(header_elt[i].val);
      headers_.emplace_back(key, val);

      if (key == String::AsciiStringManip::Caseless("content-type") &&
        FORMURL.start(val) && NONTOKEN(val[FORMURL.str.length()]))
      {
        params_in_body = true;
      }
    }

    // Fill RemoteHost
#if __GNUC__ == 4 && __GNUC_MINOR__ == 8
    headers_.emplace_back(REMOTE_HOST_HEADER, r->connection->client_ip);
#else
    headers_.emplace_back(REMOTE_HOST_HEADER, r->connection->remote_ip);
#endif

    if (r->method_number == M_POST || r->method_number == M_PUT)
    {
      if (params_in_body)
      {
        if (!has_req_body)
        {
          Stream::Error ostr;
          ostr << FNS << "could not find parameters in body while request "
            "proposes to do so.";
          throw Exception(ostr, HTTP_BAD_REQUEST);
        }

        Stream::BinaryStreamReader in(&input_stream_);
        in >> body_;
      }
    }

    // fill protocols
    ap_filter_t* proto_filter = r->proto_input_filters;
    while (proto_filter)
    {
      ap_filter_rec_t* filter_rec = proto_filter->frec;
      if (filter_rec->name)
      {
        input_protocols_.emplace_back(filter_rec->name);
        if (String::SubString(filter_rec->name) == SECURE_PROTOCOL_NAME)
        {
          secure_ = true;
        }
      }

      proto_filter = proto_filter->next;
    }
  }

  std::istream&
  HttpRequest::get_token_(std::istream& istr, std::string& dst, char delim)
    throw (eh::Exception)
  {
    char buf[1024];

    for (;;)
    {
      try
      {
        istr.get(buf, sizeof(buf), delim);
      }
      catch (...)
      {
        break;
      }
      if (istr.bad())
      {
        break;
      }
      else if (istr.fail())
      {
        if (!istr.eof())
        {
          istr.clear();
          if (istr.peek() == std::istream::traits_type::to_int_type(delim))
          {
            istr.ignore();
          }
        }
        else if (dst.size() > 0)
        {
          istr.clear(istr.rdstate() & ~std::ios::failbit);
        }
        break;
      }
      else
      {
        dst += buf; // This may throw
        if (istr.eof())
        {
          break;
        }

        if (istr.peek() == std::istream::traits_type::to_int_type(delim))
        {
          istr.ignore();
          break;
        }
      }
    }
    return istr;
  }

  void
  HttpRequest::parse_params(const String::SubString& str,
    HTTP::ParamList& params) throw (eh::Exception)
  {
    String::StringManip::SplitAmp tokenizer(str);
    String::SubString token;
    while (tokenizer.get_token(token))
    {
      String::SubString enc_name;
      String::SubString enc_value;
      String::SubString::SizeType pos = token.find('=');
      if (pos == String::SubString::NPOS)
      {
        enc_name = token;
      }
      else
      {
        enc_name = token.substr(0, pos);
        enc_value = token.substr(pos + 1);
      }

      try
      {
        HTTP::Param param;

        String::StringManip::mime_url_decode(enc_name, param.name);
        String::StringManip::mime_url_decode(enc_value, param.value);

        params.push_back(std::move(param));
      }
      catch (const String::StringManip::InvalidFormatException&)
      {
      }
    }
  }

  //
  // HttpResponse
  //
  void
  HttpResponse::add_header(const char* name, const char* value)
    throw (eh::Exception)
  {
    apr_table_add(r_->err_headers_out, name, value);
  }

  void
  HttpResponse::set_content_type(const char* value)
    throw (eh::Exception)
  {
    ap_set_content_type(r_, apr_pstrdup(r_->pool, value));
  }

  void
  HttpResponse::add_cookie(const char* value)
    throw (eh::Exception)
  {
    apr_table_add(r_->err_headers_out, "Set-Cookie", value);
  }
} // namespace Apache
