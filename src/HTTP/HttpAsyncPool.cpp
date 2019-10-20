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



#include <cassert>
#include <iostream>

#include <eh/Errno.hpp>

#include <HTTP/UrlAddress.hpp>

#include "HttpAsyncPoolInternals.hpp"


namespace HTTP
{
  namespace HttpInternals
  {
    //
    // ServerInterface class
    //

    ServerInterface::~ServerInterface() throw ()
    {
    }


    //
    // ConnServInterface class
    //

    ConnServInterface::~ConnServInterface() throw ()
    {
    }


    //
    // ConnThreadInterface class
    //

    ConnThreadInterface::~ConnThreadInterface() throw ()
    {
    }


    //
    // ThrPoolThrInterface class
    //

    ThrPoolThrInterface::~ThrPoolThrInterface() throw ()
    {
    }


    //
    // RequestsTransfererInterface class
    //

    RequestsTransfererInterface::~RequestsTransfererInterface() throw ()
    {
    }


    //
    // RequestsTransferer class
    //

    RequestsTransferer::RequestsTransferer(
      RequestsTransfererInterface* requests_transferer_interface,
      const String::SubString& error, Request* request, Requests& requests)
      throw (eh::Exception)
      : requests_transferer_interface_(
          ReferenceCounting::add_ref(requests_transferer_interface)),
        error_(error.data(), error.size()),
        request_(ReferenceCounting::add_ref(request))
    {
      requests_.splice(requests_.end(), std::move(requests));
    }

    RequestsTransferer::~RequestsTransferer() throw ()
    {
    }

    void
    RequestsTransferer::execute() throw ()
    {
      if (request_)
      {
        requests_transferer_interface_->process_request(
          request_, error_);
      }

      if (!requests_.empty())
      {
        requests_transferer_interface_->process_requests(
          requests_, error_);
      }
    }

    //
    // Connection class
    //

    Connection::Connection(ConnServInterface* server_interface,
      const char* host, int port)
      throw (eh::Exception, Exception)
      : serv_interf_(ReferenceCounting::add_ref(server_interface)),
        policy_(serv_interf_->policy()),
        queue_(*this, &Connection::process_request_,
          &Connection::process_close, &Connection::try_close_),
        terminating_(false)
    {
      conn_ = evhttp_connection_new(host, port);
      if (!conn_)
      {
        Stream::Error ostr;
        ostr << FNS << "Can't create connection object (" <<
          host << ':' << port << ')';
        throw Exception(ostr);
      }

      evtimer_set(&term_event_, close_callback_, this);
      evtimer_set(&try_close_event_, try_close_callback_, this);
    }

    Connection::~Connection() throw ()
    {
      try
      {
        queue_.flush();
      }
      catch (...)
      {
        Stream::Error ostr;
        ostr << FNS << "exception caught.";
        policy_->error(ostr.str());
      }

      if (!requests_.empty())
      {
        Stream::Error error;
        try
        {
          error << "Cancelling of requests due to "
            "destruction of connection";
          if (!error_.empty())
          {
            error << ": " << error_;
          }
        }
        catch (...)
        {
          Stream::Error ostr;
          ostr << FNS <<
            "Can't send error description to HTTP::HttpInternals::Server.";
          policy_->error(ostr.str());
        }

        serv_interf_->transf_unused_requests(requests_, error.str());
      }

      if (conn_)
      {
        evhttp_connection_free(conn_);
        if (evtimer_pending(&try_close_event_, 0))
        {
          evtimer_del(&try_close_event_);
        }
        if (evtimer_pending(&term_event_, 0))
        {
          evtimer_del(&term_event_);
        }
      }
    }

    void
    Connection::add_request(Request* request)
      throw (eh::Exception, Exception)
    {
      try
      {
        Request_var req(ReferenceCounting::add_ref(request));
        queue_.add(req);
      }
      catch (const Queue::SyscallFailure& ex)
      {
        Stream::Error ostr;
        ostr << FNS << "SyscallFailure exception caught: " << ex.what();
        throw Exception(ostr);
      }
    }

    bool
    Connection::deactivate() throw ()
    {
      try
      {
        queue_.quit();
        return true;
      }
      catch (const Queue::SyscallFailure& ex)
      {
        Stream::Error ostr;
        ostr << FNS << "SyscallFailure exception caught." << ex.what();
        policy_->error(ostr.str());
      }
      catch (...)
      {
        Stream::Error ostr;
        ostr << FNS << "exception caught.";
        policy_->error(ostr.str());
      }
      return false;
    }

