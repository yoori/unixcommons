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



// Listener.cpp

#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>

#include <eh/Errno.hpp>

#include <Generics/Listener.hpp>
#include <Generics/Singleton.hpp>


//////////////////////////////////////////////////////////////////////////
//  Descriptor listener objects implementations
//////////////////////////////////////////////////////////////////////////

namespace
{
  /**
   * Auxiliary class DescriptorsHolder
   * We fill descriptors, class guarantee that all filled
   * descriptors will be close when destroy DescriptorsHolder object.
   * i.e. class is close guard.
   */
  class DescriptorsHolder : private Generics::Uncopyable
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    typedef char Error[sizeof(Exception)];

    /**
     * Allocates memory enough to contain count descriptors.
     * Does not own any descriptors.
     * @param count max count of descriptors, that we can own.
     */
    DescriptorsHolder(size_t count) throw (eh::Exception);

    /**
     * Destructor, closes all owned descriptors.
     */
    ~DescriptorsHolder() throw ();

    /**
     * Add at end of descriptors array open file descriptor.
     * @param fd must be open file descriptor
     */
    void
    push_back(int fd) throw (Exception);

    /**
     * For redefining aims we will close descriptors consequently
     * from end of DescriptorsHolder.
     * @return the last descriptor available.
     */
    int
    pop_back(Error& error) throw ();

    /**
     * Close all owned descriptors. Call before destruction to get
     * potential closing troubles error info.
     */
    void
    close_all(Error& error) throw ();

    /**
     * Close all owned descriptors. Call before destruction to get
     * potential closing troubles error info.
     */
    void
    close_all() throw (Exception);

    /**
     * Get descriptors array pointer, need to placing at
     * DescriptorListener disposal.
     * @return pointer to descriptors array
     */
    int*
    get() const throw ();

    /**
     * Find fd element in descriptors container.
     * @param fd number for finding in descriptors container.
     * @return pointer to found element, 0 if doesn't contain
     * such element.
     */
    int*
    find(int fd) const throw ();

    /**
     * Get count of open descriptors that owned by descriptor object
     * @return current number of descriptors
     */
    size_t
    count() const throw ();

  private:
    /**
     * Close descriptors from in (begin, end) range.
     * @param begin range start pointer
     * @param end range end pointer
     */
    void
    close_(const int* begin, const int* end, Error& error)
      throw (eh::Exception);

