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



#include "Tests.hpp"
#include <HTTP/HttpConnection.hpp>
#include <HTTP/HttpAsyncPool.hpp>
#include <Generics/Time.hpp>
#include <cstdlib>

//
// General constants
//

const std::string ECHO_GET_STRING =
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "body&glbfcap=0&referer=act.com&delay=";
const std::string ECHO_POST_STRING =
  "login=Petya%20Vasechkin&password=qq";

//
// BasicsTest
//

//
// class BasicsTestEmptyThreadPolicy
//

BasicsTestEmptyThreadPolicy::BasicsTestEmptyThreadPolicy(
  std::ostringstream& log, TestCommons::Errors& errors,
  Sync::Semaphore& work_finished, unsigned short closure_delay)
    throw():
  CheckSimpleEmptyThread(closure_delay),
  log_(log),
  errors_(errors),
  work_finished_(work_finished)
{
}

int
BasicsTestEmptyThreadPolicy::when_close_thread(Identifier thread) throw()
{
  Sync::PosixGuard guard(mutex_);

  int res = CheckSimpleEmptyThread::when_close_thread(thread);

  try
  {
    const StateHistory* cur_history = cur_history_;
    //std::cout << time(0) << std::endl;
    if (cur_history->next)
    {
      while (cur_history->next->next != 0) cur_history = cur_history->next;

      print_state_history("  thread:", thread, *cur_history->next, log_);
      dynamic_states_checker_("  thread:", thread, cur_history);
    }
    else
    {
      print_state_history("  thread:", thread, *cur_history, log_);
    }
  }
  catch(...) {};

  return res;
}

BasicsTestEmptyThreadPolicy::~BasicsTestEmptyThreadPolicy() throw()
{
}

void
BasicsTestEmptyThreadPolicy::check_thread_connection_added(
  Identifier thread, Identifier connection) throw ()
{
  Sync::PosixGuard guard(mutex_);

  CheckSimpleEmptyThread::check_thread_connection_added(thread, connection);

  try
  {
    const StateHistory* cur_history = cur_history_;
    //std::cout << time(0) << std::endl;
    if (cur_history->next)
    {
      while (cur_history->next->next != 0) cur_history = cur_history->next;

      print_state_history("  thread:", thread, *cur_history->next, log_);
      dynamic_states_checker_("  thread:", thread, cur_history);
    }
    else
    {
      print_state_history("  thread:", thread, *cur_history, log_);
    }
  }
  catch(...) {};
}

void
BasicsTestEmptyThreadPolicy::check_choose_thread(Identifier thread) throw ()
{
  Sync::PosixGuard guard(mutex_);

  if (thread)
  {
    CheckSimpleEmptyThread::check_choose_thread(thread);

    try
    {
      const StateHistory* cur_history = cur_history_;
      //std::cout << time(0) << std::endl;
      if (cur_history->next)
      {
        while (cur_history->next->next != 0) cur_history = cur_history->next;

        print_state_history("  thread:", thread, *cur_history->next, log_);
        dynamic_states_checker_("  thread:", thread, cur_history);
      }
      else
      {
        print_state_history("  thread:", thread, *cur_history, log_);
      }
    }
    catch(...) {};
  }
}

void
BasicsTestEmptyThreadPolicy::check_thread_added(Identifier thread) throw ()
{
  Sync::PosixGuard guard(mutex_);

  CheckSimpleEmptyThread::check_thread_added(thread);

  try
  {
    const StateHistory* cur_history = cur_history_;
    //std::cout << time(0) << std::endl;
    if (cur_history->next)
    {
      while (cur_history->next->next != 0) cur_history = cur_history->next;

      print_state_history("  thread:", thread, *cur_history->next, log_);
      dynamic_states_checker_("  thread:", thread, cur_history);
    }
    else
    {
      print_state_history("  thread:", thread, *cur_history, log_);
    }
  }
  catch(...) {};
}

void
BasicsTestEmptyThreadPolicy::check_thread_removed(Identifier thread) throw ()
{
  Sync::PosixGuard guard(mutex_);

  CheckSimpleEmptyThread::check_thread_removed(thread);

  try
  {
    const StateHistory* cur_history = cur_history_;
    //std::cout << time(0) << std::endl;
    if (cur_history->next)
    {
      while (cur_history->next->next != 0) cur_history = cur_history->next;

      print_state_history("  thread:", thread, *cur_history->next, log_);
      dynamic_states_checker_("  thread:", thread, cur_history);
    }
    else
    {
      print_state_history("  thread:", thread, *cur_history, log_);
    }

    remove_history_(thread);
  }
  catch(...) {};

  //Is not safe:
  if (get_threads_().size() == 1)
  {
    work_finished_.release();
  }
}