    void
    Connection::register_connection(ConnThreadInterface* thread_interf)
      throw (eh::Exception, Exception)
    {
      thread_interf_ = ReferenceCounting::add_ref(thread_interf);
      queue_.register_event(*thread_interf->get_base());
      event_base_set(thread_interf->get_base(), &term_event_);
      event_base_set(thread_interf->get_base(), &try_close_event_);
      evhttp_connection_set_base(conn_, thread_interf->get_base());

      int timeout = policy_->expiration_timeout(this);
      if (timeout)
      {
        evhttp_connection_set_timeout(conn_, timeout);
      }
    }

    void
    Connection::process_request_(Request_var& request) throw ()
    {
      try
      {
        requests_.push_back(request);
      }
      catch (...)
      {
        policy_->connection_request_removed(this, request);

        Stream::Error ostr;
        ostr << FNS << "Can't put request into queue.";
        serv_interf_->transf_failed_request(request, ostr.str());

        if (requests_.empty())
        {
          try_close_();
        }

        return;
      }

      if (terminating_)
      {
        return;
      }

      evhttp_request* req = evhttp_request_new(response_callback_, this);

      int result = req ? 0 : -1;

      if (result != -1)
      {
        result = evhttp_add_header(req->output_headers, "Host",
          request->address().first.c_str());

        if (result != -1)
        {
          result = evhttp_add_header(req->output_headers,
            "Connection", "keep-alive");
        }

        HeaderList::const_iterator end = request->headers().end();
        for (HeaderList::const_iterator it = request->headers().begin();
          it != end && result != -1; ++it)
        {
          result = evhttp_add_header(req->output_headers, it->name.c_str(),
                    it->value.c_str());
        }
      }

      if (result != -1)
      {
        String::SubString req_body = request->req_body();
        if (!req_body.empty())
        {
          result = evbuffer_add(req->output_buffer, req_body.data(),
            req_body.size());
        }
      }

      if (result != -1)
      {
        evhttp_make_request(conn_, req, request->evhttp_method(),
          request->http_request());
      }
      else
      {
        if (req)
        {
          evhttp_request_free(req);
        }

        requests_.pop_back();

        policy_->connection_request_removed(this, request);

        Stream::Error ostr;
        ostr << FNS << "Can't send request.";
        serv_interf_->transf_failed_request(request, ostr.str());

        if (requests_.empty())
        {
          try_close_();
        }
      }
    }

    void
    Connection::process_close() throw ()
    {
      process_partial_close_();

      if (conn_)
      {
        evhttp_connection_free(conn_);
        conn_ = 0;
        if (evtimer_pending(&try_close_event_, 0))
        {
          evtimer_del(&try_close_event_);
        }
        if (evtimer_pending(&term_event_, 0))
        {
          evtimer_del(&term_event_);
        }
      }

      if (thread_interf_)
      {
        thread_interf_->exclude_connection(this);
      }
    }

    void
    Connection::process_partial_close_() throw ()
    {
      terminating_ = true;
      serv_interf_->exclude_connection(this);
      try
      {
        queue_.flush();
      }
      catch (...)
      {
        Stream::Error ostr;
        ostr << FNS <<
          "Can't get requests from SignalQueue, some requests may be lost.";
        policy_->error(ostr.str());
      }
    }

    void
    Connection::close_callback_(int /*fd*/, short /*type*/, void* arg)
      throw ()
    {
      static_cast<Connection*>(arg)->process_close();
    }

    void
    Connection::process_response_(evhttp_request* req) throw ()
    {
      if (!req)
      {
        try
        {
          Stream::Stack<1024> ostr;
          ostr << FNS << "Either connection refused or bad response.";
          ostr.str().assign_to(error_);
        }
        catch (...)
        {
          Stream::Error ostr;
          ostr << FNS <<
            "Can't send error description (Connection refused) to "
            "HTTP::HttpInternals::Server.";
          policy_->error(ostr.str());
        }

        process_close();
        return;
      }

      if (req->response_code == 0)
      {
        if (!terminating_)
        {
          try
          {
            Stream::Stack<1024> ostr;
            ostr << FNS << "Connection refused (Invalid address).";
            ostr.str().assign_to(error_);
          }
          catch (...)
          {
            Stream::Error ostr;
            ostr << FNS << "Can't send error description "
              "(Connection refused (Invalid address)) to "
              "HTTP::HttpInternals::Server.";
            policy_->error(ostr.str());
          }

          process_partial_close_();
          if (evtimer_add(&term_event_, &Generics::Time::ZERO) == -1)
          {
            Stream::Error ostr;
            ostr << FNS << "evtimer_add failed.";
            policy_->error(ostr.str());
          }
        }
        return;
      }

      if (requests_.empty())
      {
        Stream::Error ostr;
        ostr << FNS << "invoked while requests list is empty.";
        policy_->error(ostr.str());

        try_close_();

        return;
      }

      Request_var user_req(requests_.front());
      requests_.pop_front();

      evhttp_request* req_buf = evhttp_request_new(response_callback_, this);
      if (!req_buf)
      {
        policy_->connection_request_removed(this, user_req);

        Stream::Error ostr;
        ostr << FNS <<
          "Can't process response due to evhttp_request allocation error.";
        serv_interf_->transf_failed_request(user_req, ostr.str());

        if (requests_.empty())
        {
          try_close_();
        }

        return;
      }

      std::swap(*req_buf, *req);
      req->evcon = req_buf->evcon;
      req_buf->evcon = 0;

      user_req->set_response(req_buf);

      policy_->connection_request_removed(this, user_req);

      serv_interf_->add_task_on_response(user_req);

      if (requests_.empty())
      {
        try_close_();
      }
    }

