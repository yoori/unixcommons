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



#include "CommonClasses.hpp"
#include <iostream>

//
// class TestInterface
//

TestInterface::~TestInterface() throw()
{
}

const std::string
TestInterface::additional_http_query() throw (eh::Exception)
{
  return std::string();
}

//
// class SimplePolicy
//

SimplePolicy::SimplePolicy(int connections_per_server,
  int connections_per_threads) throw (eh::Exception):
    PoolPolicySimpleDecider(connections_per_server, connections_per_threads)
{
}

void
SimplePolicy::report_error(Severity /*severity*/, const String::SubString& description,
  const char* /*error_code*/)
  throw ()
{
  errors_.add(description, true);
}

SimplePolicy::~SimplePolicy() throw ()
{
  if (!errors_.empty())
  {
    std::cout << "[ERROR] Policy errors:" << std::endl;
    errors_.print();
  }
}

//
// class EventLog
//

const char* EventLog::HEADERS_TO_LOG[] =
{
  "Content-type",
  "Content-length",
  "Connection",
  "Set-Cookie"
};

EventLog::EventLog(EventLogStrategies strategy)
  throw(eh::Exception):
    strategy_(strategy)
{
}

void
EventLog::log_valid(const char* data) throw(eh::Exception)
{
  if (strategy_ == ELS_DONT_LOG || strategy_ == ELS_LOG_FAILS)
  {
    return;
  }

  Sync::PosixGuard guard(mutex_);

  last_valid_ = "SUCCESS data:\n\t";
  last_valid_ += data;
  last_valid_ += "\n\n";

  if (strategy_ == ELS_LOG_FAILS_LAST_VALID)
  {
    return;
  }

  log_ += last_valid_;
}

void
EventLog::log_valid(const HTTP::ResponseInformation& data) throw(eh::Exception)
{
  if (strategy_ == ELS_DONT_LOG || strategy_ == ELS_LOG_FAILS)
  {
    return;
  }

  Sync::PosixGuard guard(mutex_);

  std::ostringstream last_valid;
  String::SubString body(data.body());
  const char* http_req = data.http_request();

  last_valid << "SUCCESS data:\n\tResponseCode: " << data.response_code() <<
    "\n\tMethod: " << HTTP::method_name(data.method()) << "\n\tURI: " <<
    (http_req ? http_req : "Empty") << "\n\tBody:\n" << (body.empty() ?
    String::SubString("Empty") : body) << "\n\tHeaders: ";
  for (size_t i = 0; i < sizeof(HEADERS_TO_LOG) / sizeof(HEADERS_TO_LOG[0]); ++i)
  {
    HTTP::HeaderList headers;
    data.find_headers(HEADERS_TO_LOG[i], headers);
    if (!headers.empty())
    {
      last_valid << "\n" << HEADERS_TO_LOG[i] << " : "
                 << headers.front().value;
    }
  }
  last_valid << "\n\n";

  last_valid_ = last_valid.str();

  if (strategy_ == ELS_LOG_FAILS_LAST_VALID)
  {
    return;
  }

  log_ += last_valid_;
}

void
EventLog::log_invalid(const char* data) throw(eh::Exception)
{
  if (strategy_ == ELS_DONT_LOG)
  {
    return;
  }

  Sync::PosixGuard guard(mutex_);

  if (strategy_ == ELS_LOG_FAILS_LAST_VALID)
  {
    if (!last_valid_.empty())
    {
      log_ += last_valid_;
      last_valid_.clear();
    }
  }

  log_ += "FAIL data:\n\t";
  log_ += data;
  log_ += "\n\n";
}

void
EventLog::log_invalid(const HTTP::RequestInformation& data) throw(eh::Exception)
{
  if (strategy_ == ELS_DONT_LOG)
  {
    return;
  }

  Sync::PosixGuard guard(mutex_);

  if (strategy_ == ELS_LOG_FAILS_LAST_VALID)
  {
    if (!last_valid_.empty())
    {
      log_ += last_valid_;
      last_valid_.clear();
    }
  }

  std::ostringstream buf;
  const char* http_req = data.http_request();
  buf << "FAIL data:\n\tMethod: " << HTTP::method_name(data.method())
      << "\n\tURI: " << (http_req? http_req: "Empty") << "\n\n";

  log_ += buf.str();
}

