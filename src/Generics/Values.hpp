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



#ifndef GENERICS_VALUES_HPP
#define GENERICS_VALUES_HPP

#include <limits>
#include <functional>

#include <ReferenceCounting/ReferenceCounting.hpp>

#include <Generics/GnuHashTable.hpp>
#include <Generics/HashTableAdapters.hpp>
#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>
#include <Sync/PosixLock.hpp>

namespace CORBACommons
{
  class ValuesConverter;
}

namespace Generics
{
  namespace ValuesHelper
  {
    template <typename ParamValue>
    struct StoredMember;
  }


  /**
   * Thread-safe associative container for data of types of
   * double, long, unsigned long and string. Allows addition of values to
   * existing keys (numeric addition for numeric types and concatenation
   * for string types).
   */
  class Values : public virtual ReferenceCounting::AtomicImpl
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(InvalidType, Exception);
    DECLARE_EXCEPTION(KeyNotFound, Exception);

  public:
    typedef StringHashAdapter Key;

    typedef long SignedInt;
    typedef unsigned long UnsignedInt;
    typedef double Floating;
    typedef std::string String;

    /**
     * Constructor
     * @param table_size initial size for underlying hash
     */
    explicit
    Values(size_t table_size = 0) throw (eh::Exception);

    /**
     * Getter
     * @param key key to search
     * @return value associated with the key (if any)
     */
    template <typename Type>
    Type
    get(const Key& key) const
      throw (eh::Exception, KeyNotFound, InvalidType);

    /**
     * Getter
     * @param key key to search
     * @param value associated with the key (if any)
     * @return if the operation has been completed successfully or not
     */
    template <typename Type>
    bool
    get(const Key& key, Type& value) const
      throw (eh::Exception, InvalidType);

    /**
     * Assigns new or existing record a new value of the certain type.
     * @param key associative key for the record
     * @param value initial value for the record
     */
    template <typename Type>
    void
    set(const Key& key, const Type& value) throw (eh::Exception);

    /**
     * Adds a value to the existing record
     * @param key associative key for the record
     * @param value additional value for the record
     */
    template <typename Type>
    void
    add(const Key& key, const Type& value)
      throw (eh::Exception, KeyNotFound, InvalidType);

    /**
     * If a key exists it modifies existing record by adding the value to
     * it otherwise it behaves like set().
     * @param key associative key for the record
     * @param value increasing/setting value
     */
    template <typename Type>
    void
    add_or_set(const Key& key, const Type& value)
      throw (eh::Exception, InvalidType);

    /**
     * If a key exists it modifies existing record by applying the functor
     * to the stored and new values otherwise it behaves like set().
     * @param key associative key for the record
     * @param value increasing/setting value
     * @param functor functor to apply if the record exists
     */
    template <typename Functor, typename Type>
    void
    func_or_set(const Key& key, const Type& value, Functor functor)
      throw (eh::Exception, InvalidType);

    /**
     * Generalized setter.
     * Outputs the object into stream and stores the resulted string
     * @param key associative key for the record
     * @param object object to serialize and store
     */
    template <typename T>
    bool
    set_as_string(const Key& key, const T& object) throw (eh::Exception);

    /**
     * Generalized getter.
     * Finds the string record associated with the key and inputs the object
     * from the stream
     * @param key associative key for the record
     * @param object object to deserialize from the record
     * @return if input stream has been processed fully and without errors
     */
    template <typename T>
    bool
    get_as_string(const Key& key, T& object) const
      throw (eh::Exception, KeyNotFound);

    /**
     * Calls functor for each value stored.
     * @param functor functor to call for each value
     */
    template <typename Functor>
    void
    enumerate_all(Functor& functor) const throw (eh::Exception);

    /**
     * Locks the current object and swaps its content with the supplied one.
     * The supplied object is not locked.
     * @param values object to swap content with
     */
    void
    swap(Values& values) throw (eh::Exception);

  protected:
    virtual
    ~Values() throw () = default;

  protected:
    /**
     * Unsafe getter
     * @param key key to search
     * @return value associated with the key (if any) or 0
     */
    template <typename Type>
    typename ValuesHelper::StoredMember<Type>::Type*
    get_(const Key& key) const
      throw (eh::Exception, InvalidType);

    /**
     * Unsafe setter
     * Assigns new or existing record a new value of the certain type.
     * @param key associative key for the record
     * @param value initial value for the record
     */
    template <typename Type>
    void
    set_(const Key& key, const Type& value) throw (eh::Exception);

