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





#ifndef CORBA_COMMONS_OBJECT_POOL_HPP
#define CORBA_COMMONS_OBJECT_POOL_HPP

#include <deque>
#include <cassert>

#include <tao/ORB.h>

#include <Sync/PosixLock.hpp>

#include <Generics/Time.hpp>
#include <Generics/Rand.hpp>

#include <CORBACommons/CorbaAdapters.hpp>

//#define BUILD_WITH_DEBUG_MESSAGES
#ifdef BUILD_WITH_DEBUG_MESSAGES
#include <iostream>
#endif

namespace CORBACommons
{
  /**
   * ObjectPlainVar
   * Determines operations on type T, expecting that is not a pointer held
   */
  template <typename T>
  class ObjectPlainVar
  {
  public:
    typedef ObjectPlainVar _obj_type;

    template <typename... Args>
    ObjectPlainVar(Args... data) throw ();

    T*
    operator ->() throw ();
    T&
    operator *() throw ();
    T
    _retn() throw ();
    bool
    operator ==(const ObjectPlainVar& p) const throw ();

    static
    T
    _nil() throw ();
    static
    T
    _duplicate(ObjectPlainVar& ref) throw ();

  private:
    T data_;
  };
}

namespace CORBA
{
  template <typename T>
  Boolean
  is_nil(CORBACommons::ObjectPlainVar<T>& p) throw (eh::Exception)
  {
    return *p == CORBACommons::ObjectPlainVar<T>::_nil();
  }
}

namespace CORBACommons
{
  template <class T, class Conf, class TVar, const bool RERESOLVE>
  class ObjectPool;

  /**
   * Wrapper around T, which allows the poll to free it.
   */
  template <class ObjectPoolType>
  class ObjectHandler
  {
    template <class T, class Conf, class TVar, const bool RERESOLVE>
    friend class ObjectPool;

  public:
    typedef typename ObjectPoolType::Object Object;

    ~ObjectHandler() throw ();

    ObjectHandler&
    operator =(ObjectHandler&& src) throw (eh::Exception);

    Object&
    operator *() throw ();

    const Object&
    operator *() const throw ();

    Object*
    operator ->() throw ();

    const Object*
    operator ->() const throw ();

    void
    release() throw ();

    /**
     * Release object to pool and set his state to bad.
     * @param dsc Optional parameter inform about reason to release
     * bad, this texts could be used in NoGoodReference exception
     */
    void
    release_bad(const String::SubString& dsc = String::SubString())
      throw ();

    ObjectHandler(ObjectHandler&& src) throw ();

  protected:
    ObjectHandler() throw ();
    template <typename D>
    ObjectHandler(D&& p_object, ObjectPoolType* pool)
      throw ();

  private:
    typename ObjectPoolType::ObjectRef p_object_;
    ObjectPoolType* pool_;
  };


  /**
   * Because types of objects stored in Configuration and ObjectPool
   * can be different, to allow usage Config on Base objects with
   * ObjectPool of Derived objects we should call _narrow at this case
   * instead _duplicate. Compiler detects other case: Base Objects Pool -
   * Derived Objects Config and calls _duplicate.
   */
  namespace ResolveHelper
  {
    /**
     * Call if UP cast allowed
     * @param ptr Pointer to do _duplicate
     * @param dummy Not used, need to compiler case detection
     * @return Resolved CORBA object pointer
     */
    template <typename Conf, typename Pool>
    Pool*
    do_resolve(Conf* ptr, Pool* dummy) throw ();

    /**
     * Call to try DOWN cast
     * @param ptr Pointer to do _narrow
     * @return Resolved CORBA object pointer
     */
    template <typename Conf, typename Pool>
    Pool*
    do_resolve(Conf* ptr, ...) throw ();
  }


  /**
   * ObjectPool configuration for already resolved object ref's
   */
  template <class T, typename Ref = TAO_Objref_Var_T<T>>
  struct ObjectPoolConfiguration
  {
    typedef Ref ObjectRef;

    ObjectPoolConfiguration() throw ();

    struct RefAndNumber
    {
      explicit
      RefAndNumber(T* ior, int count = 0) throw ();
      RefAndNumber(const T& ior, int count = 0) throw ();

      ObjectRef ior;
      int count;
    };

    struct Resolver
    {
      template <typename Pool>
      Pool*
      resolve(const ObjectRef& ref) throw ();
    };

