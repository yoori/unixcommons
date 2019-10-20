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



// file      : ReferenceCounting/SmartPtr.hpp




#ifndef REFERENCE_COUNTING_SMART_PTR_HPP
#define REFERENCE_COUNTING_SMART_PTR_HPP

#include <eh/Exception.hpp>

#include <ReferenceCounting/Interface.hpp>
#include <ReferenceCounting/NullPtr.hpp>

#include <Generics/TypeTraits.hpp>


namespace ReferenceCounting
{
  /**
   * Default policy of SmartPtr. SmartPtr can hold any value and the
   * check (and exception throwing) is performed on dereference
   */
  class PolicyThrow
  {
  public:
    typedef eh::Exception NullPointer;
    DECLARE_EXCEPTION(NotInitialized, eh::DescriptiveException);

    static
    void
    check_init(const void* ptr) throw (NullPointer)
      __attribute__((always_inline));

    static
    void
    check_dereference(const void* ptr) throw (NotInitialized)
      __attribute__((always_inline));

    static
    void
    default_constructor() throw ()
      __attribute__((always_inline));

    static
    void
    retn() throw ()
      __attribute__((always_inline));

  private:
    ~PolicyThrow() throw ()
      __attribute__((always_inline));
  };

  /**
   * Assert policy of SmartPtr. SmartPtr can hold any value and the
   * check (via assert) is performed on dereference
   */
  class PolicyAssert
  {
  public:
    typedef eh::Exception NullPointer;
    typedef eh::Exception NotInitialized;

    static
    void
    check_init(const void* ptr) throw (NullPointer)
      __attribute__((always_inline));

    static
    void
    check_dereference(const void* ptr) throw (NotInitialized)
      __attribute__((always_inline));

    static
    void
    default_constructor() throw ()
      __attribute__((always_inline));

    static
    void
    retn() throw ()
      __attribute__((always_inline));

  private:
    ~PolicyAssert() throw ()
      __attribute__((always_inline));
  };

  /**
   * Non-null policy of SmartPtr. SmartPtr cannot have null pointer stored.
   * Thus default constructor and retn() are inapplicable.
   */
  class PolicyNotNull
  {
  public:
    DECLARE_EXCEPTION(NullPointer, eh::DescriptiveException);
    typedef eh::Exception NotInitialized;

    static
    void
    check_init(const void* ptr) throw (NullPointer)
      __attribute__((always_inline));

    static
    void
    check_dereference(const void* ptr) throw (NotInitialized)
      __attribute__((always_inline));

  private:
    ~PolicyNotNull() throw ()
      __attribute__((always_inline));
  };

  struct PolicyChecker
  {
    void
    check_policy_(const PolicyThrow* policy) throw ()
      __attribute__((always_inline));
    void
    check_policy_(const PolicyAssert* policy) throw ()
      __attribute__((always_inline));
    void
    check_policy_(const PolicyNotNull* policy) throw ()
      __attribute__((always_inline));
  };

  template <typename T, typename Policy>
  class SmartPtr;
  template <typename T, typename Policy>
  class FixedPtr;
  template <typename T, typename Policy>
  class QualPtr;
  template <typename T, typename Policy>
  class ConstPtr;


  std::nullptr_t
  add_ref(std::nullptr_t ptr) throw ();

  template <typename T, typename Policy>
  T*
  add_ref(const SmartPtr<T, Policy>& ptr) throw ();

  template <typename T, typename Policy>
  T*
  add_ref(SmartPtr<T, Policy>&& ptr) throw ();

  template <typename T, typename Policy>
  const T*
  add_ref(const FixedPtr<T, Policy>& ptr) throw ();

  template <typename T, typename Policy>
  T*
  add_ref(FixedPtr<T, Policy>& ptr) throw ();

  template <typename T, typename Policy>
  T*
  add_ref(FixedPtr<T, Policy>&& ptr) throw ();


  template <typename T, typename Policy = PolicyThrow>
  class SmartPtr : private PolicyChecker
  {
  public:
    typedef T Type;

    typedef typename Policy::NullPointer NullPointer;
    typedef typename Policy::NotInitialized NotInitialized;

  public:
    // c-tors
    SmartPtr(std::nullptr_t ptr = nullptr) throw ();

    SmartPtr(const SmartPtr& sptr) throw (NullPointer);

    SmartPtr(SmartPtr&& sptr) throw (NullPointer);

    template <typename Other>
    SmartPtr(Other&& sptr) throw (NullPointer);

    // d-tor
    ~SmartPtr() throw ();

    // assignment & copy-assignment operators
    SmartPtr&
    operator =(const SmartPtr& sptr) throw (NullPointer);

    template <typename Other>
    SmartPtr&
    operator =(Other&& sptr) throw (NullPointer);

    template <typename OtherPolicy>
    void
    swap(SmartPtr<Type, OtherPolicy>& sptr)
      throw (NullPointer, typename OtherPolicy::NullPointer);