    /**
     * Unsafe modifier
     * If a key exists it modifies existing record by applying the functor
     * to the stored and new values otherwise it behaves like set().
     * @param key associative key for the record
     * @param value modifying/setting value
     * @param functor functor to apply if the record exists
     * @return reference to stored resulted value
     */
    template <typename Functor, typename Type>
    typename ValuesHelper::StoredMember<Type>::Type&
    func_or_set_(const Key& key, const Type& value, Functor functor)
      throw (eh::Exception, InvalidType);

    /**
     * Unsafe swap
     * @param values object to swap content with
     */
    void
    swap_(Values& values) throw (eh::Exception);

  protected:
    enum StoredType
    {
      ST_SIGNEDINT,
      ST_UNSIGNEDINT,
      ST_FLOATING,
      ST_STRING,
    };

    static const char* const STORED_TYPES_[];

    struct StoredValue
    {
      StoredType type;
      union
      {
        SignedInt signed_int;
        UnsignedInt unsigned_int;
        Floating floating;
      };
      String string;
    };

    typedef GnuHashTable<Key, StoredValue> Data;


    template <typename Type>
    typename ValuesHelper::StoredMember<Type>::Type*
    set_(StoredValue& data, const Type& value) throw (eh::Exception);

    template <typename Functor>
    static
    void
    enumerate_one_(const Data::value_type& one, Functor& functor)
      throw (eh::Exception);

  protected:
    mutable Sync::PosixMutex mutex_;

    Data data_;

    template <typename ParamValue>
    friend struct ValuesHelper::StoredMember; 

    friend class CORBACommons::ValuesConverter;
  };
  typedef ReferenceCounting::QualPtr<Values> Values_var;

  namespace ValuesHelper
  {
    template <>
    struct StoredMember<Values::SignedInt>
    {
      static const Values::StoredType TYPE = Values::ST_SIGNEDINT;
      typedef Values::SignedInt Type;
      static Type Values::StoredValue::* const MEMBER;
    };

    template <>
    struct StoredMember<Values::UnsignedInt>
    {
      static const Values::StoredType TYPE = Values::ST_UNSIGNEDINT;
      typedef Values::UnsignedInt Type;
      static Type Values::StoredValue::* const MEMBER;
    };

    template <>
    struct StoredMember<Values::Floating>
    {
      static const Values::StoredType TYPE = Values::ST_FLOATING;
      typedef Values::Floating Type;
      static Type Values::StoredValue::* const MEMBER;
    };

    template <>
    struct StoredMember<Values::String>
    {
      static const Values::StoredType TYPE = Values::ST_STRING;
      typedef Values::String Type;
      static Type Values::StoredValue::* const MEMBER;
    };

    template <>
    struct StoredMember<char*>
    {
      static const Values::StoredType TYPE = Values::ST_STRING;
      typedef Values::String Type;
      static Type Values::StoredValue::* const MEMBER;
    };

    template <>
    struct StoredMember<const char*>
    {
      static const Values::StoredType TYPE = Values::ST_STRING;
      typedef Values::String Type;
      static Type Values::StoredValue::* const MEMBER;
    };

    template <const size_t SIZE>
    struct StoredMember<char [SIZE]>
    {
      static const Values::StoredType TYPE = Values::ST_STRING;
      typedef Values::String Type;
      static Type Values::StoredValue::* const MEMBER;
    };

    template <const size_t SIZE>
    struct StoredMember<const char [SIZE]>
    {
      static const Values::StoredType TYPE = Values::ST_STRING;
      typedef Values::String Type;
      static Type Values::StoredValue::* const MEMBER;
    };

    template <const size_t SIZE>
    Values::String
    Values::StoredValue::* const StoredMember<char [SIZE]>::MEMBER =
      &Values::StoredValue::string;

    template <const size_t SIZE>
    Values::String
    Values::StoredValue::* const StoredMember<const char [SIZE]>::MEMBER =
      &Values::StoredValue::string;
  }
}

//
// INLINES
//

namespace Generics
{
  //
  // Values class
  //

  template <typename Type>
  typename ValuesHelper::StoredMember<Type>::Type*
  Values::get_(const Key& key) const throw (eh::Exception, InvalidType)
  {
    typename Data::const_iterator itor(data_.find(key));
    if (itor == data_.end())
    {
      return 0;
    }
    if (ValuesHelper::StoredMember<Type>::TYPE != itor->second.type)
    {
      Stream::Error ostr;
      ostr << FNS << "for key '" << key << "' requested type is " <<
        STORED_TYPES_[ValuesHelper::StoredMember<Type>::TYPE] <<
          " but stored one is " << STORED_TYPES_[itor->second.type];
      throw InvalidType(ostr);
    }
    return const_cast<typename ValuesHelper::StoredMember<Type>::Type*>(
      &(itor->second.*ValuesHelper::StoredMember<Type>::MEMBER));
  }