inline
void
BasicsTestEmptyThreadPolicy::dynamic_states_checker_(const char* prefix,
  const void* addr, const StateHistory* prev_n_now) throw (eh::Exception)
{
  std::ostringstream error_issues;
  CheckSimpleEmptyCommons::dynamic_states_checker_(
    prefix, addr, prev_n_now, error_issues);
  if (!error_issues.str().empty())
  {
    error_issues << '\n';
    errors_.add(error_issues.str());
  }
}

//
// class BasicsTestEmptyConnectionPolicy
//

BasicsTestEmptyConnectionPolicy::BasicsTestEmptyConnectionPolicy(
  std::ostringstream& log, TestCommons::Errors& errors,
  Sync::Semaphore& work_finished, unsigned short closure_delay)
    throw():
  CheckSimpleEmptyConnection(closure_delay),
  log_(log),
  errors_(errors),
  work_finished_(work_finished)
{
}

int
BasicsTestEmptyConnectionPolicy::when_close_connection(Identifier connection) throw()
{
  Sync::PosixGuard guard(mutex_);

  int res = CheckSimpleEmptyConnection::when_close_connection(connection);

  try
  {
    const StateHistory* cur_history = cur_history_;
    //std::cout << time(0) << std::endl;
    if (cur_history->next)
    {
      while (cur_history->next->next != 0) cur_history = cur_history->next;

      print_state_history("  connection:", connection, *cur_history->next, log_);
      dynamic_states_checker_("  connection:", connection, cur_history);
    }
    else
    {
      print_state_history("  connection:", connection, *cur_history, log_);
    }
  }
  catch(...) {};

  return res;
}

BasicsTestEmptyConnectionPolicy::~BasicsTestEmptyConnectionPolicy() throw()
{
}

void
BasicsTestEmptyConnectionPolicy::check_connection_request_added(
  Identifier connection, Identifier request) throw ()
{
  Sync::PosixGuard guard(mutex_);

  CheckSimpleEmptyConnection::check_connection_request_added(connection, request);

  try
  {
    const StateHistory* cur_history = cur_history_;
    //std::cout << time(0) << std::endl;
    if (cur_history->next)
    {
      while (cur_history->next->next != 0) cur_history = cur_history->next;

      print_state_history("  connection:", connection, *cur_history->next, log_);
      dynamic_states_checker_("  connection:", connection, cur_history);
    }
    else
    {
      print_state_history("  connection:", connection, *cur_history, log_);
    }
  }
  catch(...) {};
}

void
BasicsTestEmptyConnectionPolicy::check_choose_connection(Identifier connection,
  Identifier server, Identifier request) throw ()
{
  Sync::PosixGuard guard(mutex_);

  if (connection)
  {
    CheckSimpleEmptyConnection::check_choose_connection(connection, server, request);

    try
    {
      const StateHistory* cur_history = cur_history_;
      //std::cout << time(0) << std::endl;
      if (cur_history->next)
      {
        while (cur_history->next->next != 0) cur_history = cur_history->next;

        print_state_history("  connection:", connection, *cur_history->next, log_);
        dynamic_states_checker_("  connection:", connection, cur_history);
      }
      else
      {
        print_state_history("  connection:", connection, *cur_history, log_);
      }
    }
    catch(...) {};
  }
}

void
BasicsTestEmptyConnectionPolicy::check_server_connection_added(Identifier server,
  Identifier connection) throw ()
{
  Sync::PosixGuard guard(mutex_);

  CheckSimpleEmptyConnection::check_server_connection_added(server, connection);

  try
  {
    const StateHistory* cur_history = cur_history_;
    //std::cout << time(0) << std::endl;
    if (cur_history->next)
    {
      while (cur_history->next->next != 0) cur_history = cur_history->next;

      print_state_history("  connection:", connection, *cur_history->next, log_);
      dynamic_states_checker_("  connection:", connection, cur_history);
    }
    else
    {
      print_state_history("  connection:", connection, *cur_history, log_);
    }
  }
  catch(...) {};
}