    Generics::ArrayAutoPtr<int> descriptors_;
    size_t used_count_;
    size_t allocated_size_;
  };


  DescriptorsHolder::DescriptorsHolder(size_t count)
    throw (eh::Exception)
    : descriptors_(count), used_count_(0), allocated_size_(count)
  {
  }

  DescriptorsHolder::~DescriptorsHolder() throw ()
  {
    if (used_count_)
    {
      Error error = "";
      close_all(error);
    }
  }

  void
  DescriptorsHolder::push_back(int fd) throw (Exception)
  {
    if (used_count_ == allocated_size_)
    {
      Stream::Error ostr;
      ostr << FNS << "exhausted place to store descriptor";
      throw Exception(ostr);
    }
    descriptors_[used_count_++] = fd;
  }

  int
  DescriptorsHolder::pop_back(Error& error) throw ()
  {
    if (used_count_ == 0)
    {
      String::StringManip::concat(error, sizeof(error),
        FNE, "has not descriptors on hold");
      return -1;
    }
    return descriptors_[--used_count_];
  }

  int*
  DescriptorsHolder::find(int fd) const throw ()
  {
    for (int* p_fd = descriptors_.get();
      p_fd < descriptors_.get() + used_count_; ++p_fd)
    {
      if (*p_fd == fd)
      {
        return p_fd;
      }
    }
    return 0;
  }

  void
  DescriptorsHolder::close_all(Error& error) throw ()
  {
    close_(descriptors_.get(), descriptors_.get() + used_count_, error);
    used_count_ = 0;
  }

  void
  DescriptorsHolder::close_all() throw (Exception)
  {
    Error error = "";
    close_all(error);
    if (*error)
    {
      throw Exception(error);
    }
  }

  int*
  DescriptorsHolder::get() const throw ()
  {
    return descriptors_.get();
  }

  size_t
  DescriptorsHolder::count() const throw ()
  {
    return used_count_;
  }

  void
  DescriptorsHolder::close_(const int* begin, const int* end, Error& error)
    throw (eh::Exception)
  {
    for (const int* fd = begin; fd != end; fd++)
    {
      if (close(*fd) == -1)
      {
        Error string;
        eh::ErrnoHelper::compose_safe(string, sizeof(string), errno,
          FNE, "error closing descriptor ", *fd);

        if (*error)
        {
          String::StringManip::strlcat(error, "\n", sizeof(error));
        }
        String::StringManip::strlcat(error, string, sizeof(error));
      }
    }
  }


  Sync::PosixMutex execute_and_listen_mutex;


  void
  create_pipes(bool error_pipe, size_t descriptors_amount,
    DescriptorsHolder& read_descriptors, DescriptorsHolder& write_descriptors)
    throw (eh::Exception)
  {
    int error_piped[2];
    if (error_pipe)
    {
      if (pipe(error_piped) == -1)
      {
        eh::throw_errno_exception<
          Generics::DescriptorListener::SysCallFailure>(FNE, "pipe fail");
      }
      write_descriptors.push_back(error_piped[1]);
    }

    try
    {
      for (size_t i = 0; i < descriptors_amount; i++)
      {
        int piped[2];
        if (pipe(piped) == -1)
        {
          eh::throw_errno_exception<
            Generics::DescriptorListener::SysCallFailure>(FNE, "pipe fail");
        }
        read_descriptors.push_back(piped[0]);
        write_descriptors.push_back(piped[1]);
      }
    }
    catch (...)
    {
      if (error_pipe)
      {
        read_descriptors.push_back(error_piped[0]);
      }
      throw;
    }

    if (error_pipe)
    {
      read_descriptors.push_back(error_piped[0]);
    }
  }

  void
  child(const char* program_name, char* const argv[],
    size_t descriptors_amount, const int descriptors[],
    size_t redirect_descriptors_amount, const int redirect_descriptors[],
    bool error_pipe, int devnull,
    DescriptorsHolder& read_descriptors, DescriptorsHolder& write_descriptors)
    throw ()
  {
    // Only reenterable functions are allowed here.
    // STL and other stuff could be non-reenterable (esp. streams).
    // Exceptions are not fork()-compatible too

    DescriptorsHolder::Error error = "";

    setpgrp();

    // Close unused read end
    read_descriptors.close_all(error);

    if (!*error)
    {
      // redirect descriptors to /dev/null
      for (unsigned i = 0; i < redirect_descriptors_amount; i++)
      {
        if (dup2(devnull, redirect_descriptors[i]) < 0)
        {
          eh::ErrnoHelper::compose_safe(error, sizeof(error), errno,
            FNE, "dup2 failed");
          break;
        }
      }
    }

    if (!*error)
    {
      // redefine supplied descriptors with created pipes
      for (int i = descriptors_amount - 1; i >= 0; i--)
      {
        // To avoid possible clash between parent and child.
        // We must check, that don't redefined yet piping descriptors
        // do not equal child application descriptor, that we will forward
        // to write_descriptor on current iteration.
        // If equal - we dup application descriptor (the same as pipe).
        // Save descriptor from closing by dup2.

        int write_descriptor = write_descriptors.pop_back(error);
        if (*error)
        {
          break;
        }

        if (write_descriptor == descriptors[i])
        {
          continue;
        }
        if (int* clash_with_pipes = write_descriptors.find(descriptors[i]))
        {
          // avoid descriptors[i] closing by dup2 call.
          int new_fd = dup(descriptors[i]);
          if (new_fd == -1)
          {
            eh::ErrnoHelper::compose_safe(error, sizeof(error), errno,
              FNE, "dup failed");
            break;
          }
          *clash_with_pipes = new_fd;
        }
        // simple forward descriptors in our pipes
        if (dup2(write_descriptor, descriptors[i]) == -1)
        {
          eh::ErrnoHelper::compose_safe(error, sizeof(error), errno,
            FNE, "dup2 failed");
          break;
        }
        if (close(write_descriptor) == -1)
        {
          eh::ErrnoHelper::compose_safe(error, sizeof(error), errno,
              FNE, "close failed");
          break;
        }
      }
    }

    if (!*error)
    {
      if (error_pipe)
      {
        if (Generics::set_cloexec(*write_descriptors.get()) < 0)
        {
          eh::ErrnoHelper::compose_safe(error, sizeof(error), errno,
              FNE, "set_cloexec failed");
        }
      }
    }

    if (!*error)
    {
      // and execute the program
      execvp(program_name, argv);

      eh::ErrnoHelper::compose_safe(error, sizeof(error), errno,
        FNE, "execvp failed for '", program_name, "'");
    }

    assert(*error);

    if (error_pipe)
    {
      write(*write_descriptors.get(), error, strlen(error));
    }

    //write_descriptors.close_all(error);

    _exit(255);
  }
}

