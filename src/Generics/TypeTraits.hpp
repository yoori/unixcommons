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



// Generics/TypeTraits.hpp
#ifndef GENERICS_TYPE_TRAITS_HPP
#define GENERICS_TYPE_TRAITS_HPP

#include <limits>

#include <Generics/Uncopyable.hpp>


namespace Generics
{
  template <typename T, typename A, typename B>
  struct IfConst
  {
    typedef B Result;
  };
  template <typename T, typename A, typename B>
  struct IfConst<const T, A, B>
  {
    typedef A Result;
  };

  template <typename Container>
  class Inserter : private Generics::Uncopyable
  {
  public:
    explicit
    Inserter(Container& container) throw ();
    Inserter&
    operator *() throw ();
    template <typename T>
    Inserter&
    operator =(T&& data) throw (eh::Exception);

  private:
    Container& container_;
  };

  template <typename Integer>
  Integer
  safe_next(Integer number) throw ()
  {
    return number < std::numeric_limits<Integer>::max() ?
      number + 1 : number;
  }

  template <typename From, typename To>
  class PairPtr : private Generics::Uncopyable
  {
  public:
    PairPtr(From* from) throw ();
    PairPtr(PairPtr&& other) throw ();

    To*
    operator ->() throw ();

  private:
    To to;
  };
}

namespace Generics
{
  //
  // Inserter class
  //

  template <typename Container>
  Inserter<Container>::Inserter(Container& container) throw ()
    : container_(container)
  {
  }

  template <typename Container>
  Inserter<Container>&
  Inserter<Container>::operator *() throw ()
  {
    return *this;
  }

  template <typename Container>
  template <typename T>
  Inserter<Container>&
  Inserter<Container>::operator =(T&& data) throw (eh::Exception)
  {
    container_.insert(std::forward<T>(data));
    return *this;
  }


  //
  // PairPtr class
  //

  template <typename From, typename To>
  PairPtr<From, To>::PairPtr(From* from) throw ()
    : to(from->first, from->second)
  {
  }

  template <typename From, typename To>
  PairPtr<From, To>::PairPtr(PairPtr&& other) throw ()
    : to(other.to)
  {
  }

  template <typename From, typename To>
  To*
  PairPtr<From, To>::operator ->() throw ()
  {
    return &to;
  }
}

#endif