void
EventLog::print(std::ostream& out) throw(eh::Exception)
{
  out << "Log strategy: ";
  switch (strategy_)
  {
    case ELS_DONT_LOG: out << "ELS_DONT_LOG\n"; break;
    case ELS_LOG_FAILS: out << "ELS_LOG_FAILS\n"; break;
    case ELS_LOG_FAILS_LAST_VALID: out << "ELS_LOG_FAILS_LAST_VALID\n"; break;
    case ELS_LOG_EVERYTHING: out << "ELS_LOG_EVERYTHING\n"; break;
    default: out << "UNKNOWN\n"; break;
  }
  out << log_;
}

//
// class SimpleCounterCallback
//

SimpleCounterCallback::SimpleCounterCallback(HTTP::PoolPolicy* policy,
    EventLog::EventLogStrategies strategy) throw(eh::Exception):
  policy_(ReferenceCounting::add_ref(policy)),
  event_log_(strategy)
{
}

void
SimpleCounterCallback::on_response(const HTTP::ResponseInformation& data) throw ()
{
  counter_.success();
  try
  {
    event_log_.log_valid(data);
  }
  catch (const eh::Exception& e)
  {
    const char error_prefix[] = "SimpleCounterCallback::on_response(2). EventLog::"
                                "log_valid(1) throws eh::Exception: ";
    char error[BUFFER_SIZE];
    String::StringManip::strlcpy(error, error_prefix, BUFFER_SIZE);
    String::StringManip::strlcpy(error + sizeof(error_prefix), e.what(),
      BUFFER_SIZE - sizeof(error_prefix));
    policy_->error(String::SubString(error));
  }
}

void
SimpleCounterCallback::on_error(const String::SubString& description,
  const HTTP::RequestInformation& data) throw ()
{
  errors_.add(description);
  counter_.failure();
  try
  {
    event_log_.log_invalid(data);
  }
  catch (const eh::Exception& e)
  {
    const char error_prefix[] = "SimpleCounterCallback::on_error(2). EventLog::"
                                "log_invalid(1) throws eh::Exception: ";
    char error[BUFFER_SIZE];
    String::StringManip::strlcpy(error, error_prefix, BUFFER_SIZE);
    String::StringManip::strlcpy(error + sizeof(error_prefix),
      e.what(), BUFFER_SIZE - sizeof(error_prefix));
    policy_->error(String::SubString(error));
  }
}

void
SimpleCounterCallback::print_stat(std::ostream& ostr) const throw (eh::Exception)
{
  ostr << "Execution: ";
  counter_.print(ostr);
}

void
SimpleCounterCallback::print_errors(std::ostream& ostr, bool log_needed)
  throw (eh::Exception)
{
  ostr << "Errors: ";
  errors_.print(ostr);
  if (log_needed && !errors_.empty())
  {
    ostr << "Test log:\n";
    event_log_.print(ostr);
  }
}

const TestCommons::Counter&
SimpleCounterCallback::get_counter() const throw ()
{
  return counter_;
}

SimpleCounterCallback::~SimpleCounterCallback() throw ()
{
}

//
// class Requester
//

Requester::Requester(TestInterface& test, HTTP::HttpInterface* pool,
  HTTP::ResponseCallback* cb, const std::string& get_req,
  const std::string& post_req, const std::string& post_body)
  : pool_(ReferenceCounting::add_ref(pool)),
    cb_(ReferenceCounting::add_ref(cb)),
    get_req_(get_req), post_req_(post_req),
    post_body_(post_body.c_str(), post_body.length()), test_(test)
{
}

void
Requester::print_stat(std::ostringstream& ostr) const throw (eh::Exception)
{
  ostr << "Addition: ";
  counter_.print(ostr);
}

void
Requester::operator ()() throw ()
{
  for (int i = 0; i < 100; i++)
  {
    try
    {
      const std::string add_str = test_.additional_http_query();
      if (i % 2)
      {
        pool_->add_get_request((get_req_ + add_str).c_str(), cb_);
      }
      else
      {
        pool_->add_post_request((post_req_ + add_str).c_str(), cb_, post_body_);
      }
      counter_.success();
    }
    catch (const eh::Exception& e)
    {
      std::cerr << "[ERROR]: Requester::operator(). eh::Exception caught: "
                << e.what() << std::endl;
      counter_.failure();
    }
  }
}

void
Requester::release_callback() throw()
{
  cb_.reset();
}

const TestCommons::Counter&
Requester::get_counter() const throw ()
{
  return counter_;
}
