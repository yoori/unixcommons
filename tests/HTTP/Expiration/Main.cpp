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
#include <HTTP/HttpTestCommons/ApachePorts.hpp>

#include <TestCommons/MTTester.hpp>
#include <TestCommons/Counter.hpp>
#include <TestCommons/Error.hpp>

using namespace HTTP;

char hostname[HOST_NAME_MAX];
int hostname_res = gethostname(hostname, sizeof(hostname));

const HttpServer SERVER(hostname, ApachePorts::get_port(34));
const std::string REQUEST = std::string("http://") + SERVER.first + ':'
  + ApachePorts::get_port_string(34);
const std::string NORMAL_REQUEST = REQUEST + "/cgi-bin/echo.pl?A";
const std::string FAIL_REQUEST = REQUEST + "/cgi-bin/wait.pl?3";

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

  int
  succeeded() const throw ()
  {
    return counter_.succeeded();
  }

  int
  failed() const throw ()
  {
    return counter_.failed();
  }

private:
  TestCommons::Counter counter_;
  TestCommons::Errors errors_;
};

class MyPolicy :
  public virtual PoolPolicy,
  public PoolPolicySimpleDecider,
  public PoolPolicyWaitRequests,
  public HTTP::PoolPolicySimpleEmptyConnection,
  public HTTP::PoolPolicySimpleEmptyThread,
  public PoolPolicySimpleTimeout,
  public virtual Generics::ActiveObjectCallback
{
public:

  MyPolicy() throw (eh::Exception)
    : PoolPolicySimpleDecider(20, 5), PoolPolicyWaitRequests(50),
      PoolPolicySimpleTimeout(2), connections_(0)
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

class CallbackRequester :
  public ResponseCallback,
  public ReferenceCounting::AtomicImpl
{
public:
  CallbackRequester(HttpInterface* pool,
    Sync::Semaphore& semaphore) throw ()
    : pool_(ReferenceCounting::add_ref(pool)), semaphore_(semaphore)
  {
  }

  virtual void
  on_response(const ResponseInformation& /*data*/) throw ()
  {
    response_counter_.success();
  }

  virtual void
  on_error(const String::SubString& description, const RequestInformation& data) throw ()
  {
    std::string error;
    try
    {
      description.assign_to(error);
      error += ": ";
      error += data.http_request();
    }
    catch (...)
    {
    }
    response_counter_.failure(error.empty() ? description : error);
  }

  void
  operator ()() throw ()
  {
    ResponseCallback_var cb(this);
    add_ref();

    for (int i = 0; i < 100; i++)
    {
      try
      {
        try
        {
          if (rand() & 7)
          {
            pool_->add_get_request(NORMAL_REQUEST.c_str(), cb);
            ratio_.success();
          }
          else
          {
            pool_->add_get_request(FAIL_REQUEST.c_str(), cb);
            ratio_.failure();
          }
          addition_.success();
        }
        catch (const eh::Exception& ex)
        {
          errors_.add(String::SubString(ex.what()));
          throw;
        }
      }
      catch (...)
      {
        addition_.failure();
      }
    }
  }

protected:
  virtual
  ~CallbackRequester() throw ()
  {
    std::cout << "Addition: ";
    addition_.print();
    std::cout << "Errors:" << std::endl;
    errors_.print();
    std::cout << std::endl;

    std::cout << "Expected response (at least failed): ";
    ratio_.print();

    response_counter_.print();

    if (ratio_.failed() > response_counter_.failed())
    {
      std::cerr << "Number of failed requests is too low" << std::endl;
    }

    semaphore_.release();
  }

private:
  HttpInterface_var pool_;

  TestCommons::Counter addition_;
  TestCommons::Counter ratio_;
  TestCommons::Errors errors_;
  Sync::Semaphore& semaphore_;

  ResponseCounter response_counter_;
};

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

    HttpActiveInterface_var pool(CreatePool(policy.in(), task_runner.in()));
    pool->add_ref();
    HttpInterface_var npool(pool.in());

    pool->activate_object();

    Sync::Semaphore semaphore(0);
    ReferenceCounting::QualPtr<CallbackRequester> cr(
      new CallbackRequester(npool.in(), semaphore));
    {
      TestCommons::MTTester<CallbackRequester&> tester(*cr, 5);
      tester.run(10, 3);
    }

    cr.reset();
    semaphore.acquire();

    pool->deactivate_object();
    pool->wait_object();

    task_runner->deactivate_object();
    task_runner->wait_object();
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
