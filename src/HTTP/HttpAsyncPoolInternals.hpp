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



#ifndef HTTP_HTTPASYNCPOOLINTERNALS_HPP
#define HTTP_HTTPASYNCPOOLINTERNALS_HPP

#include <signal.h>
#include <event.h>
#include <evhttp.h>

#include <vector>

#include <Sync/Semaphore.hpp>

#include <ReferenceCounting/List.hpp>
#include <ReferenceCounting/Map.hpp>

#include <Generics/Descriptors.hpp>

#include <HTTP/HttpAsyncPool.hpp>


namespace HTTP
{
  namespace HttpInternals
  {
    /**
     * This class allows transfer of Data from different threads into
     * the working thread where Object works with event_base
     */
    template <typename Object, typename Data>
    class SignalQueue
    {
    public:
      DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
      DECLARE_EXCEPTION(SyscallFailure, Exception);

      typedef void (Object::*DataCallback)(Data& data);
      typedef void (Object::*QuitCallback)();
      typedef void (Object::*CheckCallback)();


      /**
       * Constructor
       * @param object callback object
       * @param data_callback callback of object for data arrival
       * @param quit_callback callback of object for quit request
       * @param check_callback callback of object for check request
       */
      SignalQueue(Object& object, DataCallback data_callback,
        QuitCallback quit_callback, CheckCallback check_callback)
        throw (eh::Exception, SyscallFailure);

      /**
       * Registers reading event in the working thread
       * @param base event_base of the working thread
       */
      void
      register_event(event_base& base) throw (eh::Exception, Exception);

      /**
       * Adds data to the queue informing working thread about it
       * @param data data to transfer
       */
      void
      add(Data& data) throw (eh::Exception, SyscallFailure);

      /**
       * Posts quit message into the working thread
       * No add() calls are allowed after this call
       */
      void
      quit() throw (SyscallFailure);

      /**
       * Posts check message into the working thread
       * No add() calls are allowed after this call
       */
      void
      check() throw (SyscallFailure);

      /**
       * Allows to flush all of untransferred data calling data_callback
       */
      void
      flush() throw (eh::Exception);

    private:
      enum RequestType
      {
        RT_DATA,
        RT_QUIT,
        RT_CHECK,

        RT_LAST
      };

      void
      handle_read_() throw ();

      static
      void
      read_callback_(int fd, short type, void* arg) throw ();

      void
      signal(unsigned char data) throw (SyscallFailure);

      void
      terminate_() throw ();

      void
      remove_event_() throw ();


      typedef ReferenceCounting::List<Data> Queue;

      Sync::PosixMutex mutex_;
      Queue queue_;
      Generics::NonBlockingReadPipe pipe_;

      Object& object_;
      DataCallback data_callback_;
      QuitCallback quit_callback_;
      CheckCallback check_callback_;

      event pipe_read_;
      bool removed_;
    };


    class Request;
    typedef ReferenceCounting::QualPtr<Request> Request_var;
    class Connection;
    typedef ReferenceCounting::QualPtr<Connection> Connection_var;
    class Server;
    typedef ReferenceCounting::QualPtr<Server> Server_var;
    class ThrPoolThrInterface;
    typedef ReferenceCounting::QualPtr<ThrPoolThrInterface>
      ThrPoolThrInterface_var;
    class EventThread;
    typedef ReferenceCounting::QualPtr<EventThread> EventThread_var;
    class EventThreadPool;
    typedef ReferenceCounting::FixedPtr<EventThreadPool>
      EventThreadPool_var;
    class Informer;
    typedef ReferenceCounting::QualPtr<Informer> Informer_var;

    class ConnServInterface;
    typedef ReferenceCounting::FixedPtr<ConnServInterface>
      ConnServInterface_var;
    class ConnThreadInterface;
    typedef ReferenceCounting::QualPtr<ConnThreadInterface>
      ConnThreadInterface_var;
    class ServerInterface;
    typedef ReferenceCounting::FixedPtr<ServerInterface>
      ServerInterface_var;
    class RequestsTransfererInterface;
    typedef ReferenceCounting::FixedPtr<RequestsTransfererInterface>
      RequestsTransfererInterface_var;


    typedef ReferenceCounting::List<Request_var> Requests;
    typedef ReferenceCounting::Map<PoolPolicy::Identifier, Connection_var>
      Connections;


    /**
     * Interface for HttpAsyncPool and HttpAsyncPool::Server interaction
     */
    class ServerInterface : public virtual ReferenceCounting::Interface
    {
    protected:
      /**
       * Destructor
       */
      virtual
      ~ServerInterface() throw ();

