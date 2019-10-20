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



#ifndef HTTP_HTTPASYNCPOOL_HPP
#define HTTP_HTTPASYNCPOOL_HPP

#include <HTTP/HttpAsync.hpp>

#include <Generics/TaskRunner.hpp>


namespace HTTP
{
  /**
   * Base class for PoolPolicy ancestors
   */
  class PoolPolicyCommon :
    public ReferenceCounting::AtomicImpl,
    public virtual Generics::ActiveObjectCallback
  {
  public:
    typedef const void* Identifier;

    static const Identifier SPECIAL_IDENTIFIER;

  protected:
    /**
     * Destructor
     */
    virtual
    ~PoolPolicyCommon() throw ();


    Sync::PosixMutex mutex_;
  };

  /**
   * Policy ancestor providing interface of statistics gathering
   * Called by HttpAsyncPool on events
   */
  class PoolPolicyStatistics : public virtual PoolPolicyCommon
  {
  public:
    /**
     * Called when a new server is created in HttpAsyncPool
     * @param server server identifier
     */
    virtual
    void
    server_added(Identifier server) throw () = 0;

    /**
     * Called when a new server is deleted in HttpAsyncPool
     * @param server server identifier
     */
    virtual
    void
    server_removed(Identifier server) throw () = 0;

    /**
     * Called when a new connection for a server is created in HttpAsyncPool
     * @param server server identifier
     * @param connection connection identifier
     */
    virtual
    void
    server_connection_added(Identifier server, Identifier connection)
      throw () = 0;

    /**
     * Called when a new connection for a server is deleted in HttpAsyncPool
     * @param server server identifier
     * @param connection connection identifier
     */
    virtual
    void
    server_connection_removed(Identifier server, Identifier connection)
      throw () = 0;


    /**
     * Called when a new thread is created in HttpAsyncPool
     * @param thread thread identifier
     */
    virtual
    void
    thread_added(Identifier thread) throw () = 0;

    /**
     * Called when a new thread is deleted in HttpAsyncPool
     * @param thread thread identifier
     */
    virtual
    void
    thread_removed(Identifier thread) throw () = 0;

    /**
     * Called when a connection is attached for a thread in HttpAsyncPool
     * @param thread thread identifier
     * @param connection connection identifier
     */
    virtual
    void
    thread_connection_added(Identifier thread, Identifier connection)
      throw () = 0;

    /**
     * Called when a connection is detached from a thread in HttpAsyncPool
     * @param thread thread identifier
     * @param connection connection identifier
     */
    virtual
    void
    thread_connection_removed(Identifier thread, Identifier connection)
      throw () = 0;


    /**
     * Called when a new request is added to a connection in HttpAsyncPool
     * @param server server identifier
     * @param connection connection identifier
     * @param request request identifier
     */
    virtual
    void
    connection_request_added(Identifier server, Identifier connection,
      Identifier request) throw () = 0;

    /**
     * Called when a new request is removed from a connection in HttpAsyncPool
     * @param connection connection identifier
     * @param request request identifier
     */
    virtual
    void
    connection_request_removed(Identifier connection, Identifier request)
      throw () = 0;

    /**
     * Called when a new request is added to a server in HttpAsyncPool
     * @param server server identifier
     * @param request request identifier
     */
    virtual
    void
    server_request_added(Identifier server, Identifier request)
      throw () = 0;

    /**
     * Called when a new request is removed from a server in HttpAsyncPool
     * @param server server identifier
     * @param request request identifier
     */
    virtual
    void
    server_request_removed(Identifier server, Identifier request)
      throw () = 0;


  protected:
    /**
     * Destructor
     */
    virtual
    ~PoolPolicyStatistics() throw ();
  };

  /**
   * Policy ancestor providing interface for decision making
   * Called by HttpAsyncPool on events
   */
  class PoolPolicyDecider : public virtual PoolPolicyCommon
  {
  public:
    enum RequestPolicy
    {
      RP_RESEND_ALL,
      RP_CANCEL_ALL,
      RP_CANCEL_FIRST_RESEND_OTHERS,
      RP_MORE_DETAILS_REQUIRED
    };

    /**
     * Determines which thread to choose for a connection (or create new)
     * @return thread identifier of SPECIAL_IDENTIFIER for a new thread
     */
    virtual
    Identifier
    choose_thread() throw () = 0;

