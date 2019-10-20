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



#ifndef REFERENCECOUNTING_CONTAINERS_HPP
#define REFERENCECOUNTING_CONTAINERS_HPP

#include <eh/Exception.hpp>


namespace ReferenceCounting
{
  namespace Helper
  {
    template <typename T, typename Alloc>
    struct Allocator : public Alloc::template rebind<T>::other
    {
      template <typename D>
      struct rebind
      {
        typedef Allocator<D, Alloc> other;
      };
      Allocator() throw () = default;
      template <typename D>
      Allocator(const Allocator<D, Alloc>&) throw ();
      template <typename... Args>
      void
      construct(T* p, Args&&... args) throw (eh::Exception);
    };

    template <class Key>
    struct HashFunForHashAdapter
    {
      size_t
      operator ()(const Key& key) const throw (eh::Exception);
    };
  }
}

namespace ReferenceCounting
{
  namespace Helper
  {
    //
    // Allocator class
    //

    template <typename T, typename Alloc>
    template <typename D>
    Allocator<T, Alloc>::Allocator(const Allocator<D, Alloc>&) throw ()
      : Alloc::template rebind<T>::other()
    {
    }

    template <typename T, typename Alloc>
    template <typename... Args>
    void
    Allocator<T, Alloc>::construct(T* p, Args&&... args)
      throw (eh::Exception)
    {
      ::new(p) T(const_cast<typename std::remove_const<
        typename std::remove_reference<Args>::type>::type&&>(args)...);
    }


    //
    // HashFunForHashAdapter class
    //

    template <class Key>
    size_t
    HashFunForHashAdapter<Key>::operator ()(const Key& key) const
      throw (eh::Exception)
    {
      return key.hash();
    }
  }
}

#endif
