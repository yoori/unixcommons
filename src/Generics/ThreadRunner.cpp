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



#include <eh/Errno.hpp>

#include <Generics/ThreadRunner.hpp>
#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>


namespace Generics
{
  //
  // ThreadJob class
  //

  ThreadJob::~ThreadJob() throw ()
  {
  }


  //
  // ThreadCallback class
  //

  ThreadCallback::~ThreadCallback() throw ()
  {
  }

  void
  ThreadCallback::on_start() throw ()
  {
  }

  void
  ThreadCallback::on_stop() throw ()
  {
  }


  //
  // ThreadRunner::Options class
  //

  const size_t ThreadRunner::Options::DEFAULT_STACK_SIZE;

  ThreadRunner::Options::Options(size_t stack_size,
    ThreadCallback* thread_callback) throw ()
    : stack_size(stack_size < PTHREAD_STACK_MIN ? DEFAULT_STACK_SIZE :
        stack_size),
      thread_callback(ReferenceCounting::add_ref(thread_callback))
  {
  }


  //
  // ThreadRunner::PThreadAttr class
  //

  ThreadRunner::PThreadAttr::PThreadAttr(size_t stack_size)
    throw (PosixException)
  {
    int res = ::pthread_attr_init(&attr_);
    if (res)
    {
      eh::throw_errno_exception<PosixException>(res, FNE,
        "failed to initialize attribute");
    }
    res = ::pthread_attr_setstacksize(&attr_, stack_size);
    if (res)
    {
      eh::throw_errno_exception<PosixException>(res, FNE,
        "tried to set stack size ", stack_size);
    }
  }

  ThreadRunner::PThreadAttr::~PThreadAttr() throw ()
  {
    ::pthread_attr_destroy(&attr_);
  }

  ThreadRunner::PThreadAttr::operator pthread_attr_t*() throw ()
  {
    return &attr_;
  }


  //
  // ThreadRunner class
  //

  ThreadRunner::ThreadRunner(ThreadJob* job, unsigned number_of_jobs,
    const Options& options) throw (eh::Exception, PosixException)
    : attr_(options.stack_size), thread_callback_(options.thread_callback),
      start_semaphore_(0), number_running_(0),
      number_of_jobs_(number_of_jobs), jobs_(number_of_jobs_)
  {
    for (unsigned i = 0; i < number_of_jobs_; i++)
    {
      jobs_[i].runner = this;
      jobs_[i].job = ReferenceCounting::add_ref(job);
    }
  }

  ThreadRunner::~ThreadRunner() throw ()
  {
    try
    {
      wait_for_completion();
    }
    catch (...)
    {
    }
  }

  void
  ThreadRunner::thread_func_(ThreadJob& job) throw ()
  {
    start_semaphore_.acquire();
    start_semaphore_.release();

    if (number_running_ > 0)
    {
      if (thread_callback_)
      {
        thread_callback_->on_start();
      }
      job.work();
      if (thread_callback_)
      {
        thread_callback_->on_stop();
      }
    }
  }

  void*
  ThreadRunner::thread_func_(void* arg) throw ()
  {
    JobInfo* info = static_cast<JobInfo*>(arg);
    info->runner->thread_func_(*info->job);
    return 0;
  }

  void
  ThreadRunner::start_one_thread_() throw (PosixException)
  {
    const int RES = pthread_create(&jobs_[number_running_].thread_id,
      attr_, thread_func_, &jobs_[number_running_]);
    if (RES)
    {
      eh::throw_errno_exception<PosixException>(RES, FNE, "thread start");
    }
    number_running_++;
  }

  void
  ThreadRunner::wait_for_completion() throw (PosixException)
  {
    if (number_running_)
    {
      Stream::Error ostr;
      for (int i = 0; i < abs(number_running_); i++)
      {
        const int RES = pthread_join(jobs_[i].thread_id, 0);
        if (RES)
        {
          char error[sizeof(PosixException)];
          eh::ErrnoHelper::compose_safe(error, sizeof(error), RES,
            FNE, "join failure");
          ostr << error << "\n";
        }
      }
      start_semaphore_.acquire();
      number_running_ = 0;

      const String::SubString& str = ostr.str();
      if (str.size())
      {
        throw PosixException(str);
      }
    }
  }

  void
  ThreadRunner::start(unsigned to_start)
    throw (AlreadyStarted, PosixException, eh::Exception)
  {
    if (number_running_)
    {
      Stream::Error ostr;
      ostr << FNS << "already started";
      throw AlreadyStarted(ostr);
    }

    if (!to_start || to_start > number_of_jobs_)
    {
      to_start = number_of_jobs_;
    }

    try
    {
      while (static_cast<unsigned>(number_running_) < to_start)
      {
        start_one_thread_();
      }
    }
    catch (const eh::Exception&)
    {
      number_running_ = -number_running_;
      start_semaphore_.release();
      try
      {
        wait_for_completion();
      }
      catch (const eh::Exception& ex)
      {
        abort();
      }
      throw;
    }

    start_semaphore_.release();
  }

  void
  ThreadRunner::start_one() throw (AlreadyStarted, PosixException)
  {
    if (static_cast<unsigned>(number_running_) == number_of_jobs_)
    {
      Stream::Error ostr;
      ostr << FNS << "all threads are already started";
      throw AlreadyStarted(ostr);
    }

    start_one_thread_();
  }
}
