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



#include <stdio.h>
#include <event.h>
#include <evhttp.h>

#include <Sync/Semaphore.hpp>

#include <HTTP/HttpAsync.hpp>
#include <HTTP/HttpAsyncPool.hpp>
#include <HTTP/HttpAsyncPolicies.hpp>
#include <HTTP/HttpClient.hpp>
#include <HTTP/HttpSync.hpp>

#include <TestCommons/MTTester.hpp>
#include <TestCommons/Counter.hpp>
#include <TestCommons/Error.hpp>
#include <HTTP/HttpTestCommons/ApachePorts.hpp>

using namespace HTTP;

char hostname[HOST_NAME_MAX];
int hostname_res = gethostname(hostname, sizeof(hostname));

const HttpServer SERVER_1(hostname, ApachePorts::get_port(34));
const HttpServer SERVER_2(hostname, ApachePorts::get_port(35));
const std::string REQUEST_1 = std::string("http://") + SERVER_1.first + ":"
  + ApachePorts::get_port_string(34);
const std::string REQUEST_2 = std::string("http://") + SERVER_2.first + ":"
  + ApachePorts::get_port_string(35);

const std::string GET_STRING =
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "body&glbfcap=0&referer=act.com";
const std::string POST_STRING =
  "login=Petya%20Vasechkin&password=qq";

const std::string GET_RESPONSE_BEGIN = "<BODY>\n";
const std::string GET_RESPONSE_END = "\n</BODY>";

const std::string GET_REQUEST = REQUEST_1 + "/cgi-bin/echo.pl?" + GET_STRING;
const std::string POST_REQUEST = REQUEST_2 + "/cgi-bin/echo.pl";


class ResponseChecker
{
public:
  void
  print() throw (eh::Exception)
  {
    std::cout << "Check up: ";
    response_checkup_.print();
    std::cout << "Check up data:" << std::endl;
    response_checkup_data_.print();
  }

  void
  operator ()(HTTP::HttpMethod method,
    const String::SubString& body)
  {
    const std::string& CHECKUP_STR =
      method == HM_GET ? GET_STRING : POST_STRING;

    String::SubString::SizeType beg = body.find(GET_RESPONSE_BEGIN);
    String::SubString::SizeType end = body.rfind(GET_RESPONSE_END);
    if (beg != String::SubString::NPOS && end != String::SubString::NPOS)
    {
      beg += GET_RESPONSE_BEGIN.length();
      String::SubString pattern = body.substr(beg, end - beg);
      if (pattern == CHECKUP_STR)
      {
        response_checkup_.success();
        return;
      }
    }

    response_checkup_.failure();
    response_checkup_data_.add(body);
  }

private:
  TestCommons::Counter response_checkup_;
  TestCommons::Errors response_checkup_data_;
};

class ResponseCounter
{
public:
  void
  success() throw ()
  {
    counter_.success();
  }

  void
  failure(const String::SubString& description)
  {
    counter_.failure();
    errors_.add(description);
  }

  void
  print() throw (eh::Exception)
  {
    std::cout << "Execution: ";
    counter_.print();
    std::cout << "Execution errors:" << std::endl;
    errors_.print();
  }

private:
  TestCommons::Counter counter_;
  TestCommons::Errors errors_;
};

void
sync_calls(const char* type, HttpInterface& http)
{
  std::cout << type << std::endl;
  ResponseCounter counter;
  ResponseChecker checker;
  TestCommons::Errors exceptions;

  try
  {
    for (int i = 0; i < 1000; i++)
    {
      HTTP::HttpMethod method;
      int response_code;
      HeaderList response_headers;
      ResponseBody response_body;
      std::string response_error;

      try
      {
        if (i % 2)
        {
          method = HTTP::HM_GET;
          HTTP::syncronous_get_request(response_code, response_headers,
            response_body, response_error, http, GET_REQUEST.c_str());
        }
        else
        {
          HTTP::syncronous_post_request(response_code, response_headers,
            response_body, response_error, http, POST_REQUEST.c_str(),
            POST_STRING);
          method = HTTP::HM_POST;
        }
        if (response_error.empty())
        {
          counter.success();
          checker(method, String::SubString(&response_body[0],
            response_body.size()));
        }
        else
        {
          counter.failure(response_error);
        }
      }
      catch (const eh::Exception& ex)
      {
        exceptions.add(String::SubString(ex.what()), true);
      }
    }
    counter.print();
    checker.print();
    std::cout << "Exceptions:" << std::endl;
    exceptions.print();
    std::cout << std::endl;
  }
  catch (eh::Exception& ex)
  {
    std::cerr << "Exception " << ex.what() << std::endl;
  }
}

