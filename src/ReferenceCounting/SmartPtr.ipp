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



#include <cassert>

#include <Generics/Proc.hpp>


namespace ReferenceCounting
{
  //
  // PolicyThrow class
  //

  inline
  void
  PolicyThrow::check_init(const void* /*ptr*/) throw (NullPointer)
  {
  }

  inline
  void
  PolicyThrow::check_dereference(const void* ptr)
    throw (NotInitialized)
  {
    if (!ptr)
    {
      char buf[sizeof(NotInitialized)] =
        "ReferenceCounting::PolicyThrow::check_dereference(): "
        "unable to dereference NULL pointer: ";
      Generics::Proc::backtrace(buf + 89, sizeof(buf) - 89, 1, 5);
      throw NotInitialized(buf);
    }
  }

  inline
  void
  PolicyThrow::default_constructor() throw ()
  {
  }

  inline
  void
  PolicyThrow::retn() throw ()
  {
  }


  //
  // PolicyAssert class
  //

  inline
  void
  PolicyAssert::check_init(const void* /*ptr*/) throw (NullPointer)
  {
  }

  inline
  void
  PolicyAssert::check_dereference(const void* ptr)
    throw (NotInitialized)
  {
    (void)ptr;
    assert(ptr);
  }

  inline
  void
  PolicyAssert::default_constructor() throw ()
  {
  }

  inline
  void
  PolicyAssert::retn() throw ()
  {
  }


  //
  // PolicyNotNull class
  //

  inline
  void
  PolicyNotNull::check_init(const void* ptr) throw (NullPointer)
  {
    if (!ptr)
    {
      throw NullPointer(
        "ReferenceCounting::PolicyNotNull::check_init(): "
        "unable to init with NULL pointer.");
    }
  }

  inline
  void
  PolicyNotNull::check_dereference(const void* /*ptr*/)
    throw (NotInitialized)
  {
  }


  //
  // PolicyChecker class
  //

  inline
  void
  PolicyChecker::check_policy_(const PolicyThrow* /*policy*/)
    throw ()
  {
  }

  inline
  void
  PolicyChecker::check_policy_(const PolicyAssert* /*policy*/)
    throw ()
  {
  }

  inline
  void
  PolicyChecker::check_policy_(const PolicyNotNull* /*policy*/)
    throw ()
  {
  }
}
