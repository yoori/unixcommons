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



#ifndef GENERICS_BOUNDED_MAP_HPP
#define GENERICS_BOUNDED_MAP_HPP

#include <list>
#include <cassert>

#include <Sync/SyncPolicy.hpp>

#include <ReferenceCounting/HashTable.hpp>

#include <Generics/Time.hpp>
#include <Generics/TypeTraits.hpp>


namespace Generics
{
  struct BoundedMapStat
  {
    BoundedMapStat() throw ();

    int inserted_new;      // A new value was inserted into container
    int insert_existing;   // Tried to insert a value with existing key
    int removed_outdated;  // Removed old values to insert something new
    int removed_updated;   // Removed values not fit after update or replace
    int not_inserted;      // Failed to insert because of bound
    int replaced;          // Successfully replaced.

    // removed_outdated is less than inserted_new always!
    // removed_outdated + removed_updated is less than or
    // equal to inserted_new
    // total number of successful (without exception) insert() calls =
    //   inserted_new + insert_existing + not_inserted
  };

  template <typename Key, typename Data>
  struct DefaultSizePolicy
  {
    size_t
    operator ()(const Key&, const Data&) const throw ();
  };

  /**
   * Helper class for Container specification of BoundedMap class
   */
  template <typename Key, typename Data>
  struct BoundedMapTypes
  {
    /**
     * Ordered is used in ordered list for determination of least recent
     * used item
     */
    struct Ordered
    {
      Key key;
      Time last_used;
    };

    typedef std::list<Ordered> Queue;

    /**
     * Item is stored in the hash allowing to update last used time on usage
     */
    struct Item
    {
      Data data;
      size_t size;
      typename Queue::iterator order;

      Item(Data& data, size_t size, typename Queue::iterator order)
        throw (eh::Exception);
      Item(Data&& data, size_t size, typename Queue::iterator order)
        throw (eh::Exception);
      Item(Item&& i) throw ();
      Item(Item&) throw () = delete;
      Item(const Item&) throw () = delete;
    };
  };


  /**
   * Class designed to be an associative [hash-based] container with limited
   * capacity. find() and insert() updates the last usage time of the item
   * and if it is required to insert another item and capacity is exceeded
   * and the least recent item was used more than timeout ago it is removed
   * from the container in order to add a new item. When it is impossible
   * to insert a new item iterator equal to end() is returned.
   *
   * Container is thread safety compliant (with a proper SyncPolicy).
   *
   * Iterators hold value of the item (not a reference to it) because an item
   * referenced can be removed during usage of iterator. It makes difficult
   * to use this container with types differing from [smart] pointers.
   *
   * Iterations through iterators (++ and --) are not design compliant.
   */
  template <typename Key, typename Data,
    typename SizePolicy = DefaultSizePolicy<Key, Data>,
    typename SyncPolicy = Sync::Policy::PosixThread,
    typename Container =
      ReferenceCounting::HashTable<Key,
        typename BoundedMapTypes<Key, Data>::Item> >
  class BoundedMap
  {
  public:
    typedef Key key_type;
    typedef Data data_type;
    typedef Data mapped_type;
    typedef std::pair<const Key, Data> value_type;

    typedef std::pair<Key, Data> Value;
    typedef std::pair<const Key&, Data&> ValueRef;
    typedef std::pair<const Key&, const Data&> ValueCRef;

    typedef typename Container::size_type size_type;
    typedef PairPtr<Value, ValueRef> pointer;
    typedef PairPtr<const Value, ValueCRef> const_pointer;
    typedef ValueRef reference;
    typedef ValueCRef const_reference;

    /**
     * Iterator Base
     * It is a base for iterators
     * Holds both key and data
     * Provides comparison between iterators (iterators are equal only if
     * they are both equal to end()).
     */
    class IteratorBase
    {
    public:
      /**
       * Equal comparison operator
       * @param itor another IteratorBase to compare with
       * @return true only if both this and itor equal to end()
       */
      bool
      operator ==(const IteratorBase& itor) const throw ();

      /**
       * Non-equal comparison operator
       * @param itor another IteratorBase to compare with
       * @return false only if both this and itor equal to end()
       */
      bool
      operator !=(const IteratorBase& itor) const throw ();

      /**
       * Asterisk operator
       * @return reference (const) to the stored pair of key and data
       */
      const_reference
      operator *() const throw ();

