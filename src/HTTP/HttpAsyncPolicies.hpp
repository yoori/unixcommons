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



#ifndef HTTP_HTTPASYNCPOLICIES_HPP
#define HTTP_HTTPASYNCPOLICIES_HPP

#include <map>

#include <Sync/Semaphore.hpp>

#include <HTTP/HttpAsyncPool.hpp>


namespace HTTP
{
  //Forward declarations
  class EmptyPoliciesCommonMethods;


  //
  // class PoolPolicySimpleStatistics
  //

  class PoolPolicySimpleStatistics : public virtual PoolPolicyStatistics
  {
  public:
    virtual
    void
    server_added(Identifier server) throw ();

    virtual
    void
    server_removed(Identifier server) throw ();

    virtual
    void
    server_connection_added(Identifier server, Identifier connection)
      throw ();

    virtual
    void
    server_connection_removed(Identifier server, Identifier connection)
      throw ();


    virtual
    void
    thread_added(Identifier thread) throw ();

    virtual
    void
    thread_removed(Identifier thread) throw ();

    virtual
    void
    thread_connection_added(Identifier thread, Identifier connection)
      throw ();

    virtual
    void
    thread_connection_removed(Identifier thread, Identifier connection)
      throw ();


    virtual
    void
    connection_request_added(Identifier server, Identifier connection,
      Identifier request) throw ();

    virtual
    void
    connection_request_removed(Identifier connection, Identifier request)
      throw ();

    virtual
    void
    server_request_added(Identifier server, Identifier request)
      throw ();

    virtual
    void
    server_request_removed(Identifier server, Identifier request)
      throw ();

  protected:
    virtual
    void
    server_added_i(Identifier server) throw (eh::Exception);

    virtual
    void
    server_removed_i(Identifier server) throw (eh::Exception);

    virtual
    void
    server_connection_added_i(Identifier server, Identifier connection)
      throw (eh::Exception);

    virtual
    void
    server_connection_removed_i(Identifier server, Identifier connection)
      throw (eh::Exception);


    virtual
    void
    thread_removed_i(Identifier thread) throw (eh::Exception);

    virtual
    void
    thread_connection_added_i(Identifier thread, Identifier connection)
      throw (eh::Exception);

    virtual
    void
    thread_connection_removed_i(Identifier thread, Identifier connection)
      throw (eh::Exception);


    virtual
    void
    connection_request_added_i(Identifier server, Identifier connection,
      Identifier request) throw (eh::Exception);

    virtual
    void
    connection_request_removed_i(Identifier connection, Identifier request)
      throw (eh::Exception);

  protected:
    struct StateInfo
    {
      enum States
      {
        ACTIVE_AWAITING,
        ACTIVE,
        CLOSURE_AWAITING,
        CLOSURE_ON_NEXT_TRY,
        CLOSING
      };

      mutable States state;

      StateInfo() throw ();
    };

    struct SimpleStat : public StateInfo
    {
      unsigned items_count;

      SimpleStat() throw ();
    };

    struct Connection : public SimpleStat
    {
      Identifier server;
      Identifier thread;

      Connection() throw ();
      Connection(Identifier server) throw ();
    };
    typedef std::map<Identifier, Connection> Connections;

    struct Thread : public SimpleStat
    {
      mutable bool full;

      Thread() throw ();
    };
    typedef std::map<Identifier, Thread> Threads;

    typedef std::map<Identifier, Connections::iterator> ConnectionPtrs;
    typedef std::map<Identifier, ConnectionPtrs> Servers;

    virtual
    ~PoolPolicySimpleStatistics() throw ();

    const Servers&
    get_servers_() const throw ();

    const Threads&
    get_threads_() const throw ();

    const Connections&
    get_connections_() const throw ();

  private:
    Servers servers_;
    Threads threads_;
    Connections connections_;
  };


  //
  // class PoolPolicyAdvancedStatistics
  //

  class PoolPolicyAdvancedStatistics :
    public virtual PoolPolicySimpleStatistics
  {
  public:
    virtual
    void
    server_request_added(Identifier server, Identifier request) throw ();

    virtual
    void
    server_request_removed(Identifier server, Identifier request) throw ();

    virtual
    void
    server_added_i(Identifier server) throw (eh::Exception);

    virtual
    void
    server_removed_i(Identifier server) throw (eh::Exception);

    virtual
    void
    connection_request_added_i(Identifier server, Identifier connection,
      Identifier request) throw (eh::Exception);

  protected:
    virtual
    ~PoolPolicyAdvancedStatistics() throw ();

    typedef std::map<Identifier, int> Requests;
    typedef std::map<Identifier, Requests> ServerRequests;

    ServerRequests server_requests_;
  };