    typedef std::deque<RefAndNumber> References;
    References iors_list;
    Generics::Time timeout;
    Resolver resolver;
    bool object_once;
    bool all_bad_no_wait;
  };


  /**
   * ObjectPoolRefConfiguration
   * ObjectPool configuration with references that need resolve before use
   */
  class ObjectPoolRefConfiguration :
    public ObjectPoolConfiguration<CorbaObjectRef, CorbaObjectRef>
  {
  public:
    explicit
    ObjectPoolRefConfiguration(
      const CorbaClientAdapter* corba_client_adapter) throw ();

    struct Resolver
    {
      DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

      explicit
      Resolver(const CorbaClientAdapter* corba_client_adapter) throw ();

      template <typename Pool>
      Pool*
      resolve(const ObjectRef& ref) throw (eh::Exception);

    private:
      CorbaClientAdapter_var corba_client_adapter_;
    };

    Resolver resolver;
  };


  struct ChoosePolicyType
  {
    enum POLICY_TYPE
    {
      // Round robin - cycle from last saved until valid
      PT_LOOP,

      // Return objects[safe_rand(number_of_objects)],
      // if it's not valid cycle from it until valid
      PT_RAND,

      // Return last object until it's valid, then use round robin
      PT_BAD_SWITCH,

      // Return objects[key % number_of_objects],
      // if it's not valid cycle from it until valid
      PT_PERSISTENT,

      // Return key < number_of_objects ? objects[key] : round robin
      PT_PRECISE
    };
  };


  template <class T, class Conf = ObjectPoolConfiguration<T>,
    class TVar = TAO_Objref_Var_T<T>, const bool RERESOLVE = false>
  class ObjectPool
  {
    friend class ObjectHandler<ObjectPool>;
    friend class ChoosePolicyType;

  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(InvalidReference, Exception);
    DECLARE_EXCEPTION(BadObject, Exception);
    DECLARE_EXCEPTION(NoFreeObject, Exception);
    DECLARE_EXCEPTION(NoGoodReference, NoFreeObject);

    typedef Conf ConfigType;
    typedef T Object;
    typedef TVar ObjectRef;
    typedef ObjectHandler<ObjectPool> ObjectHandlerType;

    static const unsigned SPECIAL_KEY = -1;

  public:
    explicit
    ObjectPool(const Conf& configuration,
      ChoosePolicyType::POLICY_TYPE policy_type = ChoosePolicyType::PT_LOOP)
      throw (Exception, eh::Exception);

    // This function MUST NOT throw anything except ObjectPool::Exception
    // and derivatives.
    ObjectHandlerType
    get_object(unsigned key = SPECIAL_KEY)
      throw (InvalidReference, NoGoodReference, NoFreeObject, Exception);

    template <typename UserException>
    ObjectHandlerType
    get_object(unsigned key = SPECIAL_KEY) throw (UserException);

    template <typename UserException>
    ObjectHandlerType
    get_object(Logging::Logger* logger,
      unsigned long severity = Logging::Logger::ERROR,
      const char* aspect = nullptr, const char* code = nullptr,
      unsigned first_key = SPECIAL_KEY, unsigned next_key = SPECIAL_KEY)
      throw (UserException);

  private:
    /**
     * Badness status remove from objects which worthless time is over
     */
    void
    check_bad_refs_(bool force) throw (eh::Exception);

    void
    check_all_are_bad_or_busy_()
      throw (eh::Exception, NoGoodReference, NoFreeObject);

    enum GiveOnce
    {
      GO_NOT_GIVEN,
      GO_FIRST,
      GO_OTHERS
    };

    struct ConnData
    {
      ConnData(typename Conf::ObjectRef object_ref, int use_max) throw ();

      Sync::PosixMutex obj_lock;
      typename Conf::ObjectRef object_ref;
      ObjectRef object;

      volatile sig_atomic_t resolve;

      volatile sig_atomic_t is_bad;
      Generics::Time bad_mark_time;
      std::string badness_description;

      bool made_good;

      int use_count;
      int use_max;
      GiveOnce give_once;
    };

    typedef std::deque<ConnData> Objects;
    Objects objects_;

    ConnData&
    get_conndata_(ObjectRef& t) throw ();

    void
    release_object_(ConnData& conn_data,
      const String::SubString& bad_dsc)
      throw ();

    /**
     * @param object object to be released
     * @param bad_dsc parameter with description of reason to be bad
     * if equal to zero object release in good state
     */
    void
    release_object_(ObjectRef& object,
      const String::SubString& bad_dsc = String::SubString())
      throw ();