    void
    Connection::response_callback_(evhttp_request* req, void* arg) throw ()
    {
      static_cast<Connection*>(arg)->process_response_(req);
    }

    void
    Connection::try_close_callback_(int, short, void* arg) throw ()
    {
      static_cast<Connection*>(arg)->try_close_();
    }

    void
    Connection::try_close_() throw ()
    {
      if (evtimer_pending(&try_close_event_, 0))
      {
        return;
      }

      int wait_period = policy_->when_close_connection(this);
      if (wait_period > 0)
      {
        Generics::Time tv(wait_period);
        if (evtimer_add(&try_close_event_, &tv) == -1)
        {
          Stream::Error ostr;
          ostr << FNS << "evtimer_add failed.";
          policy_->error(ostr.str());
        }
      }
      else if (wait_period == 0)
      {
        process_close();
      }
    }

    void
    Connection::check_try_close() throw ()
    {
      try
      {
        queue_.check();
      }
      catch (...)
      {
        Stream::Error ostr;
        ostr << FNS << "failed.";
        policy_->error(ostr.str());
      }
    }


    //
    // EventThread class
    //

    EventThread::EventThread(PoolPolicy* policy,
      ThrPoolThrInterface* pool_interf) throw (eh::Exception, Exception)
      : policy_(ReferenceCounting::add_ref(policy)),
        queue_(*this, &EventThread::process_connection_,
          &EventThread::process_quit_, &EventThread::try_close_),
        pool_interf_(ReferenceCounting::add_ref(pool_interf))
    {
      base_ = event_base_new();
      if (!base_)
      {
        Stream::Error ostr;
        ostr << FNS << "event_base_new() failed.";
        throw Exception(ostr);
      }

      queue_.register_event(*base_);

      if (pthread_create(&thread_pid_, 0, thread_proc_, this) == -1)
      {
        eh::throw_errno_exception<Exception>(FNE,
          "Can't create a new thread");
      }

      evtimer_set(&try_close_event_, try_close_callback_, this);
      event_base_set(base_, &try_close_event_);
    }

    EventThread::~EventThread() throw ()
    {
      //TODO: ASSERT: Thread is not active here

      event_base_free(base_);
    }

    void
    EventThread::thread_proc_() throw ()
    {
      event_base_dispatch(base_);

      queue_.flush();
      for (Connections::iterator itor(connections_.begin());
        itor != connections_.end();)
      {
        itor++->second->process_close();
      }
      assert(connections_.empty());
    }

    void*
    EventThread::thread_proc_(void* arg) throw ()
    {
      static_cast<EventThread*>(arg)->thread_proc_();
      return 0;
    }

    void
    EventThread::add_connection(Connection* connection)
      throw (eh::Exception)
    {
      Connection_var conn(ReferenceCounting::add_ref(connection));
      queue_.add(conn);
    }

    void
    EventThread::deactivate() throw ()
    {
      queue_.quit();
      pthread_join(thread_pid_, 0);
    }

    void
    EventThread::process_connection_(Connection_var& connection)
      throw ()
    {
      try
      {
        try
        {
          Connections::iterator itor;
          try
          {
            itor = connections_.insert(
              Connections::value_type(connection, connection)).first;
          }
          catch (...)
          {
            policy_->thread_connection_removed(this, connection);
            throw;
          }

          connection->register_connection(this);
        }
        catch (const eh::Exception& ex)
        {
          throw;
        }
        catch (...)
        {
          Stream::Error ostr;
          ostr << FNS << "failed to process connection";
          throw Exception(ostr);
        }
      }
      catch (const eh::Exception& ex)
      {
        connection->process_close();
        policy_->error(String::SubString(ex.what()));
      }
    }

