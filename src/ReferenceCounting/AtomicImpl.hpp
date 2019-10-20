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




#ifndef REFERENCE_COUNTING_ATOMIC_IMPL_HPP
#define REFERENCE_COUNTING_ATOMIC_IMPL_HPP

#include <cassert>

#include <ext/atomicity.h>

#include <ReferenceCounting/Interface.hpp>


namespace ReferenceCounting
{
  /**
   * For performance purposes usage of atomic arithmetic
   * functions are required for reference counting
   */
  class AtomicImpl :
    public virtual Interface,
    private Generics::Uncopyable
  {
  public:
    virtual
    void
    add_ref() const throw ();

    virtual
    void
    remove_ref() const throw ();

  protected:
    AtomicImpl() throw ();
    virtual
    ~AtomicImpl() throw ();

    virtual
    bool
    remove_ref_no_delete_() const throw ();

    virtual
    void
    delete_this_() const throw ();

  protected:
    mutable volatile _Atomic_word ref_count_;
  };

  class AtomicCopyImpl : public AtomicImpl
  {
  protected:
    AtomicCopyImpl() throw ();
    AtomicCopyImpl(const volatile AtomicCopyImpl&) throw ();
    virtual
    ~AtomicCopyImpl() throw ();
  };
}

namespace ReferenceCounting
{
  inline
  AtomicImpl::AtomicImpl() throw ()
    : ReferenceCounting::Interface(), ref_count_(1)
  {
  }

  inline
  AtomicImpl::~AtomicImpl() throw ()
  {
#ifndef NVALGRIND
    RunningOnValgrind<>::check_ref_count(ref_count_);
#endif
  }

  inline
  void
  AtomicImpl::add_ref() const throw ()
  {
    __gnu_cxx::__atomic_add(&ref_count_, 1);
  }

  inline
  void
  AtomicImpl::delete_this_() const throw ()
  {
    delete this;
  }

  inline
  bool
  AtomicImpl::remove_ref_no_delete_() const throw ()
  {
    _Atomic_word old = __gnu_cxx::__exchange_and_add(&ref_count_, -1);
    assert(old > 0);
    return old == 1;
  }

  inline
  void
  AtomicImpl::remove_ref() const throw ()
  {
    if (remove_ref_no_delete_())
    {
      delete_this_();
    }
  }


  inline
  AtomicCopyImpl::AtomicCopyImpl() throw ()
    : ReferenceCounting::Interface()
  {
  }

  inline
  AtomicCopyImpl::AtomicCopyImpl(const volatile AtomicCopyImpl&) throw ()
    : ReferenceCounting::Interface(), AtomicImpl()
  {
  }

  inline
  AtomicCopyImpl::~AtomicCopyImpl() throw ()
  {
  }
}

#endif