      /**
       * Arrow operator
       * @return pointer (const) to the stored pair of key and data
       */
      const_pointer
      operator ->() const throw ();

    protected:
      /**
       * Constructor
       * Creates IteratorBase equal to end()
       */
      IteratorBase() throw (eh::Exception);

      /**
       * Constructor
       * Creates IteratorBase non-equal to end()
       * @param key key of the item
       * @param data data of the item
       */
      IteratorBase(const key_type& key, data_type& data)
        throw (eh::Exception);

      /**
       * Constructor
       * Copies data from another IteratorBase
       * @param itor another IteratorBase
       */
      IteratorBase(IteratorBase& itor) throw (eh::Exception);

      /**
       * Constructor
       * Moves data from another IteratorBase
       * @param itor another IteratorBase
       */
      IteratorBase(IteratorBase&& itor) throw ();

      /**
       * Destructor
       * Destroys value_ if necessary
       */
      ~IteratorBase() throw ();

      /**
       * Assignment operator
       * Copies value from another one
       * @param itor source iterator
       */
      void
      operator =(IteratorBase& itor) throw (eh::Exception);

      /**
       * Assignment operator
       * Moves value from another one
       * @param itor source iterator
       */
      void
      operator =(IteratorBase&& itor) throw ();

      /**
       * Destroys value_ if necessary
       */
      void
      clear_() throw ();


      char buf_[sizeof(Value)];
      Value* value_;

      friend class BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>;
    };

    /**
     * Const iterator
     * Return type of find() const and end() const member functions
     * Allows usage of found element outside of container and comparison
     * of success of find() and insert() operations
     */
    class const_iterator : public IteratorBase
    {
    public:
      /**
       * Constructor
       * Creates const iterator equal to end()
       */
      const_iterator() throw (eh::Exception) = default;

      /**
       * Constructor
       * Creates const iterator non-equal to end()
       * @param key key of the item
       * @param data data of the item
       */
      const_iterator(const key_type& key, data_type& data)
        throw (eh::Exception);

      /**
       * Constructor
       * Creates a copy of another one
       * @param itor source iterator
       */
      const_iterator(const IteratorBase& itor) throw (eh::Exception);

      /**
       * Constructor
       * Moves from another one
       * @param itor source iterator
       */
      const_iterator(IteratorBase&& itor) throw (eh::Exception);

      /**
       * Assignment operator
       * Copies value from another one
       * @param itor source iterator
       */
      const_iterator&
      operator =(const IteratorBase& itor) throw (eh::Exception);

      /**
       * Assignment operator
       * Moves value from another one
       * @param itor source iterator
       */
      const_iterator&
      operator =(IteratorBase&& itor) throw (eh::Exception);
    };

    /**
     * Iterator
     * Return type of find() and insert() member functions
     * Allows usage of found or inserted element outside of container
     */
    class iterator : public IteratorBase
    {
    public:
      using IteratorBase::operator *;
      using IteratorBase::operator ->;

      /**
       * Asterisk operator
       * @return reference to the stored pair of key and data
       */
      reference
      operator *() throw ();

      /**
       * Arrow operator
       * @return pointer to the stored pair of key and data
       */
      pointer
      operator ->() throw ();

      /**
       * Constructor
       * Creates iterator equal to end()
       */
      iterator() throw (eh::Exception) = default;

      /**
       * Constructor
       * Creates iterator non-equal to end()
       * @param key key of the item
       * @param data data of the item
       */
      iterator(const key_type& key, data_type& data) throw (eh::Exception);

      /**
       * Constructor
       * Creates a copy of another one
       * @param itor source iterator
       */
      iterator(iterator& itor) throw (eh::Exception);

      iterator(const iterator&) = delete;

      /**
       * Constructor
       * Moves from another one
       * @param itor source iterator
       */
      iterator(iterator&& itor) throw (eh::Exception);

      /**
       * Assignment operator
       * Copies value from another one
       * @param itor source iterator
       */
      iterator&
      operator =(iterator& itor) throw (eh::Exception);

      /**
       * Assignment operator
       * Moves value from another one
       * @param itor source iterator
       */
      iterator&
      operator =(iterator&& itor) throw (eh::Exception);
    };

    class Inserter
    {
    public:
      void
      operator =(Data& data) throw (eh::Exception);
      void
      operator =(Data&& data) throw (eh::Exception);

