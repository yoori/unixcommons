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



#ifndef _EMPTY_POLICIES_INTERNAL_TEST_COMMON_CLASSES_HPP_
#define _EMPTY_POLICIES_INTERNAL_TEST_COMMON_CLASSES_HPP_

#include <HTTP/HttpTestCommons/CommonClasses.hpp>
#include <Generics/Time.hpp>
#include <vector>
#include <string>
#include <climits>

//
// class PoliciesTestInterface
//

class PoliciesTestInterface: public TestInterface
{
public:

  PoliciesTestInterface(Sync::Semaphore& finish_sem) throw(eh::Exception);

  virtual void print_stats(std::ostream& out) throw(eh::Exception) = 0;

  virtual void print_errors(std::ostream& out) throw(eh::Exception) = 0;

  virtual void execute() throw();

protected:

  virtual
  ~PoliciesTestInterface() throw ();

  virtual void exec_init_() throw() = 0;

  virtual void exec_main_() throw() = 0;

  virtual void exec_finish_() throw() = 0;

  Sync::Semaphore& finish_sem_;
};

typedef ReferenceCounting::QualPtr<PoliciesTestInterface> PoliciesTestInterface_var;

//
// Check policies classes
//

class CheckSimpleStatistics;
class CheckSimpleDecider;

class CheckSimpleEmptyCommons: public virtual HTTP::PoolPolicySimpleStatistics
{
public:

  typedef HTTP::PoolPolicySimpleStatistics::StateInfo StateInfo;

  enum ObjectType{
    OT_CONNECTION,
    OT_THREAD
  };

  struct StateHistory
  {
    ObjectType object_type;
    int result;
    StateInfo::States state;
    Generics::Time time;
    StateHistory* next;

    StateHistory(ObjectType new_type, int new_result,
      StateInfo::States new_state, const Generics::Time& new_time) throw();
    StateHistory(const StateHistory& src) throw(eh::Exception);

    ~StateHistory() throw();

    bool operator== (const StateHistory& src) const throw();
  };

  typedef std::map<Identifier, StateHistory> Histories;
  typedef std::list<std::pair<Identifier, StateHistory> > CompletedHistories;


  CheckSimpleEmptyCommons(unsigned int closure_delay_value) throw();

  static void print_state_history(const char* prefix, const void* addr,
    const StateHistory& obj, std::ostream& out) throw(eh::Exception);

protected:

  Histories histories_;
  CompletedHistories completed_histories_;
  const StateHistory* cur_history_;
  int closure_delay_value_;
  Sync::PosixMutex mutex_;
  static Sync::PosixMutex dump_mutex_;


  virtual ~CheckSimpleEmptyCommons() throw();

  virtual void dynamic_states_checker_(const char* prefix, const void* addr,
    const StateHistory* prev_n_now, std::ostream& error) throw (eh::Exception);

  //Is not protected by mutex!
  void set_history_event_(Identifier id, ObjectType type,
    StateInfo::States state, int result) throw (eh::Exception);

  //Is not protected by mutex!
  void remove_history_(Identifier id) throw (eh::Exception);
};

class CheckSimpleEmptyThread: public HTTP::PoolPolicySimpleEmptyThread,
                              public CheckSimpleEmptyCommons
{
public:

  CheckSimpleEmptyThread(unsigned short closure_delay = 0) throw();

  //Is not protected by mutex!
  virtual int
  when_close_thread(Identifier thread) throw();

  //Is not protected by mutex!
  const CompletedHistories&
  get_thr_history() throw();

protected:

  virtual ~CheckSimpleEmptyThread() throw();

  //Is not protected by mutex!
  StateInfo::States
  get_thread_state(Identifier thread) throw(eh::Exception);

  //Is not protected by mutex!
  virtual void
  check_thread_connection_added(Identifier thread, Identifier connection)
    throw ();

  //Is not protected by mutex!
  virtual void
  check_choose_thread(Identifier thread) throw ();

  //Is not protected by mutex!
  virtual void
  check_thread_added(Identifier thread) throw ();

  //Is not protected by mutex!
  virtual void
  check_thread_removed(Identifier thread) throw ();


  friend class CheckSimpleStatistics;
  friend class CheckSimpleDecider;
};