  //
  // class PoolPolicySimpleDecider
  //

  class PoolPolicySimpleDecider :
    public virtual PoolPolicySimpleStatistics,
    public virtual PoolPolicyDecider
  {
  public:
    PoolPolicySimpleDecider(unsigned connections_per_server,
      unsigned connections_per_threads)
      throw (eh::Exception);

    virtual
    Identifier
    choose_thread() throw ();

    virtual
    Identifier
    choose_connection(Identifier server, Identifier request) throw ();

    virtual
    RequestPolicy
    request_failed(Identifier server, Identifier request) throw ();

    virtual
    RequestPolicy
    requests_failed(Identifier server) throw ();

  private:
    const unsigned CONNECTIONS_PER_SERVER_;
    const unsigned CONNECTIONS_PER_THREADS_;
  };


  //
  // class PoolPolicySimpleEmptyThread
  //

  class PoolPolicySimpleEmptyThread :
    public virtual PoolPolicySimpleStatistics,
    public virtual PoolPolicyEmptyThread
  {
  public:
    PoolPolicySimpleEmptyThread(time_t closure_delay = 3)
      throw ();

    virtual
    int
    when_close_thread(Identifier thread) throw ();

  protected:
    virtual
    ~PoolPolicySimpleEmptyThread() throw ();

    int
    process_active_(Threads::const_iterator& cur_thr,
      const Threads&) throw (eh::Exception);

    int
    process_closure_awaiting_(Threads::const_iterator& cur_thr,
      const Threads&) throw (eh::Exception);

  private:
    const time_t CLOSURE_DELAY_;

    friend class EmptyPoliciesCommonMethods;
  };


  //
  // class PoolPolicySimpleEmptyConnection
  //

  class PoolPolicySimpleEmptyConnection :
    public virtual PoolPolicySimpleStatistics,
    public virtual PoolPolicyEmptyConnection
  {
  public:
    PoolPolicySimpleEmptyConnection(time_t closure_delay = 3)
      throw ();

    virtual
    int
    when_close_connection(Identifier connection) throw ();

  protected:
    virtual
    ~PoolPolicySimpleEmptyConnection() throw ();

    int
    process_active_(Connections::const_iterator& conn_it,
      const ConnectionPtrs& aux_map) throw (eh::Exception);

    int
    process_closure_awaiting_(Connections::const_iterator& conn_it,
      const ConnectionPtrs& aux_map) throw (eh::Exception);

  private:
    const time_t CLOSURE_DELAY_;

    friend class EmptyPoliciesCommonMethods;
  };


  //
  // class PoolPolicySimpleRequests
  //

  class PoolPolicySimpleRequests : public virtual PoolPolicyRequests
  {
  public:
    virtual
    void
    request_constructing() throw (eh::Exception);

    virtual
    void
    request_destroying() throw ();

  protected:
    virtual
    ~PoolPolicySimpleRequests() throw ();
  };


  //
  // class PoolPolicyWaitRequests
  //

  class PoolPolicyWaitRequests : public virtual PoolPolicyRequests
  {
  public:
    PoolPolicyWaitRequests(unsigned requests) throw (eh::Exception);

    virtual
    void
    request_constructing() throw (eh::Exception);

    virtual
    void
    request_destroying() throw ();

  protected:
    virtual
    ~PoolPolicyWaitRequests() throw ();

  private:
    Sync::Semaphore semaphore_;
  };


  //
  // class PoolPolicyThrowRequests
  //

  class PoolPolicyThrowRequests : public virtual PoolPolicyRequests
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    PoolPolicyThrowRequests(unsigned requests) throw (eh::Exception);

    virtual
    void
    request_constructing() throw (eh::Exception);

    virtual
    void
    request_destroying() throw ();

  protected:
    virtual
    ~PoolPolicyThrowRequests() throw ();

  private:
    volatile _Atomic_word requests_;
  };


  //
  // class PoolPolicySimpleTimeout
  //

  class PoolPolicySimpleTimeout : public virtual PoolPolicyTimeout
  {
  public:
    PoolPolicySimpleTimeout(const time_t timeout = 0) throw ();

    virtual
    int
    expiration_timeout(Identifier connection) throw ();

  protected:
    virtual
    ~PoolPolicySimpleTimeout() throw ();

  private:
    const time_t TIMEOUT_;
  };
}

#endif