    void
    EventThread::process_quit_() throw ()
    {
      if (event_base_loopexit(base_, 0) == -1)
      {
        Stream::Error ostr;
        ostr << FNS << "Can't stop event dispatching.";
        policy_->error(ostr.str());
      }
    }

    PoolPolicy_var
    EventThread::policy() throw ()
    {
      return policy_;
    }

    void
    EventThread::exclude_connection(Connection* connection) throw ()
    {
      Connections::iterator itor(connections_.find(connection));
      if (itor != connections_.end())
      {
        policy_->thread_connection_removed(this, connection);

        connections_.erase(itor);
      }

      if (connections_.empty())
      {
        try_close_();
      }
    }

    event_base*
    EventThread::get_base() throw ()
    {
      return base_;
    }

    void
    EventThread::try_close_callback_(int, short, void* arg) throw ()
    {
      static_cast<EventThread*>(arg)->try_close_();
    }

    void
    EventThread::try_close_() throw ()
    {
      if (evtimer_pending(&try_close_event_, 0))
      {
        return;
      }

      int wait_period = policy_->when_close_thread(this);
      if (wait_period > 0)
      {
        Generics::Time tv(wait_period);
        if (evtimer_add(&try_close_event_, &tv) == -1)
        {
          Stream::Error ostr;
          ostr << FNS << "evtimer_add(try_close_event_) failed.";
          policy_->error(ostr.str());
        }
      }
      else if (wait_period == 0 &&
        pool_interf_->exclude_thread_from_choice_list(this))
      {
        process_quit_();
      }
    }

    void
    EventThread::execute() throw ()
    {
      Sync::PosixGuard guard(mutex_);

      if (pool_interf_)
      {
        pthread_join(thread_pid_, 0);
        pool_interf_->exclude_thread_from_pool(this);
        pool_interf_.reset();
      }
    }

    void
    EventThread::check_try_close() throw ()
    {
      try
      {
        queue_.check();
      }
      catch (...)
      {
        Stream::Error ostr;
        ostr << FNS << "failed.";
        policy_->error(ostr.str());
      }
    }


    //
    // EventThreadPool class
    //

    EventThreadPool::EventThreadPool(PoolPolicy* policy,
      Generics::TaskRunner* task_runner) throw (eh::Exception)
      : policy_(ReferenceCounting::add_ref(policy)), active_(false),
        task_runner_(ReferenceCounting::add_ref(task_runner))
    {
    }

    EventThreadPool::~EventThreadPool() throw ()
    {
      assert(!active_);
    }

    void
    EventThreadPool::add_connection(Connection* connection)
      throw (eh::Exception, Exception)
    {
      EventThread_var thread;

      {
        Sync::PosixGuard guard(mutex_);

        if (!active_)
        {
          Stream::Error ostr;
          ostr << FNS << "Not active";
          throw Exception(ostr);
        }

        PoolPolicy::Identifier id(policy_->choose_thread());
        if (id != PoolPolicy::SPECIAL_IDENTIFIER)
        {
          Threads::iterator itor(threads_.begin());
          for (; itor != threads_.end(); ++itor)
          {
            if (itor->in() == id)
            {
              thread = *itor;
              break;
            }
          }
          if (itor == threads_.end())
          {
            Stream::Error ostr;
            ostr << FNS << "Unknown thread";
            throw Exception(ostr);
          }
        }
        else
        {
          thread = new EventThread(policy_, this);
          threads_.push_back(thread);

          policy_->thread_added(thread);
        }

        policy_->thread_connection_added(thread, connection);
      }

      try
      {
        thread->add_connection(connection);
      }
      catch (...)
      {
        policy_->thread_connection_removed(thread, connection);
        thread->check_try_close();
      }
    }

    void
    EventThreadPool::activate_object()
      throw (AlreadyActive, Exception, eh::Exception)
    {
      Sync::PosixGuard guard(mutex_);

      if (active_)
      {
        Stream::Error ostr;
        ostr << FNS << "already active";
        throw AlreadyActive(ostr);
      }

      active_ = true;
    }