class CheckSimpleEmptyConnection: public HTTP::PoolPolicySimpleEmptyConnection,
                                  public CheckSimpleEmptyCommons
{
public:

  CheckSimpleEmptyConnection(unsigned short closure_delay = 0) throw();

  //Is not protected by mutex!
  virtual int
  when_close_connection(Identifier connection) throw();

  //Is not protected by mutex!
  const CompletedHistories&
  get_conn_history() throw();

protected:

  virtual ~CheckSimpleEmptyConnection() throw();
  
  StateInfo::States
  get_connection_state(Identifier connection) throw(eh::Exception);

  //Is not protected by mutex!
  virtual void
  check_connection_request_added(Identifier connection, Identifier request)
    throw ();

  //Is not protected by mutex!
  virtual void
  check_choose_connection(Identifier connection, Identifier server,
    Identifier request) throw ();

  //Is not protected by mutex!
  virtual void
  check_server_connection_added(Identifier server, Identifier connection)
    throw ();

  //Is not protected by mutex!
  virtual void
  check_server_connection_removed(Identifier server, Identifier connection)
    throw ();


  friend class CheckSimpleStatistics;
  friend class CheckSimpleDecider;
};

class CheckSimpleDecider: public HTTP::PoolPolicySimpleDecider
{
public:

  CheckSimpleDecider(int connections_per_server, int connections_per_threads,
    CheckSimpleEmptyConnection& conn_policy, CheckSimpleEmptyThread& thr_policy)
    throw (eh::Exception);

  virtual Identifier
  choose_thread() throw ();

  virtual Identifier
  choose_connection(Identifier server, Identifier request) throw ();

  virtual void
  connection_request_added(Identifier server, Identifier connection,
    Identifier request) throw ();

  virtual void
  thread_connection_added(Identifier thread, Identifier connection)
    throw ();

  virtual void
  server_connection_added(Identifier server, Identifier connection)
    throw ();

  virtual void
  server_connection_removed(Identifier server, Identifier connection)
    throw ();

  virtual void
  thread_added(Identifier thread) throw ();

  virtual void
  thread_removed(Identifier thread) throw ();

private:

  CheckSimpleEmptyConnection& conn_policy_;
  CheckSimpleEmptyThread& thr_policy_;
};

//
// class ConnThrScenarios
//

class ConnThrScenarios
{
public:

  typedef CheckSimpleEmptyCommons::StateHistory Scenario;
  typedef std::pair<int, CheckSimpleEmptyCommons::StateInfo::States> ScenarioArrayElem;
  typedef std::vector<Scenario> Scenarios;
  typedef std::vector<char> ScenariosCompleted;

  ConnThrScenarios() throw(eh::Exception);

  void add_scenario(const Scenario& new_scen) throw(eh::Exception);

  void add_scenario(CheckSimpleEmptyCommons::ObjectType type,
    const ScenarioArrayElem* new_scen, size_t length) throw(eh::Exception);

  bool check_conn_scenario(const Scenario& new_scen) throw(eh::Exception);

  bool check_thr_scenario(const Scenario& new_scen) throw(eh::Exception);

  const ScenariosCompleted& conn_scens_completed() throw();

  const ScenariosCompleted& thr_scens_completed() throw();

  bool all_completed(std::ostringstream& log) throw(eh::Exception);

  void print_scenario(std::ostream& log, const Scenario* scen)
    throw(eh::Exception);

private:

  Scenarios conn_scenarios_;
  Scenarios thr_scenarios_;
  ScenariosCompleted thr_scens_completed_;
  ScenariosCompleted conn_scens_completed_;
};

#endif
