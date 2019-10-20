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



// Condition.cpp

#include <eh/Errno.hpp>

#include <Sync/Condition.hpp>

#include <Generics/Function.hpp>


namespace Sync
{
  //
  // class Conditional
  //

  Conditional::~Conditional() throw ()
  {
    pthread_cond_destroy(&cond_);
  }

  void
  Conditional::wait(pthread_mutex_t& mutex)
    throw (Exception, eh::Exception)
  {
    // When we call pthread_cond_wait mutex_ should be locked,
    // otherwise - U.B.
    // pthread_cond_wait releases the mutex_ and blocks the thread
    // until another thread calls signal().
    // Mutex is used to protect data required for condition calculation.
    const int RES = pthread_cond_wait(&cond_, &mutex);
    if (RES)
    {
      eh::throw_errno_exception<Exception>(RES, FNE,
        "Failed to wait on condition");
    }
  }

  bool
  Conditional::timed_wait(pthread_mutex_t& mutex,
    const Generics::Time* time, bool time_is_relative)
    throw (Exception, eh::Exception)
  {
    if (!time)
    {
      wait(mutex);
      return true;
      //We don't reach INFINITY time and cannot reach timeout
    }
    Generics::Time real_time(time_is_relative ?
      Generics::Time::get_time_of_day() + *time : *time);
    // When we call pthread_cond_wait mutex_ should be locked,
    // otherwise - U.B.
    // pthread_cond_wait releases the mutex_ and blocks the thread
    // until another thread calls signal().
    // Mutex is used to protect data required for condition calculation.
    const timespec RESTRICT =
      { real_time.tv_sec, real_time.tv_usec * 1000 };
    const int RES = pthread_cond_timedwait(&cond_, &mutex, &RESTRICT);
    if (RES == ETIMEDOUT)
    {
      return false;
    }
    if (RES)
    {
      eh::throw_errno_exception<Exception>(RES, FNE,
        "Failed to wait on condition");
    }
    return true;
  }

  void
  Conditional::signal() throw (Exception, eh::Exception)
  {
    // When we call pthread_cond_signal mutex_ should be locked,
    // otherwise - U.B.
    const int RES = pthread_cond_signal(&cond_);
    if (RES)
    {
      eh::throw_errno_exception<Exception>(RES, FNE,
        "Failed to signal condition");
    }
  }

  void
  Conditional::broadcast() throw (Exception, eh::Exception)
  {
    // When we call pthread_cond_blodcast mutex_ should be locked,
    // otherwise - U.B.
    const int RES = pthread_cond_broadcast(&cond_);
    if (RES)
    {
      eh::throw_errno_exception<Exception>(RES, FNE,
        "Failed to broadcast condition");
    }
  }


  //
  // class Condition
  //

  void
  Condition::wait() throw (Exception, eh::Exception)
  {
    Conditional::wait(*this);
  }

  bool
  Condition::timed_wait(const Generics::Time* time, bool time_is_relative)
    throw (Exception, eh::Exception)
  {
    return Conditional::timed_wait(*this, time, time_is_relative);
  }


  //
  // class ConditionalGuard
  //

  ConditionalGuard::ConditionalGuard(Condition& condition)
    throw ()
    : PosixGuard(condition), conditional_(condition), mutex_(condition)
  {
  }

  ConditionalGuard::ConditionalGuard(Conditional& conditional,
    pthread_mutex_t& mutex)
    throw ()
    : PosixGuard(mutex), conditional_(conditional), mutex_(mutex)
  {
  }

  void
  ConditionalGuard::wait()
    throw (Conditional::Exception, eh::Exception)
  {
    conditional_.wait(mutex_);
  }

  bool
  ConditionalGuard::timed_wait(const Generics::Time* time,
    bool time_is_relative)
    throw (Conditional::Exception, eh::Exception)
  {
    return conditional_.timed_wait(mutex_, time, time_is_relative);
  }
}