void
BasicsTestEmptyConnectionPolicy::check_server_connection_removed(Identifier server,
  Identifier connection) throw ()
{
  Sync::PosixGuard guard(mutex_);

  CheckSimpleEmptyConnection::check_server_connection_removed(server, connection);

  try
  {
    const StateHistory* cur_history = cur_history_;
    //std::cout << time(0) << std::endl;
    if (cur_history->next)
    {
      while (cur_history->next->next != 0) cur_history = cur_history->next;

      print_state_history("  connection:", connection, *cur_history->next, log_);
      dynamic_states_checker_("  connection:", connection, cur_history);
    }
    else
    {
      print_state_history("  connection:", connection, *cur_history, log_);
    }

    remove_history_(connection);
  }
  catch(...) {};
}

inline
void
BasicsTestEmptyConnectionPolicy::dynamic_states_checker_(const char* prefix,
  const void* addr, const StateHistory* prev_n_now) throw (eh::Exception)
{
  std::ostringstream error_issues;
  CheckSimpleEmptyCommons::dynamic_states_checker_(
    prefix, addr, prev_n_now, error_issues);
  if (!error_issues.str().empty())
  {
    error_issues << '\n';
    errors_.add(error_issues.str());
  }
}

//
// class BasicsTestPolicy
//

BasicsTestPolicy::BasicsTestPolicy(std::ostringstream& log,
  Sync::Semaphore& work_finished, int connections_per_server,
  int connections_per_threads, unsigned int thr_states_delay, unsigned int conn_states_delay)
    throw (eh::Exception):
  CheckSimpleDecider(connections_per_server, connections_per_threads, *this, *this),
  BasicsTestEmptyConnectionPolicy(log, errors_, work_finished, conn_states_delay),
  BasicsTestEmptyThreadPolicy(log, errors_, work_finished, thr_states_delay),
  HTTP::PoolPolicySimpleTimeout(15),
  log_(log)
{
}

void
BasicsTestPolicy::report_error(Severity /*severity*/, const String::SubString& description,
  const char* /*error_code*/)
  throw ()
{
  errors_.add(description);
}

BasicsTestPolicy::~BasicsTestPolicy() throw ()
{
}

void
BasicsTestPolicy::dump_errors(std::ostringstream& err_stream)
  throw(eh::Exception)
{
  if (!errors_.empty())
  {
    errors_.print(err_stream);
  }
}

//
// class BasicsTest
//

BasicsTest::BasicsTest(Sync::Semaphore& finish_sem,
  std::vector<HTTP::HttpServer>& servers, int connections_per_server,
  int connections_per_threads, unsigned int thr_states_delay,
  unsigned int conn_states_delay, bool check_http_request_errors)
    throw (eh::Exception):
  PoliciesTestInterface(finish_sem),
  sem_(0),
  servers_(servers),
  http_request_(),
  policy_ptr_(new BasicsTestPolicy(log_, sem_, connections_per_server,
    connections_per_threads, thr_states_delay, conn_states_delay)),
  check_http_request_errors_(check_http_request_errors)
{
  init_("echo.pl");
}

BasicsTest::~BasicsTest() throw ()
{
}

void
BasicsTest::init_(const char* pl_script_name, size_t serv_numb)
  throw (eh::Exception)
{
  std::ostringstream base;
  base << "http://" << servers_[serv_numb].first << ':' 
       << servers_[serv_numb].second << "/cgi-bin/"
       << pl_script_name << '?' << ECHO_GET_STRING;
  http_request_ = base.str();
}

const char*
BasicsTest::name() throw()
{
  return "BasicsTest";
}

void
BasicsTest::exec_main_() throw()
{
  try
  {
    HTTP::PoolPolicy_var policy(ReferenceCounting::add_ref(policy_ptr_));
    Generics::TaskRunner_var tests_runner(
      new Generics::TaskRunner(policy_ptr_, 1));
    HTTP::HttpActiveInterface_var pool(
      HTTP::CreatePool(policy.in(), tests_runner));

    tests_runner->activate_object();
    pool->activate_object();

    SimpleCounterCallback_var my_cb(
      new SimpleCounterCallback(policy.in()));

    scenario_(pool.in(), my_cb.in());

    sem_.acquire();

    sleep(8);
    pool->deactivate_object();
    pool->wait_object();
    tests_runner->deactivate_object();
    tests_runner->wait_object();

    if (check_http_request_errors_)
    {
      callback_error_(my_cb);
    }
  }
  catch (const eh::Exception& e)
  {
    try
    {
      error_ << "BasicsTest::exec_main_(): exception: " << e.what() << '\n';
    }
    catch(...) {}
  }
}

