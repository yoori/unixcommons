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



// @file Sync/PosixLock.hpp
#ifndef SYNC_POSIX_LOCK_HPP
#define SYNC_POSIX_LOCK_HPP

#include <pthread.h>

#include <Generics/Uncopyable.hpp>


namespace Sync
{
  class PosixMutex : private Generics::Uncopyable
  {
  public:
    constexpr
    PosixMutex() throw ();

    /**
     * Create mutex and set pshared attribute
     * @param pshared value to do system call pthread_mutexattr_setpshared
     */
    explicit
    PosixMutex(int pshared) throw ();

    ~PosixMutex() throw ();

    operator pthread_mutex_t&() throw ();

    void
    lock() throw ();

    void
    unlock() throw ();

  private:
    pthread_mutex_t mutex_;
  };

  class PosixGuard : private Generics::Uncopyable
  {
  public:
    explicit
    PosixGuard(pthread_mutex_t& mutex) throw ();
    ~PosixGuard() throw ();

  private:
    pthread_mutex_t& mutex_;
  };

  class PosixTryGuard : private Generics::Uncopyable
  {
  public:
    explicit
    PosixTryGuard(pthread_mutex_t& mutex) throw ();
    ~PosixTryGuard() throw ();
    operator bool() const throw ();

  private:
    pthread_mutex_t& mutex_;
    bool locked_;
  };

  class PosixRWLock : private Generics::Uncopyable
  {
  public:
    constexpr
    PosixRWLock() throw ();
    ~PosixRWLock() throw ();
    operator pthread_rwlock_t&() throw ();
    void
    lock_read() throw ();
    void
    lock_write() throw ();
    void
    unlock() throw ();

  private:
    pthread_rwlock_t lock_;
  };

  class PosixRGuard : private Generics::Uncopyable
  {
  public:
    explicit
    PosixRGuard(pthread_rwlock_t& lock) throw ();
    ~PosixRGuard() throw ();

  private:
    pthread_rwlock_t& lock_;
  };

  class PosixWGuard : private Generics::Uncopyable
  {
  public:
    explicit
    PosixWGuard(pthread_rwlock_t& lock) throw ();
    ~PosixWGuard() throw ();

  private:
    pthread_rwlock_t& lock_;
  };

  class PosixSpinLock : private Generics::Uncopyable
  {
  public:
    /**
     * Create spinlock and set pshared attribute
     * @param pshared value to do system call pthread_spin_init
     */
    explicit
    PosixSpinLock(int pshared = PTHREAD_PROCESS_PRIVATE) throw ();

    ~PosixSpinLock() throw ();

    operator pthread_spinlock_t&() throw ();

    void
    lock() throw ();

    void
    unlock() throw ();

  private:
    pthread_spinlock_t spinlock_;
  };

  class PosixSpinGuard : private Generics::Uncopyable
  {
  public:
    explicit
    PosixSpinGuard(pthread_spinlock_t& mutex) throw ();
    ~PosixSpinGuard() throw ();

  private:
    pthread_spinlock_t& spinlock_;
  };
}

//
// INLINES
//

namespace Sync
{
  //
  // PosixMutex class
  //

  inline
  constexpr
  PosixMutex::PosixMutex() throw ()
    : mutex_ PTHREAD_MUTEX_INITIALIZER
  {
  }

  inline
  PosixMutex::PosixMutex(int pshared) throw ()
  {
    pthread_mutexattr_t mutex_attributes;
    pthread_mutexattr_init(&mutex_attributes);
    pthread_mutexattr_setpshared(&mutex_attributes, pshared);
    pthread_mutex_init(&mutex_, &mutex_attributes);
  }

  inline
  PosixMutex::~PosixMutex() throw ()
  {
    pthread_mutex_destroy(&mutex_);
  }

  inline
  PosixMutex::operator pthread_mutex_t&() throw ()
  {
    return mutex_;
  }

