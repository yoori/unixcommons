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
// class CallBackProxy
//

CallBackProxy::CallBackProxy(Sync::Semaphore& finish_semaphore,
                             HTTP::ResponseCallback *p_impl)
  throw(eh::Exception)
  : p_impl_(ReferenceCounting::add_ref(p_impl)),
    finish_semaphore_(finish_semaphore)
{
}

CallBackProxy::~CallBackProxy() throw()
{
  finish_semaphore_.release();
}

void
CallBackProxy::quick_on_response(const HTTP::ResponseInformation& data) throw ()
{
  p_impl_->quick_on_response(data);
}

void
CallBackProxy::quick_on_error(const String::SubString& description,
                              const HTTP::RequestInformation& data) throw ()
{
  p_impl_->quick_on_error(description, data);
}

void
CallBackProxy::on_response(const HTTP::ResponseInformation& data) throw ()
{
  p_impl_->on_response(data);
}

void
CallBackProxy::on_error(const String::SubString& description,
                        const HTTP::RequestInformation& data)
  throw ()
{
  p_impl_->on_error(description, data);
}

//
// class CheckUpCallback
//

CheckUpCallback::CheckUpCallback(HTTP::PoolPolicy* policy,
  const std::string& get_str, const std::string& post_str,
  const std::string& pattern_beg, const std::string& pattern_end)
  : SimpleCounterCallback(policy),
    GET_STR_(get_str), POST_STR_(post_str),
    PATTERN_BEG_(pattern_beg), PATTERN_END_(pattern_end)
{
}

void
CheckUpCallback::on_response(const HTTP::ResponseInformation& data) throw ()
{
  SimpleCounterCallback::on_response(data);

  const std::string& CHECKUP_STR =
    data.method() == HTTP::HM_GET ? GET_STR_ : POST_STR_;

  String::SubString body(data.body());
  String::SubString::SizeType beg = body.find(PATTERN_BEG_);
  String::SubString::SizeType end = body.rfind(PATTERN_END_);
  if (beg != String::SubString::NPOS && end != String::SubString::NPOS)
  {
    beg += PATTERN_BEG_.length();
    String::SubString pattern = body.substr(beg, end - beg);
    if (pattern == CHECKUP_STR)
    {
      response_checkup_.success();
      return;
    }
  }

  response_checkup_.failure();
}

void
CheckUpCallback::on_error(const String::SubString& descr,
  const HTTP::RequestInformation& data) throw ()
{
  SimpleCounterCallback::on_error(descr, data);
}

void
CheckUpCallback::print_stat(std::ostream& ostr) throw (eh::Exception)
{
  SimpleCounterCallback::print_stat(ostr);
  ostr << "Check up: ";
  response_checkup_.print(ostr);
}

const TestCommons::Counter&
CheckUpCallback::get_checkup_counter() const throw ()
{
  return response_checkup_;
}

CheckUpCallback::~CheckUpCallback() throw ()
{
}

//
// class CTTestInterface
//

CTTestInterface::CTTestInterface(HTTP::HttpInterface* pool,
    unsigned int test_duration, unsigned int making_requests_duration,
    unsigned int tasks_per_test, unsigned int functors_per_task)
  throw (eh::Exception):
    pool_(ReferenceCounting::add_ref(pool)),
    run_period_(making_requests_duration < test_duration?
                making_requests_duration: test_duration),
    tasks_count_(tasks_per_test),
    functors_count_(functors_per_task)
{
}

const std::string
CTTestInterface::additional_http_query() throw (eh::Exception)
{
  return std::string();
}

void
CTTestInterface::execute() throw ()
{
  try
  {
    TestCommons::MTTester<Requester&> tester(*requester_, tasks_count_);
    tester.run(functors_count_, run_period_, functors_count_);
    requester_->release_callback();
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "[ERROR]: Exception caught: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "[ERROR]: Unknown exception caught" << std::endl;
  }
}

bool
CTTestInterface::is_error(const char* test_name, const TestCommons::Counter* add_counter,
  const TestCommons::Counter* callb_counter, const TestCommons::Counter* checkup_counter)
  throw (eh::Exception)
{
  if (test_name == 0 || add_counter == 0 || callb_counter == 0)
  {
    std::cerr << "[ERROR] " << (test_name? test_name: "Unknown test")
              << " failed. Description: Invalid args in TestInterface::is_error"
                 "(4)." << std::endl;
    return true;
  }

  int added = add_counter->succeeded();
  if (add_counter->failed() != 0)
  {
    std::cerr << "[ERROR] " << test_name << " failed. Description: "
              << "Not all requests were added ( " << added << " added, "
              << add_counter->failed() << " failed )" << std::endl;
    return true;
  }
  int got_reqs = callb_counter->succeeded() + callb_counter->failed();
  if (added != got_reqs)
  {
    std::cerr << "[ERROR] " << test_name << " failed. Description: "
              << "Some requests were lost (The quantity of added requests is "
                 "not equal the quantity of invoked callbacks): " << got_reqs
              << " instead of " << added << std::endl;
    return true;
  }
  if (checkup_counter && checkup_counter->failed())
  {
    std::cerr << "[ERROR] " << test_name << " failed. Description: "
              << "Some requests were invalid ( " << checkup_counter->succeeded()
              << " succeeded and " << checkup_counter->failed() << " failed )"
              << std::endl;
    return true;
  }

  return false;
}

CTTestInterface::~CTTestInterface() throw ()
{
}