    /**
     * Determines which connection to choose for a request in the server
     * (or create new)
     * @param server server identifier
     * @param request request identifier
     * @return connection identifier of SPECIAL_IDENTIFIER for a new connection
     */
    virtual
    Identifier
    choose_connection(Identifier server, Identifier request) throw () = 0;

    /**
     * Determines the future of the failed request - resending or
     * error returning
     * @param server server identifier
     * @param request request identifier
     * @return whether request should be resent or finished failed
     */
    virtual
    RequestPolicy
    request_failed(Identifier server, Identifier request) throw () = 0;

    /**
     * Determines the future of the failed requests - resending or
     * error returning
     * @param server server identifier
     * @return whether requests should be resent or finished failed or
     * more detailed information on each request is required
     */
    virtual
    RequestPolicy
    requests_failed(Identifier server) throw () = 0;


  protected:
    /**
     * Destructor
     */
    virtual
    ~PoolPolicyDecider() throw ();
  };

  /**
   * Policy ancestor providing interface for making a decision about
   * connection closure
   * Called by Connection when requests end up
   */
  class PoolPolicyEmptyConnection : public virtual PoolPolicyCommon
  {
  public:
    /**
     * Determines is connection closure required
     * @param connection connection identifier
     * @return int: "-1" do not close, "0" close, "positive number"
     * re-invoke this method after this time (in sec) will pass
     */
    virtual
    int
    when_close_connection(Identifier connection) throw () = 0;

  protected:
    /**
     * Destructor
     */
    virtual
    ~PoolPolicyEmptyConnection() throw ();
  };

  /**
   * Policy ancestor providing interface for making a decision about
   * thread closure
   * Called by EventThread when connections end up
   */
  class PoolPolicyEmptyThread : public virtual PoolPolicyCommon
  {
  public:
    /**
     * Determines is thread closure required
     * @param thread thread identifier
     * @return int: "0" close, "-1" do not close, "positive number"
     * re-invoke this method after this time (in sec) will pass
     */
    virtual
    int
    when_close_thread(Identifier thread) throw () = 0;

  protected:
    /**
     * Destructor
     */
    virtual
    ~PoolPolicyEmptyThread() throw ();
  };

  /**
   * Policy ancestor providing interface for requests counting
   * Called by HttpAsyncPool on events
   */
  class PoolPolicyRequests : public virtual PoolPolicyCommon
  {
  public:
    /**
     * Controls number of requests in progress
     * Proceeds normally, waits or throws exception
     * Called by HttpAsyncPool on request creation
     */
    virtual
    void
    request_constructing() throw (eh::Exception) = 0;

    /**
     * Called by HttpAsyncPool on request destruction
     */
    virtual
    void
    request_destroying() throw () = 0;

  protected:
    /**
     * Destructor
     */
    virtual
    ~PoolPolicyRequests() throw ();
  };

  /**
   * Policy ancestor providing interface for timeout invalidation
   * Called by HttpAsyncPool on requests creations
   */
  class PoolPolicyTimeout : public virtual PoolPolicyCommon
  {
  public:
    /**
     * Provides request handling timeout per connection
     * Called by HttpAsyncPool on request creation
     * @param connection connection identifier
     * @return request expiration timeout (in seconds)
     */
    virtual
    int
    expiration_timeout(Identifier connection) throw () = 0;

  protected:
    /**
     * Destructor
     */
    virtual
    ~PoolPolicyTimeout() throw ();
  };

  /**
   * Base abstract class for request/connection management policies
   */
  class PoolPolicy :
    public virtual PoolPolicyStatistics,
    public virtual PoolPolicyDecider,
    public virtual PoolPolicyRequests,
    public virtual PoolPolicyEmptyConnection,
    public virtual PoolPolicyEmptyThread,
    public virtual PoolPolicyTimeout
  {
  protected:
    /**
     * Destructor
     */
    virtual
    ~PoolPolicy() throw ();
  };
  typedef ReferenceCounting::QualPtr<PoolPolicy> PoolPolicy_var;


  /**
   * Helper function for creation of HttpAsyncPool
   * @param policy controlling policy
   * @param task_runner task runner for callbacks execution
   * @return pointer to created HttpAsyncPool
   */
  HttpActiveInterface*
  CreatePool(PoolPolicy* policy, Generics::TaskRunner* task_runner)
    throw (eh::Exception);
}

#endif