class MyPolicy :
  public virtual PoolPolicy,
  public PoolPolicySimpleDecider,
  public PoolPolicySimpleEmptyConnection,
  public PoolPolicySimpleEmptyThread,
  public PoolPolicyWaitRequests,
  public PoolPolicySimpleTimeout,
  public virtual Generics::ActiveObjectCallback
{
public:
  MyPolicy() throw (eh::Exception)
    : PoolPolicySimpleDecider(20, 5),
      PoolPolicyWaitRequests(300),
      connections_(0)
  {
  }

  virtual void
  server_connection_added(Identifier server, Identifier connection)
    throw ()
  {
    PoolPolicySimpleDecider::server_connection_added(server, connection);
    __gnu_cxx::__atomic_add(&connections_, 1);
  }

  virtual void
  report_error(Severity /*severity*/, const String::SubString& description,
    const char* /*error_code*/) throw ()
  {
    errors_.add(description, true);
  }
protected:
  virtual
  ~MyPolicy() throw ()
  {
    std::cout << "Number of connections created: " << connections_ << std::endl;
    std::cout << "Policy errors:" << std::endl;
    errors_.print();
  }
private:
  volatile _Atomic_word connections_;
  TestCommons::Errors errors_;
};

class MyCallback :
  public ResponseCallback,
  public ReferenceCounting::AtomicImpl
{
public:
  MyCallback(Sync::Semaphore& semaphore) throw ()
    : semaphore_(semaphore)
  {
  }

  virtual void
  on_response(const ResponseInformation& data) throw ()
  {
    counter_.success();
    checker_(data.method(), data.body());
  }

  virtual void
  on_error(const String::SubString& description,
    const RequestInformation& /*data*/) throw ()
  {
    counter_.failure(description);
  }
protected:
  virtual
  ~MyCallback() throw ()
  {
    counter_.print();
    checker_.print();

    semaphore_.release();
  }
private:
  Sync::Semaphore& semaphore_;

  ResponseCounter counter_;
  ResponseChecker checker_;
};

class Requester
{
public:
  Requester(HttpInterface* pool, ResponseCallback* cb,
    const char* type)
    : pool_(ReferenceCounting::add_ref(pool)),
      cb_(ReferenceCounting::add_ref(cb)), type_(type)
  {
  }
  ~Requester() throw ()
  {
    std::cout << type_ << std::endl;
    std::cout << "Addition: ";
    counter_.print();
    std::cout << "Errors:" << std::endl;
    errors_.print();
    std::cout << std::endl;
  }

  void
  operator ()() throw ()
  {
    for (int i = 0; i < 100; i++)
    {
      try
      {
        try
        {
          if (i % 2)
          {
            pool_->add_get_request(GET_REQUEST.c_str(), cb_);
          }
          else
          {
            pool_->add_post_request(POST_REQUEST.c_str(), cb_, POST_STRING);
          }
          counter_.success();
        }
        catch (const eh::Exception& ex)
        {
          errors_.add(String::SubString(ex.what()));
          throw;
        }
      }
      catch (...)
      {
        counter_.failure();
      }
    }
  }

private:
  HttpInterface_var pool_;
  ResponseCallback_var cb_;
  const char* type_;

  TestCommons::Counter counter_;
  TestCommons::Errors errors_;
};

void
print_cookie(const CookieDef& cookie)
{
  std::cout << cookie.name << "=" << cookie.value << " " << cookie.domain << " "
    << cookie.path << " " << HTTP::cookie_date(cookie.expires)
    << (cookie.secure ? " secure" : "") << std::endl;
}

class PClientCookieFacility : public HTTP::ClientCookieFacility
{
public:
  void
  print_cookies() throw (eh::Exception);
};

void
PClientCookieFacility::print_cookies() throw (eh::Exception)
{
  std::cout << std::endl << "Cookies:" << std::endl;
  std::for_each(begin(), end(), print_cookie);
  std::cout << std::endl;
}

int
main()
{
  try
  {
    MyPolicy* policy_ptr = new MyPolicy;
    PoolPolicy_var policy(policy_ptr);

    Generics::TaskRunner_var task_runner(
      new Generics::TaskRunner(policy_ptr, 5));
    task_runner->activate_object();

    HttpActiveInterface_var pool(CreatePool(policy.in(), task_runner));

    CookiePool_var cookie(
      new CookiePoolPtr(new PClientCookieFacility));
    HttpInterface_var npool(CreateCookieClient(pool.in(), cookie.in()));

    pool->activate_object();

    HttpInterface_var spool(CreateSyncHttp());
    spool = CreateCookieClient(spool.in(), cookie.in());


    sync_calls("Sync calls on async implementation", *pool);
    sync_calls("Sync calls on sync implementation", *spool);


    Sync::Semaphore semaphore(0);
    ResponseCallback_var my_cb(new MyCallback(semaphore));
    {
      Requester requester(spool.in(), my_cb,
        "Async calls on sync implementation");
      TestCommons::MTTester<Requester&> tester(requester, 5);
      tester.run(10, 3);
    }
    {
      Requester requester(npool.in(), my_cb,
        "Async calls on async implementation");
      TestCommons::MTTester<Requester&> tester(requester, 5);
      tester.run(10, 3);
    }

    my_cb.reset();
    semaphore.acquire();

    pool->deactivate_object();
    pool->wait_object();

    task_runner->deactivate_object();
    task_runner->wait_object();

    cookie->as<PClientCookieFacility>()->print_cookies();
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "Exception caught: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception caught" << std::endl;
  }

  return 0;
}
