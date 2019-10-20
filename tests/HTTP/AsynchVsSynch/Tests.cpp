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
#include <vector>
#include <HTTP/HttpConnection.hpp>
#include <HTTP/HttpAsyncPool.hpp>

using namespace HTTP;

namespace
{
  const String::SubString CANNOT_MAKE_REQUEST(
    "CommonTest::synch_process(0): Can't make request"
    " task (can't allocate memory or can't start new thread).");
}

//
// General constants
//

const std::string ECHO_GET_STRING =
  "app=PS&v=1.3.0-3.ssv1&tid=108&rnd=388334&"
  "xinfopsid=0&format=html&require-debug-info="
  "body&glbfcap=0&referer=act.com";
const std::string ECHO_POST_STRING =
  "login=Petya%20Vasechkin&password=qq";

//
// CommonTest
//

//
// CommonTest Constants
//

//
// class CommonTest
//


CommonTest::CommonTest(Sync::Semaphore& finish_sem, size_t threads_count,
  size_t requests_count, size_t pools_count, size_t units_count,
  size_t conns_per_serv_count, size_t conns_per_thr_count,
  std::vector<HTTP::HttpServer>& servers, bool keep_alive, bool asynch_only)
    throw (eh::Exception):
  VSTestInterface(finish_sem),
  pools_count_((units_count < pools_count)? units_count: pools_count),
  units_count_(units_count),
  threads_count_((threads_count > pools_count_)? threads_count: pools_count_ + 1),
  requests_count_(requests_count / 100 * 100),
  conns_per_serv_count_(conns_per_serv_count),
  conns_per_thr_count_(conns_per_thr_count),
  servers_(servers),
  requests_by_type_(),
  policy_ptr_(new SimplePolicy),
  policy_(policy_ptr_),
  tests_runner_(new Generics::TaskRunner(policy_ptr_, threads_count_ - pools_count_ + 1)),
  keep_alive_(keep_alive),
  asynch_only_(asynch_only),
  param_(0),
  threads_sem_(1),
  main_sem_(0)
{
  size_t servs_end = servers_.size();
  size_t ind = 0;
  for (ind = 0; ind < servs_end; ++ind)
  {
    std::ostringstream base;
    base << "http://" << servers_[ind].first << ':' << servers_[ind].second << "/cgi-bin/echo.pl?";
    requests_by_type_.push_back(base.str() + ECHO_GET_STRING);
    requests_by_type_.push_back(base.str() + "ff=0");
  }

  warning_[0] = '\0';
}

CommonTest::~CommonTest() throw ()
{
}

void
CommonTest::execute() throw()
{
  if (!asynch_only_)
  {
    synch_process();
  }
  asynch_process();
  finish_sem_.release();
}

void
CommonTest::synch_process() throw()
{
  Generics::Timer timer;
  timer.start();

  size_t released_thrs_count = requests_count_;
  size_t ind = 0;
  size_t serv_end = servers_.size();
  size_t serv_ind = 0;
  for (ind = 0; ind < threads_count_; ++ind)
  {
    pthread_t pid;
    InfoToCallback* info = new(std::nothrow) InfoToCallback(policy_,
      (serv_ind << 1) + ind % 2, requests_by_type_, threads_sem_, main_sem_);
    serv_ind = (++serv_ind >= serv_end? 0: serv_ind);
    if (info == 0 || -1 == pthread_create(&pid, 0, send_synch_req, info))
    {
      delete info;
      policy_->error(CANNOT_MAKE_REQUEST);
      return;
    }

    pthread_detach(pid);
  }

  while (released_thrs_count)
  {
    main_sem_.acquire();
    threads_sem_.release();
    --released_thrs_count;

    if (released_thrs_count < threads_count_)
    {
      continue;
    }

    pthread_t pid;
    InfoToCallback* info = new(std::nothrow) InfoToCallback(policy_,
      (serv_ind << 1) + (released_thrs_count - threads_count_) % 2,
      requests_by_type_, threads_sem_, main_sem_);
    serv_ind = (++serv_ind >= serv_end? 0: serv_ind);
    if (info == 0 || -1 == pthread_create(&pid, 0, send_synch_req, info))
    {
      delete info;
      policy_->error(CANNOT_MAKE_REQUEST);
      return;
    }

    pthread_detach(pid);
  }

  timer.stop();
  synch_timer_ = timer;
}

