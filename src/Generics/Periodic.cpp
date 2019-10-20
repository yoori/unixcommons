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



#include <poll.h>

#include <iostream>

#include <Generics/Function.hpp>
#include <Generics/Periodic.hpp>

#include <Stream/MemoryStream.hpp>

//#define BUILD_WITH_DEBUG_MESSAGES
#include "Trace.hpp"


namespace Generics
{
  //
  // PeriodicTask class
  //

  PeriodicTask::PeriodicTask(const Generics::Time& period)
    throw (eh::Exception)
    : period_(period), quit_(false), start_(false)
  {
  }

  PeriodicTask::~PeriodicTask() throw ()
  {
  }

  void
  PeriodicTask::set_period(const Generics::Time& period) throw ()
  {
    Sync::PosixGuard guard(mutex_);
    period_ = period;
  }

  Generics::Time
  PeriodicTask::wait_period(const Generics::Time& elapsed) const throw ()
  {
    Sync::PosixGuard guard(mutex_);
    return period_ > elapsed ? period_ - elapsed : Time::ZERO;
  }

  void
  PeriodicTask::enforce_start() throw (eh::Exception)
  {
    trace_message(FNB, this);
    Sync::PosixGuard guard(mutex_);
    start_ = true;
    cond_.signal();
  }

  void
  PeriodicTask::stop() throw (eh::Exception)
  {
    trace_message(FNB, this);
    Sync::PosixGuard guard(mutex_);
    quit_ = true;
    cond_.signal();
  }

  void
  PeriodicTask::run_once(ActiveObjectCallback* callback, bool forced)
    throw ()
  {
    trace_message(FNB, this);
    try
    {
      task(forced);
    }
    catch (const eh::Exception& ex)
    {
      if (callback)
      {
        Stream::Error ostr;
        ostr << FNS << "task() failed: " << ex.what();
        callback->warning(ostr.str());
      }
    }
  }

  void
  PeriodicTask::run(ActiveObjectCallback* callback) throw ()
  {
    trace_message(FNB, this);

    Timer timer;

    Generics::Time elapsed;

    for (;;)
    {
      bool forced = false;
      Generics::Time wait(wait_period(elapsed));
      do
      {
        timer.start();
        try
        {
          Sync::ConditionalGuard guard(cond_, mutex_);
          guard.timed_wait(&wait, true);
        }
        catch (const eh::Exception& ex)
        {
          if (callback)
          {
            Stream::Error ostr;
            ostr << FNS << "failed to read the signal: " << ex.what();
            callback->error(ostr.str());
          }
        }
        if (quit_)
        {
          trace_message(FNB, "exiting");
          quit_ = false;
          return;
        }
        if (start_)
        {
          trace_message(FNB, "breaking");
          forced = true;
          start_ = false;
          break;
        }
        timer.stop();
        elapsed += timer.elapsed_time();
        wait = wait_period(elapsed);
      }
      while (wait > Time::ZERO);

      timer.start();
      run_once(callback, forced);
      timer.stop();

      elapsed = timer.elapsed_time();
    }
  }


  //
  // PeriodicRunner::PeriodicJob class
  //

  PeriodicRunner::PeriodicJob::PeriodicJob(ActiveObjectCallback* callback,
    PeriodicTask* task) throw ()
    : callback_(ReferenceCounting::add_ref(callback)),
      task_(ReferenceCounting::add_ref(task))
  {
  }

  PeriodicRunner::PeriodicJob::~PeriodicJob() throw ()
  {
  }

  void
  PeriodicRunner::PeriodicJob::work() throw ()
  {
    task_->run(callback_);
  }

  void
  PeriodicRunner::PeriodicJob::signal(void (PeriodicTask::*signal)())
    throw (eh::Exception)
  {
    (task_->*signal)();
  }


  //
  // PeriodicRunner class
  //

  PeriodicRunner::PeriodicRunner(ActiveObjectCallback* callback,
    std::size_t stack_size) throw (eh::Exception)
    : callback_(ReferenceCounting::add_ref(callback)),
      active_state_(AS_NOT_ACTIVE),
      stack_size_(stack_size)
  {
  }

