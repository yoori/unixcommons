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



#ifndef GENERICS_THREAD_BUFFER_HPP
#define GENERICS_THREAD_BUFFER_HPP

#include <Sync/PosixLock.hpp>
#include <Sync/Key.hpp>


namespace Generics
{
  template <typename Tag, const size_t BUFFER_SIZE, const size_t THREADS>
  class ThreadBuffer : private Uncopyable
  {
  public:
    ThreadBuffer() throw ();

    static
    char*
    get_buffer() throw ();

  private:
    typedef char Buffer[BUFFER_SIZE];

    static Sync::Key<char> buffer_key_;
    static Sync::Key<void> type_key_;
    static Sync::PosixMutex mutex_;
    static Buffer buffers_[THREADS];
    static char* buffer_pointers_[THREADS];
    static size_t available_;

    static
    void
    free_buffer_(void* buffer) throw ();
  };
}

//
// INLINES
//

namespace Generics
{
  template <typename Tag, const size_t BUFFER_SIZE, const size_t THREADS>
  Sync::Key<char>
    ThreadBuffer<Tag, BUFFER_SIZE, THREADS>::buffer_key_(free_buffer_);
  template <typename Tag, const size_t BUFFER_SIZE, const size_t THREADS>
  Sync::Key<void> ThreadBuffer<Tag, BUFFER_SIZE, THREADS>::type_key_;
  template <typename Tag, const size_t BUFFER_SIZE, const size_t THREADS>
  Sync::PosixMutex ThreadBuffer<Tag, BUFFER_SIZE, THREADS>::mutex_;
  template <typename Tag, const size_t BUFFER_SIZE, const size_t THREADS>
  typename ThreadBuffer<Tag, BUFFER_SIZE, THREADS>::Buffer
    ThreadBuffer<Tag, BUFFER_SIZE, THREADS>::buffers_[THREADS];
  template <typename Tag, const size_t BUFFER_SIZE, const size_t THREADS>
  char* ThreadBuffer<Tag, BUFFER_SIZE, THREADS>::buffer_pointers_[THREADS];
  template <typename Tag, const size_t BUFFER_SIZE, const size_t THREADS>
  size_t ThreadBuffer<Tag, BUFFER_SIZE, THREADS>::available_;

  template <typename Tag, const size_t BUFFER_SIZE, const size_t THREADS>
  ThreadBuffer<Tag, BUFFER_SIZE, THREADS>::ThreadBuffer() throw ()
  {
    for (available_ = 0; available_ < THREADS; available_++)
    {
      buffer_pointers_[available_] = buffers_[available_];
    }
  }

  template <typename Tag, const size_t BUFFER_SIZE, const size_t THREADS>
  char*
  ThreadBuffer<Tag, BUFFER_SIZE, THREADS>::get_buffer() throw ()
  {
    char* buffer = buffer_key_.get_data();
    if (buffer)
    {
      return buffer;
    }

    {
      Sync::PosixGuard guard(mutex_);

      if (available_)
      {
        buffer = buffer_pointers_[--available_];
      }
    }

    if (!buffer)
    {
      try
      {
        buffer = new char[BUFFER_SIZE];
      }
      catch (...)
      {
        return 0;
      }
      type_key_.set_data(buffer);
    }

    buffer_key_.set_data(buffer);
    return buffer;
  }

  template <typename Tag, const size_t BUFFER_SIZE, const size_t THREADS>
  void
  ThreadBuffer<Tag, BUFFER_SIZE, THREADS>::free_buffer_(void* buffer) throw ()
  {
    if (type_key_.get_data())
    {
      delete [] static_cast<char*>(buffer);
    }
    else
    {
      Sync::PosixGuard guard(mutex_);

      buffer_pointers_[available_++] = static_cast<char*>(buffer);
    }

    buffer_key_.set_data(0);
  }
}

#endif