namespace Generics
{
  //
  // class DescriptorListenerCallback
  //

  void
  DescriptorListenerCallback::on_all_closed() throw ()
  {
    if (listener())
    {
      listener()->terminate();
      listener(0);
    }
  }


  //
  // class DescriptorListener
  //

  const Time DescriptorListener::PERIOD = Time::ONE_SECOND;

  DescriptorListener::DescriptorListener(DescriptorListenerCallback* callback,
    const int* descriptors, size_t dscs_amount,
    size_t buffers_size, bool full_lines_only)
    throw (InvalidArgument, SysCallFailure, EventFailure, eh::Exception)
    : callback_(ReferenceCounting::add_ref(callback)),
      DESCRIPTORS_COUNT_(dscs_amount), BUFFERS_LENGTH_(buffers_size),
      FULL_LINES_ONLY_(full_lines_only), read_contexts_(DESCRIPTORS_COUNT_),
      closed_descriptors_(0)
  {
    if (!buffers_size)
    {
      Stream::Error ostr;
      ostr << FNS << "buffer_size is zero";
      throw InvalidArgument(ostr);
    }

    // Initialize the event library
    base_ = event_base_new();
    if (base_ == 0)
    {
      eh::throw_errno_exception<EventFailure>(FNE,
        "event_base_new() failed.");
    }

    try
    {
      // initialize the members of the event structure
      event_set(&termination_, termination_pipe_.read_descriptor(),
        EV_READ, terminate_callback_, this);
      event_base_set(base_, &termination_);
      if (event_add(&termination_, 0) == -1)
      {
        eh::throw_errno_exception<EventFailure>(FNE,
          "event_add(termination_) failed.");
      }

      // periodic callback
      evtimer_set(&periodic_, periodic_callback_, this);
      event_base_set(base_, &periodic_);
      if (evtimer_add(&periodic_, &PERIOD) == -1)
      {
        eh::throw_errno_exception<EventFailure>(FNE,
          "event_add(periodic_) failed.");
      }

      // set descriptors for dispatching
      for (size_t i = 0; i < DESCRIPTORS_COUNT_; i++)
      {
        int flags = fcntl(descriptors[i], F_GETFL);
        if (flags == -1 ||
          fcntl(descriptors[i], F_SETFL, flags | O_NONBLOCK) == -1)
        {
          eh::throw_errno_exception<SysCallFailure>(FNE, "fcntl() failed");
        }
        read_contexts_[i].init(base_, this, BUFFERS_LENGTH_,
          descriptors[i]);
      }
    }
    catch (...)
    {
      event_base_free(base_);
      throw;
    }
  }

  DescriptorListener::~DescriptorListener() throw ()
  {
    event_base_free(base_);
  }

  void
  DescriptorListener::terminate() throw ()
  {
    termination_pipe_.signal();
  }

  void
  DescriptorListener::read_callback_(int fd, short /*type*/, void* arg)
    throw ()
  {
    DescriptorActionContext* context =
      static_cast<DescriptorActionContext*>(arg);
    context->owner->handle_read_(fd, *context);
  }

