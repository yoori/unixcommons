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



#include <HTTP/HttpAsyncPolicies.hpp>


namespace HTTP
{
  //
  // class EmptyPoliciesCommonMethods
  //

  class EmptyPoliciesCommonMethods
  {
  public:
    template <class Policy, class EntitiesMap, class AuxiliaryMap>
    static
    int
    when_close_entity(typename EntitiesMap::const_iterator& cur_obj_it,
      const AuxiliaryMap& map, Policy& policy) throw ();

    template <class Policy, class EntitiesMap, class AuxiliaryMap>
    static
    int
    process_entity_active_state(
      typename EntitiesMap::const_iterator& cur_obj_it,
      const AuxiliaryMap& map, Policy& policy) throw ();

    template <class Policy, class EntitiesMap, class AuxiliaryMap>
    static
    int
    process_entity_closure_awaiting_state(
      typename EntitiesMap::const_iterator& cur_obj_it,
      const AuxiliaryMap& map, Policy& policy) throw ();

    static
    PoolPolicySimpleEmptyThread::Threads::const_iterator
    convert(PoolPolicySimpleEmptyThread::Threads::const_iterator& src)
      throw ();

    static
    PoolPolicySimpleEmptyConnection::Connections::const_iterator
    convert(
      PoolPolicySimpleEmptyConnection::ConnectionPtrs::const_iterator& src)
      throw ();

    static
    bool
    additional_closing_validator(
      PoolPolicySimpleEmptyThread::Threads::const_iterator& it) throw ();

    static
    bool
    additional_closing_validator(
      PoolPolicySimpleEmptyConnection::Connections::const_iterator& it)
      throw ();
  };

  template <class Policy, class EntitiesMap, class AuxiliaryMap>
  int
  EmptyPoliciesCommonMethods::when_close_entity(
    typename EntitiesMap::const_iterator& cur_obj_it,
    const AuxiliaryMap& map, Policy& policy) throw ()
  {
    int res = -1;

    switch (cur_obj_it->second.state)
    {
    case Policy::StateInfo::ACTIVE_AWAITING:
      break;

    case Policy::StateInfo::CLOSING:
      {
        Stream::Error ostr;
        ostr << FNS << "got an invalid entity state: CLOSING";
        policy.error(ostr.str());
      }
      break;

    case Policy::StateInfo::CLOSURE_ON_NEXT_TRY:
      cur_obj_it->second.state = Policy::StateInfo::CLOSING;
      res = 0;
      break;

    case Policy::StateInfo::ACTIVE:
      res = policy.process_active_(cur_obj_it, map);
      break;

    case Policy::StateInfo::CLOSURE_AWAITING:
      res = policy.process_closure_awaiting_(cur_obj_it, map);
      break;

    default:
      Stream::Error ostr;
      ostr << FNS << "got an unexpected entity state";
      policy.error(ostr.str());
      break;
    }

    return res;
  }

  PoolPolicySimpleEmptyThread::Threads::const_iterator
  EmptyPoliciesCommonMethods::convert(
    PoolPolicySimpleEmptyThread::Threads::const_iterator& src)
    throw ()
  {
    return src;
  }

  PoolPolicySimpleEmptyConnection::Connections::const_iterator
  EmptyPoliciesCommonMethods::convert(
    PoolPolicySimpleEmptyConnection::ConnectionPtrs::const_iterator& src)
    throw ()
  {
    return src->second;
  }

  bool
  EmptyPoliciesCommonMethods::additional_closing_validator(
    PoolPolicySimpleEmptyThread::Threads::const_iterator& it) throw ()
  {
    return !it->second.full;
  }

  bool
  EmptyPoliciesCommonMethods::additional_closing_validator(
    PoolPolicySimpleEmptyConnection::Connections::const_iterator& /*it*/)
    throw ()
  {
    return true;
  }

