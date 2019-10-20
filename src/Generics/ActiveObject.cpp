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



// Generics/ActiveObject.cpp

#include <iostream>

#include <Generics/ActiveObject.hpp>

//#define BUILD_WITH_DEBUG_MESSAGES
#include "Trace.hpp"


namespace Generics
{
  //
  // ActiveObject class
  //

  const char ActiveObject::PRINTABLE_NAME[] = "Generics::ActiveObject";


  //
  // SimpleActiveObject class
  //

  SimpleActiveObject::~SimpleActiveObject() throw ()
  {
    if (state_ != AS_NOT_ACTIVE)
    {
      std::cerr << "SimpleActiveObject is not deactivated" << std::endl;
    }
  }

  void
  SimpleActiveObject::activate_object()
    throw (AlreadyActive, Exception, eh::Exception)
  {
    {
      Sync::PosixGuard guard(cond_);
      if (state_ == AS_NOT_ACTIVE)
      {
        activate_object_();
        state_ = AS_ACTIVE;
        return;
      }
    }
    Stream::Error ostr;
    ostr << FNS << "already active";
    throw AlreadyActive(ostr);
  }

  void
  SimpleActiveObject::deactivate_object() throw (Exception, eh::Exception)
  {
    Sync::PosixGuard guard(cond_);
    if (state_ != AS_ACTIVE)
    {
      return;
    }
    state_ = AS_DEACTIVATING;
    cond_.broadcast();
    try
    {
      deactivate_object_();
    }
    catch (...)
    {
      state_ = AS_ACTIVE;
      throw;
    }
  }

  void
  SimpleActiveObject::wait_object() throw (Exception, eh::Exception)
  {
    {
      Sync::ConditionalGuard guard(cond_);
      while (state_ == AS_ACTIVE || wait_more_())
      {
        guard.wait();
      }
    }
    wait_object_();
    Sync::ConditionalGuard guard(cond_);
    if (state_ == AS_DEACTIVATING)
    {
      state_ = AS_NOT_ACTIVE;
    }
  }

  bool
  SimpleActiveObject::active() throw (eh::Exception)
  {
    return state_ == AS_ACTIVE;
  }

  void
  SimpleActiveObject::activate_object_() throw (Exception, eh::Exception)
  {
  }

  void
  SimpleActiveObject::deactivate_object_() throw (Exception, eh::Exception)
  {
  }

  bool
  SimpleActiveObject::wait_more_() throw (Exception, eh::Exception)
  {
    return false;
  }

  void
  SimpleActiveObject::wait_object_() throw (Exception, eh::Exception)
  {
  }


  //
  // ActiveObjectCommonImpl class
  //

  ActiveObjectCommonImpl::ActiveObjectCommonImpl(SingleJob* job,
    unsigned threads_number, size_t stack_size, unsigned start_threads)
    throw (InvalidArgument)
    : SINGLE_JOB_(ReferenceCounting::add_ref(job)),
      thread_runner_(job, threads_number,
        ThreadRunner::Options(stack_size, job->callback())),
      start_threads_(start_threads), work_mutex_(job->mutex()),
      active_state_(AS_NOT_ACTIVE)
  {
    if (!threads_number)
    {
      Stream::Error ostr;
      ostr << FNS << "threads_number == 0";
      throw InvalidArgument(ostr);
    }
  }

  ActiveObjectCommonImpl::~ActiveObjectCommonImpl() throw ()
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
        {
          Sync::PosixGuard guard(work_mutex_);
          SINGLE_JOB_->make_terminate();
        }

        thread_runner_.wait_for_completion();

        {
          Sync::PosixGuard guard(work_mutex_);
          SINGLE_JOB_->terminated();
        }

        ActiveObjectCallback_var callback = SINGLE_JOB_->callback();
        if (!callback)
        {
          std::cerr << ostr << std::endl;
        }
        else
        {
          callback->warning(ostr.str());
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
  ActiveObjectCommonImpl::activate_object()
    throw (AlreadyActive, Exception, eh::Exception)
  {
    Sync::PosixGuard guard(work_mutex_);

    if (active_state_ != AS_NOT_ACTIVE)
    {
      Stream::Error ostr;
      ostr << FNS << "still active";
      throw ActiveObject::AlreadyActive(ostr);
    }

    try
    {
      active_state_ = AS_ACTIVE;
      thread_runner_.start(start_threads_);
    }
    catch (const eh::Exception& ex)
    {
      active_state_ = AS_NOT_ACTIVE;

      Stream::Error ostr;
      ostr << FNS << "start failure: " << ex.what();
      throw Exception(ostr);
    }
    SINGLE_JOB_->started(start_threads_);

    trace_message(FNB, "activated");
  }

  void
  ActiveObjectCommonImpl::wait_object() throw (Exception, eh::Exception)
  {
    Sync::PosixGuard termination_guard(termination_mutex_);
    if (active_state_ != AS_NOT_ACTIVE)
    {
      try
      {
        thread_runner_.wait_for_completion();
        SINGLE_JOB_->terminated();
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
  ActiveObjectCommonImpl::deactivate_object()
    throw (Exception, eh::Exception)
  {
    Sync::PosixGuard guard(work_mutex_);
    if (active_state_ == AS_ACTIVE)
    {
      active_state_ = AS_DEACTIVATING;
      SINGLE_JOB_->make_terminate();
    }
  }

  bool
  ActiveObjectCommonImpl::active() throw (eh::Exception)
  {
    return active_state_ == AS_ACTIVE;
  }
} // namespace Generics
