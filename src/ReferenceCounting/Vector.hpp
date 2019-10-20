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



#ifndef REFERENCECOUNTING_VECTOR_HPP
#define REFERENCECOUNTING_VECTOR_HPP

#include <vector>

#include <eh/Exception.hpp>


namespace ReferenceCounting
{
  /**
   * Const-preserving version of std::vector.
   * No f(const T&) functions are available, they are replaced with
   * f(T&) and f(T&&) ones.
   */
  template <typename T, typename Allocator = std::allocator<T>>
  class Vector : protected std::vector<T, Allocator>
  {
  public:
    typedef std::vector<T, Allocator> Base;

    typedef typename Base::value_type value_type;
    typedef typename Base::pointer pointer;
    typedef typename Base::const_pointer const_pointer;
    typedef typename Base::reference reference;
    typedef typename Base::const_reference const_reference;
    typedef typename Base::iterator iterator;
    typedef typename Base::const_iterator const_iterator;
    typedef typename Base::reverse_iterator reverse_iterator;
    typedef typename Base::const_reverse_iterator const_reverse_iterator;
    typedef typename Base::size_type size_type;
    typedef typename Base::difference_type difference_type;

    using Base::begin;
    using Base::end;
    using Base::cbegin;
    using Base::cend;
    using Base::rbegin;
    using Base::rend;
    using Base::crbegin;
    using Base::crend;
    using Base::size;
    using Base::max_size;
    using Base::capacity;
    using Base::empty;
    using Base::reserve;
    using Base::operator [];
    using Base::at;
    using Base::front;
    using Base::back;
    using Base::emplace_back;
    using Base::pop_back;
    using Base::emplace;
    using Base::erase;
    using Base::clear;

    Vector() throw ();
    explicit
    Vector(size_type n) throw (eh::Exception);
    Vector(size_type n, value_type& x) throw (eh::Exception);
    Vector(Vector& v) throw (eh::Exception);
    Vector(const Vector&) = delete;
    Vector(Vector&& v) throw ();
    template <typename InputIterator>
    Vector(InputIterator first, InputIterator last) throw (eh::Exception);

    Vector&
    operator =(Vector& v) throw (eh::Exception);
    Vector&
    operator =(Vector&& v) throw ();

    void
    assign(size_type n, value_type& x) throw (eh::Exception);
    template <typename InputIterator>
    void
    assign(InputIterator first, InputIterator last) throw (eh::Exception);

    void
    resize(size_type n) throw (eh::Exception);
    void
    resize(size_type n, value_type& v) throw (eh::Exception);

    void
    push_back(value_type& x) throw (eh::Exception);
    void
    push_back(value_type&& x) throw (eh::Exception);

    iterator
    insert(iterator position, value_type& x) throw (eh::Exception);
    iterator
    insert(iterator position, value_type&& x) throw (eh::Exception);
    void
    insert(iterator position, size_type n, value_type& x)
      throw (eh::Exception);
    template <typename InputIterator>
    void
    insert(iterator position, InputIterator first, InputIterator last)
      throw (eh::Exception);

#if __GNUC__ == 4 && __GNUC_MINOR__ == 4
    void
    swap(Vector&& v) throw ();
#else
    void
    swap(Vector& v) throw ();
#endif

  private:
    size_type
    resize_(size_type n) throw (eh::Exception);
  };
}

namespace ReferenceCounting
{
  template <typename T, typename Allocator>
  Vector<T, Allocator>::Vector() throw ()
  {
  }

  template <typename T, typename Allocator>
  Vector<T, Allocator>::Vector(size_type n) throw (eh::Exception)
  {
    resize(n);
  }

  template <typename T, typename Allocator>
  Vector<T, Allocator>::Vector(size_type n, value_type& x)
    throw (eh::Exception)
  {
    assign(n, x);
  }

  template <typename T, typename Allocator>
  Vector<T, Allocator>::Vector(Vector& v) throw (eh::Exception)
    : Base()
  {
    assign(v.begin(), v.end());
  }

  template <typename T, typename Allocator>
  Vector<T, Allocator>::Vector(Vector&& v) throw ()
    : Base(std::move(v))
  {
  }

  template <typename T, typename Allocator>
  template <typename InputIterator>
  Vector<T, Allocator>::Vector(InputIterator first, InputIterator last)
    throw (eh::Exception)
  {
    assign(first, last);
  }

  template <typename T, typename Allocator>
  Vector<T, Allocator>&
  Vector<T, Allocator>::operator =(Vector& v) throw (eh::Exception)
  {
    assign(v.begin(), v.end());
    return *this;
  }

  template <typename T, typename Allocator>
  Vector<T, Allocator>&
  Vector<T, Allocator>::operator =(Vector&& v) throw ()
  {
    Base::operator =(std::move(v));
    return *this;
  }