    public:
      /**
       * Remove Server from Servers
       * @param address unique id of Server
       */
      virtual
      void
      remove_by_address(const HttpServer& address) throw () = 0;

      /**
       * Receive common pool policy
       * @return common pool policy
       */
      virtual
      PoolPolicy_var
      policy() throw () = 0;

      /**
       * Places connection to event pool
       * @param connection connection to place
       */
      virtual
      void
      place_connection(Connection* connection)
        throw (eh::Exception) = 0;
    };


    class ConnServInterface : public virtual ReferenceCounting::Interface
    {
    protected:
      virtual
      ~ConnServInterface() throw ();

    public:
      virtual
      PoolPolicy_var
      policy() throw () = 0;

      virtual
      void
      exclude_connection(Connection* connection) throw () = 0;

      virtual
      void
      transf_failed_request(Request* req,
        const String::SubString& error) throw () = 0;

      virtual
      void
      transf_unused_requests(Requests& requests, const String::SubString& error)
        throw () = 0;

      virtual
      void
      add_task_on_response(Request* req) throw () = 0;
    };


    class ConnThreadInterface : public virtual ReferenceCounting::Interface
    {
    protected:
      virtual
      ~ConnThreadInterface() throw ();

    public:
      virtual
      PoolPolicy_var
      policy() throw () = 0;

      virtual
      void
      exclude_connection(Connection* connection) throw () = 0;

      virtual
      event_base*
      get_base() throw () = 0;
    };


    class RequestsTransfererInterface :
      public virtual ReferenceCounting::Interface
    {
    public:
      virtual
      void
      process_requests(Requests& src, const String::SubString& error)
        throw () = 0;

      virtual
      void
      process_request(Request* req, const String::SubString& error)
        throw () = 0;

    protected:
      virtual
      ~RequestsTransfererInterface() throw ();
    };


    /**
     * Complete HTTP request and response
     */
    class Request :
      protected ResponseInformation,
      public ReferenceCounting::AtomicImpl,
      public Generics::Task
    {
    public:
      DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

      Request(Informer* informer, PoolPolicy* policy,
        const char* http_request, HttpMethod method,
        ResponseCallback* callback, const HttpServer& peer,
        const HeaderList& headers, const String::SubString& body)
        throw (eh::Exception, Exception);

      void
      set_response(evhttp_request* request) throw ();

      void
      set_error(const String::SubString& description) throw (eh::Exception);

      evhttp_cmd_type
      evhttp_method() const throw ();

      String::SubString
      req_body() throw ();

      const HttpServer&
      address() const throw ();

      void
      quick_on_response() throw ();

      void
      quick_on_error(const String::SubString& description) throw ();


    public:
      virtual
      const char*
      http_request() const throw ();

      virtual
      const HeaderList&
      headers() const throw ();


    protected:
      virtual
      int
      response_code() const throw ();

      virtual
      const HeaderList&
      response_headers() const throw ();

      virtual
      String::SubString
      body() const throw ();

      virtual
      HttpMethod
      method() const throw ();


    public:
      virtual
      void
      execute() throw ();


    protected:
      /**
       * Destructor
       */
      virtual
      ~Request() throw ();


    private:
      PoolPolicy_var policy_;

      HttpServer address_;
      std::string http_request_;
      ResponseCallback_var callback_;
      HttpMethod method_;
      HeaderList headers_;
      std::vector<char> body_;

      evhttp_request* response_data_;
      HeaderList response_headers_;

      std::string error_;

      Informer_var informer_;
    };


    class Connection : public ReferenceCounting::AtomicImpl
    {
    public:
      DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

      Connection(ConnServInterface* server_interface,
        const char* host, int port)
        throw (eh::Exception, Exception);

      void
      add_request(Request* request)
        throw (eh::Exception, Exception);

      bool
      deactivate() throw ();

      void
      register_connection(ConnThreadInterface* thread_interf)
        throw (eh::Exception, Exception);

      void
      process_close() throw ();

      void
      check_try_close() throw ();

    protected:
      virtual
      ~Connection() throw ();

    private:
      void
      process_request_(Request_var& request) throw ();

      static
      void
      close_callback_(int fd, short type, void* arg) throw ();

      void
      process_response_(evhttp_request* req) throw ();

      static
      void
      response_callback_(evhttp_request* req, void* arg) throw ();

      void
      process_partial_close_() throw ();

      static
      void
      try_close_callback_(int, short, void* arg) throw ();

      void
      try_close_() throw ();


      ConnThreadInterface_var thread_interf_;
      ConnServInterface_var serv_interf_;

      PoolPolicy_var policy_;