void
CommonTest::asynch_process() throw()
{
  try
  {
    Generics::Timer timer;
    timer.start();
    Semaphores_ sems;
    sems.reserve(units_count_);

    activation_();

    mt_testers_gen_(sems);

    for (size_t i = 0; i < units_count_; ++i)
    {
      sems[i]->acquire();
    }

    deactivation_();

    timer.stop();
    asynch_timer_ = timer;

    char* error_ptr = warning_;
    check_error_(error_ptr, C_BUFFER_SIZE);
  }
  catch (const eh::Exception& e)
  {
    Stream::Stack<C_BUFFER_SIZE> ostr;
    ostr << "CommonTest::asynch_process(0): eh::Exception caught: "
      << e.what();
    policy_->error(ostr.str());
  }
}

void*
CommonTest::send_synch_req(void* arg) throw()
{
  InfoToCallback* info = static_cast<InfoToCallback*>(arg);
  try
  {
    HTTP_Connection::HttpBody* body = 0;
    HTTP_Connection::HTTP_Method method;
    std::string http_request;
    http_request = info->requests_by_type[info->type];
    if ((info->type & 0x01) == C_GET_TYPE)
    {
      method = HTTP_Connection::HM_Get;
    }
    else if ((info->type & 0x01) == C_POST_TYPE)
    {
      body = new HTTP_Connection::HttpBody(
        ECHO_POST_STRING.c_str(), ECHO_POST_STRING.length());
      method = HTTP_Connection::HM_Post;
    }

    HTTPAddress addr(http_request);
    HTTP_Connection http_connection(addr, 0);
    http_connection.connect(0);
    HTTP::ParamList params;
    HTTP::HeaderList headers;
    unsigned int bytes_out = 0;
    unsigned int bytes_in = 0;
    Generics::Time timeout(5);
    Generics::Time latency(0);
    http_connection.process_request(method,
                                    params,
                                    headers,
                                    body,
                                    true,
                                    &timeout,
                                    &timeout,
                                    &bytes_out,
                                    &bytes_in,
                                    &latency);

    body->release();
  }
  catch(const eh::Exception& e)
  {
    Stream::Stack<C_BUFFER_SIZE> ostr;
    ostr << "CommonTest::send_synch_req(1): eh::Exception caught: "
      << e.what();
    const String::SubString& error = ostr.str();
    std::cerr << error << std::endl;
    info->policy->error(error);
  }

  info->threads_sem.acquire();
  info->main_sem.release();

  delete info;
  return 0;
}

inline
void
CommonTest::mt_testers_gen_(Semaphores_& sems) throw (eh::Exception)
{
  const int MT_TESTER_TASKS = 1;
  const int MT_TESTER_TMOUT = 0;
  const size_t MT_TESTER_REQS_CNT = requests_count_ / units_count_ / 100;

  test_units_.reserve(units_count_);

  size_t servs_end = servers_.size();
  size_t servs_ind = 0;
  for (size_t i = 0; i < units_count_; ++i)
  {
    HTTP::PoolPolicy_var loc_policy(new SimplePolicy(conns_per_serv_count_,
                                                     conns_per_thr_count_));

    if (i < pools_count_)
    {
      pools_.push_back(HTTP::HttpActiveInterface_var(
        HTTP::CreatePool(loc_policy.in(), tests_runner_.in())));
      pools_.back()->activate_object();
    }

    callbacks_.push_back(NotificationCallback_var(
      new NotificationCallback(
        loc_policy, requests_count_ / units_count_)));

    sems.push_back(&callbacks_.back()->get_semaphore());

    HTTP::ResponseCallback_var cb(callbacks_.back().in());
    cb->add_ref();

    std::unique_ptr<Requester> requester(
      new Requester(*this,
                    pools_[(i < pools_count_? i: (pools_count_ - 1))],
                    cb,
                    requests_by_type_[servs_ind * 2],
                    requests_by_type_[servs_ind * 2 + 1],
                    ECHO_POST_STRING));
    std::unique_ptr<TestCommons::MTTester<Requester&> > tester(
      new TestCommons::MTTester<Requester&>(*requester, tests_runner_));

    tester->run(MT_TESTER_TASKS, MT_TESTER_TMOUT, MT_TESTER_REQS_CNT);

    test_units_.emplace_back(requester.release(), tester.release());

    servs_ind = (++servs_ind >= servs_end? 0: servs_ind);
  }
}