    private:
      Inserter(BoundedMap& map, const Key& key) throw ();
      Inserter(Inserter&& inserter) throw ();

      BoundedMap& map_;
      const Key& key_;

      friend class BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>;
    };


    /**
     * Constructor
     * @param bound upper limit of number of elements allows to be stored
     * (positive)
     * @param timeout time interval allowing to name an element outdated
     * @param size_policy size policy object
     */
    BoundedMap(size_type bound, const Time& timeout,
      SizePolicy size_policy = SizePolicy())
      throw (eh::Exception);

    /**
     * Constructor with one template argument for container's constructor
     * @param bound upper limit of number of elements allows to be stored
     * (positive)
     * @param timeout time interval allowing to name an element outdated
     * @param size_policy size policy object
     * @param args arguments for container's constructor
     */
    template <typename... T>
    BoundedMap(size_type bound, const Time& timeout,
      SizePolicy size_policy, T... args)
      throw (eh::Exception);

    /**
     * Finds element by the key.
     * Sets the most recency for the found item.
     * @param key key to search for
     * @return iterator with the value of found element or
     * iterator equal to end() if not found
     */
    iterator
    find(const key_type& key) throw (eh::Exception);

    /**
     * Finds element by the key (const version).
     * Sets the most recency for the found item.
     * @param key key to search for
     * @return const iterator with the value of found element or
     * const iterator equal to end() if not found
     */
    const_iterator
    find(const key_type& key) const throw (eh::Exception);

    /**
     * Tries to inserts another item into the container.
     * If an item with the equal key exists no insertion occur.
     * If the bound is reached and some least recent items (with
     * insufficient size) are outdated enough they are removed from
     * the container.
     * Most recency is set for returned (existing or inserted) element.
     * This function also updates usage statistics.
     * @param value value to insert
     * @return pair<end(), false> if insert is impossible or
     * pair<iterator to the existing/inserted item,
     * whether or not insertion occured>
     */
    std::pair<iterator, bool>
    insert(value_type& value) throw (eh::Exception);

    /**
     * Tries to inserts another item into the container.
     * If an item with the equal key exists no insertion occur.
     * If the bound is reached and some least recent items (with
     * insufficient size) are outdated enough they are removed from
     * the container.
     * Most recency is set for returned (existing or inserted) element.
     * This function also updates usage statistics.
     * @param value value to insert (move semantic is used)
     * @return pair<end(), false> if insert is impossible or
     * pair<iterator to the existing/inserted item,
     * whether or not insertion occured>
     */
    std::pair<iterator, bool>
    insert(value_type&& value) throw (eh::Exception);

    /**
     * Updates the size of the specified item
     * If there is no item with such key in the container, nothing happens
     * If the new size is small enough not to reach the bound
     * only size is updated
     * If it is impossible to remove enough outdated items not to reach
     * the bound the item is removed
     * Otherwise some of the outdated elements are removed and the
     * size is updated
     * @param key key describing changing element
     */
    void
    update(const Key& key) throw ();

    /**
     * Updates the size of the specified item
     * If there is no item with such key in the container, nothing happens
     * If the new size is small enough not to reach the bound
     * only size is updated
     * If it is impossible to remove enough outdated items not to reach
     * the bound the item is removed
     * Otherwise some of the outdated elements are removed and the
     * size is updated
     * @param iterator iterator describing changing element
     */
    void
    update(const IteratorBase& iterator) throw ();

    /**
     * Either replaces the existing item or tries to insert it if it's
     * absent.
     * It also calls update() allowing to erase the item if it's too big.
     */
    void
    insert_or_update(const Key& key, Data& data) throw (eh::Exception);

    /**
     * Either replaces the existing item or tries to insert it if it's
     * absent.
     * It also calls update() allowing to erase the item if it's too big.
     * Move semantics is used.
     */
    void
    insert_or_update(const Key& key, Data&& data) throw (eh::Exception);

    /**
     * Calls insert_or_update in std::map-compatible way.
     * @param key key of the item
     * @return proxy object allowing assignment of Data
     */
    Inserter
    operator [](const Key& key) throw ();

    /**
     * Removes the item from the container by the key.
     * It is possible to remove a different item with the same key.
     * @param key item key
     */
    void
    erase(const Key& key) throw ();

