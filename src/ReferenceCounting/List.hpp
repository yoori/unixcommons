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



#ifndef REFERENCECOUNTING_LIST_HPP
#define REFERENCECOUNTING_LIST_HPP

#include <list>

#include <eh/Exception.hpp>


namespace ReferenceCounting
{
  /**
   * Const-preserving version of std::list.
   * No f(const T&) functions are available, they are replaced with
   * f(T&) and f(T&&) ones.
   */
  template <typename T, typename Allocator = std::allocator<T>>
  class List : protected std::list<T, Allocator>
  {
  public:
    typedef std::list<T, Allocator> Base;

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
    using Base::empty;
    using Base::front;
    using Base::back;
    using Base::emplace_front;
    using Base::emplace_back;
    using Base::pop_front;
    using Base::pop_back;
    using Base::emplace;
    using Base::erase;
    using Base::clear;
    using Base::remove;
    using Base::remove_if;
    using Base::unique;
    using Base::reverse;
    using Base::sort;

    List() throw ();
    List(size_type n) throw (eh::Exception);
    List(size_type n, value_type& x) throw (eh::Exception);
    List(List& l) throw (eh::Exception);
    List(const List&) = delete;
    List(List&& l) throw ();
    template <typename InputIterator>
    List(InputIterator first, InputIterator last) throw (eh::Exception);

    List&
    operator =(List& l) throw (eh::Exception);
    List&
    operator =(List&& l) throw ();

    void
    assign(size_type n, value_type& x) throw (eh::Exception);
    template <typename InputIterator>
    void
    assign(InputIterator first, InputIterator last) throw (eh::Exception);

    void
    push_front(value_type& x) throw (eh::Exception);
    void
    push_front(value_type&& x) throw (eh::Exception);
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

    void
    splice(iterator position, List&& l) throw ();
    void
    splice(iterator position, List&& l, iterator i) throw ();
    void
    splice(iterator position, List&& l, iterator first, iterator last)
      throw ();

#if __GNUC__ == 4 && __GNUC_MINOR__ == 4
    void
    swap(List&& l) throw ();
#else
    void
    swap(List& l) throw ();
#endif
  };
}

namespace ReferenceCounting
{
  template <typename T, typename Allocator>
  List<T, Allocator>::List() throw ()
  {
  }

  template <typename T, typename Allocator>
  List<T, Allocator>::List(size_type n) throw (eh::Exception)
  {
    for (; n; n--)
    {
      emplace_back();
    }
  }

  template <typename T, typename Allocator>
  List<T, Allocator>::List(size_type n, value_type& x) throw (eh::Exception)
  {
    assign(n, x);
  }

  template <typename T, typename Allocator>
  List<T, Allocator>::List(List& l) throw (eh::Exception)
    : Base()
  {
    assign(l.begin(), l.end());
  }

  template <typename T, typename Allocator>
  List<T, Allocator>::List(List&& l) throw ()
    : Base(std::move(l))
  {
  }

  template <typename T, typename Allocator>
  template <typename InputIterator>
  List<T, Allocator>::List(InputIterator first, InputIterator last)
    throw (eh::Exception)
  {
    assign(first, last);
  }

  template <typename T, typename Allocator>
  List<T, Allocator>&
  List<T, Allocator>::operator =(List& l) throw (eh::Exception)
  {
    assign(l.begin(), l.end());
    return *this;
  }

  template <typename T, typename Allocator>
  List<T, Allocator>&
  List<T, Allocator>::operator =(List&& l) throw ()
  {
    Base::operator =(std::move(l));
    return *this;
  }

  template <typename T, typename Allocator>
  void
  List<T, Allocator>::assign(size_type n, value_type& x)
    throw (eh::Exception)
  {
    iterator i = begin();
    for (; i != end() && n; ++i, n--)
    {
      *i = x;
    }
    if (n)
    {
      while (n--)
      {
        emplace_back(x);
      }
    }
    else
    {
      erase(i, end());
    }
  }

  template <typename T, typename Allocator>
  template <typename InputIterator>
  void
  List<T, Allocator>::assign(InputIterator first, InputIterator last)
    throw (eh::Exception)
  {
    iterator i = begin();
    for (; i != end() && first != last; ++i, ++first)
    {
      *i = *first;
    }
    if (first != last)
    {
      do
      {
        emplace_back(*first);
        ++first;
      }
      while (first != last);
    }
    else
    {
      erase(i, end());
    }
  }

  template <typename T, typename Allocator>
  void
  List<T, Allocator>::push_front(value_type& x) throw (eh::Exception)
  {
    emplace_front(x);
  }

  template <typename T, typename Allocator>
  void
  List<T, Allocator>::push_front(value_type&& x) throw (eh::Exception)
  {
    emplace_front(std::move(x));
  }

  template <typename T, typename Allocator>
  void
  List<T, Allocator>::push_back(value_type& x) throw (eh::Exception)
  {
    emplace_back(x);
  }

  template <typename T, typename Allocator>
  void
  List<T, Allocator>::push_back(value_type&& x) throw (eh::Exception)
  {
    emplace_back(std::move(x));
  }

  template <typename T, typename Allocator>
  typename List<T, Allocator>::iterator
  List<T, Allocator>::insert(iterator position, value_type& x)
    throw (eh::Exception)
  {
    return emplace(position, x);
  }

  template <typename T, typename Allocator>
  typename List<T, Allocator>::iterator
  List<T, Allocator>::insert(iterator position, value_type&& x)
    throw (eh::Exception)
  {
    return emplace(position, std::move(x));
  }

  template <typename T, typename Allocator>
  void
  List<T, Allocator>::insert(iterator position, size_type n, value_type& x)
    throw (eh::Exception)
  {
    splice(position, List(n, x));
  }

  template <typename T, typename Allocator>
  template <typename InputIterator>
  void
  List<T, Allocator>::insert(iterator position, InputIterator first,
    InputIterator last) throw (eh::Exception)
  {
    splice(position, List(first, last));
  }

  template <typename T, typename Allocator>
  void
  List<T, Allocator>::splice(iterator position, List&& l) throw ()
  {
    Base::splice(position, std::move(l));
  }

  template <typename T, typename Allocator>
  void
  List<T, Allocator>::splice(iterator position, List&& l, iterator i)
    throw ()
  {
    Base::splice(position, std::move(l), i);
  }

  template <typename T, typename Allocator>
  void
  List<T, Allocator>::splice(iterator position, List&& l, iterator first,
    iterator last) throw ()
  {
    Base::splice(position, std::move(l), first, last);
  }

#if __GNUC__ == 4 && __GNUC_MINOR__ == 4
  template <typename T, typename Allocator>
  void
  List<T, Allocator>::swap(List&& l) throw ()
  {
    Base::swap(std::move(l));
  }
#else
  template <typename T, typename Allocator>
  void
  List<T, Allocator>::swap(List& l) throw ()
  {
    Base::swap(l);
  }
#endif


  template <typename T, typename Allocator>
  void
  swap(List<T, Allocator>& x, List<T, Allocator>& y) throw ()
  {
    x.swap(y);
  }

#if __GNUC__ == 4 && __GNUC_MINOR__ == 4
  template <typename T, typename Allocator>
  void
  swap(List<T, Allocator>&& x, List<T, Allocator>& y) throw ()
  {
    x.swap(y);
  }

  template <typename T, typename Allocator>
  void
  swap(List<T, Allocator>& x, List<T, Allocator>&& y) throw ()
  {
    x.swap(y);
  }
#endif
}

#endif
