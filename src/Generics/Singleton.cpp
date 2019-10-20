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



#include <cstdlib>

#include <Generics/Singleton.hpp>


namespace Generics
{
  //
  // AtExitDestroying class
  //

  // All of these are initialized statically
  Sync::PosixMutex AtExitDestroying::mutex_;
  AtExitDestroying* AtExitDestroying::lower_priority_head_ = 0;
  bool AtExitDestroying::registered_ = false;

  AtExitDestroying::AtExitDestroying(int priority) throw ()
    : priority_(priority)
  {
    Sync::PosixGuard guard(mutex_);
    if (!registered_)
    {
      registered_ = !std::atexit(destroy_at_exit_);
    }
    AtExitDestroying** pptr = &lower_priority_head_;
    while (*pptr && (*pptr)->priority_ < priority_)
    {
      pptr = &(*pptr)->lower_priority_;
    }
    if (*pptr && (*pptr)->priority_ == priority_)
    {
      lower_priority_ = (*pptr)->lower_priority_;
      //(*pptr)->lower_priority_ = 0;
      equal_priority_ = *pptr;
      *pptr = this;
    }
    else
    {
      equal_priority_ = 0;
      lower_priority_ = *pptr;
      *pptr = this;
    }
  }

  void
  AtExitDestroying::destroy_at_exit_() throw ()
  {
    Sync::PosixGuard guard(mutex_);
    for (AtExitDestroying* current_priority = lower_priority_head_;
      current_priority;)
    {
      AtExitDestroying* next_priority = current_priority->lower_priority_;
      for (AtExitDestroying* current = current_priority; current;)
      {
        AtExitDestroying* next = current->equal_priority_;
        delete current;
        current = next;
      }
      current_priority = next_priority;
    }
  }
}
