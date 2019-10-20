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



#ifndef _HTTP_TEST_COMMONS_COMMON_CLASSES_HPP_
#define _COMPLEX_TEST_COMMON_CLASSES_HPP_

#include <Generics/TaskRunner.hpp>

#include <HTTP/HttpAsync.hpp>
#include <HTTP/HttpAsyncPool.hpp>
#include <HTTP/HttpAsyncPolicies.hpp>
#include <TestCommons/MTTester.hpp>
#include <TestCommons/Counter.hpp>
#include <TestCommons/Error.hpp>

//
// class TestInterface
//

class TestInterface :
  public Generics::Task,
  public ReferenceCounting::AtomicImpl
{
public:

  virtual const std::string
  additional_http_query() throw (eh::Exception);

  virtual void
  execute() throw () = 0;

protected:

  virtual
  ~TestInterface() throw () = 0;
};

//
// class SimplePolicy
//

class SimplePolicy :
  public virtual HTTP::PoolPolicy,
  public HTTP::PoolPolicySimpleDecider,
  public HTTP::PoolPolicySimpleRequests,
  public HTTP::PoolPolicySimpleEmptyConnection,
  public HTTP::PoolPolicySimpleEmptyThread,
  public HTTP::PoolPolicySimpleTimeout,
  public virtual Generics::ActiveObjectCallback
{
public:

  SimplePolicy(int connections_per_server = 20, int connections_per_threads = 5)
    throw (eh::Exception);

  virtual void
  report_error(Severity /*severity*/, const String::SubString& description,
    const char* error_code = 0) throw ();

protected:

  virtual
  ~SimplePolicy() throw ();

private:
  TestCommons::Errors errors_;
};

typedef ReferenceCounting::QualPtr<SimplePolicy> SimplePolicy_var;

//
// class EventLog
//

class EventLog
{
public:

  enum EventLogStrategies
  {
    ELS_DONT_LOG,
    ELS_LOG_FAILS,
    ELS_LOG_FAILS_LAST_VALID,
    ELS_LOG_EVERYTHING
  };

  EventLog(EventLogStrategies strategy) throw(eh::Exception);

  void log_valid(const char*) throw(eh::Exception);

  void log_valid(const HTTP::ResponseInformation& data) throw(eh::Exception);

  void log_invalid(const char*) throw(eh::Exception);

  void log_invalid(const HTTP::RequestInformation& data) throw(eh::Exception);

  void print(std::ostream& out) throw(eh::Exception);

private:

  static const char* HEADERS_TO_LOG[];
  EventLogStrategies strategy_;
  std::string log_;
  std::string last_valid_;
  Sync::PosixMutex mutex_;
};

//
// class SimpleCounterCallback
//

class SimpleCounterCallback :
  public HTTP::ResponseCallback,
  public ReferenceCounting::AtomicImpl
{

  enum {
    BUFFER_SIZE = 4096
  };

public:

  SimpleCounterCallback(HTTP::PoolPolicy* policy,
      EventLog::EventLogStrategies strategy = EventLog::ELS_LOG_FAILS_LAST_VALID)
    throw(eh::Exception);

  virtual void
  on_response(const HTTP::ResponseInformation& data) throw ();

  virtual void
  on_error(const String::SubString& description,
    const HTTP::RequestInformation& data) throw ();

  virtual void
  print_stat(std::ostream& ostr) const throw (eh::Exception);
  
  virtual void
  print_errors(std::ostream& ostr, bool log_needed = false) throw (eh::Exception);

  const TestCommons::Counter&
  get_counter() const throw ();

protected:

  virtual
  ~SimpleCounterCallback() throw ();
  
  HTTP::PoolPolicy_var policy_;

private:

  EventLog event_log_;
  TestCommons::Counter counter_;
  TestCommons::Errors errors_;
};

typedef ReferenceCounting::QualPtr<SimpleCounterCallback>
  SimpleCounterCallback_var;

//
// class Requester
//

class Requester
{
public:

  Requester(TestInterface& test, HTTP::HttpInterface* pool,
    HTTP::ResponseCallback* cb, const std::string& get_req,
    const std::string& post_req, const std::string& post_body = std::string());

  void
  print_stat(std::ostringstream& ostr) const throw (eh::Exception);

  void
  operator ()() throw ();

  const TestCommons::Counter&
  get_counter() const throw ();

  void
  release_callback() throw();

private:
  HTTP::HttpInterface_var pool_;
  HTTP::ResponseCallback_var cb_;
  TestCommons::Counter counter_;
  const std::string get_req_;
  const std::string post_req_;
  const String::SubString post_body_;
  TestInterface& test_;
};

#endif