void
BasicsTest::exec_finish_() throw()
{
  try
  {
    {
      CheckSimpleEmptyCommons::CompletedHistories::const_iterator it =
        policy_ptr_->get_conn_history().begin();
      CheckSimpleEmptyCommons::CompletedHistories::const_iterator end =
        policy_ptr_->get_conn_history().end();
      for (; it != end; ++it)
      {
        scens_.check_conn_scenario(it->second);
      }
    }

    {
      CheckSimpleEmptyCommons::CompletedHistories::const_iterator it =
        policy_ptr_->get_thr_history().begin();
      CheckSimpleEmptyCommons::CompletedHistories::const_iterator end =
        policy_ptr_->get_thr_history().end();
      for (; it != end; ++it)
      {
        scens_.check_thr_scenario(it->second);
      }
    }

    policy_ptr_->dump_errors(error_);
    if (scens_.all_completed(error_) && error_.str().empty())
    {
      out_ << "Results: success\n";
    }
    else
    {
      out_ << "Results: failure\n";
    }
  }
  catch (const eh::Exception& e)
  {
    try
    {
      error_ << "BasicsTest::exec_main_(): exception: " << e.what() << '\n';
    }
    catch(...) {}
  }
}

inline
void
BasicsTest::callback_error_(SimpleCounterCallback* callback)
  throw(eh::Exception)
{
  if (callback->get_counter().failed())
  {
    error_ << callback->get_counter().failed()
           << " requests failed\n";
  }
}

void
BasicsTest::print_stats(std::ostream& out) throw(eh::Exception)
{
  out << '\n' << name() << ":\n" << out_.str() << std::endl;
}

void
BasicsTest::print_errors(std::ostream& out) throw(eh::Exception)
{
  if (!error_.str().empty())
  {
    out << '\n' << name() << " ERRORS:\n"
        << error_.str()
        << "Test Log:\n" << log_.str()
        << std::endl;
  }
}

//
// class BasicsTest01
//

const char*
BasicsTest01::scenario_descr() throw()
{
  return "This is a description of BasicsTest01 scenario.\n"
         "  Params: 1 connection per server, 1 connection per thread.\n"
         "  Test makes 1 request and expects linear states's changes for both\n"
         "  connection and thread:\n"
         "    ACTIVE_AWAITING (is chosen)\n"
         "    ACTIVE (got object)\n"
         "    CLOSURE_AWAITING (there is no objects more and there is no other\n"
         "    threads/connections in this state)\n"
         "    CLOSURE_ON_NEXT_TRY (there is no objects more and there is no other\n"
         "    threads/connections)\n"
         "    CLOSING (recommendation to close)\n"
         "    CLOSING (closed)\n";
}

BasicsTest01::BasicsTest01(Sync::Semaphore& finish_sem,
    std::vector<HTTP::HttpServer>& servers) throw (eh::Exception):
  BasicsTest(finish_sem, servers, 1, 1, 3, 3)
{
  init_("echo_w_optional_delay.pl");
}

BasicsTest01::~BasicsTest01() throw ()
{
}

const char*
BasicsTest01::name() throw()
{
  return "BasicsTest01";
}

void
BasicsTest01::exec_init_() throw()
{
  typedef ConnThrScenarios::ScenarioArrayElem ScenarioArrayElem;
  typedef CheckSimpleEmptyCommons::StateInfo StateInfo;

  const ScenarioArrayElem conn_scenario[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 3, StateInfo::CLOSURE_AWAITING),
    ScenarioArrayElem( 3, StateInfo::CLOSURE_ON_NEXT_TRY),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  const ScenarioArrayElem thr_scenario[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 3, StateInfo::CLOSURE_AWAITING),
    ScenarioArrayElem( 3, StateInfo::CLOSURE_ON_NEXT_TRY),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  scens_.add_scenario(CheckSimpleEmptyCommons::OT_CONNECTION,
                      conn_scenario,
                      sizeof(conn_scenario) / sizeof(conn_scenario[0]));
  scens_.add_scenario(CheckSimpleEmptyCommons::OT_THREAD,
                      thr_scenario,
                      sizeof(thr_scenario) / sizeof(thr_scenario[0]));
}

