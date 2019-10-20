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



#ifndef _EMPTY_POLICIES_INTERNAL_TEST_TESTS_HPP_
#define _EMPTY_POLICIES_INTERNAL_TEST_TESTS_HPP_

#include "CommonClasses.hpp"
#include <climits>

//
// class BasicsTestEmptyThreadPolicy
//

class BasicsTestEmptyThreadPolicy :
  public CheckSimpleEmptyThread
{
public:

  BasicsTestEmptyThreadPolicy(std::ostringstream& log, TestCommons::Errors& errors,
    Sync::Semaphore& work_finished, unsigned short closure_delay/* = 8*/) throw();

  virtual int
  when_close_thread(Identifier thread) throw();

protected:

  virtual ~BasicsTestEmptyThreadPolicy() throw();

  virtual void
  check_thread_connection_added(Identifier thread, Identifier connection) throw ();

  virtual void
  check_choose_thread(Identifier thread) throw ();

  virtual void
  check_thread_added(Identifier thread) throw ();

  virtual void
  check_thread_removed(Identifier thread) throw ();

  void dynamic_states_checker_(const char* prefix, const void* addr,
    const StateHistory* prev_n_now) throw (eh::Exception);

protected:

  std::ostringstream& log_;
  TestCommons::Errors& errors_;
  Sync::Semaphore& work_finished_;
};

//
// class BasicsTestEmptyConnectionPolicy
//

class BasicsTestEmptyConnectionPolicy :
  public CheckSimpleEmptyConnection
{
public:

  BasicsTestEmptyConnectionPolicy(std::ostringstream& log, TestCommons::Errors& errors,
    Sync::Semaphore& work_finished, unsigned short closure_delay/* = 8*/) throw();

  virtual int
  when_close_connection(Identifier connection) throw();

protected:

  virtual ~BasicsTestEmptyConnectionPolicy() throw();

  virtual void
  check_connection_request_added(Identifier connection, Identifier request) throw ();

  virtual void
  check_choose_connection(Identifier connection, Identifier server,
    Identifier request) throw ();

  virtual void
  check_server_connection_added(Identifier server, Identifier connection)
    throw ();

  virtual void
  check_server_connection_removed(Identifier server, Identifier connection)
    throw ();

  void dynamic_states_checker_(const char* prefix, const void* addr,
    const StateHistory* prev_n_now) throw (eh::Exception);

protected:

  std::ostringstream& log_;
  TestCommons::Errors& errors_;
  Sync::Semaphore& work_finished_;
};

//
// class BasicsTestPolicy
//

class BasicsTestPolicy :
  public virtual HTTP::PoolPolicy,
  public CheckSimpleDecider,
  public HTTP::PoolPolicySimpleRequests,
  public BasicsTestEmptyConnectionPolicy,
  public BasicsTestEmptyThreadPolicy,
  public HTTP::PoolPolicySimpleTimeout,
  public virtual Generics::ActiveObjectCallback
{
public:

  BasicsTestPolicy(std::ostringstream& log,
      Sync::Semaphore& work_finished, int connections_per_server,
      int connections_per_threads, unsigned int thr_states_delay,
      unsigned int conn_states_delay)
    throw (eh::Exception);

  virtual void
  report_error(Severity severity, const String::SubString& description,
    const char* error_code = 0) throw ();

  void
  dump_errors(std::ostringstream& err_stream) throw(eh::Exception);

protected:
  virtual
  ~BasicsTestPolicy() throw ();
private:

  TestCommons::Errors errors_;
  std::ostringstream& log_;
};

typedef ReferenceCounting::QualPtr<BasicsTestPolicy> BasicsTestPolicy_var;

//
// class BasicsTest
//

