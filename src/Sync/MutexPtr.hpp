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



#ifndef SYNC_MUTEXPTR_HPP
#define SYNC_MUTEXPTR_HPP

#include <memory>

#include <ReferenceCounting/ReferenceCounting.hpp>
#include <Sync/PosixLock.hpp>

namespace Sync
{
  /**
   * Performs mutex lock and unlock around each -> call
   */
  template <typename Object>
  class MutexPtr : private Generics::Uncopyable
  {
  public:
    template <typename ObjectType>
    class ProtectedObject : private Generics::Uncopyable
    {
    public:
      ProtectedObject(ObjectType* ptr, Sync::PosixMutex& mutex)
        throw (eh::Exception);

      ProtectedObject(ProtectedObject&& object) throw ();

      ~ProtectedObject() throw ();

      ObjectType*
      operator ->() const throw ();

    private:
      ObjectType* object_;
      Sync::PosixMutex* mutex_;
    };


    explicit
    MutexPtr(Object* object) throw (eh::Exception);

    ProtectedObject<Object>
    operator ->() throw (eh::Exception);

    ProtectedObject<const Object>
    operator ->() const throw (eh::Exception);

    template <typename OtherObject>
    ProtectedObject<OtherObject>
    as() throw (eh::Exception);

    template <typename OtherObject>
    ProtectedObject<const OtherObject>
    as() const throw (eh::Exception);

  private:
    std::unique_ptr<Object> object_;
    mutable Sync::PosixMutex mutex_;
  };

  template <typename Object>
  class MutexRefPtr :
    public MutexPtr<Object>,
    public ReferenceCounting::AtomicImpl
  {
  public:
    MutexRefPtr(Object* object) throw (eh::Exception);

  protected:
    virtual
    ~MutexRefPtr() throw () = default;
  };
}

//
// INLINES
//
namespace Sync
{
  //
  // MutexPtr::ProtectedObject class
  //

  template <typename Object>
  template <typename ObjectType>
  MutexPtr<Object>::ProtectedObject<ObjectType>::ProtectedObject(
    ObjectType* ptr, Sync::PosixMutex& mutex) throw (eh::Exception)
    : object_(ptr), mutex_(&mutex)
  {
    mutex.lock();
  }

  template <typename Object>
  template <typename ObjectType>
  MutexPtr<Object>::ProtectedObject<ObjectType>::ProtectedObject(
    ProtectedObject&& object) throw ()
    : Generics::Uncopyable(), object_(object.object_), mutex_(object.mutex_)
  {
    object.mutex_ = nullptr;
  }

  template <typename Object>
  template <typename ObjectType>
  MutexPtr<Object>::ProtectedObject<ObjectType>::~ProtectedObject() throw ()
  {
    if (mutex_)
    {
      mutex_->unlock();
    }
  }

  template <typename Object>
  template <typename ObjectType>
  ObjectType*
  MutexPtr<Object>::ProtectedObject<ObjectType>::operator ->() const
    throw ()
  {
    return object_;
  }


  //
  // MutexPtr class
  //

  template <typename Object>
  MutexPtr<Object>::MutexPtr(Object* object) throw (eh::Exception)
    : object_(object)
  {
  }

  template <typename Object>
  typename MutexPtr<Object>::template ProtectedObject<Object>
  MutexPtr<Object>::operator ->() throw (eh::Exception)
  {
    return ProtectedObject<Object>(object_.get(), mutex_);
  }

  template <typename Object>
  typename MutexPtr<Object>::template ProtectedObject<const Object>
  MutexPtr<Object>::operator ->() const throw (eh::Exception)
  {
    return ProtectedObject<const Object>(object_.get(), mutex_);
  }

  template <typename Object>
  template <typename OtherObject>
  typename MutexPtr<Object>::template ProtectedObject<OtherObject>
  MutexPtr<Object>::as() throw (eh::Exception)
  {
    return ProtectedObject<OtherObject>(
      static_cast<OtherObject*>(object_.get()), mutex_);
  }

  template <typename Object>
  template <typename OtherObject>
  typename MutexPtr<Object>::template ProtectedObject<const OtherObject>
  MutexPtr<Object>::as() const throw (eh::Exception)
  {
    return ProtectedObject<const OtherObject>(
      static_cast<const OtherObject*>(object_.get()), mutex_);
  }


  //
  // MutexRefPtr class
  //

  template <typename Object>
  MutexRefPtr<Object>::MutexRefPtr(Object* object) throw (eh::Exception)
    : MutexPtr<Object>(object)
  {
  }
}

#endif