void
BasicsTest01::scenario_(HTTP::HttpActiveInterface* pool,
  SimpleCounterCallback* callback) throw(eh::Exception)
{
  pool->add_get_request((http_request_ + '1').c_str(), callback);
}

//
// class BasicsTest02
//

const char*
BasicsTest02::scenario_descr() throw()
{
  return "This is a description of BasicsTest02 scenario.\n"
         "  Params: 2 connections per server, 2 connections per thread.\n"
         "  Test makes 2 requests and expects states's changes printed below:\n"
         "  1 connection's scenario:\n"
         "    ACTIVE_AWAITING (is chosen)\n"
         "    ACTIVE (got object)\n"
         "    CLOSURE_AWAITING (there is no objects more and there is no other\n"
         "    threads/connections in this state)\n"
         "    CLOSURE_ON_NEXT_TRY (there is no objects more and there is no other\n"
         "    threads/connections)\n"
         "    CLOSING (recommendation to close)\n"
         "    CLOSING (closed)\n"
         "  2 connection's scenario:\n"
         "    ACTIVE_AWAITING (is chosen)\n"
         "    ACTIVE (got object)\n"
         "    CLOSING (recommendation to close; leaves CLOSURE_AWAITING state\n"
         "    because there is connection in CLOSURE_AWAITING state (see 1 scen)\n"
         "    CLOSING (closed)\n"
         "  1 thread's scenario:\n"
         "    ACTIVE_AWAITING (is chosen)\n"
         "    ACTIVE (got object - first connection)\n"
         "    ACTIVE_AWAITING (is chosen again - there is no other threads)\n"
         "    ACTIVE (got object - second connection)\n"
         "    CLOSURE_AWAITING (there is no objects more and there is no other\n"
         "    threads/connections in this state)\n"
         "    CLOSURE_ON_NEXT_TRY (there is no objects more and there is no other\n"
         "    threads/connections)\n"
         "    CLOSING (recommendation to close)\n"
         "    CLOSING (closed)\n";
}

BasicsTest02::BasicsTest02(Sync::Semaphore& finish_sem,
    std::vector<HTTP::HttpServer>& servers) throw (eh::Exception):
  BasicsTest(finish_sem, servers, 2, 2, 3, 3)
{
  init_("echo_w_optional_delay.pl");
}

BasicsTest02::~BasicsTest02() throw ()
{
}

const char*
BasicsTest02::name() throw()
{
  return "BasicsTest02";
}

