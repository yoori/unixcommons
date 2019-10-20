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



#include <Sync/Semaphore.hpp>

#include <HTTP/HttpSync.hpp>


namespace HTTP
{
  namespace
  {
    DECLARE_EXCEPTION(CaughtException, eh::DescriptiveException);

    class SyncCallback :
      public ResponseCallback,
      public ReferenceCounting::AtomicImpl
    {
    public:
      SyncCallback(Sync::Semaphore& semaphore, int& response_code,
        HeaderList& response_headers, ResponseBody& response_body,
        std::string& response_error, CaughtException& exception) throw ();

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
      ~SyncCallback() throw ();

    private:
      Sync::Semaphore& semaphore_;
      int& response_code_;
      HeaderList& response_headers_;
      ResponseBody& response_body_;
      std::string& response_error_;
      CaughtException& exception_;
    };


    SyncCallback::SyncCallback(Sync::Semaphore& semaphore,
      int& response_code,
      HeaderList& response_headers, ResponseBody& response_body,
      std::string& response_error, CaughtException& exception) throw ()
      : semaphore_(semaphore), response_code_(response_code),
        response_headers_(response_headers), response_body_(response_body),
        response_error_(response_error), exception_(exception)
    {
    }

    SyncCallback::~SyncCallback() throw ()
    {
    }

    void
    SyncCallback::on_response(const ResponseInformation& data) throw ()
    {
      try
      {
        response_code_ = data.response_code();
        response_headers_ = data.response_headers();
        const String::SubString& body = data.body();
        if (!body.empty())
        {
          response_body_.assign(body.begin(), body.end());
        }
      }
      catch (const eh::Exception& ex)
      {
        Stream::Error ostr;
        ostr << FNS << ex.what();
        exception_ = CaughtException(ostr);
      }
      catch (...)
      {
        Stream::Error ostr;
        ostr << FNS << "Unknown exception";
        exception_ = CaughtException(ostr);
      }
      semaphore_.release();
    }

    void
    SyncCallback::on_error(const String::SubString& description,
      const RequestInformation& /*data*/) throw ()
    {
      try
      {
        description.assign_to(response_error_);
      }
      catch (const eh::Exception& ex)
      {
        Stream::Error ostr;
        ostr << FNS << ex.what();
        exception_ = CaughtException(ostr);
      }
      catch (...)
      {
        Stream::Error ostr;
        ostr << FNS << "Unknown exception";
        exception_ = CaughtException(ostr);
      }
      semaphore_.release();
    }
  }


  void
  syncronous_get_request(int& response_code, HeaderList& response_headers,
    ResponseBody& response_body, std::string& response_error,
    HttpInterface& http, const char* http_request,
    const HttpServer& peer, const HeaderList& headers)
    throw (eh::Exception, eh::DescriptiveException)
  {
    Sync::Semaphore semaphore(0);
    CaughtException exception(0);

    ResponseCallback_var callback(new SyncCallback(semaphore,
      response_code, response_headers, response_body, response_error,
      exception));
    http.add_get_request(http_request, callback, peer, headers);
    semaphore.acquire();

    if (*exception.what())
    {
      throw exception;
    }
  }

  void
  syncronous_post_request(int& response_code, HeaderList& response_headers,
    ResponseBody& response_body, std::string& response_error,
    HttpInterface& http, const char* http_request,
    const String::SubString& body,
    const HttpServer& peer, const HeaderList& headers)
    throw (eh::Exception, eh::DescriptiveException)
  {
    Sync::Semaphore semaphore(0);
    CaughtException exception(0);

    ResponseCallback_var callback(new SyncCallback(semaphore,
      response_code, response_headers, response_body, response_error,
      exception));
    http.add_post_request(http_request, callback, body, peer, headers);
    semaphore.acquire();

    if (*exception.what())
    {
      throw exception;
    }
  }
}