    typedef Sync::PosixMutex Mutex;
    typedef Sync::PosixGuard Guard;

    class ChoosePolicy;
    class LoopPolicy;
    class SwitchOnBadPolicy;
    class RandPolicy;
    class PersistentPolicy;
    class PrecisePolicy;

    typedef ::ReferenceCounting::QualPtr<ChoosePolicy> ChoosePolicy_var;

    const Generics::Time TIMEOUT_;
    typename Conf::Resolver resolver_;
    ChoosePolicy_var choose_policy_;
    Mutex lock_;
    const bool OBJECT_ONCE_;
    const bool ALL_BAD_NO_WAIT_;
  };

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  class ObjectPool<T, Conf, TVar, RERESOLVE>::ChoosePolicy :
    public ::ReferenceCounting::AtomicImpl
  {
  public:
    explicit
    ChoosePolicy(ObjectPool& pool) throw ();

    virtual
    ConnData&
    get_valid_object(unsigned key) throw (eh::Exception) = 0;

    /**
     * Added because some strategies have special opinion
     * about badness status of connection. But we should be sure
     * that pool have valid objects at get_object call time.
     * @param conn The connection context object
     * @return true if conn is good
     */
    bool
    soft_suitable(const ConnData& conn) const throw ();

    /**
     * Get parameters of usage
     * @param conn The connection context object
     * @param busy returns false if object is free for usage
     * @return true if conn is good
     */
    bool
    usable(const ConnData& conn, bool& busy) const throw ();

    /**
     * Find next (in cycle) soft usable object
     * @param itor the object to start from (not including)
     * @return the next usable object
     */
    typename Objects::iterator
    cycle_next(typename Objects::iterator itor)
      throw (eh::Exception);

    /**
     * Checks the current object and if it's not usable,
     * find next (in cycle) soft usable object
     * @param itor the object to start from (including)
     * @return the next usable object
     */
    typename Objects::iterator
    check_and_cycle_next(typename Objects::iterator itor)
      throw (eh::Exception);

  protected:
    virtual
    ~ChoosePolicy() throw () = default;

    ObjectPool& pool_;
  };

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  class ObjectPool<T, Conf, TVar, RERESOLVE>::LoopPolicy :
    public ChoosePolicy
  {
  public:
    explicit
    LoopPolicy(ObjectPool& pool) throw (eh::Exception);

    virtual
    ConnData&
    get_valid_object(unsigned key) throw (eh::Exception);

  protected:
    virtual
    ~LoopPolicy() throw () = default;

    typename Objects::iterator last_object_;
  };

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  class ObjectPool<T, Conf, TVar, RERESOLVE>::SwitchOnBadPolicy :
    public LoopPolicy
  {
  public:
    explicit
    SwitchOnBadPolicy(ObjectPool& pool) throw (eh::Exception);

    virtual
    ConnData&
    get_valid_object(unsigned key) throw (eh::Exception);

    /**
     * Hard condition for object fitting while get_object call
     * @param conn The descriptive data for connected object
     * @return true, if object can be return as valid
     */
    bool
    hard_suitable(const ConnData& conn) const throw ();

  protected:
    virtual
    ~SwitchOnBadPolicy() throw () = default;
  };

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  class ObjectPool<T, Conf, TVar, RERESOLVE>::RandPolicy :
    public ChoosePolicy
  {
  public:
    explicit
    RandPolicy(ObjectPool& pool) throw (eh::Exception);

    virtual
    ConnData&
    get_valid_object(unsigned key) throw (eh::Exception);

  protected:
    virtual
    ~RandPolicy() throw () = default;
  };

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  class ObjectPool<T, Conf, TVar, RERESOLVE>::PersistentPolicy :
    public ChoosePolicy
  {
  public:
    explicit
    PersistentPolicy(ObjectPool& pool) throw (eh::Exception);

    virtual
    ConnData&
    get_valid_object(unsigned key) throw (eh::Exception);

  protected:
    virtual
    ~PersistentPolicy() throw () = default;
  };

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  class ObjectPool<T, Conf, TVar, RERESOLVE>::PrecisePolicy :
    public LoopPolicy
  {
  public:
    explicit
    PrecisePolicy(ObjectPool& pool) throw (eh::Exception);

    virtual
    ConnData&
    get_valid_object(unsigned key) throw (eh::Exception);

  protected:
    virtual
    ~PrecisePolicy() throw () = default;
  };

} // namespace ObjectPool