class BasicsTest :
  public PoliciesTestInterface
{
public:

  BasicsTest(Sync::Semaphore& finish_sem,
      std::vector<HTTP::HttpServer>& servers, int connections_per_server/* = 20*/,
      int connections_per_threads/* = 5*/, unsigned int thr_states_delay,
      unsigned int conn_states_delay, bool check_http_request_errors = true)
    throw (eh::Exception);

  virtual void init_(const char* pl_script_name, size_t serv_numb = 0)
    throw (eh::Exception);

  virtual const char* name() throw();

  //virtual void execute() throw();

  void print_stats(std::ostream& out) throw(eh::Exception);

  void print_errors(std::ostream& out) throw(eh::Exception);

protected:

  virtual ~BasicsTest() throw ();

  virtual void exec_main_() throw();

  virtual void exec_finish_() throw();

  void callback_error_(SimpleCounterCallback* callback)
    throw(eh::Exception);

  virtual void scenario_(HTTP::HttpActiveInterface* pool,
    SimpleCounterCallback* callback) throw(eh::Exception) = 0;

protected:

  std::ostringstream out_;
  std::ostringstream log_;
  std::ostringstream error_;
  Sync::Semaphore sem_;
  std::vector<HTTP::HttpServer> servers_;
  std::string http_request_;
  BasicsTestPolicy_var policy_ptr_;
  ConnThrScenarios scens_;
  bool check_http_request_errors_;
};

//
// class BasicsTest01
//

class BasicsTest01 :
  public BasicsTest
{
public:

  static const char* scenario_descr() throw();

  BasicsTest01(Sync::Semaphore& finish_sem,
    std::vector<HTTP::HttpServer>& servers) throw (eh::Exception);

  virtual const char* name() throw();

protected:

  virtual ~BasicsTest01() throw ();

  virtual void exec_init_() throw();

  virtual void scenario_(HTTP::HttpActiveInterface* pool,
    SimpleCounterCallback* callback) throw(eh::Exception);
};

//
// class BasicsTest02
//

class BasicsTest02 :
  public BasicsTest
{
public:

  static const char* scenario_descr() throw();

  BasicsTest02(Sync::Semaphore& finish_sem,
    std::vector<HTTP::HttpServer>& servers) throw (eh::Exception);

  virtual const char* name() throw();

protected:

  virtual ~BasicsTest02() throw ();

  virtual void exec_init_() throw();

  virtual void scenario_(HTTP::HttpActiveInterface* pool,
    SimpleCounterCallback* callback) throw(eh::Exception);
};

//
// class BasicsTest03
//

class BasicsTest03 :
  public BasicsTest
{
public:

  static const char* scenario_descr() throw();

  BasicsTest03(Sync::Semaphore& finish_sem,
    std::vector<HTTP::HttpServer>& servers) throw (eh::Exception);

  virtual const char* name() throw();

protected:

  virtual ~BasicsTest03() throw ();

  virtual void exec_init_() throw();

  virtual void scenario_(HTTP::HttpActiveInterface* pool,
    SimpleCounterCallback* callback) throw(eh::Exception);
};

//
// class BasicsTest04
//

class BasicsTest04 :
  public BasicsTest
{
public:

  static const char* scenario_descr() throw();

  BasicsTest04(Sync::Semaphore& finish_sem,
    std::vector<HTTP::HttpServer>& servers) throw (eh::Exception);

  virtual const char* name() throw();

protected:

  virtual ~BasicsTest04() throw ();

  virtual void exec_init_() throw();

  virtual void scenario_(HTTP::HttpActiveInterface* pool,
    SimpleCounterCallback* callback) throw(eh::Exception);
};

//
// class RandomLoadingTest
//

class RandomLoadingTest :
  public BasicsTest
{
  std::vector<std::string> requests_;

public:

  static const char* scenario_descr() throw();

  RandomLoadingTest(Sync::Semaphore& finish_sem,
    std::vector<HTTP::HttpServer>& servers) throw (eh::Exception);

  virtual const char* name() throw();

  virtual void execute() throw();

protected:

  virtual ~RandomLoadingTest() throw ();

  virtual void scenario_(HTTP::HttpActiveInterface* pool,
    SimpleCounterCallback* callback) throw(eh::Exception);

  virtual void exec_init_() throw();
};

#endif
