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



#ifndef GENERICS_LASTPTR_HPP
#define GENERICS_LASTPTR_HPP

#include <signal.h>

#include <Sync/Semaphore.hpp>

#include <Generics/Uncopyable.hpp>
#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>


namespace Generics
{
  /**
   * Simple adapter for LastPtr class. Extends child of AtomicImpl.
   */
  template <typename AtomicImplChild>
  class Last : public AtomicImplChild
  {
  public:
    /**
     * Constructor
     */
    Last() throw (eh::Exception);

    /**
     * Waits for last remove_ref for the object.
     */
    void
    last_wait() const throw ();

    /**
     * Erases the object.
     */
    void
    last_delete_this() const throw ();

  protected:
    /**
     * Destructor
     */
    virtual
    ~Last() throw ();

    /**
     * Overrider of AtomicImpl::delete_this_ - notifies last_wait() instead
     * of erasing the object.
     */
    virtual
    void
    delete_this_() const throw ();

  private:
    mutable volatile sig_atomic_t wait_mode_;
    mutable Sync::Semaphore sem_;
  };

  /**
   * Smart pointer. Constructor waits until the last reference to the object
   * disappears and then allows to use the object. Destructor erases the
   * object.
   */
  template <typename LastChild>
  class LastPtr : private Uncopyable
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    /**
     * Constructor
     * Waits for the object to be unreferenced.
     * Usually it calls to ptr->remove_ref(), that's why use additional
     * ptr->add_ref() before creating LastPtr object.
     * @param ptr non-NULL pointer to the object to hold.
     */
    explicit
    LastPtr(LastChild* ptr) throw (Exception);

    /**
     * Destructor
     * Erases the object.
     */
    ~LastPtr() throw ();

    /**
     * Allows to access the object members.
     * @return pointer to the stored object.
     */
    LastChild*
    operator ->() const throw ();

  private:
    LastChild* ptr_;
  };
};

//
// INLINES
//

namespace Generics
{
  //
  // Last class
  //

  template <typename AtomicImplChild>
  Last<AtomicImplChild>::Last() throw (eh::Exception)
    : wait_mode_(false), sem_(0)
  {
  }

  template <typename AtomicImplChild>
  Last<AtomicImplChild>::~Last() throw ()
  {
  }

  template <typename AtomicImplChild>
  void
  Last<AtomicImplChild>::last_wait() const throw ()
  {
    wait_mode_ = true;
    this->remove_ref();
    sem_.acquire();
  }

  template <typename AtomicImplChild>
  void
  Last<AtomicImplChild>::last_delete_this() const throw ()
  {
    AtomicImplChild::delete_this_();
  }

  template <typename AtomicImplChild>
  void
  Last<AtomicImplChild>::delete_this_() const throw ()
  {
    if (!wait_mode_)
    {
      last_delete_this();
      return;
    }
    sem_.release();
  }


  //
  // LastPtr class
  //

  template <typename LastChild>
  LastPtr<LastChild>::LastPtr(LastChild* ptr) throw (Exception)
    : ptr_(ptr)
  {
    if (!ptr_)
    {
      Stream::Error ostr;
      ostr << FNS << "Attempt to initialize with NULL pointer";
      throw Exception(ostr);
    }
    ptr_->last_wait();
  }

  template <typename LastChild>
  LastPtr<LastChild>::~LastPtr() throw ()
  {
    ptr_->last_delete_this();
  }

  template <typename LastChild>
  LastChild*
  LastPtr<LastChild>::operator ->() const throw ()
  {
    return ptr_;
  }
}

#endif