  template <class Policy, class EntitiesMap, class AuxiliaryMap>
  int
  EmptyPoliciesCommonMethods::process_entity_active_state(
    typename EntitiesMap::const_iterator& cur_obj_it,
    const AuxiliaryMap& map, Policy& policy) throw ()
  {
    if (cur_obj_it->second.items_count)
    {
      return -1;
    }

    bool additional_condition = false;
    bool closure_awaiting_exists = false;

    typename AuxiliaryMap::const_iterator it = map.begin();
    typename AuxiliaryMap::const_iterator end = map.end();
    for (; it != end; ++it)
    {
      typename EntitiesMap::const_iterator current =
        EmptyPoliciesCommonMethods::convert(it);

      if (current->first == cur_obj_it->first)
      {
        continue;
      }

      if (EmptyPoliciesCommonMethods::additional_closing_validator(current))
      {
        additional_condition = true;
        if (closure_awaiting_exists)
        {
          break;
        }
      }

      if (current->second.state == Policy::StateInfo::CLOSURE_AWAITING)
      {
        closure_awaiting_exists = true;
        if (additional_condition)
        {
          break;
        }
      }
    }

    if ((additional_condition && closure_awaiting_exists) ||
      !policy.CLOSURE_DELAY_)
    {
      cur_obj_it->second.state = Policy::StateInfo::CLOSING;
      return 0;
    }
    else
    {
      cur_obj_it->second.state = Policy::StateInfo::CLOSURE_AWAITING;
      return policy.CLOSURE_DELAY_;
    }
  }

  template <class Policy, class EntitiesMap, class AuxiliaryMap>
  int
  EmptyPoliciesCommonMethods::process_entity_closure_awaiting_state(
    typename EntitiesMap::const_iterator& cur_obj_it,
    const AuxiliaryMap& map, Policy& policy) throw ()
  {
    typename AuxiliaryMap::const_iterator it = map.begin();
    typename AuxiliaryMap::const_iterator end = map.end();
    for (; it != end; ++it)
    {
      typename EntitiesMap::const_iterator current =
        EmptyPoliciesCommonMethods::convert(it);

      if (current->second.state == Policy::StateInfo::ACTIVE ||
        current->second.state == Policy::StateInfo::ACTIVE_AWAITING)
      {
        break;
      }
    }

    if (it == end)
    {
      cur_obj_it->second.state = Policy::StateInfo::CLOSURE_ON_NEXT_TRY;
    }

    return policy.CLOSURE_DELAY_;
  }


  //
  // PoolPolicyCommon class
  //

  const PoolPolicyCommon::Identifier
    PoolPolicyCommon::SPECIAL_IDENTIFIER = 0;

  PoolPolicyCommon::~PoolPolicyCommon() throw ()
  {
  }


  //
  // PoolPolicyStatistics class
  //

  PoolPolicyStatistics::~PoolPolicyStatistics() throw ()
  {
  }


  //
  // PoolPolicyDecider class
  //

  PoolPolicyDecider::~PoolPolicyDecider() throw ()
  {
  }


  //
  // PoolPolicyEmptyThread class
  //

  PoolPolicyEmptyThread::~PoolPolicyEmptyThread() throw ()
  {
  }


  //
  // PoolPolicyEmptyConnection class
  //

  PoolPolicyEmptyConnection::~PoolPolicyEmptyConnection() throw ()
  {
  }


  //
  // PoolPolicyRequests class
  //

  PoolPolicyRequests::~PoolPolicyRequests() throw ()
  {
  }


  //
  // PoolPolicyTimeout class
  //

  PoolPolicyTimeout::~PoolPolicyTimeout() throw ()
  {
  }


  //
  // PoolPolicySimpleStatistics::StateInfo class
  //

  inline
  PoolPolicySimpleStatistics::StateInfo::StateInfo() throw ()
    : state(ACTIVE_AWAITING)
  {
  }

  //
  // PoolPolicySimpleStatistics::StateInfo class
  //

  inline
  PoolPolicySimpleStatistics::SimpleStat::SimpleStat() throw ()
    : items_count(0)
  {
  }


  //
  // PoolPolicySimpleStatistics::Connection class
  //

  PoolPolicySimpleStatistics::Connection::Connection() throw ()
    : server(0), thread(0)
  {
  }

  PoolPolicySimpleStatistics::Connection::Connection(Identifier server)
    throw ()
    : server(server), thread(0)
  {
  }

  //
  // PoolPolicySimpleStatistics::Thread class
  //

  PoolPolicySimpleStatistics::Thread::Thread() throw ()
    : full(false)
  {
  }

  //
  // PoolPolicySimpleStatistics class
  //

  PoolPolicySimpleStatistics::~PoolPolicySimpleStatistics() throw ()
  {
    assert(servers_.empty());
    assert(threads_.empty());
    assert(connections_.empty());
  }