void
BasicsTest02::exec_init_() throw()
{
  typedef ConnThrScenarios::ScenarioArrayElem ScenarioArrayElem;
  typedef CheckSimpleEmptyCommons::StateInfo StateInfo;

  const ScenarioArrayElem conn_scenario1[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  const ScenarioArrayElem conn_scenario2[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 3, StateInfo::CLOSURE_AWAITING),
    ScenarioArrayElem( 3, StateInfo::CLOSURE_ON_NEXT_TRY),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  const ScenarioArrayElem thr_scenario[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 3, StateInfo::CLOSURE_AWAITING),
    ScenarioArrayElem( 3, StateInfo::CLOSURE_ON_NEXT_TRY),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  scens_.add_scenario(CheckSimpleEmptyCommons::OT_CONNECTION,
                      conn_scenario1,
                      sizeof(conn_scenario1) / sizeof(conn_scenario1[0]));
  scens_.add_scenario(CheckSimpleEmptyCommons::OT_CONNECTION,
                      conn_scenario2,
                      sizeof(conn_scenario2) / sizeof(conn_scenario2[0]));
  scens_.add_scenario(CheckSimpleEmptyCommons::OT_THREAD,
                      thr_scenario,
                      sizeof(thr_scenario) / sizeof(thr_scenario[0]));
}

void
BasicsTest02::scenario_(HTTP::HttpActiveInterface* pool,
  SimpleCounterCallback* callback) throw(eh::Exception)
{
  pool->add_get_request((http_request_ + '1').c_str(), callback);
  pool->add_get_request((http_request_ + '1').c_str(), callback);
}

//
// class BasicsTest03
//

const char*
BasicsTest03::scenario_descr() throw()
{
  return "This is a description of BasicsTest03 scenario.\n"
         "  Params: 2 connections per server, 2 connections per thread.\n"
         "  Test makes 8 requests and expects states's changes printed below:\n"
         "  1 connection's scenario:\n"
         "    ACTIVE_AWAITING (is chosen)\n"
         "    ACTIVE (got object)\n"
         "    ACTIVE_AWAITING (is chosen - has minimum number of requests)\n"
         "    ACTIVE (got object)\n"
         "    ACTIVE_AWAITING (is chosen - has minimum number of requests)\n"
         "    ACTIVE (got object)\n"
         "    CLOSING (recommendation to close; leaves CLOSURE_AWAITING state\n"
         "    because there is connection in CLOSURE_AWAITING state (see 3 scen)\n"
         "    CLOSING (closed)\n"
         "  2 connection's scenario:\n"
         "    ACTIVE_AWAITING (is chosen)\n"
         "    ACTIVE (got object)\n"
         "    ACTIVE_AWAITING (is chosen - has minimum number of requests)\n"
         "    ACTIVE (got object)\n"
         "    CLOSING (recommendation to close; leaves CLOSURE_AWAITING state\n"
         "    because there is connection in CLOSURE_AWAITING state (see 3 scen)\n"
         "    and leaves third ACTIVE_AWAITING state because there are just 7\n"
         "    requests and 7th already got (see 1 scen)\n"
         "    CLOSING (closed)\n"
         "  3 connection's scenario:\n"
         "    ACTIVE_AWAITING (is chosen)\n"
         "    ACTIVE (got object)\n"
         "    ACTIVE_AWAITING (is chosen - has minimum number of requests)\n"
         "    ACTIVE (got object)\n"
         "    CLOSURE_AWAITING (there is no objects more and there is no other\n"
         "    threads/connections in this state)\n"
         "    1 second passes\n"
         "    ACTIVE_AWAITING (is chosen - there is no other connections)\n"
         "    ACTIVE (got object)\n"
         "    CLOSURE_AWAITING (there is no objects more and there is no other\n"
         "    threads/connections in this state)\n"
         "    CLOSURE_ON_NEXT_TRY (there is no objects more and there is no other\n"
         "    threads/connections)\n"
         "    CLOSING (recommendation to close)\n"
         "    CLOSING (closed)\n"
         "  1 thread's scenario:\n"
         "    ACTIVE_AWAITING (is chosen)\n"
         "    ACTIVE (got object - first connection)\n"
         "    ACTIVE_AWAITING (is chosen again - there is no other threads)\n"
         "    ACTIVE (got object - second connection)\n"
         "    ACTIVE_AWAITING (is chosen again - there is no other threads)\n"
         "    ACTIVE (got object - third connection)\n"
         "    CLOSURE_AWAITING (there is no objects more and there is no other\n"
         "    threads/connections in this state)\n"
         "    CLOSURE_ON_NEXT_TRY (there is no objects more and there is no other\n"
         "    threads/connections)\n"
         "    CLOSING (recommendation to close)\n"
         "    CLOSING (closed)\n";
}

BasicsTest03::BasicsTest03(Sync::Semaphore& finish_sem,
    std::vector<HTTP::HttpServer>& servers) throw (eh::Exception):
  BasicsTest(finish_sem, servers, 3, 3, 10, 10)
{
  init_("echo_w_optional_delay.pl");
}

BasicsTest03::~BasicsTest03() throw ()
{
}

const char*
BasicsTest03::name() throw()
{
  return "BasicsTest03";
}

void
BasicsTest03::exec_init_() throw()
{
  typedef ConnThrScenarios::ScenarioArrayElem ScenarioArrayElem;
  typedef CheckSimpleEmptyCommons::StateInfo StateInfo;

  const ScenarioArrayElem conn_scenario1[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  const ScenarioArrayElem conn_scenario2[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  const ScenarioArrayElem conn_scenario3[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 10, StateInfo::CLOSURE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 10, StateInfo::CLOSURE_AWAITING),
    ScenarioArrayElem( 10, StateInfo::CLOSURE_ON_NEXT_TRY),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  const ScenarioArrayElem thr_scenario[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 10, StateInfo::CLOSURE_AWAITING),
    ScenarioArrayElem( 10, StateInfo::CLOSURE_ON_NEXT_TRY),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  scens_.add_scenario(CheckSimpleEmptyCommons::OT_CONNECTION,
                      conn_scenario1,
                      sizeof(conn_scenario1) / sizeof(conn_scenario1[0]));
  scens_.add_scenario(CheckSimpleEmptyCommons::OT_CONNECTION,
                      conn_scenario2,
                      sizeof(conn_scenario2) / sizeof(conn_scenario2[0]));
  scens_.add_scenario(CheckSimpleEmptyCommons::OT_CONNECTION,
                      conn_scenario3,
                      sizeof(conn_scenario3) / sizeof(conn_scenario3[0]));
  scens_.add_scenario(CheckSimpleEmptyCommons::OT_THREAD,
                      thr_scenario,
                      sizeof(thr_scenario) / sizeof(thr_scenario[0]));
}

void
BasicsTest03::scenario_(HTTP::HttpActiveInterface* pool,
  SimpleCounterCallback* callback) throw(eh::Exception)
{
  pool->add_get_request((http_request_ + '1').c_str(), callback);
  pool->add_get_request((http_request_ + '1').c_str(), callback);
  pool->add_get_request((http_request_ + '1').c_str(), callback);
  pool->add_get_request((http_request_ + '1').c_str(), callback);
  pool->add_get_request((http_request_ + '1').c_str(), callback);
  pool->add_get_request((http_request_ + '1').c_str(), callback);
  pool->add_get_request((http_request_ + '2').c_str(), callback);

  sleep(11);
  pool->add_get_request((http_request_ + '1').c_str(), callback);
}

//
// class BasicsTest04
//

const char*
BasicsTest04::scenario_descr() throw()
{
  return "This is a description of BasicsTest04 scenario.\n"
         "  Params: 3 connections per server, 1 connections per thread.\n"
         "  Test makes 8 requests and expects states's changes printed below:\n"
         "  1 connection's scenario:\n"
         "    see BasicsTest03 scenario\n"
         "  2 connection's scenario:\n"
         "    see BasicsTest03 scenario\n"
         "  3 connection's scenario:\n"
         "    see BasicsTest03 scenario\n"
         "  1 thread's scenario:\n"
         "    ACTIVE_AWAITING (is chosen)\n"
         "    ACTIVE (got connection)\n"         
         "    CLOSURE_AWAITING (there is no objects more and there is no other\n"
         "    threads/connections in this state)\n"
         "    CLOSURE_AWAITING (there is no objects more and there is no other\n"
         "    threads/connections in this state; one more thread exists, because\n"
         "    a connection in state CLOSURE_ON_NEXT_TRY still exists)\n"
         "    CLOSURE_AWAITING (there is no objects more and there is no other\n"
         "    threads/connections in this state) one more thread exists, because\n"
         "    a connection in state CLOSING still exists)\n"
         "    CLOSURE_ON_NEXT_TRY (there is no objects more and there is no other\n"
         "    threads/connections)\n"
         "    CLOSING (recommendation to close)\n"
         "    CLOSING (closed)\n"
         "  2 thread's scenario / 3 thread's scenario:\n"
         "    ACTIVE_AWAITING (is chosen)\n"
         "    ACTIVE (got connection)\n"
         "    CLOSING (recommendation to close; state CLOSURE_AWAITING is passed\n"
         "    because there is a thread in state CLOSURE_AWAITING (see 1 scen))\n"
         "    CLOSING (closed)\n";
}

BasicsTest04::BasicsTest04(Sync::Semaphore& finish_sem,
    std::vector<HTTP::HttpServer>& servers) throw (eh::Exception):
  BasicsTest(finish_sem, servers, 3, 1, 11, 10)
{
  init_("echo_w_optional_delay.pl");
}

BasicsTest04::~BasicsTest04() throw ()
{
}

const char*
BasicsTest04::name() throw()
{
  return "BasicsTest04";
}

void
BasicsTest04::exec_init_() throw()
{
  typedef ConnThrScenarios::ScenarioArrayElem ScenarioArrayElem;
  typedef CheckSimpleEmptyCommons::StateInfo StateInfo;

  const ScenarioArrayElem conn_scenario1[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  const ScenarioArrayElem conn_scenario2[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  const ScenarioArrayElem conn_scenario3[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 10, StateInfo::CLOSURE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 10, StateInfo::CLOSURE_AWAITING),
    ScenarioArrayElem( 10, StateInfo::CLOSURE_ON_NEXT_TRY),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  const ScenarioArrayElem thr_scenario1[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 11, StateInfo::CLOSURE_AWAITING),
    ScenarioArrayElem( 11, StateInfo::CLOSURE_AWAITING),
    ScenarioArrayElem( 11, StateInfo::CLOSURE_AWAITING),
    ScenarioArrayElem( 11, StateInfo::CLOSURE_ON_NEXT_TRY),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  const ScenarioArrayElem thr_scenario2[] = {
    ScenarioArrayElem(-2, StateInfo::ACTIVE_AWAITING),
    ScenarioArrayElem(-2, StateInfo::ACTIVE),
    ScenarioArrayElem( 0, StateInfo::CLOSING),
    ScenarioArrayElem(-2, StateInfo::CLOSING)
  };

  scens_.add_scenario(CheckSimpleEmptyCommons::OT_CONNECTION,
                      conn_scenario1,
                      sizeof(conn_scenario1) / sizeof(conn_scenario1[0]));
  scens_.add_scenario(CheckSimpleEmptyCommons::OT_CONNECTION,
                      conn_scenario2,
                      sizeof(conn_scenario2) / sizeof(conn_scenario2[0]));
  scens_.add_scenario(CheckSimpleEmptyCommons::OT_CONNECTION,
                      conn_scenario3,
                      sizeof(conn_scenario3) / sizeof(conn_scenario3[0]));
  scens_.add_scenario(CheckSimpleEmptyCommons::OT_THREAD,
                      thr_scenario1,
                      sizeof(thr_scenario1) / sizeof(thr_scenario1[0]));
  scens_.add_scenario(CheckSimpleEmptyCommons::OT_THREAD,
                      thr_scenario2,
                      sizeof(thr_scenario2) / sizeof(thr_scenario2[0]));
}

void
BasicsTest04::scenario_(HTTP::HttpActiveInterface* pool,
  SimpleCounterCallback* callback) throw(eh::Exception)
{
  pool->add_get_request((http_request_ + '1').c_str(), callback);
  pool->add_get_request((http_request_ + '1').c_str(), callback);
  pool->add_get_request((http_request_ + '1').c_str(), callback);
  pool->add_get_request((http_request_ + '1').c_str(), callback);
  pool->add_get_request((http_request_ + '1').c_str(), callback);
  pool->add_get_request((http_request_ + '1').c_str(), callback);
  pool->add_get_request((http_request_ + '2').c_str(), callback);

  sleep(11);
  pool->add_get_request((http_request_ + '1').c_str(), callback);
}

//
// class RandomLoadingTest
//

const char*
RandomLoadingTest::scenario_descr() throw()
{
  return "This is a description of RandomLoadingTest. It is intended for\n"
         "  dynamic checking of states switchings (for both threads and connections).\n"
         "  It is consists of two cycles. Inner cycle randomly chooses 100 requests\n"
         "  (the number of requests to send is a hard-coded constant) and sends them\n"
         "  to various servers. Outer cycle waits n seconds (n - randomly chosen\n"
         "  number between 0 and 4) and runs Inner cycle. Outer cycle has 100\n"
         "  iterations (this number is a hard-coded constant).";
}

RandomLoadingTest::RandomLoadingTest(Sync::Semaphore& finish_sem,
    std::vector<HTTP::HttpServer>& servers) throw (eh::Exception):
  BasicsTest(finish_sem, servers, 3, 4, 1, 1, false),
  requests_()
{
  requests_.reserve(servers.size());

  for (size_t ind = 0; ind < servers.size(); ++ind)
  {
    init_("echo.pl", ind);
    requests_.push_back(http_request_);
  }
}

RandomLoadingTest::~RandomLoadingTest() throw ()
{
}

const char*
RandomLoadingTest::name() throw()
{
  return "RandomLoadingTest";
}

void
RandomLoadingTest::execute() throw()
{
  exec_main_();

  if (error_.str().empty())
  {
    out_ << "Results: success\n";
  }
  else
  {
    out_ << "Results: failure\n";
  }

  finish_sem_.release();
}

void
RandomLoadingTest::scenario_(HTTP::HttpActiveInterface* pool,
  SimpleCounterCallback* callback) throw(eh::Exception)
{
  pool->add_get_request(http_request_.c_str(), callback);

  for (size_t ind1 = 0; ind1 < 100; ++ind1)
  {
    sleep(rand() % 5);
    for (size_t ind2 = 0; ind2 < 100; ind2++)
    {
      pool->add_get_request(requests_[rand() % requests_.size()].c_str(), callback);
    }
  }
}

void
RandomLoadingTest::exec_init_() throw()
{
}