    void
    reset() throw (NullPointer);

    // conversions
    operator Type*() const throw ()
      __attribute__((always_inline));

    // accessors
    Type*
    operator ->() const throw (NotInitialized)
      __attribute__((always_inline));

    Type&
    operator *() const throw (NotInitialized)
      __attribute__((always_inline));

    Type*
    in() const throw ()
      __attribute__((always_inline));

    Type*
    retn() throw ();

  private:
    Type* ptr_;
  };

  template <typename T, typename Policy = PolicyThrow>
  class FixedPtr :
    private PolicyChecker,
    private Generics::Uncopyable
  {
  public:
    typedef T Type;

    typedef typename Policy::NullPointer NullPointer;
    typedef typename Policy::NotInitialized NotInitialized;

  public:
    // c-tors
    FixedPtr(typename Generics::IfConst<T, const FixedPtr, FixedPtr>::
      Result& sptr) throw (NullPointer);

    FixedPtr(FixedPtr&& sptr) throw (NullPointer);

    FixedPtr(typename Generics::IfConst<T, int, const FixedPtr&>::Result) =
      delete;

    template <typename Other>
    FixedPtr(Other&& sptr) throw (NullPointer);

    // d-tor
    ~FixedPtr() throw ();

    // conversions
    operator Type*() throw ()
      __attribute__((always_inline));

    operator const Type*() const throw ()
      __attribute__((always_inline));

    // accessors
    Type*
    operator ->() throw (NotInitialized)
      __attribute__((always_inline));

    const Type*
    operator ->() const throw (NotInitialized)
      __attribute__((always_inline));

    Type&
    operator *() throw (NotInitialized)
      __attribute__((always_inline));

    const Type&
    operator *() const throw (NotInitialized)
      __attribute__((always_inline));

    Type*
    in() throw ()
      __attribute__((always_inline));

    const Type*
    in() const throw ()
      __attribute__((always_inline));

  protected:
    FixedPtr() throw ();

    Type*
    retn() throw ();

    Type* ptr_;

    friend
    Type*
    add_ref<T, Policy>(FixedPtr&& ptr) throw ();
  };

  template <typename T, typename Policy = PolicyThrow>
  class QualPtr : public FixedPtr<T, Policy>
  {
  public:
    typedef FixedPtr<T, Policy> Base;

    typedef typename Base::Type Type;

    typedef typename Base::NullPointer NullPointer;
    typedef typename Base::NotInitialized NotInitialized;

  public:
    // c-tors
    QualPtr() throw (NullPointer);

    template <typename Other>
    QualPtr(Other&& sptr) throw (NullPointer);

    // assignment & copy-assignment operators
    QualPtr&
    operator =(typename Generics::IfConst<T, const QualPtr, QualPtr>::
      Result& sptr) throw (NullPointer);

    QualPtr&
    operator =(typename Generics::IfConst<T, int, const QualPtr&>::Result) =
      delete;

    template <typename Other>
    QualPtr&
    operator =(Other&& sptr) throw (NullPointer);

    template <typename OtherPolicy>
    void
    swap(QualPtr<Type, OtherPolicy>& sptr)
      throw (NullPointer, typename OtherPolicy::NullPointer);

    void
    reset() throw (NullPointer);

    using Base::retn;

  private:
    using Base::ptr_;
  };

  template <typename T, typename Policy = PolicyThrow>
  class ConstPtr : public QualPtr<const T, Policy>
  {
  public:
    typedef QualPtr<const T, Policy> Base;

    typedef typename Base::Type Type;

    typedef typename Base::NullPointer NullPointer;
    typedef typename Base::NotInitialized NotInitialized;

  public:
    // c-tors
    ConstPtr() throw (NullPointer);

    ConstPtr(const ConstPtr& sptr) throw (NullPointer);

    template <typename Other>
    ConstPtr(Other&& sptr) throw (NullPointer);

    // assignment & copy-assignment operators
    using Base::operator =;

    ConstPtr&
    operator =(const ConstPtr& sptr) throw (NullPointer);
  };


  template <typename T>
  struct ThrowPtr
  {
    typedef SmartPtr<T, PolicyThrow> Ptr;
    typedef FixedPtr<T, PolicyThrow> FPtr;
    typedef QualPtr<T, PolicyThrow> QPtr;
  };

  template <typename T>
  struct AssertPtr
  {
    typedef SmartPtr<T, PolicyAssert> Ptr;
    typedef FixedPtr<T, PolicyAssert> FPtr;
    typedef QualPtr<T, PolicyAssert> QPtr;
  };

  template <typename T>
  struct NonNullPtr
  {
    typedef SmartPtr<T, PolicyNotNull> Ptr;
    typedef FixedPtr<T, PolicyNotNull> FPtr;
    typedef QualPtr<T, PolicyNotNull> QPtr;
  };
}

#include <ReferenceCounting/SmartPtr.ipp>
#include <ReferenceCounting/SmartPtr.tpp>

#endif