  template <typename T, typename Allocator>
  void
  Vector<T, Allocator>::assign(size_type n, value_type& x)
    throw (eh::Exception)
  {
    n = resize_(n);
    for (iterator i(begin()); i != end(); ++i)
    {
      *i = x;
    }
    while (n--)
    {
      emplace_back(x);
    }
  }

  template <typename T, typename Allocator>
  template <typename InputIterator>
  void
  Vector<T, Allocator>::assign(InputIterator first, InputIterator last)
    throw (eh::Exception)
  {
    size_type n = std::distance(first, last);
    n = resize_(n);
    for (iterator i(begin()); i != end(); ++i)
    {
      *i = *first;
      ++first;
    }
    while (n--)
    {
      emplace_back(*first);
      ++first;
    }
  }

  template <typename T, typename Allocator>
  void
  Vector<T, Allocator>::resize(size_type n) throw (eh::Exception)
  {
    for (n = resize_(n); n--;)
    {
      emplace_back();
    }
  }

  template <typename T, typename Allocator>
  void
  Vector<T, Allocator>::resize(size_type n, value_type& x)
    throw (eh::Exception)
  {
    for (n = resize_(n); n--;)
    {
      emplace_back(x);
    }
  }

  template <typename T, typename Allocator>
  void
  Vector<T, Allocator>::push_back(value_type& x) throw (eh::Exception)
  {
    Base::emplace_back(x);
  }

  template <typename T, typename Allocator>
  void
  Vector<T, Allocator>::push_back(value_type&& x) throw (eh::Exception)
  {
    emplace_back(std::move(x));
  }

  template <typename T, typename Allocator>
  typename Vector<T, Allocator>::iterator
  Vector<T, Allocator>::insert(iterator position, value_type& x)
    throw (eh::Exception)
  {
    return emplace(position, x);
  }

  template <typename T, typename Allocator>
  typename Vector<T, Allocator>::iterator
  Vector<T, Allocator>::insert(iterator position, value_type&& x)
    throw (eh::Exception)
  {
    return emplace(position, std::move(x));
  }

  template <typename T, typename Allocator>
  void
  Vector<T, Allocator>::insert(iterator position, size_type n,
    value_type& x) throw (eh::Exception)
  {
    if (!n)
    {
      return;
    }

    size_type sz = size();
    size_type offset = position - begin();
    reserve(sz + n);
    size_type r = sz - offset;
    if (n >= r)
    {
      for (size_type i = n - r; i--;)
      {
        emplace_back(x);
      }
      iterator it = begin() + offset;
      for (size_type i = r; i--;)
      {
        value_type& v = *it;
        emplace_back(std::move(v));
        v = x;
        ++it;
      }
    }
    else
    {
      iterator it = begin() + (sz - n);
      for (size_type i = n; i--;)
      {
        emplace_back(std::move(*it));
        ++it;
      }
      it = begin() + sz;
      iterator it2 = it - n;
      for (size_type i = r - n; i--;)
      {
        --it;
        --it2;
        *it = std::move(*it2);
      }
      it = begin() + offset;
      for (size_type i = n; i--;)
      {
        *it = x;
        ++it;
      }
    }
  }

  template <typename T, typename Allocator>
  template <typename InputIterator>
  void
  Vector<T, Allocator>::insert(iterator position, InputIterator first,
    InputIterator last) throw (eh::Exception)
  {
    size_type offset = position - begin();
    size_type n = std::distance(first, last);
    {
      value_type v;
      insert(position, n, v);
    }
    for (iterator i(begin() + offset); n--; ++i)
    {
      *i = *first;
      ++first;
    }
  }

#if __GNUC__ == 4 && __GNUC_MINOR__ == 4
  template <typename T, typename Allocator>
  void
  Vector<T, Allocator>::swap(Vector&& v) throw ()
  {
    Base::swap(std::move(v));
  }
#else
  template <typename T, typename Allocator>
  void
  Vector<T, Allocator>::swap(Vector& v) throw ()
  {
    Base::swap(v);
  }
#endif

  template <typename T, typename Allocator>
  typename Vector<T, Allocator>::size_type
  Vector<T, Allocator>::resize_(size_type n) throw (eh::Exception)
  {
    size_type sz = size();
    if (sz >= n)
    {
      if (sz > n)
      {
        erase(begin() + n, end());
      }
      return 0;
    }
    reserve(n);
    return n - sz;
  }


  template <typename T, typename Allocator>
  void
  swap(Vector<T, Allocator>& x, Vector<T, Allocator>& y) throw ()
  {
    x.swap(y);
  }

#if __GNUC__ == 4 && __GNUC_MINOR__ == 4
  template <typename T, typename Allocator>
  void
  swap(Vector<T, Allocator>&& x, Vector<T, Allocator>& y) throw ()
  {
    x.swap(y);
  }

  template <typename T, typename Allocator>
  void
  swap(Vector<T, Allocator>& x, Vector<T, Allocator>&& y) throw ()
  {
    x.swap(y);
  }
#endif
}

#endif
