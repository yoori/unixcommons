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



#ifndef REFERENCECOUNTING_PTRHOLDER_HPP
#define REFERENCECOUNTING_PTRHOLDER_HPP

#include <Sync/PosixLock.hpp>

#include <ReferenceCounting/SmartPtr.hpp>


namespace ReferenceCounting
{
  /**
   * Safe holder for a pointer. Allows serialized "get" and assignment
   * operations on stored RC object pointer.
   * SmartPtr is a type of "get" operation return value (defines pointer
   * type as well).
   */
  template <typename SmartPtr>
  class PtrHolder : private Generics::Uncopyable
  {
  public:
    typedef typename SmartPtr::Type Type;

    explicit
    PtrHolder(std::nullptr_t ptr = nullptr) throw (eh::Exception);

    template <typename Other>
    explicit
    PtrHolder(Other&& sptr) throw (eh::Exception);

    ~PtrHolder() throw ();

    template <typename Other>
    PtrHolder&
    operator =(Other&& sptr) throw (eh::Exception);

    SmartPtr
    get() throw (eh::Exception);

    const SmartPtr
    get() const throw (eh::Exception);

  private:
    mutable Sync::PosixSpinLock mutex_;
    Type* ptr_;
  };
}

namespace ReferenceCounting
{
  template <typename SmartPtr>
  PtrHolder<SmartPtr>::PtrHolder(std::nullptr_t) throw (eh::Exception)
    : ptr_(0)
  {
  }

  template <typename SmartPtr>
  template <typename Other>
  PtrHolder<SmartPtr>::PtrHolder(Other&& sptr) throw (eh::Exception)
    : ptr_(cond_add_ref(std::forward<Other>(sptr)))
  {
  }

  template <typename SmartPtr>
  PtrHolder<SmartPtr>::~PtrHolder() throw ()
  {
    if (ptr_)
    {
      ptr_->remove_ref();
    }
  }

  template <typename SmartPtr>
  template <typename Other>
  PtrHolder<SmartPtr>&
  PtrHolder<SmartPtr>::operator =(Other&& sptr) throw (eh::Exception)
  {
    Type* ptr = cond_add_ref(std::forward<Other>(sptr));
    Type* old_ptr;
    {
      Sync::PosixSpinGuard lock(mutex_);
      old_ptr = ptr_;
      ptr_ = ptr;
    }
    if (old_ptr)
    {
      old_ptr->remove_ref();
    }
    return *this;
  }

  template <typename SmartPtr>
  SmartPtr
  PtrHolder<SmartPtr>::get() throw (eh::Exception)
  {
    Sync::PosixSpinGuard lock(mutex_);
    return SmartPtr(add_ref(ptr_));
  }

  template <typename SmartPtr>
  const SmartPtr
  PtrHolder<SmartPtr>::get() const throw (eh::Exception)
  {
    Sync::PosixSpinGuard lock(mutex_);
    return SmartPtr(add_ref(ptr_));
  }
}

#endif