//////////////////////////////////////////////////////////////////////////
// Inlines implementations
//

namespace CORBACommons
{
  //
  // ObjectPlainVar class
  //

  template <typename T>
  template <typename... Args>
  ObjectPlainVar<T>::ObjectPlainVar(Args... data) throw ()
    : data_(std::forward<Args>(data)...)
  {
  }

  template <typename T>
  T*
  ObjectPlainVar<T>::operator ->() throw ()
  {
    return &data_;
  }

  template <typename T>
  T&
  ObjectPlainVar<T>::operator *() throw ()
  {
    return data_;
  }

  template <typename T>
  T
  ObjectPlainVar<T>::_retn() throw ()
  {
    T tmp(std::move(data_));
    data_ = _nil();
    return tmp;
  }

  template <typename T>
  bool
  ObjectPlainVar<T>::operator ==(const ObjectPlainVar& p) const throw ()
  {
    return data_ == p.data_;
  }

  template <typename T>
  T
  ObjectPlainVar<T>::_nil() throw ()
  {
    return T();
  }

  template <typename T>
  T
  ObjectPlainVar<T>::_duplicate(ObjectPlainVar& ptr) throw ()
  {
    return *ptr;
  }


  //
  // ObjectHandler class
  //

  template <class ObjectPool>
  ObjectHandler<ObjectPool>::ObjectHandler() throw ()
    : p_object_(), pool_()
  {
  }

  template <class ObjectPool>
  template <typename D>
  ObjectHandler<ObjectPool>::ObjectHandler(
    D&& p_object, ObjectPool* pool)
    throw ()
    : p_object_(std::forward<D>(p_object)), pool_(pool)
  {
  }

  template <class ObjectPool>
  ObjectHandler<ObjectPool>::ObjectHandler(
    ObjectHandler&& src) throw ()
    : p_object_(src.p_object_._retn()), pool_(src.pool_)
  {
    src.pool_ = 0;
  }

  template <class ObjectPool>
  ObjectHandler<ObjectPool>::~ObjectHandler() throw ()
  {
    release();
  }

  template <class ObjectPool>
  ObjectHandler<ObjectPool>&
  ObjectHandler<ObjectPool>::operator =(
    ObjectHandler&& src) throw (eh::Exception)
  {
    if (this != &src)
    {
      release();

      p_object_ = src.p_object_._retn();
      pool_ = src.pool_;
      src.pool_ = 0;
    }

    return *this;
  }

  template <class ObjectPool>
  typename ObjectPool::Object&
  ObjectHandler<ObjectPool>::operator *() throw ()
  {
    return *p_object_;
  }

  template <class ObjectPool>
  const typename ObjectPool::Object&
  ObjectHandler<ObjectPool>::operator *() const throw ()
  {
    return *p_object_;
  }

  template <class ObjectPool>
  typename ObjectPool::Object*
  ObjectHandler<ObjectPool>::operator ->() throw ()
  {
    return p_object_;
  }

  template <class ObjectPool>
  const typename ObjectPool::Object*
  ObjectHandler<ObjectPool>::operator ->() const throw ()
  {
    return p_object_;
  }

  template <class ObjectPool>
  void
  ObjectHandler<ObjectPool>::release() throw ()
  {
    if (pool_)
    {
      pool_->release_object_(p_object_);
      p_object_ = 0;
      pool_ = 0;
    }
  }

  template <class ObjectPool>
  void
  ObjectHandler<ObjectPool>::release_bad(const String::SubString& dsc)
    throw ()
  {
    if (pool_)
    {
      pool_->release_object_(p_object_,
        dsc.empty() ? String::SubString("reason unknown") : dsc);
      p_object_ = 0;
      pool_ = 0;
    }
  }

  namespace ResolveHelper
  {
    template <typename Conf, typename Pool>
    Pool*
    do_resolve(Conf* ptr, Pool*) throw ()
    {
      return Pool::_duplicate(ptr);
    }

    template <typename Conf, typename Pool>
    Pool*
    do_resolve(Conf* ptr, ...) throw ()
    {
      return Pool::_narrow(ptr);
    }
  }


  //
  // ObjectPoolConfiguration<T>::RefAndNumber class
  //