    void
    EventThreadPool::deactivate_object()
      throw (Exception, eh::Exception)
    {
      if (!active_)
      {
        return;
      }

      for (;;)
      {
        EventThread_var thread;
        {
          Sync::PosixGuard guard(mutex_);

          if (threads_.empty())
          {
            break;
          }

          thread = threads_.front();

          policy_->thread_removed(thread);

          threads_.pop_front();
        }
        thread->deactivate();
      }

      for (;;)
      {
        EventThread_var thread;
        {
          Sync::PosixGuard guard(mutex_);

          if (deactivating_threads_.empty())
          {
            break;
          }

          thread = deactivating_threads_.front();
          deactivating_threads_.pop_front();
        }
        thread->execute();
      }

      active_ = false;
    }

    void
    EventThreadPool::wait_object() throw (Exception, eh::Exception)
    {
      Stream::Error ostr;
      ostr << FNS << "not supported";
      throw NotSupported(ostr);
    }

    bool
    EventThreadPool::active() throw (eh::Exception)
    {
      return active_;
    }

    bool
    EventThreadPool::exclude_thread_from_choice_list(EventThread* thread)
      throw ()
    {
      Sync::PosixGuard guard(mutex_);

      Threads::iterator thr_it = threads_.begin();
      Threads::iterator thr_end = threads_.end();
      for (; thr_it != thr_end; ++thr_it)
      {
        if (thr_it->in() == thread)
        {
          policy_->thread_removed(thread);

          try
          {
            task_runner_->enqueue_task(thread);
          }
          catch (...)
          {
            Stream::Error ostr;
            ostr << FNS <<
              "can't enqueue task (HTTP::HttpInternals::EventThread "
              "deactivation failed)";
            policy_->error(ostr.str());
          }

          deactivating_threads_.splice(
            deactivating_threads_.end(), std::move(threads_), thr_it);

          return true;
        }
      }

      return false;
    }

    bool
    EventThreadPool::exclude_thread_from_pool(EventThread* thread) throw ()
    {
      Sync::PosixGuard guard(mutex_);

      Threads::iterator thr_it = deactivating_threads_.begin();
      Threads::iterator thr_end = deactivating_threads_.end();
      for (; thr_it != thr_end; ++thr_it)
      {
        if (thr_it->in() == thread)
        {
          deactivating_threads_.erase(thr_it);

          return true;
        }
      }

      return false;
    }


    //
    // Server class
    //

    Server::Server(const HttpServer& address,
      ServerInterface* server_interface, Generics::TaskRunner* task_runner)
      throw (eh::Exception)
      : deactivating_(false),
        connections_are_deactivated_(0),
        server_(address),
        policy_(server_interface->policy()),
        server_interface_(ReferenceCounting::add_ref(server_interface)),
        task_runner_(ReferenceCounting::add_ref(task_runner))
    {
    }

    Server::~Server() throw ()
    {
      assert(connections_.empty());
    }

    void
    Server::add_request(Request* request)
      throw (eh::Exception)
    {
      Connection_var connection;
      {
        Sync::PosixGuard guard(mutex_);

        if (deactivating_)
        {
          Stream::Error ostr;
          ostr << FNS << "Deactivated";
          throw Exception(ostr);
        }

        PoolPolicy::Identifier id(policy_->choose_connection(
          this, request));
        if (id != PoolPolicy::SPECIAL_IDENTIFIER)
        {
          Connections::iterator itor(connections_.find(id));
          if (itor == connections_.end())
          {
            Stream::Error ostr;
            ostr << FNS << "Invalid connection";
            throw Exception(ostr);
          }
          connection = itor->second;
        }
        else
        {
          const HttpServer& addr = request->address();
          connection = new Connection(this, addr.first.c_str(), addr.second);

          Connections::iterator conn_it = connections_.insert(
            Connections::value_type(connection, connection)).first;

          policy_->server_connection_added(this, connection);

          try
          {
            server_interface_->place_connection(connection);
          }
          catch (...)
          {
            policy_->server_connection_removed(this, connection);
            connections_.erase(conn_it);
            throw;
          }
        }
        policy_->connection_request_added(this, connection, request);
      }

      try
      {
        connection->add_request(request);
      }
      catch (...)
      {
        policy_->connection_request_removed(connection, request);

        connection->check_try_close();

        throw;
      }
    }

    void
    Server::deactivate() throw ()
    {
      deactivating_ = true;
      bool wait_for_connections;

      {
        Sync::PosixGuard guard(mutex_);

        for (Connections::iterator itor(connections_.begin());
          itor != connections_.end();)
        {
          deactivate_connection_(itor++->second);
        }

        wait_for_connections = !connections_.empty();
      }

      if (wait_for_connections)
      {
        connections_are_deactivated_.acquire();
      }
      server_interface_->remove_by_address(server_);
    }