      evhttp_connection* conn_;
      typedef SignalQueue<Connection, Request_var> Queue;
      Queue queue_;
      Requests requests_;

      bool terminating_;
      event term_event_;

      std::string error_;
      event try_close_event_;
    };


    class Server :
      public ReferenceCounting::AtomicImpl,
      protected ConnServInterface,
      protected RequestsTransfererInterface
    {
    public:
      Server(const HttpServer& address, ServerInterface* server_interface,
        Generics::TaskRunner* task_runner) throw (eh::Exception);

      void
      add_request(Request* request) throw (eh::Exception);

      void
      deactivate() throw ();

    protected:
      virtual
      ~Server() throw ();

      virtual
      PoolPolicy_var
      policy() throw ();

      virtual
      void
      exclude_connection(Connection* connection) throw ();

      virtual
      void
      transf_unused_requests(Requests& src, const String::SubString& error)
        throw ();

      virtual
      void
      transf_failed_request(Request* req, const String::SubString& error)
        throw ();

      virtual
      void
      add_task_on_response(Request* req) throw ();

    protected:
      virtual
      void
      process_requests(Requests& src, const String::SubString& error)
        throw ();

      virtual
      void
      process_request(Request* req, const String::SubString& error)
        throw ();

    private:
      void
      deactivate_connection_(Connection* conn) throw ();

      void
      add_task_(Generics::Task* task) throw (eh::Exception);

      void
      add_task_on_error_(Request* req, const String::SubString& error)
        throw ();

      void
      transf_requests_(const String::SubString& error, Request* request,
        Requests& src) throw ();

    private:
      Sync::PosixMutex mutex_;

      volatile sig_atomic_t deactivating_;
      Sync::Semaphore connections_are_deactivated_;

      HttpServer server_;

      PoolPolicy_var policy_;
      ServerInterface_var server_interface_;

      Connections connections_;

      Generics::TaskRunner_var task_runner_;
    };


    class EventThread :
      public Generics::Task,
      public ReferenceCounting::AtomicImpl,
      protected ConnThreadInterface
    {
    public:
      DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

      EventThread(PoolPolicy* policy, ThrPoolThrInterface* pool_interf)
        throw (eh::Exception, Exception);

      void
      add_connection(Connection* connection) throw (eh::Exception);

      void
      deactivate() throw ();

      virtual
      void
      execute() throw ();

      void
      check_try_close() throw ();


    protected:
      virtual
      ~EventThread() throw ();


    protected:
      virtual
      PoolPolicy_var
      policy() throw ();

      virtual
      void
      exclude_connection(Connection* connection) throw ();

      virtual
      event_base*
      get_base() throw ();


    private:
      void
      thread_proc_() throw ();

      static
      void*
      thread_proc_(void* arg) throw ();

      void
      process_connection_(Connection_var& connection) throw ();

      void
      process_quit_() throw ();

      static
      void
      try_close_callback_(int, short, void* arg) throw ();

      void
      try_close_() throw ();


      PoolPolicy_var policy_;
      Connections connections_;

      typedef SignalQueue<EventThread, Connection_var> Queue;
      Queue queue_;
      event_base* base_;
      pthread_t thread_pid_;
      event try_close_event_;
      ThrPoolThrInterface_var pool_interf_;
      Sync::PosixMutex mutex_;
    };


    class ThrPoolThrInterface :
      public virtual ReferenceCounting::Interface
    {
    public:
      virtual
      bool
      exclude_thread_from_choice_list(EventThread* thread) throw () = 0;

      virtual
      bool
      exclude_thread_from_pool(EventThread* thread) throw () = 0;

    protected:
      virtual
      ~ThrPoolThrInterface() throw ();
    };


    class EventThreadPool :
      public Generics::ActiveObject,
      public ReferenceCounting::AtomicImpl,
      protected ThrPoolThrInterface
    {
    public:
      DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

      EventThreadPool(PoolPolicy* policy, Generics::TaskRunner* task_runner)
        throw (eh::Exception);

      void
      add_connection(Connection* connection) throw (eh::Exception, Exception);


    public:
      virtual
      void
      activate_object() throw (AlreadyActive, Exception, eh::Exception);

      virtual
      void
      deactivate_object() throw (Exception, eh::Exception);

      virtual
      void
      wait_object() throw (Exception, eh::Exception);

      virtual
      bool
      active() throw (eh::Exception);

    protected:
      virtual
      ~EventThreadPool() throw ();

      virtual
      bool
      exclude_thread_from_choice_list(EventThread* thread) throw ();

      virtual
      bool
      exclude_thread_from_pool(EventThread* thread) throw ();

    private:
      typedef ReferenceCounting::List<EventThread_var> Threads;

      Sync::PosixMutex mutex_;
      PoolPolicy_var policy_;
      Threads threads_;
      Threads deactivating_threads_;
      volatile sig_atomic_t active_;
      Generics::TaskRunner_var task_runner_;
    };