  template <class T, typename Ref>
  ObjectPoolConfiguration<T, Ref>::RefAndNumber::RefAndNumber(
    T* ior, int count)
    throw ()
    : ior(T::_duplicate(ior)), count(count)
  {
  }

  template <class T, typename Ref>
  ObjectPoolConfiguration<T, Ref>::RefAndNumber::RefAndNumber(
    const T& ior, int count) throw ()
    : ior(ior), count(count)
  {
  }


  //
  // ObjectPoolConfiguration<T>::Resolver class
  //

  template <class T, typename Ref>
  template <typename Pool>
  Pool*
  ObjectPoolConfiguration<T, Ref>::Resolver::resolve(const ObjectRef& ref)
    throw ()
  {
    return ResolveHelper::do_resolve<T, Pool>(ref.in(), ref.in());
  }


  //
  // ObjectPoolConfiguration class
  //

  template <class T, typename Ref>
  ObjectPoolConfiguration<T, Ref>::ObjectPoolConfiguration() throw ()
    : object_once(true), all_bad_no_wait(false)
  {
  }


  //
  // ObjectPoolRefConfiguration class
  //

  inline
  ObjectPoolRefConfiguration::ObjectPoolRefConfiguration(
    const CorbaClientAdapter* corba_client_adapter) throw ()
    : resolver(corba_client_adapter)
  {
  }


  //
  // ObjectPoolRefConfiguration::Resolver class
  //

  inline
  ObjectPoolRefConfiguration::Resolver::Resolver(
    const CorbaClientAdapter* corba_client_adapter)
    throw ()
    : corba_client_adapter_(
        ::ReferenceCounting::add_ref(corba_client_adapter))
  {
  }

  template <class T>
  T*
  ObjectPoolRefConfiguration::Resolver::resolve(const ObjectRef& ref)
    throw (eh::Exception)
  {
    try
    {
      CORBA::Object_var obj = corba_client_adapter_->resolve_object(ref);
      return T::_narrow(obj);
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << ex.what();
      throw Exception(ostr);
    }
    catch (const CORBA::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << ex;
      throw Exception(ostr);
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "unknown exception";
      throw Exception(ostr);
    }
  }