inline
void
CommonTest::check_error_(char*& error_buf, size_t buf_len) const
  throw(eh::Exception)
{
  size_t buf_ptr = 0;
  for (size_t i = 0; i < units_count_; ++i)
  {
    if (callbacks_[i]->get_counter().failed())
    {
      if (buf_ptr == 0)
      {
        const char error_prefix[] = "CommonTest::asynch_process(0): Some "
                                    "asynch requests failed: ";
        String::StringManip::strlcpy(error_buf, error_prefix, buf_len);
        buf_ptr = sizeof(error_prefix) - 1;
        if (sizeof(error_prefix) > buf_len)
        {
          break;
        }
      }

      std::ostringstream ostr;
      callbacks_[i]->print_stat(ostr);
      String::StringManip::strlcpy(error_buf + buf_ptr,
        ostr.str().c_str(), buf_len - buf_ptr);
      buf_ptr = strlen(error_buf);
      if (buf_ptr >= buf_len)
      {
        break;
      }
    }
  }
}

inline
void
CommonTest::activation_() throw (eh::Exception)
{
  tests_runner_->activate_object();
}

inline
void
CommonTest::deactivation_() throw (eh::Exception)
{
  size_t i = 0;

  for (i = 0; i < pools_count_; ++i)
  {
    pools_[i]->deactivate_object();
  }

  for (i = 0; i < pools_count_; ++i)
  {
    pools_[i]->wait_object();
  }

  tests_runner_->deactivate_object();
  tests_runner_->wait_object();
}

void
CommonTest::print_stat(std::ostream& out) const throw(eh::Exception)
{
  out << "::CommonTest::\n  Parameters:"
      << "\nKeep-Alive: " << (keep_alive_? "On": "Off")
      << "\nAsynchPools: " << pools_count_
      << "\nTesters: " << units_count_
      << "\nThreads: " << threads_count_
      << "\nRequests: " << requests_count_
      << "\nConnections per server: " << conns_per_serv_count_
      << "\nConnections per thread: " << conns_per_thr_count_
      << "\n  Results:";

  if (!asynch_only_)
  {
    out << "\nSynch test results: ";
    out << synch_timer_.elapsed_time();
  }

  out << "\nAsynch test results: ";
  out << asynch_timer_.elapsed_time();

  if (warning_[0] != '\0')
  {
    out << "\nWarnings:\n" << warning_;
  }

  out << '\n';
}

const std::string
CommonTest::additional_http_query() throw (eh::Exception)
{
  try
  {
    std::ostringstream ostr;
    ostr << "&param_cnt=" << param_++;
    return ostr.str();
  }
  catch (...)
  {
    return "";
  }
}

//
// struct CommonTest::TestSuite
//

CommonTest::TestSuite::TestSuite(RequesterType* new_requester, TesterType* new_tester):
  requester(new_requester),
  tester(new_tester)
{
}

CommonTest::TestSuite::TestSuite(TestSuite&& other)
  : requester(std::move(other.requester)), tester(std::move(other.tester))
{
}

CommonTest::TestSuite&
CommonTest::TestSuite::operator =(TestSuite&& other)
{
  requester = std::move(other.requester);
  tester = std::move(other.tester);
  return *this;
}