    class Informer : public ReferenceCounting::AtomicImpl
    {
    public:
      Informer(ServerInterface* server_interface,
        Sync::Semaphore& semaphore) throw ();

    protected:
      virtual
      ~Informer() throw ();

    private:
      ServerInterface_var server_interface_;
      Sync::Semaphore& semaphore_;
    };


    class RequestsTransferer :
      public ReferenceCounting::AtomicImpl,
      public Generics::Task
    {
    public:
      RequestsTransferer(
        RequestsTransfererInterface* requests_transferer_interface,
        const String::SubString& error, Request* request, Requests& requests)
        throw (eh::Exception);

      virtual
      void
      execute() throw ();

    protected:
      virtual
      ~RequestsTransferer() throw ();

    private:
      RequestsTransfererInterface_var requests_transferer_interface_;
      std::string error_;
      Request_var request_;
      Requests requests_;
    };


    /**
     * Asynchronous http client supporting multiple requests, multiple
     * connections and multiple servers with flexible policy management
     */
    class HttpAsyncPool :
      public HttpActiveInterface,
      public ReferenceCounting::AtomicImpl,
      protected ServerInterface
    {
    public:
      /**
       * Constructor
       * @param policy Pool policy for external management
       * @param task_runner Task Runner for execution of requests callbacks
       */
      HttpAsyncPool(PoolPolicy* policy, Generics::TaskRunner* task_runner)
        throw (eh::Exception);

      /**
       * Adds GET request for execution
       * Order of execution is unspecified
       * @param http_request request URI
       * @param callback callback to call for the request
       * @param peer http server address
       * @param headers list of additional headers for request
       */
      virtual
      void
      add_get_request(const char* http_request,
        ResponseCallback* callback = 0,
        const HttpServer& peer = HttpServer(),
        const HeaderList& headers = HeaderList())
        throw (eh::Exception, Exception);

      /**
       * Adds POST request for execution
       * Order of execution is unspecified
       * @param http_request request URI
       * @param callback callback to call for the request
       * @param body request data to post
       * @param peer http server address
       * @param headers list of additional headers for request
       */
      virtual
      void
      add_post_request(const char* http_request,
        ResponseCallback* callback = 0,
        const String::SubString& body = String::SubString(),
        const HttpServer& peer = HttpServer(),
        const HeaderList& headers = HeaderList())
        throw (eh::Exception, Exception);

      /**
       * Activates object
       */
      virtual
      void
      activate_object()
        throw (AlreadyActive, ActiveObjectException, eh::Exception);

      /**
       * Deactivates object
       */
      virtual
      void
      deactivate_object() throw (ActiveObjectException, eh::Exception);

      /**
       * Waits for object to be deactivated
       */
      virtual
      void
      wait_object() throw (ActiveObjectException, eh::Exception);

      /**
       * Returns information about object
       * @return active or not
       */
      virtual
      bool
      active() throw (eh::Exception);

    protected:
      /**
       * Destructor
       */
      virtual
      ~HttpAsyncPool() throw ();

      /**
       * Remove Server from Servers
       * @param address unique id of Server
       */
      virtual
      void
      remove_by_address(const HttpServer& address) throw ();

      /**
       * Receive common pool policy
       * @return common pool policy
       */
      virtual
      PoolPolicy_var
      policy() throw ();

      /**
       * Places connection to event pool
       * @param connection connection to place
       */
      virtual
      void
      place_connection(Connection* connection)
        throw (eh::Exception);

      /**
       * Adds request for execution
       * Order of execution is unspecified
       * @param http_request request URI
       * @param callback callback to call for the request
       * @param method request method
       * @param peer http server address
       * @param headers list of additional headers for request
       * @param body request data to post
       */
      void
      add_request_(const char* http_request,
        ResponseCallback* callback, HttpMethod method,
        const HttpServer& peer, const HeaderList& headers,
        const String::SubString& body = String::SubString())
        throw (eh::Exception, Exception);


    protected:
      typedef ReferenceCounting::Map<HttpServer, Server_var> Servers;


    private:
      Sync::PosixMutex mutex_;
      PoolPolicy_var policy_;
      EventThreadPool_var thread_pool_;
      Servers servers_;
      Generics::TaskRunner_var task_runner_;

      Sync::Semaphore semaphore_;
      Informer_var informer_;
    };
  }
}

#include "HttpAsyncPoolInternals.ipp"

#endif