  //
  // ObjectPool::ChoosePolicy class
  //

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  ObjectPool<T, Conf, TVar, RERESOLVE>::ChoosePolicy::ChoosePolicy(
    ObjectPool& pool) throw ()
    : pool_(pool)
  {
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  bool
  ObjectPool<T, Conf, TVar, RERESOLVE>::ChoosePolicy::usable(
    const ConnData& conn, bool& busy) const throw ()
  {
    busy = (conn.use_max && conn.use_count >= conn.use_max) ||
      conn.give_once == GO_FIRST;
    return !conn.is_bad;
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  bool
  ObjectPool<T, Conf, TVar, RERESOLVE>::ChoosePolicy::soft_suitable(
    const ConnData& conn) const throw ()
  {
    bool busy;
    return usable(conn, busy) && !busy;
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  typename ObjectPool<T, Conf, TVar, RERESOLVE>::Objects::iterator
  ObjectPool<T, Conf, TVar, RERESOLVE>::ChoosePolicy::cycle_next(
    typename Objects::iterator itor) throw (eh::Exception)
  {
     // There are free objects, find the nearest.
     do
     {
       if (++itor == pool_.objects_.end())
       {
         itor = pool_.objects_.begin();
       }
     }
     while (!soft_suitable(*itor));
     return itor;
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  typename ObjectPool<T, Conf, TVar, RERESOLVE>::Objects::iterator
  ObjectPool<T, Conf, TVar, RERESOLVE>::ChoosePolicy::check_and_cycle_next(
    typename Objects::iterator itor) throw (eh::Exception)
  {
     return soft_suitable(*itor) ? itor : cycle_next(itor);
  }


  //
  // ObjectPool::LoopPolicy class
  //

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  ObjectPool<T, Conf, TVar, RERESOLVE>::LoopPolicy::LoopPolicy(
    ObjectPool& pool)
    throw (eh::Exception)
    : ChoosePolicy(pool), last_object_(pool.objects_.begin())
  {
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  typename ObjectPool<T, Conf, TVar, RERESOLVE>::ConnData&
  ObjectPool<T, Conf, TVar, RERESOLVE>::LoopPolicy::get_valid_object(
    unsigned /*key*/) throw (eh::Exception)
  {
     last_object_ = ChoosePolicy::cycle_next(last_object_);
     return *last_object_;
  }


  //
  // ObjectPool::SwitchOnBadPolicy class
  //

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  ObjectPool<T, Conf, TVar, RERESOLVE>::SwitchOnBadPolicy::
    SwitchOnBadPolicy(ObjectPool& pool) throw (eh::Exception)
    : LoopPolicy(pool)
  {
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  typename ObjectPool<T, Conf, TVar, RERESOLVE>::ConnData&
  ObjectPool<T, Conf, TVar, RERESOLVE>::SwitchOnBadPolicy::
    get_valid_object(unsigned key) throw (eh::Exception)
  {
    if (hard_suitable(*LoopPolicy::last_object_))
    {
      return *LoopPolicy::last_object_;
    }

    return LoopPolicy::get_valid_object(key);
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  bool
  ObjectPool<T, Conf, TVar, RERESOLVE>::SwitchOnBadPolicy::hard_suitable(
    const ConnData& conn) const throw ()
  {
    return ChoosePolicy::soft_suitable(conn) && !conn.made_good;
  }


  //
  // ObjectPool::RandPolicy class
  //

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  ObjectPool<T, Conf, TVar, RERESOLVE>::RandPolicy::RandPolicy(
    ObjectPool& pool) throw (eh::Exception)
    : ChoosePolicy(pool)
  {
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  typename ObjectPool<T, Conf, TVar, RERESOLVE>::ConnData&
  ObjectPool<T, Conf, TVar, RERESOLVE>::RandPolicy::get_valid_object(
    unsigned /*key*/) throw (eh::Exception)
  {
    return *ChoosePolicy::check_and_cycle_next(
      ChoosePolicy::pool_.objects_.begin() +
        Generics::safe_rand(ChoosePolicy::pool_.objects_.size()));
  }


  //
  // ObjectPool::PersistentPolicy class
  //

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  ObjectPool<T, Conf, TVar, RERESOLVE>::PersistentPolicy::PersistentPolicy(
    ObjectPool& pool) throw (eh::Exception)
    : ChoosePolicy(pool)
  {
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  typename ObjectPool<T, Conf, TVar, RERESOLVE>::ConnData&
  ObjectPool<T, Conf, TVar, RERESOLVE>::PersistentPolicy::get_valid_object(
    unsigned key) throw (eh::Exception)
  {
    return *ChoosePolicy::check_and_cycle_next(
      ChoosePolicy::pool_.objects_.begin() +
        key % ChoosePolicy::pool_.objects_.size());
  }


  //
  // ObjectPool::PrecisePolicy class
  //

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  ObjectPool<T, Conf, TVar, RERESOLVE>::PrecisePolicy::
    PrecisePolicy(ObjectPool& pool) throw (eh::Exception)
    : LoopPolicy(pool)
  {
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  typename ObjectPool<T, Conf, TVar, RERESOLVE>::ConnData&
  ObjectPool<T, Conf, TVar, RERESOLVE>::PrecisePolicy::
    get_valid_object(unsigned key) throw (eh::Exception)
  {
    if (key < ChoosePolicy::pool_.objects_.size())
    {
      ConnData& conn_data = ChoosePolicy::pool_.objects_[key];
      if (conn_data.is_bad)
      {
        Stream::Error ostr;
        ostr << FNS << "Required PRECISE object is bad with status: " <<
          conn_data.badness_description;
        throw BadObject(ostr);
      }
      return conn_data;
    }

    return LoopPolicy::get_valid_object(key);
  }


  //
  // ObjectPool::ConnData class
  //

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  ObjectPool<T, Conf, TVar, RERESOLVE>::ConnData::ConnData(
    typename Conf::ObjectRef object_ref, int use_max) throw ()
    : obj_lock(), object_ref(object_ref), object(TVar::_obj_type::_nil()),
      resolve(false), is_bad(false), badness_description(),
      made_good(false), use_count(0), use_max(use_max), give_once(GO_OTHERS)
  {
  }


  //
  // ObjectPool class
  //

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  const unsigned ObjectPool<T, Conf, TVar, RERESOLVE>::SPECIAL_KEY;

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  ObjectPool<T, Conf, TVar, RERESOLVE>::ObjectPool(
    const Conf& configuration,
    ChoosePolicyType::POLICY_TYPE policy_type)
    throw (Exception, eh::Exception)
    : TIMEOUT_(configuration.timeout), resolver_(configuration.resolver),
      OBJECT_ONCE_(configuration.object_once),
      ALL_BAD_NO_WAIT_(configuration.all_bad_no_wait)
  {
    if (configuration.iors_list.empty())
    {
      Stream::Error ostr;
      ostr << FNS << "Configuration contains no references.";
      throw Exception(ostr);
    }
    for (auto ci = configuration.iors_list.begin();
      ci != configuration.iors_list.end(); ++ci)
    {
      objects_.emplace_back(ci->ior, ci->count);
    }

    switch (policy_type)
    {
    case ChoosePolicyType::PT_LOOP:
      choose_policy_ = new LoopPolicy(*this);
      break;
    case ChoosePolicyType::PT_RAND:
      choose_policy_ = new RandPolicy(*this);
      break;
    case ChoosePolicyType::PT_BAD_SWITCH:
      choose_policy_ = new SwitchOnBadPolicy(*this);
      break;
    case ChoosePolicyType::PT_PERSISTENT:
      choose_policy_ = new PersistentPolicy(*this);
      break;
    case ChoosePolicyType::PT_PRECISE:
      choose_policy_ = new PrecisePolicy(*this);
      break;
    default:
      Stream::Error ostr;
      ostr << FNS << "Invalid policy type.";
      throw Exception(ostr);
    }
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  void
  ObjectPool<T, Conf, TVar, RERESOLVE>::check_bad_refs_(bool force)
    throw (eh::Exception)
  {
    Generics::Time curtime = Generics::Time::get_time_of_day();
    for (typename Objects::iterator iter = objects_.begin();
      iter != objects_.end(); ++iter)
    {
      if (iter->is_bad)
      {
        if (force || curtime >= iter->bad_mark_time + TIMEOUT_)
        {
          if (RERESOLVE)
          {
            iter->resolve = true;
          }
          iter->is_bad = false;
          iter->made_good = true;
          if (OBJECT_ONCE_)
          {
            iter->give_once = GO_NOT_GIVEN;
          }
        }
      }
      else
      {
        iter->made_good = false;
      }
    }
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  void
  ObjectPool<T, Conf, TVar, RERESOLVE>::check_all_are_bad_or_busy_()
    throw (eh::Exception, NoGoodReference, NoFreeObject)
  {
    bool all_bad = true;
    for (typename Objects::iterator iter = objects_.begin();
      iter != objects_.end(); ++iter)
    {
      // @return true if good
      // good, but busy
      bool busy;
      if (choose_policy_->usable(*iter, busy))
      {
        // good object
        if (!busy)
        {
          return;
        }
        all_bad = false;
        // check good, but busy - unimportant
      }
    }

    if (all_bad)
    {
      // form a message about the pool state
      typedef std::map<String::SubString, int> Errors;
      Errors errors;
      for (typename Objects::const_iterator iter(objects_.begin());
        iter != objects_.end(); ++iter)
      {
        ++errors.insert(Errors::value_type(iter->badness_description, 0)).
          first->second;
      }

      Stream::Error ostr;
      ostr << FNS << "All references are bad. ObjectPool information:";
      for (typename Errors::const_iterator it(errors.begin());
        it != errors.end(); ++it)
      {
        ostr << std::endl << it->second << " object(s) with status: " <<
          it->first;
      }
      throw NoGoodReference(ostr);
    }
    Stream::Error ostr;
    ostr << FNS << "All objects are busy.";
    throw NoFreeObject(ostr);
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  ObjectHandler<ObjectPool<T, Conf, TVar, RERESOLVE>>
  ObjectPool<T, Conf, TVar, RERESOLVE>::get_object(unsigned key)
    throw (NoGoodReference, InvalidReference, NoFreeObject, Exception)
  {
    try
    {
      ConnData* conn_data;
      {
        Guard guard(lock_);

        if (ALL_BAD_NO_WAIT_)
        {
          try
          {
            check_all_are_bad_or_busy_();
            check_bad_refs_(false);
          }
          catch (const NoGoodReference&)
          {
            check_bad_refs_(true);
            check_all_are_bad_or_busy_();
          }
        }
        else
        {
          check_bad_refs_(false);
          check_all_are_bad_or_busy_();
        }

        conn_data = &choose_policy_->get_valid_object(key);

        conn_data->use_count++;
        if (conn_data->give_once == GO_NOT_GIVEN)
        {
          conn_data->give_once = GO_FIRST;
        }

#if BUILD_WITH_DEBUG_MESSAGES
        std::cerr << FNS << conn_data->use_count << ", " <<
          conn_data->use_max << ", " << conn_data->is_bad << std::endl;
#endif
      }

      Sync::PosixGuard guard(conn_data->obj_lock);
      // is_bad was false but now it may be true
      bool bad = conn_data->is_bad;
      if (!bad)
      {
        if ((RERESOLVE && conn_data->resolve) ||
          CORBA::is_nil(conn_data->object))
        {
          bad = true;
          std::string error;
          if (RERESOLVE)
          {
            conn_data->resolve = false;
          }
          try
          {
            conn_data->object =
              resolver_.template resolve<T>(conn_data->object_ref);
          }
          catch (const eh::Exception& ex)
          {
            conn_data->object = TVar::_obj_type::_nil();
            error = ex.what();
          }
          if (CORBA::is_nil(conn_data->object))
          {
            Guard guard(lock_);
            release_object_(*conn_data, error.empty() ?
              "failed to resolve" : "failed to resolve: " + error);
          }
          else
          {
            bad = false;
          }
        }
      }

      if (bad)
      {
        Stream::Error ostr;
        std::string error;
        {
          Sync::PosixGuard guard(lock_);
          error = conn_data->badness_description;
        }
        ostr << FNS << "object is bad: " << error;
        throw InvalidReference(ostr);
      }

      return ObjectHandlerType(
        TVar::_obj_type::_duplicate(conn_data->object), this);
    }
    catch (const Exception& ex)
    {
      throw;
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Exception caught: " << ex.what();
      throw Exception(ostr);
    }
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  template <typename UserException>
  ObjectHandler<ObjectPool<T, Conf, TVar, RERESOLVE>>
  ObjectPool<T, Conf, TVar, RERESOLVE>::get_object(unsigned key)
    throw (UserException)
  {
    try
    {
      return get_object(key);
    }
    catch (const Exception& ex)
    {
      Stream::Error ostr;
      ostr << "Can't find corba object: " << ex.what();
      throw UserException(ostr);
    }
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  template <typename UserException>
  ObjectHandler<ObjectPool<T, Conf, TVar, RERESOLVE>>
  ObjectPool<T, Conf, TVar, RERESOLVE>::get_object(Logging::Logger* logger,
    unsigned long severity, const char* aspect, const char* code,
    unsigned first_key, unsigned next_key) throw (UserException)
  {
    for (;;)
    {
      try
      {
        return get_object(first_key);
      }
      catch (const NoFreeObject& ex)
      {
        Stream::Error ostr;
        ostr << "Can't find corba object: " << ex.what();
        throw UserException(ostr);
      }
      catch (const Exception& ex)
      {
        logger->sstream(severity, aspect, code) << FNS << ex.what();
      }
      first_key = next_key;
    }
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  typename ObjectPool<T, Conf, TVar, RERESOLVE>::ConnData&
  ObjectPool<T, Conf, TVar, RERESOLVE>::get_conndata_(ObjectRef& t)
    throw ()
  {
    typename Objects::iterator iter = objects_.begin();
    for (; iter != objects_.end(); ++iter)
    {
      if (iter->object == t)
      {
        return *iter;
      }
    }
    // we should newer reach this code...
    assert(false);
    return *iter;
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  void
  ObjectPool<T, Conf, TVar, RERESOLVE>::release_object_(ConnData& conn_data,
    const String::SubString& dsc) throw ()
  {
    conn_data.use_count--;
    if (dsc.size())
    {
      conn_data.is_bad = true;
      conn_data.badness_description =
        "released object as bad, with reason: ";
      dsc.append_to(conn_data.badness_description);
      conn_data.bad_mark_time = Generics::Time::get_time_of_day();
    }
    else
    {
      conn_data.give_once = GO_OTHERS;
    }

#if BUILD_WITH_DEBUG_MESSAGES
    std::cerr << FNS << "(" << is_bad << "): " <<
      conn_data.use_count << ", " << conn_data.use_max << ", " <<
      conn_data.is_bad << std::endl;
#endif
  }

  template <class T, class Conf, class TVar, const bool RERESOLVE>
  void
  ObjectPool<T, Conf, TVar, RERESOLVE>::release_object_(ObjectRef& object,
    const String::SubString& dsc) throw ()
  {
    Guard guard(lock_);
    release_object_(get_conndata_(object), dsc);
  }
}

#endif