  void
  DescriptorListener::terminate_callback_(int /*fd*/, short /*type*/,
    void* arg) throw ()
  {
    DescriptorListener* listener = static_cast<DescriptorListener*>(arg);
    if (event_base_loopexit(listener->base_, 0) == -1)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't stop event dispatching.";
      listener->callback_->error(ostr.str());
    }
  }

  void
  DescriptorListener::periodic_callback_(int /*fd*/, short /*type*/,
    void* arg) throw ()
  {
    DescriptorListener* listener = static_cast<DescriptorListener*>(arg);
    listener->callback_->on_periodic();
    if (evtimer_add(&listener->periodic_, &PERIOD) == -1)
    {
      Stream::Error ostr;
      ostr << FNS << "event_add(periodic_) failed.";
      listener->callback_->error(ostr.str());
    }
  }

  void
  DescriptorListener::handle_read_(int fd, DescriptorActionContext& context)
    throw ()
  {
    for (;;)
    {
      ssize_t res;
      if (!FULL_LINES_ONLY_)
      {
        res = read(fd, context.buffer.get(), BUFFERS_LENGTH_);
      }
      else
      {
        res = read(fd, context.buffer.get() + context.used_buffer,
          BUFFERS_LENGTH_ - context.used_buffer);
      }

      int error = 0;
      switch (res)
      {
      case -1:
        if (errno == EINTR)
        {
          continue;
        }

        // All data read, and we are finishing callback...
        if (errno == EAGAIN)
        {
          return;
        }
        error = errno;

      case 0:
        event_del(&context.read_event);
        if (context.used_buffer)
        {
          callback_->on_data_ready(fd, &context - read_contexts_.get(),
            context.buffer.get(), context.used_buffer);
        }
        callback_->on_closed(fd, &context - read_contexts_.get(), error);
        if (++closed_descriptors_ == DESCRIPTORS_COUNT_)
        {
          callback_->on_all_closed();
        }
        return;

      default:
        break;
      }

      if (FULL_LINES_ONLY_)
      {
        // 1. buffer can contain rest of previous read (without \n).
        const char* chunk = context.buffer.get() + context.used_buffer;
        const char* line_start = context.buffer.get();
        while (const char* line_end =
          static_cast<const char*>(memchr(chunk, '\n', res)))
        {
          // found new line
          callback_->on_data_ready(fd, &context - read_contexts_.get(),
            line_start, line_end - line_start + 1);
          res -= line_end - chunk + 1;
          line_start = chunk = line_end + 1;
        }
        // check overflow
        if (context.used_buffer + res == BUFFERS_LENGTH_)
        {
          callback_->on_data_ready(fd, &context - read_contexts_.get(),
            line_start, BUFFERS_LENGTH_);
          context.used_buffer = 0;
          continue;
        }
        // if had new lines
        if (chunk != context.buffer.get() + context.used_buffer)
        {
          memmove(context.buffer.get(), chunk, res);
          context.used_buffer = res;
        }
        else
        {
          context.used_buffer += res;
        }
      }
      else
      {
        callback_->on_data_ready(fd, &context - read_contexts_.get(),
          context.buffer.get(), res);
      }
    }
  }

  void
  DescriptorListener::listen() throw (eh::Exception, EventFailure)
  {
    // loop and dispatch events
    if (event_base_dispatch(base_) < 0)
    {
      eh::throw_errno_exception<EventFailure>(FNE,
        "event_base_dispatch() failure");
    }
  }


  //
  // class DescriptorListener::DescriptorActionContext
  //

  void
  DescriptorListener::DescriptorActionContext::init(event_base* base,
    DescriptorListener* host, size_t buffer_size, int descriptor)
    throw (EventFailure, eh::Exception)
  {
    owner = host;
    buffer.reset(buffer_size);
    used_buffer = 0;

    // initialize the members of the event structure
    event_set(&read_event, descriptor, EV_READ | EV_PERSIST, read_callback_,
      this);
    event_base_set(base, &read_event);
    if (event_add(&read_event, 0) == -1)
    {
      eh::throw_errno_exception<EventFailure>(FNE, "event_add() failed.");
    }
  }


  //
  // class ActiveDescriptorListenerCallback
  //

  void
  ActiveDescriptorListenerCallback::on_all_closed() throw ()
  {
    if (listener())
    {
      listener()->deactivate_object();
      listener(ActiveDescriptorListener_var());
    }
  }


  //
  // ActiveDescriptorListener::ListenerJob::DLCAdapter class
  //

  ActiveDescriptorListener::ListenerJob::DLCAdapter::DLCAdapter(
    ActiveDescriptorListenerCallback* active_callback) throw ()
    : active_callback_(ReferenceCounting::add_ref(active_callback))
  {
  }

  ActiveDescriptorListener::ListenerJob::DLCAdapter::~DLCAdapter() throw ()
  {
  }

  void
  ActiveDescriptorListener::ListenerJob::DLCAdapter::active_listener(
    ActiveDescriptorListener* active_listener) throw ()
  {
    active_callback_->listener(ActiveDescriptorListener_var(
      ReferenceCounting::add_ref(active_listener)));
  }

  void
  ActiveDescriptorListener::ListenerJob::DLCAdapter::on_data_ready(
    int fd, size_t fd_index, const char* buf, size_t size) throw ()
  {
    active_callback_->on_data_ready(fd, fd_index, buf, size);
  }

  void
  ActiveDescriptorListener::ListenerJob::DLCAdapter::on_closed(
    int fd, size_t fd_index, int error) throw ()
  {
    active_callback_->on_closed(fd, fd_index, error);
  }

  void
  ActiveDescriptorListener::ListenerJob::DLCAdapter::on_all_closed()
    throw ()
  {
    active_callback_->on_all_closed();
  }

  void
  ActiveDescriptorListener::ListenerJob::DLCAdapter::report_error(
    ActiveObjectCallback::Severity severity,
    const String::SubString& description, const char* error_code) throw ()
  {
    active_callback_->report_error(severity, description,
      error_code);
  }


  //
  // class ActiveDescriptorListener::ListenerJob
  //

  ActiveDescriptorListener::ListenerJob::ListenerJob(
    ActiveDescriptorListenerCallback* callback,
    const int* descriptors, size_t number_of_descriptors,
    size_t buffers_size, bool full_lines_only)
    throw (eh::Exception)
    : SingleJob(callback),
      DescriptorListener(DLCAdapter_var(new DLCAdapter(callback)),
        descriptors, number_of_descriptors, buffers_size, full_lines_only)
  {
  }

  ActiveDescriptorListener::ListenerJob::~ListenerJob() throw ()
  {
  }

  void
  ActiveDescriptorListener::ListenerJob::active_listener(
    ActiveDescriptorListener* active_listener) throw ()
  {
    static_cast<DLCAdapter&>(*DescriptorListener::callback_).active_listener(
      active_listener);
  }

  void
  ActiveDescriptorListener::ListenerJob::terminate() throw ()
  {
    try
    {
      DescriptorListener::terminate();
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "failed to terminate DescriptorListener: " << ex.what();
      callback()->error(ostr.str());
    }
  }

  void
  ActiveDescriptorListener::ListenerJob::work() throw ()
  {
    try
    {
      listen();
    }
    catch (const eh::Exception& ex)
    {
      callback()->error(String::SubString(ex.what()));
    }
  }


  //
  // class ActiveDescriptorListener
  //

  ActiveDescriptorListener::ActiveDescriptorListener(
    ActiveDescriptorListenerCallback* callback,
    const int* descriptors, size_t number_of_descriptors,
    size_t buffers_size, bool full_lines_only)
    throw (eh::Exception)
    : ActiveObjectCommonImpl(
        ListenerJob_var(new ListenerJob(callback, descriptors,
          number_of_descriptors, buffers_size, full_lines_only)), 1)
  {
    static_cast<ListenerJob&>(*SINGLE_JOB_).active_listener(this);
  }

  ActiveDescriptorListener::~ActiveDescriptorListener() throw ()
  {
  }


  //
  // ExecuteAndListenCallback class
  //

  void
  ExecuteAndListenCallback::set_pid(pid_t /*pid*/) throw ()
  {
  }


  //
  // functions
  //

  int
  execute_and_listen(ExecuteAndListenCallback* callback,
    const char* program_name, char* const argv[],
    size_t descriptors_amount, const int descriptors[],
    size_t redirect_descriptors_amount, const int redirect_descriptors[],
    size_t listener_buffers_size, bool listener_full_lines_only,
    bool error_pipe)
    throw (eh::Exception)
  {
    // create pipes for DescriptorListener, we redefine
    // descriptors later to now creating descriptors

    int devnull = redirect_descriptors_amount ?
      Singleton<DevNull>::instance().fd() : -1;

    size_t full_descriptors_amount = error_pipe ? descriptors_amount + 1 :
      descriptors_amount;
    DescriptorsHolder read_descriptors(full_descriptors_amount);
    DescriptorsHolder write_descriptors(full_descriptors_amount);

    std::unique_ptr<DescriptorListener> dl;
    pid_t cpid;

    {
      // This mutex prevents others processes except the one we are making
      // right now to inherit write ends of pipes.
      // Unfortunately processes created by means other than
      // execute_and_listen will inherit them.

      Sync::PosixGuard guard(execute_and_listen_mutex);

      create_pipes(error_pipe, descriptors_amount,
        read_descriptors, write_descriptors);

      // This may raise an exception, doing it before fork
      dl.reset(new DescriptorListener(callback, read_descriptors.get(),
        full_descriptors_amount, listener_buffers_size,
        listener_full_lines_only));

      callback->listener(dl.get());

      cpid = fork();
      if (cpid == -1)
      {
        eh::throw_errno_exception<DescriptorListener::SysCallFailure>(
          FNE, "fork failed");
      }

      if (cpid == 0)
      {
        // Child

        child(program_name, argv, descriptors_amount, descriptors,
          redirect_descriptors_amount, redirect_descriptors,
          error_pipe, devnull, read_descriptors, write_descriptors);
      }

      // Mutex is released here
    }


    // Parent

    write_descriptors.close_all();

    callback->set_pid(cpid);

    dl->listen();
    read_descriptors.close_all();

    int status;
    if (waitpid(cpid, &status, 0) == -1)
    {
      eh::throw_errno_exception<DescriptorListener::SysCallFailure>(
        FNE, "waitpid() failed.");
    }
    return status;
  }
}
