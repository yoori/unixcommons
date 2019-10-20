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



#ifndef REFERENCECOUNTING_HASHTABLE_HPP
#define REFERENCECOUNTING_HASHTABLE_HPP

#include <unordered_map>

#include <ReferenceCounting/Containers.hpp>


namespace ReferenceCounting
{
  /**
   * Const-preserving version of std::unordered_map.
   * No f(const T&) functions are available, they are replaced with
   * f(T&) and f(T&&) ones.
   */
  template <typename Key, typename T,
    typename EqualKey = std::equal_to<Key>,
    typename Allocator = std::allocator<std::pair<const Key, T>>>
  class HashTable :
    protected std::unordered_map<Key, T,
      Helper::HashFunForHashAdapter<Key>, EqualKey,
      typename Helper::Allocator<std::pair<const Key, T>, Allocator>>
  {
  public:
    typedef std::unordered_map<Key, T,
      Helper::HashFunForHashAdapter<Key>, EqualKey,
      typename Helper::Allocator<std::pair<const Key, T>, Allocator>>
      Base;

    typedef typename Base::key_type key_type;
    typedef typename Base::mapped_type mapped_type;
    typedef typename Base::value_type value_type;
    typedef typename Base::hasher hasher;
    typedef typename Base::key_equal key_equal;
    typedef typename Base::pointer pointer;
    typedef typename Base::const_pointer const_pointer;
    typedef typename Base::reference reference;
    typedef typename Base::const_reference const_reference;
    typedef typename Base::iterator iterator;
    typedef typename Base::const_iterator const_iterator;
    typedef typename Base::size_type size_type;
    typedef typename Base::difference_type difference_type;

    using Base::begin;
    using Base::end;
    using Base::cbegin;
    using Base::cend;
    using Base::size;
    using Base::max_size;
    using Base::bucket_count;
    using Base::max_bucket_count;
    using Base::operator [];
    using Base::at;
    using Base::empty;
    using Base::erase;
    using Base::clear;
    using Base::key_eq;
    using Base::find;
    using Base::count;
    using Base::equal_range;

    explicit
    HashTable(size_type n = 10) throw (eh::Exception);
    HashTable(HashTable& h) throw (eh::Exception);
    HashTable(const HashTable&) = delete;
    HashTable(HashTable&& h) throw ();
    template <typename InputIterator>
    HashTable(InputIterator first, InputIterator last, size_type n = 10)
      throw (eh::Exception);

    HashTable&
    operator =(HashTable& h) throw (eh::Exception);
    HashTable&
    operator =(HashTable&& h) throw ();

    std::pair<iterator, bool>
    insert(value_type& x) throw (eh::Exception);
    std::pair<iterator, bool>
    insert(value_type&& x) throw (eh::Exception);
    iterator
    insert(iterator position, value_type& x) throw (eh::Exception);
    iterator
    insert(iterator position, value_type&& x) throw (eh::Exception);
    template <typename InputIterator>
    void
    insert(InputIterator first, InputIterator last) throw (eh::Exception);

#if __GNUC__ == 4 && __GNUC_MINOR__ == 4
    void
    swap(HashTable&& h) throw ();
#else
    void
    swap(HashTable& h) throw ();
#endif

  private:
    value_type
    value_type_(value_type& x) throw (eh::Exception);
  };

  template <typename Allocator = std::allocator<char>,
    template <typename> class EqualKey = std::equal_to>
  struct HashTableBind
  {
    template <typename Key, typename T>
    struct Rebind
    {
      typedef HashTable<Key, T, EqualKey<Key>, Allocator> Type;
    };
  };
}

namespace ReferenceCounting
{
  template <typename Key, typename T, typename EqualKey, typename Allocator>
  HashTable<Key, T, EqualKey, Allocator>::HashTable(size_type n)
    throw (eh::Exception)
    : Base(n)
  {
  }

  template <typename Key, typename T, typename EqualKey, typename Allocator>
  HashTable<Key, T, EqualKey, Allocator>::HashTable(HashTable& h)
    throw (eh::Exception)
    : Base()
  {
    insert(h.begin(), h.end());
  }

  template <typename Key, typename T, typename EqualKey, typename Allocator>
  HashTable<Key, T, EqualKey, Allocator>::HashTable(HashTable&& h)
    throw ()
    : Base(std::move(h))
  {
  }