    /**
     * Removes the item from the container by the key.
     * It is possible to remove a different item with the same key.
     * @param itor item descriptor
     */
    void
    erase(const IteratorBase& itor) throw ();

    /**
     * Clears the container
     */
    void
    clear() throw ();

    /**
     * Beyond-the-last iterator is required for success test of
     * find() and insert()
     * @return iterator referencing to nothing
     */
    const const_iterator&
    end() const throw ();

    /**
     * Actual number of elements stored
     * @return number of elements in the map
     */
    size_type
    size() const throw ();

    /**
     * Copies all of value pairs to insert iterator
     * @param insert insert iterator to copy data to
     * @result value of insert iterator after copying
     */
    template <typename InsertIterator>
    InsertIterator
    copy_to(InsertIterator insert) throw (eh::Exception);

    /**
     * Container usage statistics
     * @param reset whether or not reset usage statistics
     * @return gathered statistics
     */
    BoundedMapStat
    statistics(bool reset = false) throw ();

    /**
     * Current bound limit
     * @return current bound limit
     */
    size_type
    bound() const throw ();

    /**
     * Sets new bound limit. No removal of extra elements is made
     * @param new_bound new bound limit
     */
    void
    bound(size_type new_bound) throw ();

    /**
     * Current expiration timeout
     * @return expiration timeout
     */
    Time
    timeout() const throw ();

    /**
     * Sets new expiration timeout. No removal of expired elements is made
     */
    void
    timeout(Time new_timeout) throw ();


  private:
    typedef typename BoundedMapTypes<Key, Data>::Ordered Ordered;
    typedef typename BoundedMapTypes<Key, Data>::Queue Queue;
    typedef typename BoundedMapTypes<Key, Data>::Item Item;

    typename Container::iterator
    find_(const key_type& key, const Time& now) const throw ();

    template <typename DataType>
    std::pair<iterator, bool>
    insert_(const Key& key, DataType&& data, const Time& now)
      throw (eh::Exception);

    template <typename ValueType>
    std::pair<iterator, bool>
    insert_(ValueType&& value) throw (eh::Exception);

    bool
    update_(typename Container::iterator itor, const Time& now) throw ();

    template <typename DataType>
    void
    insert_or_update_(const Key& key, DataType&& data)
      throw (eh::Exception);


    size_type bound_;
    Time timeout_;
    SizePolicy size_policy_;

    mutable typename SyncPolicy::Mutex mutex_;

    Container container_;
    mutable Queue queue_;
    size_t size_;
    BoundedMapStat stat_;

    const const_iterator END_CONST_ITERATOR;
  };
}

/*
 * INLINES
 */
namespace Generics
{
  //
  // BoundedMapStat class
  //

  inline
  BoundedMapStat::BoundedMapStat() throw ()
    : inserted_new(0), insert_existing(0),
      removed_outdated(0), removed_updated(0), not_inserted(0),
      replaced(0)
  {
  }


  //
  // DefaultSizePolicy class
  //

  template <typename Key, typename Data>
  size_t
  DefaultSizePolicy<Key, Data>::operator ()(const Key&, const Data&) const
    throw ()
  {
    return 1;
  }


  //
  // BoundedMapTypes::Item
  //

  template <typename Key, typename Data>
  BoundedMapTypes<Key, Data>::Item::Item(Data& data, size_t size,
    typename Queue::iterator order) throw (eh::Exception)
    : data(data), size(size), order(std::move(order))
  {
  }

  template <typename Key, typename Data>
  BoundedMapTypes<Key, Data>::Item::Item(Data&& data, size_t size,
    typename Queue::iterator order) throw (eh::Exception)
    : data(std::move(data)), size(size), order(std::move(order))
  {
  }

  template <typename Key, typename Data>
  BoundedMapTypes<Key, Data>::Item::Item(Item&& i) throw ()
    : data(std::move(i.data)), size(i.size), order(std::move(i.order))
  {
  }