    void
    Server::deactivate_connection_(Connection* connection) throw ()
    {
      if (!connection->deactivate())
      {
        Connections::iterator itor(connections_.find(connection));
        if (itor != connections_.end())
        {
          policy_->server_connection_removed(this, connection);

          connections_.erase(itor);
        }
      }
    }

    PoolPolicy_var
    Server::policy() throw ()
    {
      return policy_;
    }

    void
    Server::exclude_connection(Connection* connection) throw ()
    {
      Sync::PosixGuard guard(mutex_);

      Connections::iterator itor(connections_.find(connection));
      if (itor != connections_.end())
      {
        policy_->server_connection_removed(this, connection);

        connections_.erase(itor);
      }

      if (deactivating_ && connections_.empty())
      {
        connections_are_deactivated_.release();
      }
    }

    void
    Server::add_task_on_error_(Request* req,
      const String::SubString& error) throw ()
    {
      policy_->server_request_removed(this, req);
      try
      {
        req->set_error(error);
        add_task_(req);
        return;
      }
      catch (...)
      {
        Stream::Error ostr;
        ostr << FNS << "Can't invoke on_error method.";
        policy_->error(ostr.str());
      }
      req->quick_on_error(error);
    }

    void
    Server::add_task_on_response(Request* req) throw ()
    {
      policy_->server_request_removed(this, req);
      try
      {
        add_task_(req);
        return;
      }
      catch (...)
      {
        Stream::Error ostr;
        ostr << FNS << "Failed to add task.";
        policy_->error(ostr.str());
      }
      req->quick_on_response();
    }

    void
    Server::transf_requests_(const String::SubString& error,
      Request* request, Requests& requests) throw ()
    {
      try
      {
        try
        {
          Generics::Task_var task(new RequestsTransferer(
            this, error, request, requests));
          add_task_(task);
          return;
        }
        catch (const eh::Exception& ex)
        {
          Stream::Error ostr;
          ostr << FNS << ex.what();
          policy_->error(ostr.str());
        }
        catch (...)
        {
          Stream::Error ostr;
          ostr << FNS << "Exception caught";
          policy_->error(ostr.str());
        }
      }
      catch (...)
      {
      }

      if (request)
      {
        policy_->server_request_removed(this, request);
        request->quick_on_error(error);
      }
      for (Requests::iterator itor(requests.begin());
        itor != requests.end(); ++itor)
      {
        policy_->server_request_removed(this, itor->in());
        (*itor)->quick_on_error(error);
      }
    }

    void
    Server::transf_unused_requests(Requests& requests,
      const String::SubString& error) throw ()
    {
      transf_requests_(error, 0, requests);
    }

    void
    Server::transf_failed_request(Request* request,
      const String::SubString& error) throw ()
    {
      Requests requests;
      transf_requests_(error, request, requests);
    }

    void
    Server::process_requests(Requests& requests,
      const String::SubString& error) throw ()
    {
      switch (policy_->requests_failed(this))
      {
      case PoolPolicy::RP_MORE_DETAILS_REQUIRED:
        while (!requests.empty() && !deactivating_)
        {
          process_request(requests.front(), error);
          requests.pop_front();
        }
        // FALLTHROUGH

      case PoolPolicy::RP_CANCEL_FIRST_RESEND_OTHERS:
        if (!requests.empty())
        {
          add_task_on_error_(requests.front(), error);
          requests.pop_front();
        }
        // FALLTHROUGH

      case PoolPolicy::RP_RESEND_ALL:
        while (!deactivating_ && !requests.empty())
        {
          try
          {
            add_request(requests.front());
          }
          catch (...)
          {
            break;
          }
          requests.pop_front();
        }
        // FALLTHROUGH

      case PoolPolicy::RP_CANCEL_ALL:
      default:
        while (!requests.empty())
        {
          add_task_on_error_(requests.front(), error);
          requests.pop_front();
        }
        break;
      }
    }

    void
    Server::process_request(Request* request,
      const String::SubString& error) throw ()
    {
      switch (policy_->request_failed(this, request))
      {
      case PoolPolicy::RP_RESEND_ALL:
        try
        {
          add_request(request);
        }
        catch (...)
        {
          add_task_on_error_(request, error);
        }
        break;

      case PoolPolicy::RP_CANCEL_ALL:
      case PoolPolicy::RP_CANCEL_FIRST_RESEND_OTHERS:
      case PoolPolicy::RP_MORE_DETAILS_REQUIRED:
      default:
        add_task_on_error_(request, error);
        break;
      }
    }