  inline
  void
  PosixMutex::lock() throw ()
  {
    pthread_mutex_lock(&mutex_);
  }

  inline
  void
  PosixMutex::unlock() throw ()
  {
    pthread_mutex_unlock(&mutex_);
  }


  //
  // PosixGuard class
  //

  inline
  PosixGuard::PosixGuard(pthread_mutex_t& mutex) throw ()
    : mutex_(mutex)
  {
    pthread_mutex_lock(&mutex_);
  }

  inline
  PosixGuard::~PosixGuard() throw ()
  {
    pthread_mutex_unlock(&mutex_);
  }


  //
  // PosixTryGuard class
  //

  inline
  PosixTryGuard::PosixTryGuard(pthread_mutex_t& mutex) throw ()
    : mutex_(mutex), locked_(!pthread_mutex_trylock(&mutex_))
  {
  }

  inline
  PosixTryGuard::~PosixTryGuard() throw ()
  {
    if (locked_)
    {
      pthread_mutex_unlock(&mutex_);
    }
  }

  inline
  PosixTryGuard::operator bool() const throw ()
  {
    return locked_;
  }


  //
  // PosixRWLock class
  //

  inline
  constexpr
  PosixRWLock::PosixRWLock() throw ()
    : lock_ PTHREAD_RWLOCK_INITIALIZER
  {
  }

  inline
  PosixRWLock::~PosixRWLock() throw ()
  {
    pthread_rwlock_destroy(&lock_);
  }

  inline
  PosixRWLock::operator pthread_rwlock_t&() throw ()
  {
    return lock_;
  }

  inline
  void
  PosixRWLock::lock_read() throw ()
  {
    pthread_rwlock_rdlock(&lock_);
  }

  inline
  void
  PosixRWLock::lock_write() throw ()
  {
    pthread_rwlock_wrlock(&lock_);
  }

  inline
  void
  PosixRWLock::unlock() throw ()
  {
    pthread_rwlock_unlock(&lock_);
  }


  //
  // PosixRGuard class
  //

  inline
  PosixRGuard::PosixRGuard(pthread_rwlock_t& lock) throw ()
    : lock_(lock)
  {
    pthread_rwlock_rdlock(&lock_);
  }

  inline
  PosixRGuard::~PosixRGuard() throw ()
  {
    pthread_rwlock_unlock(&lock_);
  }


  //
  // PosixWGuard class
  //

  inline
  PosixWGuard::PosixWGuard(pthread_rwlock_t& lock) throw ()
    : lock_(lock)
  {
    pthread_rwlock_wrlock(&lock_);
  }

  inline
  PosixWGuard::~PosixWGuard() throw ()
  {
    pthread_rwlock_unlock(&lock_);
  }


  //
  // PosixSpinLock class
  //

  inline
  PosixSpinLock::PosixSpinLock(int pshared) throw ()
  {
    pthread_spin_init(&spinlock_, pshared);
  }

  inline
  PosixSpinLock::~PosixSpinLock() throw ()
  {
    pthread_spin_destroy(&spinlock_);
  }

  inline
  PosixSpinLock::operator pthread_spinlock_t& () throw ()
  {
    return spinlock_;
  }

  inline
  void
  PosixSpinLock::lock() throw ()
  {
    pthread_spin_lock(&spinlock_);
  }

  inline
  void
  PosixSpinLock::unlock() throw ()
  {
    pthread_spin_unlock(&spinlock_);
  }


  //
  // PosixSpinGuard class
  //

  inline
  PosixSpinGuard::PosixSpinGuard(pthread_spinlock_t& spinlock) throw ()
    : spinlock_(spinlock)
  {
    pthread_spin_lock(&spinlock_);
  }

  inline
  PosixSpinGuard::~PosixSpinGuard() throw ()
  {
    pthread_spin_unlock(&spinlock_);
  }
}

#endif