  template <typename Key, typename T, typename EqualKey, typename Allocator>
  template <typename InputIterator>
  HashTable<Key, T, EqualKey, Allocator>::HashTable(InputIterator first,
    InputIterator last, size_type n) throw (eh::Exception)
    : Base(n)
  {
    insert(first, last);
  }

  template <typename Key, typename T, typename EqualKey, typename Allocator>
  HashTable<Key, T, EqualKey, Allocator>&
  HashTable<Key, T, EqualKey, Allocator>::operator =(HashTable& h)
    throw (eh::Exception)
  {
    {
      HashTable h1(h);
      swap(h1);
    }
    return *this;
  }

  template <typename Key, typename T, typename EqualKey, typename Allocator>
  HashTable<Key, T, EqualKey, Allocator>&
  HashTable<Key, T, EqualKey, Allocator>::operator =(HashTable&& h)
    throw ()
  {
    Base::operator =(std::move(h));
    return *this;
  }

  template <typename Key, typename T, typename EqualKey, typename Allocator>
  std::pair<typename HashTable<Key, T, EqualKey, Allocator>::iterator, bool>
  HashTable<Key, T, EqualKey, Allocator>::insert(value_type& x)
    throw (eh::Exception)
  {
    return insert(value_type_(x));
  }

  template <typename Key, typename T, typename EqualKey, typename Allocator>
  std::pair<typename HashTable<Key, T, EqualKey, Allocator>::iterator, bool>
  HashTable<Key, T, EqualKey, Allocator>::insert(value_type&& x)
    throw (eh::Exception)
  {
    return Base::insert(x);
  }

  template <typename Key, typename T, typename EqualKey, typename Allocator>
  typename HashTable<Key, T, EqualKey, Allocator>::iterator
  HashTable<Key, T, EqualKey, Allocator>::insert(
    iterator position, value_type& x) throw (eh::Exception)
  {
    return insert(position, value_type_(x));
  }

  template <typename Key, typename T, typename EqualKey, typename Allocator>
  typename HashTable<Key, T, EqualKey, Allocator>::iterator
  HashTable<Key, T, EqualKey, Allocator>::insert(
    iterator position, value_type&& x) throw (eh::Exception)
  {
    return Base::insert(position, x);
  }

  template <typename Key, typename T, typename EqualKey, typename Allocator>
  template <typename InputIterator>
  void
  HashTable<Key, T, EqualKey, Allocator>::insert(InputIterator first,
    InputIterator last) throw (eh::Exception)
  {
    for (; first != last; ++first)
    {
      insert(value_type((*first).first, (*first).second));
    }
  }

#if __GNUC__ == 4 && __GNUC_MINOR__ == 4
  template <typename Key, typename T, typename EqualKey, typename Allocator>
  void
  HashTable<Key, T, EqualKey, Allocator>::swap(HashTable&& h) throw ()
  {
    Base::swap(std::move(h));
  }
#else
  template <typename Key, typename T, typename EqualKey, typename Allocator>
  void
  HashTable<Key, T, EqualKey, Allocator>::swap(HashTable& h) throw ()
  {
    Base::swap(h);
  }
#endif

  template <typename Key, typename T, typename EqualKey, typename Allocator>
  typename HashTable<Key, T, EqualKey, Allocator>::value_type
  HashTable<Key, T, EqualKey, Allocator>::value_type_(value_type& x)
    throw (eh::Exception)
  {
    return value_type(x.first, x.second);
  }


  template <typename Key, typename T, typename EqualKey, typename Allocator>
  void
  swap(HashTable<Key, T, EqualKey, Allocator>& x,
    HashTable<Key, T, EqualKey, Allocator>& y) throw ()
  {
    x.swap(y);
  }

#if __GNUC__ == 4 && __GNUC_MINOR__ == 4
  template <typename Key, typename T, typename EqualKey, typename Allocator>
  void
  swap(HashTable<Key, T, EqualKey, Allocator>&& x,
    HashTable<Key, T, EqualKey, Allocator>& y) throw ()
  {
    x.swap(y);
  }

  template <typename Key, typename T, typename EqualKey, typename Allocator>
  void
  swap(HashTable<Key, T, EqualKey, Allocator>& x,
    HashTable<Key, T, EqualKey, Allocator>&& y) throw ()
  {
    x.swap(y);
  }
#endif
}

#endif
