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



#ifndef SYNC_SEMAPHORE_HPP
#define SYNC_SEMAPHORE_HPP

#include <semaphore.h>

#include <eh/Errno.hpp>

#include <Generics/Uncopyable.hpp>
#include <Generics/Time.hpp>


namespace Sync
{
  class Semaphore : private Generics::Uncopyable
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    explicit
    Semaphore(int count) throw (Exception);
    ~Semaphore() throw ();

    void
    acquire() throw (Exception);

    bool
    try_acquire() throw (Exception);

    bool
    timed_acquire(const Generics::Time* time, bool time_is_relative = false)
      throw (Exception);

    void
    release() throw (Exception);

    int
    value() throw (Exception);

  private:
    sem_t semaphore_;
  };
}

namespace Sync
{
  inline
  Semaphore::Semaphore(int count) throw (Exception)
  {
    if (sem_init(&semaphore_, 0, count))
    {
      eh::throw_errno_exception<Exception>(FNE,
        "Failed to initialize semaphore");
    }
  }

  inline
  Semaphore::~Semaphore() throw ()
  {
    sem_destroy(&semaphore_);
  }

  inline
  void
  Semaphore::acquire() throw (Exception)
  {
    for (;;)
    {
      if (!sem_wait(&semaphore_))
      {
        break;
      }
      if (errno != EINTR)
      {
        eh::throw_errno_exception<Exception>(FNE,
          "Failed to wait on semaphore");
      }
    }
  }

  inline
  bool
  Semaphore::try_acquire() throw (Exception)
  {
    if (!sem_trywait(&semaphore_))
    {
      return true;
    }
    if (errno != EAGAIN)
    {
      eh::throw_errno_exception<Exception>(FNE,
        "Failed to wait on semaphore");
    }
    return false;
  }

  inline
  bool
  Semaphore::timed_acquire(const Generics::Time* time,
    bool time_is_relative) throw (Exception)
  {
    if (!time)
    {
      acquire();
      return true;
    }
    Generics::Time real_time(time_is_relative ?
      Generics::Time::get_time_of_day() + *time : *time);
    const timespec RESTRICT =
      { real_time.tv_sec, real_time.tv_usec * 1000 };
    while (sem_timedwait(&semaphore_, &RESTRICT) < 0)
    {
      if (errno == ETIMEDOUT)
      {
        return false;
      }
      if (errno != EINTR)
      {
        eh::throw_errno_exception<Exception>(FNE,
          "Failed to wait on semaphore");
      }
    }
    return true;
  }

  inline
  void
  Semaphore::release() throw (Exception)
  {
    if (sem_post(&semaphore_))
    {
      eh::throw_errno_exception<Exception>(FNE,
        "Failed to release semaphore");
    }
  }

  inline
  int
  Semaphore::value() throw (Exception)
  {
    int value;
    if (sem_getvalue(&semaphore_, &value) < 0)
    {
      eh::throw_errno_exception<Exception>(FNE,
        "Failed to value semaphore");
    }
    return value;
  }
}

#endif