  void
  PoolPolicySimpleStatistics::server_added(Identifier server) throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      server_added_i(server);
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
  }

  void
  PoolPolicySimpleStatistics::server_added_i(Identifier server)
    throw (eh::Exception)
  {
    servers_.insert(Servers::value_type(server, ConnectionPtrs()));
  }

  void
  PoolPolicySimpleStatistics::server_removed(Identifier server) throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      server_removed_i(server);
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
  }

  void
  PoolPolicySimpleStatistics::server_removed_i(Identifier server)
    throw (eh::Exception)
  {
    Servers::iterator itor(servers_.find(server));
    if (itor != servers_.end())
    {
      servers_.erase(itor);
    }
  }

  void
  PoolPolicySimpleStatistics::server_connection_added(
    Identifier server, Identifier connection) throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      server_connection_added_i(server, connection);
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
  }

  void
  PoolPolicySimpleStatistics::server_connection_added_i(
    Identifier server, Identifier connection) throw (eh::Exception)
  {
    Connections::iterator conn_it = connections_.insert(
      Connections::value_type(connection, Connection(server))).first;
    Servers::iterator itor(servers_.find(server));
    if (itor != servers_.end())
    {
      itor->second.insert(
        ConnectionPtrs::value_type(connection, conn_it));
    }
  }

  void
  PoolPolicySimpleStatistics::server_connection_removed(
    Identifier server, Identifier connection) throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      server_connection_removed_i(server, connection);
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
  }

  void
  PoolPolicySimpleStatistics::server_connection_removed_i(
    Identifier server, Identifier connection) throw (eh::Exception)
  {
    Connections::iterator it(connections_.find(connection));
    if (it != connections_.end())
    {
      assert(it->second.server == server);

      Servers::iterator itor(servers_.find(server));
      if (itor != servers_.end())
      {
        ConnectionPtrs::iterator conn_id = itor->second.find(connection);
        if (conn_id != itor->second.end())
        {
          itor->second.erase(conn_id);
        }
        else
        {
          Stream::Error ostr;
          ostr << FNS << "got unexpected connection identifier";
          error(ostr.str());
        }
      }

      if (it->second.thread)
      {
        it->second.server = 0;
      }
      else
      {
        connections_.erase(it);
      }
    }
  }

  void
  PoolPolicySimpleStatistics::thread_added(Identifier thread) throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      threads_.insert(Threads::value_type(thread, Thread()));
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
  }

  void
  PoolPolicySimpleStatistics::thread_removed(Identifier thread) throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      thread_removed_i(thread);
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
  }

  void
  PoolPolicySimpleStatistics::thread_removed_i(Identifier thread)
    throw (eh::Exception)
  {
    Threads::iterator itor(threads_.find(thread));
    if (itor != threads_.end())
    {
      threads_.erase(itor);
    }
  }

  void
  PoolPolicySimpleStatistics::thread_connection_added(
    Identifier thread, Identifier connection) throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      thread_connection_added_i(thread, connection);
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
  }

  void
  PoolPolicySimpleStatistics::thread_connection_added_i(
    Identifier thread, Identifier connection) throw (eh::Exception)
  {
    Connections::iterator it(connections_.find(connection));
    if (it != connections_.end())
    {
      assert(it->second.server);
      assert(!it->second.thread);
      it->second.thread = thread;
    }

    Threads::iterator itor(threads_.find(thread));
    if (itor != threads_.end())
    {
      itor->second.items_count++;
      itor->second.state = StateInfo::ACTIVE;
    }
  }

  void
  PoolPolicySimpleStatistics::thread_connection_removed(
    Identifier thread, Identifier connection) throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      thread_connection_removed_i(thread, connection);
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
  }

  void
  PoolPolicySimpleStatistics::thread_connection_removed_i(
    Identifier thread, Identifier connection) throw (eh::Exception)
  {
    Connections::iterator it(connections_.find(connection));
    if (it != connections_.end())
    {
      assert(it->second.thread == thread);
      if (it->second.server)
      {
        it->second.thread = 0;
      }
      else
      {
        connections_.erase(it);
      }
    }

    Threads::iterator itor(threads_.find(thread));
    if (itor != threads_.end())
    {
      assert(itor->second.items_count > 0);
      itor->second.items_count--;
      itor->second.full = false;
    }
  }

  void
  PoolPolicySimpleStatistics::connection_request_added(
    Identifier server, Identifier connection, Identifier request) throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      connection_request_added_i(server, connection, request);
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
  }

  void
  PoolPolicySimpleStatistics::connection_request_added_i(
    Identifier /*server*/, Identifier connection, Identifier /*request*/)
    throw (eh::Exception)
  {
    Connections::iterator it(connections_.find(connection));
    if (it != connections_.end())
    {
      it->second.items_count++;
      it->second.state = StateInfo::ACTIVE;
    }
    else
    {
      Stream::Error ostr;
      ostr << FNS << "got unexpected connection identifier";
      error(ostr.str());
    }
  }

  void
  PoolPolicySimpleStatistics::connection_request_removed(
    Identifier connection, Identifier request) throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      connection_request_removed_i(connection, request);
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
  }

  void
  PoolPolicySimpleStatistics::connection_request_removed_i(
    Identifier connection, Identifier /*request*/) throw (eh::Exception)
  {
    Connections::iterator it(connections_.find(connection));
    if (it != connections_.end())
    {
      assert(it->second.items_count > 0);
      it->second.items_count--;
    }
  }

  void
  PoolPolicySimpleStatistics::server_request_added(
    Identifier /*server*/, Identifier /*request*/) throw ()
  {
  }

  void
  PoolPolicySimpleStatistics::server_request_removed(
    Identifier /*server*/, Identifier /*request*/) throw ()
  {
  }

  const PoolPolicySimpleStatistics::Servers&
  PoolPolicySimpleStatistics::get_servers_() const throw ()
  {
    return servers_;
  }

  const PoolPolicySimpleStatistics::Threads&
  PoolPolicySimpleStatistics::get_threads_() const throw ()
  {
    return threads_;
  }

  const PoolPolicySimpleStatistics::Connections&
  PoolPolicySimpleStatistics::get_connections_() const throw ()
  {
    return connections_;
  }


  //
  // class PoolPolicyAdvancedStatistics
  //

  void
  PoolPolicyAdvancedStatistics::server_request_added(
    Identifier server, Identifier request) throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      ServerRequests::iterator serv_it(server_requests_.find(server));
      if (serv_it != server_requests_.end())
      {
        serv_it->second.insert(Requests::value_type(request, -1));
      }
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
  }

  void
  PoolPolicyAdvancedStatistics::server_request_removed(
    Identifier server, Identifier request) throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      ServerRequests::iterator serv_it(server_requests_.find(server));
      if (serv_it != server_requests_.end())
      {
        Requests::iterator req_it = serv_it->second.find(request);
        if (req_it != serv_it->second.end())
        {
          serv_it->second.erase(req_it);
        }
        else
        {
          Stream::Error ostr;
          ostr << FNS << "got unexpected request identifier";
          error(ostr.str());
        }
      }
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
  }

  void
  PoolPolicyAdvancedStatistics::server_added_i(Identifier server)
    throw (eh::Exception)
  {
    PoolPolicySimpleStatistics::server_added_i(server);

    server_requests_.insert(ServerRequests::value_type(server, Requests()));
  }

  void
  PoolPolicyAdvancedStatistics::server_removed_i(Identifier server)
    throw (eh::Exception)
  {
    PoolPolicySimpleStatistics::server_removed_i(server);

    ServerRequests::iterator itor(server_requests_.find(server));
    if (itor != server_requests_.end())
    {
      server_requests_.erase(itor);
    }
  }

  void
  PoolPolicyAdvancedStatistics::connection_request_added_i(
    Identifier server, Identifier connection, Identifier request)
    throw (eh::Exception)
  {
    PoolPolicySimpleStatistics::connection_request_added_i(
      server, connection, request);

    ServerRequests::iterator serv_it(server_requests_.find(server));
    if (serv_it != server_requests_.end())
    {
      Requests::iterator req_it = serv_it->second.find(request);
      if (req_it != serv_it->second.end())
      {
        ++req_it->second;
      }
      else
      {
        Stream::Error ostr;
        ostr << FNS << "got unexpected request identifier";
        error(ostr.str());
      }
    }
    else
    {
      Stream::Error ostr;
      ostr << FNS << "got unexpected request identifier";
      error(ostr.str());
    }
  }

  PoolPolicyAdvancedStatistics::~PoolPolicyAdvancedStatistics() throw ()
  {
    assert(server_requests_.empty());
  }


  //
  // PoolPolicySimpleDecider class
  //

  PoolPolicySimpleDecider::PoolPolicySimpleDecider(
    unsigned connections_per_server, unsigned connections_per_threads)
    throw (eh::Exception)
    : CONNECTIONS_PER_SERVER_(connections_per_server),
      CONNECTIONS_PER_THREADS_(connections_per_threads)
  {
  }

  PoolPolicySimpleDecider::Identifier
  PoolPolicySimpleDecider::choose_thread() throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      const Threads& threads = get_threads_();
      for (Threads::const_iterator itor(threads.begin());
        itor != threads.end(); ++itor)
      {
        if (itor->second.state == StateInfo::CLOSING)
        {
          continue;
        }

        if (itor->second.items_count < CONNECTIONS_PER_THREADS_)
        {
          itor->second.state = StateInfo::ACTIVE_AWAITING;
          return itor->first;
        }
        else
        {
          itor->second.full = true;
        }
      }
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
    return SPECIAL_IDENTIFIER;
  }

  PoolPolicySimpleDecider::Identifier
  PoolPolicySimpleDecider::choose_connection(
    Identifier server, Identifier /*request*/) throw ()
  {
    try
    {
      Sync::PosixGuard guard(mutex_);

      unsigned requests = 0;

      const Servers& servers = get_servers_();
      Servers::const_iterator itor(servers.find(server));

      if (itor != servers.end())
      {
        ConnectionPtrs::const_iterator conn_id = itor->second.begin();
        ConnectionPtrs::const_iterator end = itor->second.end();
        ConnectionPtrs::const_iterator connection_it = end;
        for (; conn_id != end; ++conn_id)
        {
          if (conn_id->second->second.state != StateInfo::CLOSING)
          {
            if (!conn_id->second->second.items_count)
            {
              connection_it = conn_id;
              break;
            }

            if (!requests || conn_id->second->second.items_count < requests)
            {
              requests = conn_id->second->second.items_count;
              connection_it = conn_id;
            }
          }
        }

        if (connection_it != end && (
            itor->second.size() >= CONNECTIONS_PER_SERVER_
            || !connection_it->second->second.items_count))
        {
          connection_it->second->second.state = StateInfo::ACTIVE_AWAITING;
          return connection_it->second->first;
        }
      }

      if (itor == servers.end())
      {
        Stream::Error ostr;
        ostr << FNS << "got unexpected server identifier";
        error(ostr.str());
      }
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }
    return SPECIAL_IDENTIFIER;
  }

  PoolPolicySimpleDecider::RequestPolicy
  PoolPolicySimpleDecider::request_failed(Identifier /*server*/,
    Identifier /*request*/) throw ()
  {
    return RP_CANCEL_ALL;
  }

  PoolPolicySimpleDecider::RequestPolicy
  PoolPolicySimpleDecider::requests_failed(Identifier /*server*/)
    throw ()
  {
    return RP_CANCEL_FIRST_RESEND_OTHERS;
  }


  //
  // class PoolPolicySimpleEmptyThread
  //

  PoolPolicySimpleEmptyThread::PoolPolicySimpleEmptyThread(
    time_t closure_delay) throw ()
    : CLOSURE_DELAY_(closure_delay)
  {
  }

  int
  PoolPolicySimpleEmptyThread::when_close_thread(Identifier thread) throw ()
  {
    int res = -1;
    try
    {
      Sync::PosixGuard guard(mutex_);

      const Threads& threads = get_threads_();
      Threads::const_iterator thr_it = threads.find(thread);
      if (thr_it != threads.end())
      {
        res = EmptyPoliciesCommonMethods::when_close_entity
          <PoolPolicySimpleEmptyThread, Threads, Threads>(
            thr_it, threads, *this);
      }
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }

    return res;
  }

  int
  PoolPolicySimpleEmptyThread::process_active_(
    Threads::const_iterator& cur_thr, const Threads&) throw (eh::Exception)
  {
    return EmptyPoliciesCommonMethods::process_entity_active_state
      <PoolPolicySimpleEmptyThread, Threads, Threads>(
        cur_thr, get_threads_(), *this);
  }

  int
  PoolPolicySimpleEmptyThread::process_closure_awaiting_(
    Threads::const_iterator& cur_thr, const Threads&) throw (eh::Exception)
  {
    return EmptyPoliciesCommonMethods::process_entity_closure_awaiting_state
      <PoolPolicySimpleEmptyThread, Threads, Threads>(
        cur_thr, get_threads_(), *this);
  }

  PoolPolicySimpleEmptyThread::~PoolPolicySimpleEmptyThread() throw ()
  {
  }


  //
  // class PoolPolicySimpleEmptyConnection
  //

  PoolPolicySimpleEmptyConnection::PoolPolicySimpleEmptyConnection(
    time_t closure_delay) throw ()
    : CLOSURE_DELAY_(closure_delay)
  {
  }

  int
  PoolPolicySimpleEmptyConnection::when_close_connection(
    Identifier connection) throw ()
  {
    int res = -1;
    try
    {
      Sync::PosixGuard guard(mutex_);

      const Connections& connections = get_connections_();
      Connections::const_iterator conn_it = connections.find(connection);
      if (conn_it == connections.end())
      {
        Stream::Error ostr;
        ostr << FNS << "got unexpected connection identifier";
        error(ostr.str());
      }
      else
      {
        const Servers& servs = get_servers_();
        Servers::const_iterator serv_it = servs.find(conn_it->second.server);
        if (serv_it == servs.end())
        {
          Stream::Error ostr;
          ostr << FNS << "got unexpected server identifier";
          error(ostr.str());
        }
        else
        {
          res = EmptyPoliciesCommonMethods::when_close_entity
            <PoolPolicySimpleEmptyConnection, Connections, ConnectionPtrs>(
              conn_it, serv_it->second, *this);
        }
      }
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "failed";
      error(ostr.str());
    }

    return res;
  }

  int
  PoolPolicySimpleEmptyConnection::process_active_(
    Connections::const_iterator& conn_it, const ConnectionPtrs& aux_map)
    throw (eh::Exception)
  {
    return EmptyPoliciesCommonMethods::process_entity_active_state
      <PoolPolicySimpleEmptyConnection, Connections, ConnectionPtrs>(
        conn_it, aux_map, *this);
  }

  int
  PoolPolicySimpleEmptyConnection::process_closure_awaiting_(
    Connections::const_iterator& conn_it, const ConnectionPtrs& aux_map)
    throw (eh::Exception)
  {
    return EmptyPoliciesCommonMethods::process_entity_closure_awaiting_state
        <PoolPolicySimpleEmptyConnection, Connections, ConnectionPtrs>(
          conn_it, aux_map, *this);
  }

  PoolPolicySimpleEmptyConnection::~PoolPolicySimpleEmptyConnection() throw ()
  {
  }


  //
  // PoolPolicySimpleRequests class
  //

  PoolPolicySimpleRequests::~PoolPolicySimpleRequests() throw ()
  {
  }

  void
  PoolPolicySimpleRequests::request_constructing() throw (eh::Exception)
  {
  }

  void
  PoolPolicySimpleRequests::request_destroying() throw ()
  {
  }


  //
  // PoolPolicyWaitRequests class
  //

  PoolPolicyWaitRequests::PoolPolicyWaitRequests(unsigned requests)
    throw (eh::Exception)
    : semaphore_(requests)
  {
  }

  PoolPolicyWaitRequests::~PoolPolicyWaitRequests() throw ()
  {
  }

  void
  PoolPolicyWaitRequests::request_constructing() throw (eh::Exception)
  {
    semaphore_.acquire();
  }

  void
  PoolPolicyWaitRequests::request_destroying() throw ()
  {
    semaphore_.release();
  }


  //
  // PoolPolicyThrowRequests class
  //

  PoolPolicyThrowRequests::PoolPolicyThrowRequests(unsigned requests)
    throw (eh::Exception)
    : requests_(requests)
  {
  }

  PoolPolicyThrowRequests::~PoolPolicyThrowRequests() throw ()
  {
  }

  void
  PoolPolicyThrowRequests::request_constructing() throw (eh::Exception)
  {
    _Atomic_word old = __gnu_cxx::__exchange_and_add(&requests_, -1);
    if (old <= 0)
    {
      __gnu_cxx::__atomic_add(&requests_, 1);
      Stream::Error ostr;
      ostr << FNS << "Exceeded number of simultaneous requests";
      throw Exception(ostr);
    }
  }

  void
  PoolPolicyThrowRequests::request_destroying() throw ()
  {
    __gnu_cxx::__atomic_add(&requests_, 1);
  }


  //
  // PoolPolicySimpleTimeout class
  //

  PoolPolicySimpleTimeout::PoolPolicySimpleTimeout(time_t timeout) throw ()
    : TIMEOUT_(timeout)
  {
  }

  PoolPolicySimpleTimeout::~PoolPolicySimpleTimeout() throw ()
  {
  }

  int
  PoolPolicySimpleTimeout::expiration_timeout(Identifier /*connection*/)
    throw ()
  {
    return TIMEOUT_;
  }
}