  template <typename Type>
  typename ValuesHelper::StoredMember<Type>::Type*
  Values::set_(StoredValue& data, const Type& value) throw (eh::Exception)
  {
    data.type = ValuesHelper::StoredMember<Type>::TYPE;
    return &(data.*ValuesHelper::StoredMember<Type>::MEMBER = value);
  }

  template <typename Type>
  void
  Values::set_(const Key& key, const Type& value) throw (eh::Exception)
  {
    set_(data_[key], value);
  }

  template <typename Functor, typename Type>
  typename ValuesHelper::StoredMember<Type>::Type&
  Values::func_or_set_(const Key& key, const Type& value,
    Functor functor) throw (eh::Exception, InvalidType)
  {
    typename ValuesHelper::StoredMember<Type>::Type* member =
      get_<Type>(key);
    if (member)
    {
      *member = functor(value, *member);
    }
    else
    {
      member = set_(data_[key], value);
    }
    return *member;
  }

  template <typename Functor>
  void
  Values::enumerate_one_(const Data::value_type& one, Functor& functor)
    throw (eh::Exception)
  {
    switch (one.second.type)
    {
    case ST_SIGNEDINT:
      functor(one.first, one.second.*
        Generics::ValuesHelper::StoredMember<SignedInt>::MEMBER);
      break;
    case ST_UNSIGNEDINT:
      functor(one.first, one.second.*
        Generics::ValuesHelper::StoredMember<UnsignedInt>::MEMBER);
      break;
    case ST_FLOATING:
      functor(one.first, one.second.*
        Generics::ValuesHelper::StoredMember<Floating>::MEMBER);
      break;
    case ST_STRING:
      functor(one.first, one.second.*
        Generics::ValuesHelper::StoredMember<String>::MEMBER);
      break;
    }
  }

  template <typename Type>
  Type
  Values::get(const Key& key) const
    throw (eh::Exception, KeyNotFound, InvalidType)
  {
    for (;;)
    {
      Type value;
      {
        Sync::PosixGuard guard(mutex_);
        const Type* member = get_<Type>(key);
        if (!member)
        {
          break;
        }
        value = *member;
      }
      return value;
    }
    Stream::Error ostr;
    ostr << FNS << "Key '" << key << "' is not found";
    throw KeyNotFound(ostr);
  }

  template <typename Type>
  bool
  Values::get(const Key& key, Type& value) const
    throw (eh::Exception, InvalidType)
  {
    Sync::PosixGuard guard(mutex_);
    const Type* member = get_<Type>(key);
    return member ? (value = *member, true) : false;
  }

  template <typename Type>
  void
  Values::set(const Key& key, const Type& value) throw (eh::Exception)
  {
    Sync::PosixGuard guard(mutex_);
    set_(key, value);
  }

  template <typename Type>
  void
  Values::add(const Key& key, const Type& value)
    throw (eh::Exception, KeyNotFound, InvalidType)
  {
    {
      Sync::PosixGuard guard(mutex_);
      if (typename ValuesHelper::StoredMember<Type>::Type* member =
        get_<Type>(key))
      {
        *member = *member + value;
        return;
      }
    }
    Stream::Error ostr;
    ostr << FNS << "key '" << key << "' is not found";
    throw KeyNotFound(ostr);
  }

  template <typename Type>
  void
  Values::add_or_set(const Key& key, const Type& value)
    throw (eh::Exception, InvalidType)
  {
    Sync::PosixGuard guard(mutex_);
    func_or_set_(key, value,
      std::plus<typename ValuesHelper::StoredMember<Type>::Type>());
  }

  template <typename Functor, typename Type>
  void
  Values::func_or_set(const Key& key, const Type& value, Functor functor)
    throw (eh::Exception, InvalidType)
  {
    Sync::PosixGuard guard(mutex_);
    func_or_set_(key, value, functor);
  }

  template <typename T>
  bool
  Values::set_as_string(const Key& key, const T& object)
    throw (eh::Exception)
  {
    Stream::Dynamic ostr(4096);
    ostr << object;
    set(key, ostr.str().str());
    return !ostr.bad();
  }

  template <typename T>
  bool
  Values::get_as_string(const Key& key, T& object) const
    throw (eh::Exception, KeyNotFound)
  {
    const std::string& str = get<String>(key);
    Stream::Parser istr(str.data(), str.size());
    istr >> object;
    return !istr.bad() && istr.eof();
  }

  template <typename Functor>
  void
  Values::enumerate_all(Functor& functor) const throw (eh::Exception)
  {
    Sync::PosixGuard guard(mutex_);
    functor(data_.size());
    for (Data::const_iterator itor(data_.begin());
      itor != data_.end(); ++itor)
    {
      enumerate_one_(*itor, functor);
    }
  }
}

#endif
