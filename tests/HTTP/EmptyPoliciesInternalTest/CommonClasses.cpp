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
#include <HTTP/HttpConnection.hpp>
#include <HTTP/HttpAsyncPool.hpp>
#include <iostream>

//
// class PoliciesTestInterface
//

PoliciesTestInterface::PoliciesTestInterface(Sync::Semaphore& finish_sem)
    throw(eh::Exception):
  finish_sem_(finish_sem)
{
}

PoliciesTestInterface::~PoliciesTestInterface() throw ()
{
}

void
PoliciesTestInterface::execute() throw()
{
  exec_init_();
  exec_main_();
  exec_finish_();
  finish_sem_.release();
}

//
// Check policies classes
//

//
// class CheckSimpleEmptyCommons
//

Sync::PosixMutex CheckSimpleEmptyCommons::dump_mutex_;

CheckSimpleEmptyCommons::CheckSimpleEmptyCommons(
    unsigned int closure_delay_value) throw():
  closure_delay_value_(closure_delay_value)
{
}

CheckSimpleEmptyCommons::~CheckSimpleEmptyCommons() throw()
{
}

inline
void
CheckSimpleEmptyCommons::set_history_event_(Identifier id,
  ObjectType type, StateInfo::States state, int result) throw (eh::Exception)
{
  Histories::iterator h_it = histories_.find(id);
  if (h_it == histories_.end())
  {
    h_it = histories_.insert(Histories::value_type(id,
      StateHistory(type, result, state, Generics::Time::get_time_of_day()))).first;
  }
  else
  {
    StateHistory* tail = &(h_it->second);
    while (tail->next != 0) tail = tail->next;
    tail->next = new StateHistory(
      type, result, state, Generics::Time::get_time_of_day());
  }

  cur_history_ = &h_it->second;
}

void
CheckSimpleEmptyCommons::remove_history_(Identifier id) throw (eh::Exception)
{
  Histories::iterator h_it = histories_.find(id);
  if (h_it != histories_.end())
  {
    completed_histories_.push_back(std::pair<Identifier, StateHistory>
      (h_it->first, h_it->second));
    histories_.erase(h_it);
    cur_history_ = nullptr;
  }
}

void
CheckSimpleEmptyCommons::dynamic_states_checker_(const char* prefix,
  const void* addr, const StateHistory* prev_n_now, std::ostream& error)
  throw (eh::Exception)
{
  const StateHistory* prev = prev_n_now;
  const StateHistory* now = prev_n_now->next;
  bool error_detected = false;

  switch (prev->state)
  {
    case StateInfo::ACTIVE_AWAITING:
      if ((now->state != StateInfo::ACTIVE || now->result != -2) &&
          (now->state != StateInfo::ACTIVE_AWAITING || now->result != -1))
      {
        error_detected = true;
      }
      break;
    case StateInfo::ACTIVE:
      if ((now->state != StateInfo::ACTIVE_AWAITING || now->result != -2) &&
          (now->state != StateInfo::ACTIVE || now->result != -1) &&
          (now->state != StateInfo::CLOSURE_AWAITING ||
           now->result != closure_delay_value_ || closure_delay_value_ == 0) &&
          (now->state != StateInfo::CLOSING || now->result != 0))
      {
        error_detected = true;
      }
      break;
    case StateInfo::CLOSURE_AWAITING:
      if ((now->state != StateInfo::ACTIVE_AWAITING || now->result != -2) &&
          (now->state != StateInfo::CLOSURE_ON_NEXT_TRY ||
           now->result != closure_delay_value_ || closure_delay_value_ == 0) &&
          (now->state != StateInfo::CLOSURE_AWAITING ||
           now->result != closure_delay_value_ || closure_delay_value_ == 0))
      {
        error_detected = true;
      }
      break;
    case StateInfo::CLOSURE_ON_NEXT_TRY:
      if ((now->state != StateInfo::ACTIVE_AWAITING || now->result != -2) &&
          (now->state != StateInfo::CLOSING || now->result != 0))
      {
        error_detected = true;
      }
      break;
    case StateInfo::CLOSING:
      if (now->state != StateInfo::CLOSING || now->result != -2)
      {
        error_detected = true;
      }
      break;
    default:
      error_detected = true;
      break;
  }

  if (error_detected)
  {
    error << "CheckSimpleEmptyCommons::dynamic_states_checker_(2): "
             "unexpected state switching:\nfrom \n";
    print_state_history(prefix, addr, *prev, error);
    error << "to \n";
    print_state_history(prefix, addr, *now, error);
    error << '\n';
  }
}

