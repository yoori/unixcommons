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



#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <pthread.h>

#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>

#include <ReferenceCounting/ReferenceCounting.hpp>
#include <Sync/SyncPolicy.hpp>
#include <Generics/Time.hpp>

using namespace ReferenceCounting;

#define TEST(NAME, INIT, CYCLE, LOCK_COUNT) \
{ \
  INIT; \
  Generics::Timer timer; \
  \
  int i; \
  timer.start(); \
  for (i = 0; i < LOCK_COUNT; i++) \
  { \
    CYCLE; \
  } \
 \
  timer.stop(); \
 \
  long long sum_msec = timer.elapsed_time().microseconds(); \
  int msec = i ? static_cast<int>(sum_msec * 1000 / i) : 0; \
 \
  std::cout << NAME ":" << std::endl; \
  std::cout << "lock count: " << i << std::endl; \
  std::cout << "sum time: " << sum_msec << " mcs" << std::endl; \
  std::cout << "average time: " << msec / 1000 << "." << \
    std::setfill('0') << std::setw(3) << msec % 1000 << " mcs" << std::endl; \
  std::cout << std::endl; \
}

int main()
{
  const int LOCK_COUNT = 1000000;

  TEST("Vector filling",
       std::vector<int> v,
       v.push_back(i),
       LOCK_COUNT);
  TEST("Posix Mutex",
       pthread_mutex_t lock_ = PTHREAD_MUTEX_INITIALIZER,
       if (pthread_mutex_lock(&lock_) != 0) \
       { \
        std::cout << "POSIX LOCK ERROR" << std::endl; \
        break; \
       } \
       if (pthread_mutex_unlock(&lock_) != 0) \
       { \
         break; \
       },
       LOCK_COUNT);
  TEST("Posix Guard",
       Sync::PosixMutex lock_,
       Sync::PosixGuard lock(lock_),
       LOCK_COUNT);
  TEST("Posix Read Guard",
       Sync::PosixRWLock lock_,
       Sync::PosixRGuard lock(lock_),
       LOCK_COUNT);
  TEST("Posix Write Guard",
       Sync::PosixRWLock lock_,
       Sync::PosixWGuard lock(lock_),
       LOCK_COUNT);
  TEST("Ace Mutex No Guard",
       ACE_Thread_Mutex lock_,
       lock_.acquire(); \
       lock_.release(),
       LOCK_COUNT);
  TEST("Ace Mutex Guard",
       ACE_Thread_Mutex lock_,
       ACE_Guard<ACE_Thread_Mutex> lock(lock_),
       LOCK_COUNT);
  TEST("Ace Mutex Read Guard",
       ACE_Thread_Mutex lock_,
       ACE_Read_Guard<ACE_Thread_Mutex> lock(lock_),
       LOCK_COUNT);
  TEST("Ace Mutex Write Guard",
       ACE_Thread_Mutex lock_,
       ACE_Write_Guard<ACE_Thread_Mutex> lock(lock_),
       LOCK_COUNT);
  TEST("Ace RW Mutex Read Guard",
       ACE_RW_Thread_Mutex lock_,
       ACE_Read_Guard<ACE_RW_Thread_Mutex> lock(lock_),
       LOCK_COUNT);
  TEST("Ace RW Mutex Write Guard",
       ACE_RW_Thread_Mutex lock_,
       ACE_Write_Guard<ACE_RW_Thread_Mutex> lock(lock_),
       LOCK_COUNT);
  TEST("PosixThreadPolicy",
       class Lock : public DefaultImpl<Sync::Policy::PosixThread> {} lock_,
       lock_.add_ref(); \
       lock_.remove_ref(),
       LOCK_COUNT);
#if 0
  TEST("PosixThreadPolicy addref only",
       class Lock : public DefaultImpl<Sync::Policy::PosixThread> lock_,
       lock_.add_ref(),
       LOCK_COUNT);
  TEST("PosixThreadPolicy removeref only",
       class Lock : public DefaultImpl<Sync::Policy::PosixThread> {} lock_; \
       for (int i = 0; i < LOCK_COUNT; i++) lock_.add_ref(),
       lock_.remove_ref(),
       LOCK_COUNT);
#endif
  TEST("Gcc atomic test",
       _Atomic_word value = 0,
       __gnu_cxx::__atomic_add(&value, 1); \
       __gnu_cxx::__atomic_add(&value, -1),
       LOCK_COUNT);
  TEST("AtomicImpl",
       class Lock : public AtomicImpl {} lock_,
       lock_.add_ref(); \
       lock_.remove_ref(),
       LOCK_COUNT);
  TEST("AtomicImpl virtual functions",
       class Lock : public AtomicImpl {} lock; \
       ReferenceCounting::Interface& lock_ = lock,
       lock_.add_ref(); \
       lock_.remove_ref(),
       LOCK_COUNT);

  return 0;
}