  //
  // BoundedMap::IteratorBase class
  //

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    IteratorBase::IteratorBase() throw (eh::Exception)
    : value_(nullptr)
  {
  }

#if __GNUC__ == 4 && __GNUC_MINOR__ == 4
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif
  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    IteratorBase::IteratorBase(const key_type& key, data_type& data)
    throw (eh::Exception)
    : value_(new (buf_) Value(key, data))
  {
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    IteratorBase::IteratorBase(IteratorBase& itor) throw (eh::Exception)
    : value_(itor.value_ ? new (buf_) Value(*itor.value_) : nullptr)
  {
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    IteratorBase::IteratorBase(IteratorBase&& itor) throw ()
    : value_(itor.value_ ? new (buf_) Value(std::move(*itor.value_)) :
        nullptr)
  {
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    IteratorBase::clear_() throw ()
  {
    if (value_)
    {
      value_->~Value();
      value_ = nullptr;
    }
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    IteratorBase::~IteratorBase() throw ()
  {
    clear_();
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    IteratorBase::operator =(IteratorBase& itor) throw (eh::Exception)
  {
    if (itor.value_)
    {
      if (value_)
      {
        *value_ = *itor.value_;
      }
      else
      {
        value_ = new (buf_) Value(*itor.value_);
      }
    }
    else
    {
      clear_();
    }
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    IteratorBase::operator =(IteratorBase&& itor) throw ()
  {
    if (itor.value_)
    {
      if (value_)
      {
        *value_ = std::move(*itor.value_);
      }
      else
      {
        value_ = new (buf_) Value(std::move(*itor.value_));
      }
    }
    else
    {
      clear_();
    }
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  bool
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    IteratorBase::operator ==(const IteratorBase& itor) const throw ()
  {
    return !value_ && !itor.value_;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  bool
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    IteratorBase::operator !=(const IteratorBase& itor) const throw ()
  {
    return !operator ==(itor);
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    const_reference
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    IteratorBase::operator *() const throw ()
  {
    assert(value_);
    return *value_;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    const_pointer
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    IteratorBase::operator ->() const throw ()
  {
    assert(value_);
    return value_;
  }


  //
  // BoundedMap::const_iterator class
  //

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    const_iterator::const_iterator(const key_type& key, data_type& data)
    throw (eh::Exception)
    : IteratorBase(key, data)
  {
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    const_iterator::const_iterator(const IteratorBase& itor)
    throw (eh::Exception)
    : IteratorBase(const_cast<IteratorBase&>(itor))
  {
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    const_iterator::const_iterator(IteratorBase&& itor)
    throw (eh::Exception)
    : IteratorBase(std::move(itor))
  {
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    const_iterator&
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    const_iterator::operator =(const IteratorBase& itor)
    throw (eh::Exception)
  {
    IteratorBase::operator =(const_cast<IteratorBase&>(itor));
    return *this;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    const_iterator&
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    const_iterator::operator =(IteratorBase&& itor) throw (eh::Exception)
  {
    IteratorBase::operator =(std::move(itor));
    return *this;
  }


  //
  // BoundedMap::iterator class
  //

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    iterator::iterator(const key_type& key, data_type& data)
    throw (eh::Exception)
    : IteratorBase(key, data)
  {
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    iterator::iterator(iterator& itor) throw (eh::Exception)
    : IteratorBase(itor)
  {
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    iterator::iterator(iterator&& itor) throw (eh::Exception)
    : IteratorBase(std::move(itor))
  {
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    iterator&
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    iterator::operator =(iterator& itor) throw (eh::Exception)
  {
    IteratorBase::operator =(itor);
    return *this;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    iterator&
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    iterator::operator =(iterator&& itor) throw (eh::Exception)
  {
    IteratorBase::operator =(std::move(itor));
    return *this;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    reference
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    iterator::operator *() throw ()
  {
    assert(this->value_);
    return *this->value_;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::pointer
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    iterator::operator ->() throw ()
  {
    assert(this->value_);
    return this->value_;
  }


  //
  // BoundedMap::Inserter class
  //

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    Inserter::Inserter(BoundedMap& map, const Key& key) throw ()
    : map_(map), key_(key)
  {
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    Inserter::Inserter(Inserter&& inserter) throw ()
    : map_(inserter.map_), key_(inserter.key_)
  {
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    Inserter::operator =(Data& data) throw (eh::Exception)
  {
    map_.insert_or_update_(key_, data);
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    Inserter::operator =(Data&& data) throw (eh::Exception)
  {
    map_.insert_or_update_(key_, std::move(data));
  }


  //
  // BoundedMap class
  //

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    BoundedMap(size_type bound, const Time& timeout,
      SizePolicy size_policy) throw (eh::Exception)
    : bound_(bound), timeout_(timeout), size_policy_(size_policy), size_(0),
      END_CONST_ITERATOR()
  {
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  template <typename... T>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    BoundedMap(size_type bound, const Time& timeout,
      SizePolicy size_policy, T... args) throw (eh::Exception)
    : bound_(bound), timeout_(timeout), size_policy_(size_policy),
      container_(std::forward<T>(args)...), size_(0),
      END_CONST_ITERATOR()
  {
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename Container::iterator
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    find_(const key_type& key, const Time& now) const throw ()
  {
    typename Container::iterator itor(
      const_cast<Container&>(container_).find(key));

    if (itor != container_.end())
    {
      itor->second.order->last_used = now;
      queue_.splice(queue_.end(), queue_, itor->second.order);
    }

    return itor;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    iterator
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    find(const key_type& key) throw (eh::Exception)
  {
    Time now(Time::get_time_of_day());

    typename SyncPolicy::WriteGuard guard(mutex_);

    typename Container::iterator itor(find_(key, now));
    return itor == container_.end() ? iterator() :
      iterator(key, itor->second.data);
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    const_iterator
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    find(const key_type& key) const throw (eh::Exception)
  {
    Time now(Time::get_time_of_day());

    typename SyncPolicy::WriteGuard guard(mutex_);

    typename Container::const_iterator itor(find_(key, now));
    return itor == container_.end() ? end() :
      const_iterator(key, itor->second.data);
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  template <typename DataType>
  std::pair<
    typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
      iterator, bool>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    insert_(const Key& key, DataType&& data, const Time& now)
      throw (eh::Exception)
  {
    size_t size = size_policy_(key, data);
    if (size > bound_)
    {
      stat_.not_inserted++;
      return std::pair<iterator, bool>(iterator(), false);
    }

    while (size_ + size > bound_)
    {
      typename Queue::iterator order(queue_.begin());
      if (order == queue_.end() || order->last_used + timeout_ > now)
      {
        stat_.not_inserted++;
        return std::pair<iterator, bool>(iterator(), false);
      }

      stat_.removed_outdated++;
      typename Container::iterator itor(container_.find(order->key));
      assert(itor != container_.end());
      size_ -= itor->second.size;
      container_.erase(itor);
      queue_.erase(order);
    }

    Ordered ordered = { key, now };
    typename Queue::iterator order(queue_.insert(queue_.end(), ordered));

    std::pair<typename Container::iterator, bool> result;
    try
    {
      Item item(std::forward<DataType>(data), size, order);
      result = container_.insert(
        typename Container::value_type(key, std::move(item)));
      assert(result.second);
      stat_.inserted_new++;
      size_ += size;
    }
    catch (...)
    {
      queue_.erase(order);
      throw;
    }
    return std::pair<iterator, bool>(
      iterator(result.first->first, result.first->second.data), true);
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  template <typename ValueType>
  std::pair<
    typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
      iterator, bool>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    insert_(ValueType&& value) throw (eh::Exception)
  {
    Time now(Time::get_time_of_day());

    typename SyncPolicy::WriteGuard guard(mutex_);

    {
      typename Container::iterator itor(find_(value.first, now));
      if (itor != container_.end())
      {
        stat_.insert_existing++;
        return std::pair<iterator, bool>(
          iterator(itor->first, itor->second.data), false);
      }
    }

    return insert_(value.first, std::forward<ValueType>(value).second, now);
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  std::pair<
    typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
      iterator, bool>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    insert(value_type& value) throw (eh::Exception)
  {
    return insert_(value);
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  std::pair<
    typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
      iterator, bool>
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    insert(value_type&& value) throw (eh::Exception)
  {
    return insert_(std::move(value));
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  template <typename DataType>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    insert_or_update_(const Key& key, DataType&& data) throw (eh::Exception)
  {
    Time now(Time::get_time_of_day());

    typename SyncPolicy::WriteGuard guard(mutex_);

    {
      typename Container::iterator itor(find_(key, now));
      if (itor != container_.end())
      {
        itor->second.data = std::forward<DataType>(data);
        if (update_(itor, now))
        {
          stat_.replaced++;
        }
        return;
      }
    }

    insert_(key, data, now);
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    insert_or_update(const Key& key, Data& data) throw (eh::Exception)
  {
    insert_or_update_(key, data);
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    insert_or_update(const Key& key, Data&& data) throw (eh::Exception)
  {
    insert_or_update_(key, std::move(data));
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    Inserter
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    operator [](const Key& key) throw ()
  {
    return Inserter(*this, key);
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  bool
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    update_(typename Container::iterator itor, const Time& now) throw ()
  {
    size_t size = size_policy_(itor->first, itor->second.data);
    size_t new_size = size_ + size - itor->second.size;
    if (new_size <= bound_)
    {
      itor->second.size = size;
      size_ = new_size;
      return true;
    }

    if (size > bound_)
    {
      stat_.removed_updated++;
      size_ -= itor->second.size;
      queue_.erase(itor->second.order);
      container_.erase(itor);
      return false;
    }

    typename Queue::iterator order(queue_.begin());
    for (; new_size > bound_; ++order)
    {
      if (order == queue_.end() || order->last_used + timeout_ > now)
      {
        stat_.removed_updated++;
        size_ -= itor->second.size;
        queue_.erase(itor->second.order);
        container_.erase(itor);
        return false;
      }
      if (order != itor->second.order)
      {
        typename Container::iterator itor(container_.find(order->key));
        assert(itor != container_.end());
        new_size -= itor->second.size;
      }
    }

    for (typename Queue::iterator remove(queue_.begin()); remove != order;)
    {
      if (remove != itor->second.order)
      {
        stat_.removed_outdated++;
        container_.erase(remove->key);
        remove = queue_.erase(remove);
      }
      else
      {
        ++remove;
      }
    }

    itor->second.size = size;
    size_ = new_size;

    return true;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    update(const Key& key) throw ()
  {
    Time now(Time::get_time_of_day());

    typename SyncPolicy::WriteGuard guard(mutex_);

    typename Container::iterator itor(container_.find(key));
    if (itor != container_.end())
    {
      update_(itor, now);
    }
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    update(const IteratorBase& iterator) throw ()
  {
    update(iterator->first);
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    erase(const Key& key) throw ()
  {
    typename SyncPolicy::WriteGuard guard(mutex_);

    typename Container::iterator itor(container_.find(key));
    if (itor == container_.end())
    {
      return;
    }

    typename Queue::iterator order(itor->second.order);
    size_ -= itor->second.size;
    container_.erase(itor);
    queue_.erase(order);
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    erase(const IteratorBase& it) throw ()
  {
    assert(it.value_);
    erase(it.value_->first);
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    clear() throw ()
  {
    typename SyncPolicy::WriteGuard guard(mutex_);

    size_ = 0;
    container_.clear();
    queue_.clear();
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    const_iterator const &
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    end() const throw ()
  {
    return END_CONST_ITERATOR;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  template <typename InsertIterator>
  InsertIterator
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    copy_to(InsertIterator insert) throw (eh::Exception)
  {
    {
      typename SyncPolicy::ReadGuard guard(mutex_);

      for (typename Container::iterator itor(container_.begin());
        itor != container_.end(); ++itor)
      {
        *insert = value_type(itor->first, itor->second.data);
        ++insert;
      }
    }

    return insert;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    size_type
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    size() const throw ()
  {
    typename SyncPolicy::ReadGuard guard(mutex_);

    return container_.size();
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  BoundedMapStat
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    statistics(bool reset) throw ()
  {
    typename SyncPolicy::WriteGuard guard(mutex_);

    BoundedMapStat stat(stat_);
    if (reset)
    {
      stat_ = BoundedMapStat();
    }
    return stat;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  typename BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    size_type
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    bound() const throw ()
  {
    typename SyncPolicy::ReadGuard guard(mutex_);

    return bound_;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    bound(size_type new_bound) throw ()
  {
    typename SyncPolicy::WriteGuard guard(mutex_);

    bound_ = new_bound;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  Time
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    timeout() const throw ()
  {
    typename SyncPolicy::ReadGuard guard(mutex_);

    return timeout_;
  }

  template <typename Key, typename Data, typename SizePolicy,
    typename SyncPolicy, typename Container>
  void
  BoundedMap<Key, Data, SizePolicy, SyncPolicy, Container>::
    timeout(Time new_timeout) throw ()
  {
    typename SyncPolicy::WriteGuard guard(mutex_);

    timeout_ = new_timeout;
  }
}

#endif