void
CheckSimpleEmptyCommons::print_state_history(const char* prefix,
  const void* addr, const StateHistory& obj,
  std::ostream& out) throw(eh::Exception)
{
  if (!prefix) return;

  Sync::PosixGuard guard(dump_mutex_);

  out << prefix << '\t' << addr
      << "\ttime: " << obj.time 
      << "\tresult: " << obj.result
      << "\tstate: ";
  switch (obj.state)
  {
    case StateInfo::ACTIVE_AWAITING: out << "ACTIVE_AWAITING"; break;
    case StateInfo::ACTIVE: out << "ACTIVE"; break;
    case StateInfo::CLOSURE_AWAITING: out << "CLOSURE_AWAITING"; break;
    case StateInfo::CLOSURE_ON_NEXT_TRY: out << "CLOSURE_ON_NEXT_TRY"; break;
    case StateInfo::CLOSING: out << "CLOSING"; break;
    default: out << "UNKNOWN=" << obj.state << ' '; break;
  }
  out << '\n';
}

CheckSimpleEmptyCommons::StateHistory::StateHistory(ObjectType new_type,
    int new_result, StateInfo::States new_state, const Generics::Time& new_time) throw():
  object_type(new_type), result(new_result), state(new_state), time(new_time), next(0)
{
}

CheckSimpleEmptyCommons::StateHistory::StateHistory(
    const StateHistory& src) throw(eh::Exception):
  object_type(src.object_type), result(src.result), state(src.state),
  time(src.time), next(0)
{
  if (src.next)
  {
    next = new StateHistory(*(src.next));
  }
}

CheckSimpleEmptyCommons::StateHistory::~StateHistory() throw()
{
  delete next;
}

bool
CheckSimpleEmptyCommons::StateHistory::operator== (
  const StateHistory& src) const throw()
{
  //Ignoring any result equals -1
  if (result == -1 || src.result == -1)
  {
    bool wait = result == src.result && next != src.next;
    const StateHistory* next_to_check =
      (wait && next) || (!wait && result == -1) ? next : this;
    const StateHistory* src_next_to_check =
      (wait && src.next) || (!wait && src.result == -1) ? src.next : &src;

    return (!next_to_check && !src_next_to_check) ||
      (next_to_check && src_next_to_check &&
        *next_to_check == *src_next_to_check);
  }

  return result == src.result && state == src.state &&
    ((!next && !src.next) || (next && src.next && *next == *(src.next)));
}

//
// class CheckSimpleEmptyThread
//

CheckSimpleEmptyThread::CheckSimpleEmptyThread(
    unsigned short closure_delay) throw():
  PoolPolicySimpleEmptyThread(closure_delay),
  CheckSimpleEmptyCommons(closure_delay)
{
}

int
CheckSimpleEmptyThread::when_close_thread(Identifier thread) throw()
{
  int res = PoolPolicySimpleEmptyThread::when_close_thread(thread);
  StateInfo::States state = get_thread_state(thread);

  if (state != INT_MAX)
  {
    set_history_event_(thread, OT_THREAD, state, res);
  }

  return res;
}

HTTP::PoolPolicySimpleStatistics::StateInfo::States
CheckSimpleEmptyThread::get_thread_state(Identifier thread)
  throw(eh::Exception)
{
  const Threads& threads = get_threads_();
  Threads::const_iterator thr_it = threads.find(thread);
  if (thr_it == threads.end())
  {
    //Is not informative:
    //error("HTTP::CheckSimpleEmptyThread::get_thread_state(): "
    //  "got unexpected thread identifier");

    return static_cast<StateInfo::States>(INT_MAX);
  }
  else
  {
    return thr_it->second.state;
  }
}

CheckSimpleEmptyThread::~CheckSimpleEmptyThread() throw()
{
}

void
CheckSimpleEmptyThread::check_thread_connection_added(
  Identifier thread, Identifier /*connection*/) throw ()
{
  StateInfo::States state = get_thread_state(thread);
  if (state != INT_MAX)
  {
    set_history_event_(thread, OT_THREAD, state, -2);
  }
}

void
CheckSimpleEmptyThread::check_choose_thread(Identifier thread) throw ()
{
  StateInfo::States state = get_thread_state(thread);
  if (state != INT_MAX)
  {
    set_history_event_(thread, OT_THREAD, state, -2);
  }
}

void
CheckSimpleEmptyThread::check_thread_added(Identifier thread) throw ()
{
  StateInfo::States state = get_thread_state(thread);
  if (state != INT_MAX)
  {
    set_history_event_(thread, OT_THREAD, state, -2);
  }
}

