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



#ifndef _COMPLEX_TEST_COMMON_CLASSES_HPP_
#define _COMPLEX_TEST_COMMON_CLASSES_HPP_

#include <HTTP/HttpTestCommons/CommonClasses.hpp>

//
// class CallBackProxy
//

class CallBackProxy : public HTTP::ResponseCallback,
  public ReferenceCounting::AtomicImpl
{
public:
  CallBackProxy(Sync::Semaphore& finish_semaphore,
    HTTP::ResponseCallback *p_impl) throw(eh::Exception);
  virtual
  ~CallBackProxy() throw();
    /**
     * Called when request succeeded
     * @param data response
     */
    virtual void
    on_response(const HTTP::ResponseInformation& data) throw ();

    /**
     * Called when request succeeded and it is not possible to call on_response
     * Should return control ASAP
     * @param data response
     */
    virtual void
    quick_on_response(const HTTP::ResponseInformation& data) throw ();

    /**
     * Called when request failed
     * @param description error message
     * @param data request
     */
    virtual void
    on_error(const String::SubString& description, const HTTP::RequestInformation& data)
      throw ();

    /**
     * Called when request failed and it is not possible to call on_error
     * Should return control ASAP
     * @param description error message
     * @param data request
     */
    virtual void
    quick_on_error(const String::SubString& description,
      const HTTP::RequestInformation& data)
      throw ();

private:
  HTTP::ResponseCallback_var p_impl_;
  Sync::Semaphore& finish_semaphore_;
};

//
// class CheckUpCallback
//

class CheckUpCallback : public SimpleCounterCallback
{
public:

  CheckUpCallback(HTTP::PoolPolicy* policy, const std::string& get_str,
    const std::string& post_str, const std::string& pattern_beg,
    const std::string& pattern_end);

  virtual void
  on_response(const HTTP::ResponseInformation& data) throw ();

  virtual void
  on_error(const String::SubString& descr,
    const HTTP::RequestInformation& data) throw ();

  virtual void
  print_stat(std::ostream& ostr) throw (eh::Exception);

  const TestCommons::Counter&
  get_checkup_counter() const throw ();

protected:

  virtual
  ~CheckUpCallback() throw ();

private:
  const std::string GET_STR_;
  const std::string POST_STR_;
  const std::string PATTERN_BEG_;
  const std::string PATTERN_END_;

  TestCommons::Counter response_checkup_;
};

typedef ReferenceCounting::QualPtr<CheckUpCallback> CheckUpCallback_var;

//
// class CTTestInterface
//

class CTTestInterface: public TestInterface
{
public:

  CTTestInterface(HTTP::HttpInterface* pool, unsigned int test_duration,
    unsigned int making_requests_duration, unsigned int tasks_per_test,
    unsigned int functors_per_task) throw (eh::Exception);

  virtual const std::string
  additional_http_query() throw (eh::Exception);

  virtual void
  execute() throw ();

  bool
  is_error(const char* test_name, const TestCommons::Counter* add_counter,
    const TestCommons::Counter* callb_counter,
    const TestCommons::Counter* checkup_counter)
    throw (eh::Exception);

  virtual std::string
  checkup_and_print_stat() throw (eh::Exception) = 0;

protected:

  virtual
  ~CTTestInterface() throw ();

  HTTP::HttpInterface_var pool_;
  std::ostringstream stat_;
  std::unique_ptr<Requester> requester_;
  unsigned int run_period_;
  unsigned int tasks_count_;
  unsigned int functors_count_;
};

typedef ReferenceCounting::QualPtr<CTTestInterface> CTTestInterface_var;

#endif