    void
    Server::add_task_(Generics::Task* task) throw (eh::Exception)
    {
      task_runner_->enqueue_task(task);
    }


    //
    // Request class
    //

    Request::Request(Informer* informer, PoolPolicy* policy,
      const char* http_request, HttpMethod method,
      ResponseCallback* callback, const HttpServer& peer,
      const HeaderList& headers, const String::SubString& body)
        throw (eh::Exception, Exception)
      : policy_(ReferenceCounting::add_ref(policy)), address_(peer),
        http_request_(http_request),
        callback_(ReferenceCounting::add_ref(callback)), method_(method),
        headers_(headers), response_data_(0),
        informer_(ReferenceCounting::add_ref(informer))
    {
      if (!body.empty())
      {
        body_.assign(body.begin(), body.end());
      }

      // The last operation in the constructor
      policy_->request_constructing();
    }

    Request::~Request() throw ()
    {
      if (response_data_)
      {
        evhttp_request_free(response_data_);
      }

      policy_->request_destroying();
    }

    const HttpServer&
    Request::address() const throw ()
    {
      return address_;
    }

    void
    Request::quick_on_response() throw ()
    {
      if (callback_)
      {
        callback_->quick_on_response(*this);
      }
    }

    void
    Request::quick_on_error(const String::SubString& description) throw ()
    {
      if (callback_)
      {
        callback_->quick_on_error(description, *this);
      }
    }


    const char*
    Request::http_request() const throw ()
    {
      return http_request_.c_str();
    }

    const HeaderList&
    Request::headers() const throw ()
    {
      return headers_;
    }

    void
    Request::set_response(evhttp_request* response_data) throw ()
    {
      response_data_ = response_data;
      char** headers =
        evhttp_headers_to_array(response_data_->input_headers);
      try
      {
        for (char** header = headers; *header; header += 2)
        {
          response_headers_.emplace_back(header[0], header[1]);
        }
      }
      catch (...)
      {
      }
      evhttp_headers_array_free(headers);
    }

    int
    Request::response_code() const throw ()
    {
      if (response_data_)
      {
        return response_data_->response_code;
      }

      return -1;
    }

    const HeaderList&
    Request::response_headers() const throw ()
    {
      return response_headers_;
    }

    String::SubString
    Request::body() const throw ()
    {
      if (response_data_)
      {
        return String::SubString(reinterpret_cast<const char*>(
          EVBUFFER_DATA(response_data_->input_buffer)),
          EVBUFFER_LENGTH(response_data_->input_buffer));
      }

      return String::SubString();
    }

    HttpMethod
    Request::method() const throw ()
    {
      return method_;
    }

    evhttp_cmd_type
    Request::evhttp_method() const throw ()
    {
      switch (method_)
      {
      case HM_GET:
        return EVHTTP_REQ_GET;
      case HM_POST:
        return EVHTTP_REQ_POST;
      default:
        break;
      }
      return EVHTTP_REQ_GET;
    }

    void
    Request::set_error(const String::SubString& description)
      throw (eh::Exception)
    {
      if (description.empty())
      {
        Stream::Error ostr;
        ostr << FNS << "Invalid error description";
        throw Exception(ostr);
      }
      description.assign_to(error_);
      assert(!error_.empty());
    }

    void
    Request::execute() throw ()
    {
      if (callback_)
      {
        if (error_.empty())
        {
          callback_->on_response(*this);
        }
        else
        {
          callback_->on_error(error_, *this);
        }
      }
    }

    String::SubString
    Request::req_body() throw ()
    {
      return String::SubString(&body_[0], body_.size());
    }


    //
    // Informer class
    //

    Informer::Informer(ServerInterface* server_interface,
      Sync::Semaphore& semaphore) throw ()
      : server_interface_(server_interface), semaphore_(semaphore)
    {
      server_interface->add_ref();
    }

    Informer::~Informer() throw ()
    {
      semaphore_.release();
    }


    //
    // HttpAsyncPool class
    //

    HttpAsyncPool::HttpAsyncPool(PoolPolicy* policy,
      Generics::TaskRunner* task_runner)
      throw (eh::Exception)
      : policy_(ReferenceCounting::add_ref(policy)),
        thread_pool_(new HttpInternals::EventThreadPool(policy, task_runner)),
        task_runner_(ReferenceCounting::add_ref(task_runner)), semaphore_(0)
    {
      if (!policy_)
      {
        Stream::Error ostr;
        ostr << FNS << "Invalid policy";
        throw Exception(ostr);
      }

      if (!task_runner)
      {
        Stream::Error ostr;
        ostr << FNS << "Invalid task runner";
        throw Exception(ostr);
      }
    }