void
CheckSimpleEmptyThread::check_thread_removed(Identifier thread) throw ()
{
  StateInfo::States state = get_thread_state(thread);
  if (state != INT_MAX)
  {
    set_history_event_(thread, OT_THREAD, state, -2);
  }
}

const CheckSimpleEmptyCommons::CompletedHistories&
CheckSimpleEmptyThread::get_thr_history() throw()
{
  return completed_histories_;
}

//
// class CheckSimpleEmptyConnection
//

CheckSimpleEmptyConnection::CheckSimpleEmptyConnection(
    unsigned short closure_delay) throw():
  HTTP::PoolPolicySimpleEmptyConnection(closure_delay),
  CheckSimpleEmptyCommons(closure_delay)
{
}

int
CheckSimpleEmptyConnection::when_close_connection(Identifier connection) throw()
{
  int res = PoolPolicySimpleEmptyConnection::when_close_connection(connection);
  StateInfo::States state = get_connection_state(connection);

  if (state != INT_MAX)
  {
    set_history_event_(connection, OT_CONNECTION, state, res);
  }

  return res;
}

HTTP::PoolPolicySimpleStatistics::StateInfo::States
CheckSimpleEmptyConnection::get_connection_state(Identifier connection)
  throw(eh::Exception)
{
  const Connections& connections = get_connections_();
  Connections::const_iterator conn_it = connections.find(connection);
  if (conn_it == connections.end())
  {
    //Is not informative:
    //error("HTTP::CheckSimpleEmptyConnection::when_close_connection(): "
    //  "got unexpected connection identifier");

    return static_cast<StateInfo::States>(INT_MAX);
  }
  else
  {
    return conn_it->second.state;
  }
}

CheckSimpleEmptyConnection::~CheckSimpleEmptyConnection() throw()
{
}

void
CheckSimpleEmptyConnection::check_connection_request_added(
  Identifier connection, Identifier /*request*/) throw ()
{
  StateInfo::States state = get_connection_state(connection);
  if (state != INT_MAX)
  {
    set_history_event_(connection, OT_CONNECTION, state, -2);
  }
}

void
CheckSimpleEmptyConnection::check_choose_connection(Identifier connection,
  Identifier /*server*/, Identifier /*request*/) throw ()
{
  StateInfo::States state = get_connection_state(connection);
  if (state != INT_MAX)
  {
    set_history_event_(connection, OT_CONNECTION, state, -2);
  }
}

void
CheckSimpleEmptyConnection::check_server_connection_added(Identifier /*server*/,
  Identifier connection) throw ()
{
  StateInfo::States state = get_connection_state(connection);
  if (state != INT_MAX)
  {
    set_history_event_(connection, OT_CONNECTION, state, -2);
  }
}

void
CheckSimpleEmptyConnection::check_server_connection_removed(Identifier /*server*/,
  Identifier connection) throw ()
{
  StateInfo::States state = get_connection_state(connection);
  if (state != INT_MAX)
  {
    set_history_event_(connection, OT_CONNECTION, state, -2);
  }
}

const CheckSimpleEmptyCommons::CompletedHistories&
CheckSimpleEmptyConnection::get_conn_history() throw()
{
  return completed_histories_;
}

//
// class CheckSimpleDecider
//

CheckSimpleDecider::CheckSimpleDecider(int connections_per_server,
    int connections_per_threads, CheckSimpleEmptyConnection& conn_policy,
    CheckSimpleEmptyThread& thr_policy) throw (eh::Exception):
  PoolPolicySimpleDecider(connections_per_server, connections_per_threads),
  conn_policy_(conn_policy),
  thr_policy_(thr_policy)
{
}

inline
HTTP::PoolPolicyCommon::Identifier
CheckSimpleDecider::choose_thread() throw ()
{
  Identifier thread = PoolPolicySimpleDecider::choose_thread();
  thr_policy_.check_choose_thread(thread);
  return thread;
}

inline
HTTP::PoolPolicyCommon::Identifier
CheckSimpleDecider::choose_connection(
  Identifier server, Identifier request) throw ()
{
  Identifier conn = PoolPolicySimpleDecider::choose_connection(server, request);
  conn_policy_.check_choose_connection(conn, server, request);
  return conn;
}

inline
void
CheckSimpleDecider::connection_request_added(Identifier server,
  Identifier connection, Identifier request) throw ()
{
  PoolPolicySimpleStatistics::connection_request_added(server, connection, request);
  conn_policy_.check_connection_request_added(connection, request);
}