  void
  PeriodicRunner::add_task(PeriodicTask* task, bool silent, bool run)
    throw (eh::Exception)
  {
    PeriodicJob_var job(new PeriodicJob(callback_, task));
    if (run)
    {
      if (silent)
      {
        task->run_once(callback_, false);
      }
      else
      {
        task->task(false);
      }
    }
    jobs_.push_back(std::move(job));
  }

  void
  PeriodicRunner::signal_all_(void (PeriodicTask::*signal)())
    throw (eh::Exception)
  {
    trace_message(FNB, this);
    for (PeriodicJobs::iterator itor(jobs_.begin());
      itor != jobs_.end(); ++itor)
    {
      (*itor)->signal(signal);
    }
  }

  PeriodicRunner::~PeriodicRunner() throw ()
  {
    try
    {
      Stream::Error ostr;
      bool error = false;

      {
        Sync::PosixGuard guard(work_mutex_);

        if (active_state_ == AS_ACTIVE)
        {
          ostr << FNS << "wasn't deactivated.";
          error = true;
        }

        if (active_state_ != AS_NOT_ACTIVE)
        {
          if (error)
          {
            ostr << std::endl;
          }
          ostr << FNS << "didn't wait for deactivation, still active.";
          error = true;
        }
      }

      if (error)
      {
        signal_all_(&PeriodicTask::stop);

        thread_runner_->wait_for_completion();

        if (callback_)
        {
          callback_->warning(ostr.str());
        }
        else
        {
          std::cerr << ostr << std::endl;
        }
      }
    }
    catch (const eh::Exception& ex)
    {
      try
      {
        std::cerr << FNS << "eh::Exception: " << ex.what() << std::endl;
      }
      catch (...)
      {
        // Nothing we can do
      }
    }
  }

  void
  PeriodicRunner::activate_object()
    throw (AlreadyActive, Exception, eh::Exception)
  {
    trace_message(FNB, this);
    Sync::PosixGuard guard(work_mutex_);

    if (active_state_ != AS_NOT_ACTIVE)
    {
      Stream::Error ostr;
      ostr << FNS << "still active";
      throw ActiveObject::AlreadyActive(ostr);
    }

    try
    {
      thread_runner_.reset(new ThreadRunner(jobs_.begin(), jobs_.end(),
        ThreadRunner::Options(stack_size_)));
      active_state_ = AS_ACTIVE;
      thread_runner_->start();
    }
    catch (const eh::Exception& ex)
    {
      active_state_ = AS_NOT_ACTIVE;

      Stream::Error ostr;
      ostr << FNS << "start failure: " << ex.what();
      throw Exception(ostr);
    }
  }

  void
  PeriodicRunner::deactivate_object()
    throw (Exception, eh::Exception)
  {
    trace_message(FNB, this);
    Sync::PosixGuard guard(work_mutex_);
    if (active_state_ == AS_ACTIVE)
    {
      active_state_ = AS_DEACTIVATING;
      signal_all_(&PeriodicTask::stop);
    }
  }

  void
  PeriodicRunner::wait_object() throw (Exception, eh::Exception)
  {
    trace_message(FNB, this);
    Sync::PosixGuard termination_guard(termination_mutex_);
    if (active_state_ != AS_NOT_ACTIVE)
    {
      try
      {
        thread_runner_->wait_for_completion();
      }
      catch (const eh::Exception& ex)
      {
        Stream::Error ostr;
        ostr << FNS << "waiting failure: " << ex.what();
        throw Exception(ostr);
      }
    }

    Sync::PosixGuard guard(work_mutex_);
    if (active_state_ == AS_DEACTIVATING)
    {
      active_state_ = AS_NOT_ACTIVE;
    }
  }

  void
  PeriodicRunner::clear() throw (eh::Exception)
  {
    trace_message(FNB, this);
    Sync::PosixGuard guard(work_mutex_);
    jobs_.clear();
  }

  bool
  PeriodicRunner::active() throw (eh::Exception)
  {
    return active_state_ == AS_ACTIVE;
  }

  void
  PeriodicRunner::enforce_start_all() throw (eh::Exception)
  {
    trace_message(FNB, this);
    Sync::PosixGuard guard(work_mutex_);
    signal_all_(&PeriodicTask::enforce_start);
  }
}
