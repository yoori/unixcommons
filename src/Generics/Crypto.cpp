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



#include <openssl/crypto.h>

#include <Sync/PosixLock.hpp>

#include <Generics/ArrayAutoPtr.hpp>


namespace
{
  class CryptoLocks : private Generics::Uncopyable
  {
  public:
    CryptoLocks() throw (eh::Exception);
    ~CryptoLocks() throw ();

  private:
    static
    void
    locking_function_(int mode, int n, const char* file, int line)
      throw ();

    static
    unsigned long
    id_function_() throw ();

    static Generics::ArrayAutoPtr<Sync::PosixMutex> locks_;

    void (*old_locking_function_)(int, int, const char*, int);
    unsigned long (*old_id_function_)();
  };

  Generics::ArrayAutoPtr<Sync::PosixMutex> CryptoLocks::locks_;

  CryptoLocks::CryptoLocks() throw (eh::Exception)
    : old_locking_function_(CRYPTO_get_locking_callback()),
      old_id_function_(CRYPTO_get_id_callback())
  {
    locks_.reset(CRYPTO_num_locks());

    CRYPTO_set_locking_callback(locking_function_);
    CRYPTO_set_id_callback(id_function_);
  }

  CryptoLocks::~CryptoLocks() throw ()
  {
    CRYPTO_set_locking_callback(old_locking_function_);
    CRYPTO_set_id_callback(old_id_function_);
  }

  void
  CryptoLocks::locking_function_(int mode, int n,
    const char* /*file*/, int /*line*/) throw ()
  {
    if (mode & CRYPTO_LOCK)
    {
      locks_[n].lock();
    }
    else
    {
      locks_[n].unlock();
    }
  }

  unsigned long
  CryptoLocks::id_function_() throw ()
  {
    return pthread_self();
  }

  CryptoLocks crypto_locks;
}
