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



#ifndef GENERICS_GOOGLE_HASH_TABLE_HPP
#define GENERICS_GOOGLE_HASH_TABLE_HPP

#include <unordered_set>
#include <unordered_map>
#include <functional>

#include <Generics/HashTableAdapters.hpp>


namespace Generics
{  
  template <class Key>
  struct HashFunForHashAdapter
  {
    size_t
    operator()(const Key& value) const throw (eh::Exception);
  };

  template <class Key, class Value,
    class Alloc = std::allocator<std::pair<const Key, Value> >,
    class EqualKey = std::equal_to<Key> >
  class GnuHashTable :
    public std::unordered_map<Key, Value, HashFunForHashAdapter<Key>,
      EqualKey,
      typename Alloc::template rebind<std::pair<const Key, Value> >::other>
  {
  private:
    typedef std::unordered_map<Key, Value, HashFunForHashAdapter<Key>,
      EqualKey,
      typename Alloc::template rebind<std::pair<const Key, Value> >::other>
      Parent;

  public:
    typedef size_t size_type;
    typedef Value data_type;

    GnuHashTable(size_t table_size = 0) throw (eh::Exception);

    size_type
    table_size() const throw ();

    void
    table_size(const size_t&) throw ();
    void
    optimize() throw ();

    bool
    operator ==(const GnuHashTable& table) const throw ();
  };

  template <class Key, class Alloc = std::allocator<Key>,
    class EqualKey = std::equal_to<Key> >
  class GnuHashSet :
    public std::unordered_set<Key, HashFunForHashAdapter<Key>,
      EqualKey, typename Alloc::template rebind<Key>::other>
  {
  public:
    typedef std::unordered_set<Key, HashFunForHashAdapter<Key>,
      EqualKey, typename Alloc::template rebind<Key>::other>
      Parent;

    typedef Key key_type;
    typedef size_t size_type;

    bool
    operator ==(const GnuHashSet& set) const throw ();
  };
}

//
// INLINES
//

namespace Generics
{
  //
  // HashFunForHashAdapter class
  //

  template <class Key>
  size_t
  HashFunForHashAdapter<Key>::operator()(const Key& value) const
    throw (eh::Exception)
  {
    return static_cast<size_t>(value.hash());
  }

  //
  // GnuHashTable class
  //

  template <class Key, class Value, class Alloc, class EqualKey>
  GnuHashTable<Key, Value, Alloc, EqualKey>::GnuHashTable(size_t table_size)
    throw (eh::Exception)
    : Parent(table_size)
  {
  }

  template <class Key, class Value, class Alloc, class EqualKey>
  typename GnuHashTable<Key, Value, Alloc, EqualKey>::size_type
  GnuHashTable<Key, Value, Alloc, EqualKey>::table_size() const throw ()
  {
    return Parent::size();
  }

  template <class Key, class Value, class Alloc, class EqualKey>
  void
  GnuHashTable<Key, Value, Alloc, EqualKey>::optimize() throw ()
  {
  }

  template <class Key, class Value, class Alloc, class EqualKey>
  void
  GnuHashTable<Key, Value, Alloc, EqualKey>::table_size(
    const size_t& new_size) throw ()
  {
    Parent::resize(new_size);
  }

  template <class Key, class Value, class Alloc, class EqualKey>
  bool
  GnuHashTable<Key, Value, Alloc, EqualKey>::operator ==(
    const GnuHashTable& table) const throw ()
  {
    if (this->size() != table.size())
    {
      return false;
    }
    for (typename Parent::const_iterator itor(this->begin());
      itor != this->end(); ++itor)
    {
      typename Parent::const_iterator found(table.find(itor->first));
      if (found == table.end() || !(itor->second == found->second))
      {
        return false;
      }
    }
    return true;
  }

  //
  // GnuHashSet class
  //

  template <class Key, class Alloc, class EqualKey>
  bool
  GnuHashSet<Key, Alloc, EqualKey>::operator ==(const GnuHashSet& set) const
    throw ()
  {
    if (this->size() != set.size())
    {
      return false;
    }
    for (typename Parent::const_iterator itor(this->begin());
      itor != this->end(); ++itor)
    {
      if (set.find(itor->first) == set.end())
      {
        return false;
      }
    }
    return true;
  }
}

#endif
