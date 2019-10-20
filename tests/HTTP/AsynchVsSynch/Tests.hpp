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



#ifndef _ASYNCH_VS_SYNCH_TEST_TESTS_HPP_
#define _ASYNCH_VS_SYNCH_TEST_TESTS_HPP_

#include <ReferenceCounting/Vector.hpp>

#include "CommonClasses.hpp"

//
// class CommonTest
//

class CommonTest : public VSTestInterface
{
  //Constants
  enum Constants{
    C_GET_TYPE = 0,
    C_POST_TYPE = 1,
    C_BUFFER_SIZE = 4096
  };

  struct TestSuite
  {
    typedef Requester RequesterType;
    typedef TestCommons::MTTester<Requester&> TesterType;

    std::unique_ptr<RequesterType> requester;
    std::unique_ptr<TesterType> tester;

    TestSuite(RequesterType* new_requester, TesterType* new_tester);
    TestSuite(TestSuite&&);
    TestSuite&
    operator =(TestSuite&&);
  };

public:

  CommonTest(Sync::Semaphore& finish_sem, size_t threads_count,
      size_t requests_count, size_t pools_count, size_t units_count,
      size_t conns_per_serv_count, size_t conns_per_thr_count,
      std::vector<HTTP::HttpServer>& servers, bool keep_alive, bool asynch_only)
    throw (eh::Exception);

  void execute() throw();

  void synch_process() throw();

  void asynch_process() throw();

  static void* send_synch_req(void*) throw();

  virtual void print_stat(std::ostream& out) const throw(eh::Exception);
  
  virtual const std::string
  additional_http_query() throw (eh::Exception);

protected:

  typedef std::vector<Sync::Semaphore*> Semaphores_;

  virtual
  ~CommonTest() throw ();

  void
  check_error_(char*& error_buf, size_t buf_len) const throw(eh::Exception);

  void
  mt_testers_gen_(Semaphores_& sems) throw (eh::Exception);

  void
  activation_() throw (eh::Exception);

  void
  deactivation_() throw (eh::Exception);

private:

  const size_t pools_count_;
  const size_t units_count_;
  const size_t threads_count_;
  const size_t requests_count_;
  const size_t conns_per_serv_count_;
  const size_t conns_per_thr_count_;
  std::vector<HTTP::HttpServer> servers_;
  std::vector<std::string> requests_by_type_;
  SimplePolicy* policy_ptr_;
  HTTP::PoolPolicy_var policy_;
  Generics::TaskRunner_var tests_runner_;
  Generics::Timer synch_timer_;
  Generics::Timer asynch_timer_;
  bool keep_alive_;
  bool asynch_only_;
  ReferenceCounting::Vector<HTTP::HttpActiveInterface_var> pools_;
  ReferenceCounting::Vector<NotificationCallback_var> callbacks_;
  std::vector<TestSuite> test_units_;
  volatile _Atomic_word param_;
  char warning_[C_BUFFER_SIZE];
  Sync::Semaphore threads_sem_;
  Sync::Semaphore main_sem_;
};

#endif