inline
void
CheckSimpleDecider::thread_connection_added(
  Identifier thread, Identifier connection) throw ()
{
  PoolPolicySimpleStatistics::thread_connection_added(thread, connection);
  thr_policy_.check_thread_connection_added(thread, connection);
}

inline
void
CheckSimpleDecider::server_connection_added(
  Identifier server, Identifier connection) throw ()
{
  PoolPolicySimpleStatistics::server_connection_added(server, connection);
  conn_policy_.check_server_connection_added(server, connection);
}

inline
void
CheckSimpleDecider::server_connection_removed(
  Identifier server, Identifier connection) throw ()
{
  conn_policy_.check_server_connection_removed(server, connection);
  PoolPolicySimpleStatistics::server_connection_removed(server, connection);
}

inline
void
CheckSimpleDecider::thread_added(Identifier thread) throw ()
{
  PoolPolicySimpleStatistics::thread_added(thread);
  thr_policy_.check_thread_added(thread);
}

inline
void
CheckSimpleDecider::thread_removed(Identifier thread) throw ()
{
  thr_policy_.check_thread_removed(thread);
  PoolPolicySimpleStatistics::thread_removed(thread);
}

//
// class ConnThrScenarios
//

ConnThrScenarios::ConnThrScenarios() throw(eh::Exception)
{
}

inline
void
ConnThrScenarios::add_scenario(const Scenario& new_scen) throw(eh::Exception)
{
  if (new_scen.object_type == CheckSimpleEmptyCommons::OT_CONNECTION)
  {
    conn_scenarios_.push_back(new_scen);
    conn_scens_completed_.push_back(0);
  }
  else
  {
    thr_scenarios_.push_back(new_scen);
    thr_scens_completed_.push_back(0);
  }
}

void
ConnThrScenarios::add_scenario(CheckSimpleEmptyCommons::ObjectType type,
  const ScenarioArrayElem* new_scen, size_t length) throw(eh::Exception)
{
  if (length < 1)
  {
    return;
  }

  std::unique_ptr<Scenario> scen( 
    new Scenario(type, new_scen[0].first, new_scen[0].second, Generics::Time(0)));

  Scenario* scen_ptr = scen.get();
  for (size_t ind = 1; ind < length; ++ind)
  {
    scen_ptr->next = new Scenario(
      type, new_scen[ind].first, new_scen[ind].second, Generics::Time(0));
    scen_ptr = scen_ptr->next;
  }

  add_scenario(*scen);
}

bool
ConnThrScenarios::check_conn_scenario(const Scenario& new_scen) throw(eh::Exception)
{
  size_t end = conn_scenarios_.size();
  for (size_t ind = 0; ind < end; ++ind)
  {
    if (conn_scenarios_[ind] == new_scen)
    {
      ++conn_scens_completed_[ind];
      return true;
    }
  }

  return false;
}

bool
ConnThrScenarios::check_thr_scenario(const Scenario& new_scen) throw(eh::Exception)
{
  size_t end = thr_scenarios_.size();
  for (size_t ind = 0; ind < end; ++ind)
  {
    if (thr_scenarios_[ind] == new_scen)
    {
      ++thr_scens_completed_[ind];
      return true;
    }
  }

  return false;
}

const ConnThrScenarios::ScenariosCompleted&
ConnThrScenarios::conn_scens_completed() throw()
{
  return conn_scens_completed_;
}

const ConnThrScenarios::ScenariosCompleted&
ConnThrScenarios::thr_scens_completed() throw()
{
  return thr_scens_completed_;
}

bool
ConnThrScenarios::all_completed(std::ostringstream& log) throw(eh::Exception)
{
  int uncompleted = 0;
  size_t ind = 0;
  size_t end = 0;
  for (ind = 0, end = conn_scens_completed_.size(); ind < end; ++ind)
  {
    if (!conn_scens_completed_[ind])
    {
      ++uncompleted;
      log << uncompleted << " missing scenario (connection):\n";
      print_scenario(log, &conn_scenarios_[ind]);
    }
  }

  for (ind = 0, end = thr_scens_completed_.size(); ind < end; ++ind)
  {
    if (!thr_scens_completed_[ind])
    {
      ++uncompleted;
      log << uncompleted << " missing scenario (thread):\n";
      print_scenario(log, &thr_scenarios_[ind]);
    }
  }

  return uncompleted == 0;
}

void
ConnThrScenarios::print_scenario(std::ostream& log, const Scenario* scen)
  throw(eh::Exception)
{
  do
  {
    CheckSimpleEmptyCommons::print_state_history("  ", 0, *scen, log);
    scen = scen->next;
  }
  while (scen);
}