    HttpAsyncPool::~HttpAsyncPool() throw ()
    {
    }

    void
    HttpAsyncPool::add_get_request(const char* http_request,
      ResponseCallback* callback, const HttpServer& peer,
      const HeaderList& headers)
      throw (eh::Exception, Exception)
    {
      add_request_(http_request, callback, HM_GET, peer, headers);
    }

    void
    HttpAsyncPool::add_post_request(const char* http_request,
      ResponseCallback* callback,
      const String::SubString& body, const HttpServer& peer,
      const HeaderList& headers)
      throw (eh::Exception, Exception)
    {
      add_request_(http_request, callback, HM_POST, peer, headers, body);
    }

    void
    HttpAsyncPool::add_request_(const char* http_request,
      ResponseCallback* callback, HttpMethod method,
      const HttpServer& peer, const HeaderList& headers,
      const String::SubString& body)
      throw (eh::Exception, Exception)
    {
      if (!active())
      {
        Stream::Error ostr;
        ostr << FNS << "Not active";
        throw Exception(ostr);
      }

      if (!http_request)
      {
        Stream::Error ostr;
        ostr << FNS << "NULL http request";
        throw Exception(ostr);
      }

      HttpServer address;
      if (peer.first.empty())
      {
        try
        {
          BrowserAddress parser{String::SubString(http_request)};
          parser.host().assign_to(address.first);
          address.second = parser.port_number();
        }
        catch (const eh::Exception& ex)
        {
          Stream::Error ostr;
          ostr << FNS << "Can't parse received http_request: " << ex.what();
          throw Exception(ostr);
        }
      }
      else
      {
        address = peer;
      }

      HttpInternals::Server_var server_var;
      {
        Sync::PosixGuard guard(mutex_);

        Servers::iterator server = servers_.find(address);
        if (server == servers_.end())
        {
          server = servers_.insert(Servers::value_type(address,
            HttpInternals::Server_var(new HttpInternals::Server(
              address, this, task_runner_)))).first;

          policy_->server_added(server->second);
        }
        server_var = server->second;
      }

      HttpInternals::Request_var request(new HttpInternals::Request(
        informer_, policy_, http_request, method, callback,
        address, headers, body));

      try
      {
        policy_->server_request_added(server_var, request);
        server_var->add_request(request);
      }
      catch (...)
      {
        policy_->server_request_removed(server_var, request);
        throw;
      }
    }

    void
    HttpAsyncPool::remove_by_address(const HttpServer& address) throw ()
    {
      Sync::PosixGuard guard(mutex_);

      Servers::iterator server(servers_.find(address));
      if (server != servers_.end())
      {
        policy_->server_removed(server->second);

        servers_.erase(server);
      }
    }

    PoolPolicy_var
    HttpAsyncPool::policy() throw ()
    {
      return policy_;
    }

    void
    HttpAsyncPool::place_connection(HttpInternals::Connection* connection)
      throw (eh::Exception)
    {
      thread_pool_->add_connection(connection);
    }

    void
    HttpAsyncPool::activate_object()
      throw (AlreadyActive, ActiveObjectException, eh::Exception)
    {
      Sync::PosixGuard guard(mutex_);

      if (!informer_)
      {
        informer_ = new HttpInternals::Informer(this, semaphore_);
      }
      thread_pool_->activate_object();
    }

    void
    HttpAsyncPool::deactivate_object()
      throw (ActiveObjectException, eh::Exception)
    {
      if (!active())
      {
        return;
      }

      informer_.reset();

      for (;;)
      {
        HttpInternals::Server_var server;
        {
          Sync::PosixGuard guard(mutex_);
          if (servers_.empty())
          {
            thread_pool_->deactivate_object();
            break;
          }
          server = servers_.begin()->second;
        }
        server->deactivate();
      }
    }

    void
    HttpAsyncPool::wait_object()
      throw (ActiveObjectException, eh::Exception)
    {
      semaphore_.acquire();
      semaphore_.release();
    }

    bool
    HttpAsyncPool::active() throw (eh::Exception)
    {
      return thread_pool_->active();
    }
  }


  //
  // PoolPolicy class
  //

  PoolPolicy::~PoolPolicy() throw ()
  {
  }


  //
  //
  //

  HttpActiveInterface*
  CreatePool(PoolPolicy* policy, Generics::TaskRunner* task_runner)
    throw (eh::Exception)
  {
    return new HttpInternals::HttpAsyncPool(policy, task_runner);
  }
}
