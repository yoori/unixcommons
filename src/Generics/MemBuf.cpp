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



// Generics/MemBuf.cpp

#include <Generics/MemBuf.hpp>
#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>

#if DEV_MEMBUF_BOUNDS > 0
#include <cassert>
#endif


namespace Generics
{
  MemBuf::MemBuf(Allocator::Base* allocator) throw ()
    : allocator_(ReferenceCounting::add_ref(
        allocator ? allocator : Allocator::Base::get_default_allocator())),
      ptr_(0), size_(0), capacity_(2 * DEV_MEMBUF_BOUNDS)
  {
  }

  MemBuf::MemBuf(std::size_t size, Allocator::Base* allocator)
    throw (OutOfMemory)
    : allocator_(ReferenceCounting::add_ref(
        allocator ? allocator : Allocator::Base::get_default_allocator())),
      ptr_(0), size_(0), capacity_(2 * DEV_MEMBUF_BOUNDS)
  {
    try
    {
      alloc(size);
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't allocate memory: " << ex.what();
      throw OutOfMemory(ostr);
    }
  }

  MemBuf::MemBuf(const MemBuf& right) throw (OutOfMemory)
    : allocator_(right.allocator_), ptr_(0), size_(0),
      capacity_(2 * DEV_MEMBUF_BOUNDS)
  {
    try
    {
      alloc(right.size());
      memcpy(data(), right.data(), size_);
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't allocate memory: " << ex.what();
      throw OutOfMemory(ostr);
    }
  }

  MemBuf::MemBuf(const MemBuf& right, Allocator::Base* allocator)
    throw (OutOfMemory)
    : allocator_(ReferenceCounting::add_ref(
        allocator ? allocator : Allocator::Base::get_default_allocator())),
      ptr_(0), size_(0), capacity_(2 * DEV_MEMBUF_BOUNDS)
  {
    try
    {
      alloc(right.size());
      memcpy(data(), right.data(), size_);
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't allocate memory: " << ex.what();
      throw OutOfMemory(ostr);
    }
  }

  MemBuf::MemBuf(MemBuf&& right) throw ()
    : allocator_(right.allocator_), ptr_(0), size_(0),
      capacity_(2 * DEV_MEMBUF_BOUNDS)
  {
    swap(right);
  }

  MemBuf::MemBuf(const void* ptr, std::size_t size,
    Allocator::Base* allocator)
    throw (RangeError, OutOfMemory)
    : allocator_(ReferenceCounting::add_ref(allocator ? allocator :
        Allocator::Base::get_default_allocator())),
      ptr_(0), size_(0), capacity_(2 * DEV_MEMBUF_BOUNDS)
  {
    try
    {
      alloc(size);
      memcpy(data(), ptr, size_);
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't allocate memory: " << ex.what();
      throw OutOfMemory(ostr);
    }
  }

  MemBuf::~MemBuf() throw ()
  {
    clear();
  }

  void
  MemBuf::clear() throw ()
  {
    if (capacity())
    {
#if DEV_MEMBUF_BOUNDS > 0
      // check bound constraints
      const unsigned char* ptr = static_cast<const unsigned char*>(ptr_);

      for (size_t i = 0; i < DEV_MEMBUF_BOUNDS; ++i)
      {
        assert(ptr[i] == 0xDD);
      }
      for (size_t i = capacity_ - DEV_MEMBUF_BOUNDS; i < capacity_; ++i)
      {
        assert(ptr[i] == 0xDD);
      }
#endif
      allocator_->deallocate(ptr_, capacity_);
    }
    size_ = 0;
    capacity_ = 2 * DEV_MEMBUF_BOUNDS;
  }

  void
  MemBuf::alloc(std::size_t size) throw (eh::Exception, OutOfMemory)
  {
    if (capacity() < size)
    {
      clear();
      try
      {
        std::size_t tmp_capacity = size + 2 * DEV_MEMBUF_BOUNDS;
        ptr_ = allocator_->allocate(tmp_capacity);  // modify tmp_capacity
        capacity_ = tmp_capacity;
      }
      catch (const eh::Exception& ex)
      {
        Stream::Error ostr;
        ostr << FNS << size << ex.what();
        throw OutOfMemory(ostr);
      }
#if DEV_MEMBUF_BOUNDS > 0
      memset(ptr_, 0xDD, DEV_MEMBUF_BOUNDS);
      memset(static_cast<unsigned char*>(ptr_) + capacity_ -
        DEV_MEMBUF_BOUNDS, 0xDD, DEV_MEMBUF_BOUNDS);
#endif
    }
    size_ = size;
  }

  void
  MemBuf::resize(std::size_t size) throw (RangeError)
  {
    if (size > capacity())
    {
      Stream::Error ostr;
      ostr << FNS << "requested size=" << size << " exceeds capacity=" <<
        capacity();
      throw RangeError(ostr);
    }
    size_ = size;
  }

  void
  MemBuf::swap(MemBuf& right) throw ()
  {
    std::swap(ptr_, right.ptr_);
    std::swap(size_, right.size_);
    std::swap(capacity_, right.capacity_);
    std::swap(allocator_, right.allocator_);
  }

  void
  MemBuf::assign(const void* ptr, std::size_t size)
    throw (eh::Exception, OutOfMemory)
  {
    alloc(size);
    memcpy(data(), ptr, size);
  }

  MemBuf&
  MemBuf::operator =(MemBuf&& right) throw ()
  {
    if (&right != this)
    {
      swap(right);
    }
    return *this;
  }


  ConstSmartMemBuf_var
  transfer_membuf(SmartMemBuf* ptr) throw (eh::Exception)
  {
    return new ConstSmartMemBuf(std::move(ptr->membuf()));
  }
}
