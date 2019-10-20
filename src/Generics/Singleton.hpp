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



#ifndef GENERICS_SINGLETON_HPP
#define GENERICS_SINGLETON_HPP


//#define LOUD_COUNTER_BACKTRACE


#include <cstdio>
#include <memory>
#include <cassert>

#include <signal.h>
#include <unistd.h>
#include <ext/atomicity.h>

#include <Sync/PosixLock.hpp>

#include <ReferenceCounting/ReferenceCounting.hpp>

#include <Generics/Uncopyable.hpp>
#include <Generics/Function.hpp>
#ifdef LOUD_COUNTER_BACKTRACE
#include <Generics/Proc.hpp>
#endif

#include <Stream/MemoryStream.hpp>


namespace Generics
{
  /**
   * All instances of this and descending classes are removed only after
   * exit(3) call or main exit.
   * Useful for singletons.
   */
  class AtExitDestroying : private Uncopyable
  {
  public:
    enum DEFAULT_PRIORITIES
    {
      DP_USUAL_SINGLETON = 0,
      // CorbaClientAdapter::Orbs may be used by usual singletons
      DP_CLIENT_ORBS = 8192,
      // LoudCounters must be destroyed after all
      DP_LOUD_COUNTER = 16384,
    };

  protected:
    /**
     * Constructor
     * Optionally registers destroy function in atexit
     * Inserts object into the list of destroyable objects
     * @param priority objects with lesser value will be destroyed sooner
     */
    explicit
    AtExitDestroying(int priority) throw ();

    /**
     * Destructor
     */
    virtual
    ~AtExitDestroying() throw ();

  private:
    /**
     * Destroys the registered objects
     */
    static
    void
    destroy_at_exit_() throw ();

    static Sync::PosixMutex mutex_;
    static bool registered_;
    static AtExitDestroying* lower_priority_head_;
    AtExitDestroying* lower_priority_;
    AtExitDestroying* equal_priority_;
    int priority_;
  };

  namespace Helper
  {
    /**
     * Destroys Object on exit
     */
    template <typename Object, typename Pointer, const int PRIORITY>
    class AtExitDestroyer : public AtExitDestroying
    {
    public:
      /**
       * Constructor
       * @param object object to destroy at exit
       */
      explicit
      AtExitDestroyer(Object* object) throw ();

    protected:
      /**
       * Destructor
       */
      virtual
      ~AtExitDestroyer() throw ();

    private:
      Pointer object_;
    };

    /**
     * Adapter for std::unique_ptr
     */
    template <typename Type>
    class AutoPtr : public std::unique_ptr<Type>
    {
    public:
      explicit
      AutoPtr(Type* object) throw ();
      Type*
      in() throw ();
      Type*
      retn() throw ();
    };

    /**
     * Adapter for simple pointer
     */
    template <typename Type>
    class SimplePtr
    {
    public:
      explicit
      SimplePtr(Type* object) throw ();
      Type*
      in() throw ();
      Type*
      retn() throw ();

    private:
      Type* ptr_;
    };
  }

  /**
   * Singleton
   * Safe to use in multithreaded environment (even before main() call).
   * Single object is destroyed after exit(3) call or main() exit.
   * It is not safe to call instance() at that time.
   */
  template <typename Single, typename Pointer = Helper::AutoPtr<Single>,
    const int PRIORITY = AtExitDestroying::DP_USUAL_SINGLETON>
  class Singleton
  {
  public:
    /**
     * Optionally creates a new Single object or returns reference to the
     * existing.
     * It is not safe to call it after exit(3) call or main() exit.
     * @return reference to the unique object
     */
    static
    Single&
    instance() throw (eh::Exception);

  private:
    static Sync::PosixMutex mutex_;
    static volatile sig_atomic_t initialized_;
    static Single* volatile instance_;
  };


  /**
   * Template class is aimed to allow only one instance of certain type
   * to exist at the given point in time. Lifetime of each of those instances
   * are controlled manually.
   */
  template <typename Determinator,
    typename BaseException = eh::DescriptiveException>
  class Unique : private Uncopyable
  {
  public:
    DECLARE_EXCEPTION(Exception, BaseException);

  protected:
    /**
     * Constructor
     * Successfully constructs the object only if another one does not exist.
     */
    Unique() throw (eh::Exception, Exception);
    /**
     * Destructor
     * Allows creating of another object of the same type.
     */
    ~Unique() throw ();

  private:
    static Sync::PosixMutex mutex_;
    static Unique<Determinator, BaseException>* volatile existing_;
  };


  /**
   * Class informs if some objects of the specified class have not been
   * destroyed on program shutdown.
   */
  template <typename Determinator>
  class AllDestroyer
  {
  protected:
    /**
     * Constructor.
     * Increases the number of objects created.
     */
    AllDestroyer() throw ();
    /**
     * Constructor.
     * Increases the number of objects created.
     */
    AllDestroyer(const AllDestroyer&) throw ();
    /**
     * Destructor.
     * Decreases the number of objects created.
     */
    ~AllDestroyer() throw ();

  private:
    struct Info
    {
#ifdef LOUD_COUNTER_BACKTRACE
      char info[2048];
      Info* next;
#endif
    };
    Info info_;

    class LoudCounter : public ReferenceCounting::AtomicImpl
    {
    public:
      LoudCounter() throw ();
      void
      increment(Info* info) throw ();
      void
      decrement(Info* info) throw ();
      void
      check() throw ();

    private:
      virtual
      ~LoudCounter() throw ();

      _Atomic_word counter_;
#ifdef LOUD_COUNTER_BACKTRACE
      Sync::PosixMutex mutex_;
      Info* head_;
#endif
    };
    typedef ReferenceCounting::FixedPtr<LoudCounter> LoudCounter_var;

    class LoudCounterHolder : private Uncopyable
    {
    public:
      typedef Singleton<LoudCounterHolder,
        Helper::AutoPtr<LoudCounterHolder>,
        AtExitDestroying::DP_LOUD_COUNTER> Single;

      LoudCounterHolder() throw (eh::Exception);
      ~LoudCounterHolder() throw ();

      LoudCounter*
      counter() throw ();

    private:
      LoudCounter_var counter_;
    };

    LoudCounter_var counter_;
  };
}

namespace Generics
{
  //
  // AtExitDestroying class
  //

  inline
  AtExitDestroying::~AtExitDestroying() throw ()
  {
  }


  namespace Helper
  {
    //
    // AtExitDestroyer class
    //

    template <typename Object, typename Pointer, const int PRIORITY>
    AtExitDestroyer<Object, Pointer, PRIORITY>::AtExitDestroyer(
      Object* object) throw ()
      : AtExitDestroying(PRIORITY), object_(object)
    {
    }

    template <typename Object, typename Pointer, const int PRIORITY>
    AtExitDestroyer<Object, Pointer, PRIORITY>::~AtExitDestroyer() throw ()
    {
    }


    //
    // class AutoPtr
    //

    template <typename Type>
    AutoPtr<Type>::AutoPtr(Type* object) throw ()
      : std::unique_ptr<Type>(object)
    {
    }

    template <typename Type>
    Type*
    AutoPtr<Type>::in() throw ()
    {
      return this->get();
    }

    template <typename Type>
    Type*
    AutoPtr<Type>::retn() throw ()
    {
      return this->release();
    }


    //
    // class SimplePtr
    //

    template <typename Type>
    SimplePtr<Type>::SimplePtr(Type* object) throw ()
      : ptr_(object)
    {
    }

    template <typename Type>
    Type*
    SimplePtr<Type>::in() throw ()
    {
      return ptr_;
    }

    template <typename Type>
    Type*
    SimplePtr<Type>::retn() throw ()
    {
      Type* ptr(ptr_);
      ptr_ = 0;
      return ptr;
    }
  }


  //
  // Singleton class
  //

  // All of these are initialized statically
  template <typename Single, typename Pointer, const int PRIORITY>
  Sync::PosixMutex Singleton<Single, Pointer, PRIORITY>::mutex_;
  template <typename Single, typename Pointer, const int PRIORITY>
  volatile sig_atomic_t Singleton<Single, Pointer, PRIORITY>::initialized_ =
    false;
  template <typename Single, typename Pointer, const int PRIORITY>
  Single* volatile Singleton<Single, Pointer, PRIORITY>::instance_ = 0;

  template <typename Single, typename Pointer, const int PRIORITY>
  Single&
  Singleton<Single, Pointer, PRIORITY>::instance() throw (eh::Exception)
  {
    if (!initialized_)
    {
      {
        Sync::PosixGuard guard(mutex_);
        if (!instance_)
        {
          Pointer single(new Single);
          new Helper::AtExitDestroyer<Single, Pointer, PRIORITY>(single.in());
          instance_ = single.retn();
        }
      }
      initialized_ = true;
    }
    return *instance_;
  }


  //
  // Unique class
  //

  template <typename Determinator, typename BaseException>
  Sync::PosixMutex Unique<Determinator, BaseException>::mutex_;
  template <typename Determinator, typename BaseException>
  Unique<Determinator, BaseException>*
    volatile Unique<Determinator, BaseException>::existing_ = 0;

  template <typename Determinator, typename BaseException>
  Unique<Determinator, BaseException>::Unique()
    throw (eh::Exception, Exception)
  {
    Sync::PosixGuard guard(mutex_);

    if (existing_)
    {
      Stream::Error ostr;
      ostr << FNS << "another unique " << existing_ << " still exists";
      throw Exception(ostr);
    }

    existing_ = this;
  }

  template <typename Determinator, typename BaseException>
  Unique<Determinator, BaseException>::~Unique() throw ()
  {
    Sync::PosixGuard guard(mutex_);
    assert(existing_ == this);
    existing_ = 0;
  }


  //
  // AllDestroyer::LoudCounter class
  //

  template <typename Determinator>
  AllDestroyer<Determinator>::LoudCounter::LoudCounter() throw ()
    : counter_(0)
#ifdef LOUD_COUNTER_BACKTRACE
      ,
      head_(0)
#endif
  {
  }

  template <typename Determinator>
  AllDestroyer<Determinator>::LoudCounter::~LoudCounter() throw ()
  {
  }

  template <typename Determinator>
  void
  AllDestroyer<Determinator>::LoudCounter::increment(Info* info) throw ()
  {
    __gnu_cxx::__atomic_add(&counter_, 1);
#ifdef LOUD_COUNTER_BACKTRACE
    *info->info = '\0';
    Proc::backtrace(info->info, sizeof(info->info), 4, 10);
    Sync::PosixGuard guard(mutex_);
    info->next = head_;
    head_ = info;
#else
    (void)info;
#endif
  }

  template <typename Determinator>
  void
  AllDestroyer<Determinator>::LoudCounter::decrement(Info* info) throw ()
  {
    __gnu_cxx::__atomic_add(&counter_, -1);
#ifdef LOUD_COUNTER_BACKTRACE
    Sync::PosixGuard guard(mutex_);
    for (Info** p = &head_; *p; p = &(*p)->next)
    {
      if (*p == info)
      {
        *p = (*p)->next;
        break;
      }
    }
#else
    (void)info;
#endif
  }

  template <typename Determinator>
  void
  AllDestroyer<Determinator>::LoudCounter::check() throw ()
  {
    int counter = static_cast<int>(counter_);
    if (counter)
    {
      char buf[8192];
      int len = std::snprintf(buf, sizeof(buf),
        "Not been removed %i of %s\n", counter,
        Determinator::PRINTABLE_NAME);
#ifdef LOUD_COUNTER_BACKTRACE
      {
        Sync::PosixGuard guard(mutex_);
        for (Info* p = head_; p; p = p->next)
        {
          if (len == sizeof(buf))
          {
            break;
          }

          int s = std::min(strlen(p->info), sizeof(buf) - len - 1);
          memcpy(buf + len, p->info, s);
          buf[len + s] = '\n';
          len += s + 1;
        }
      }
#endif
      write(STDERR_FILENO, buf, len);
    }
  }


  //
  // AllDestroyer::LoudCounterHolder class
  //

  template <typename Determinator>
  AllDestroyer<Determinator>::LoudCounterHolder::LoudCounterHolder()
    throw (eh::Exception)
    : counter_(new LoudCounter)
  {
  }

  template <typename Determinator>
  AllDestroyer<Determinator>::LoudCounterHolder::~LoudCounterHolder() throw ()
  {
    counter_->check();
  }

  template <typename Determinator>
  typename AllDestroyer<Determinator>::LoudCounter*
  AllDestroyer<Determinator>::LoudCounterHolder::counter() throw ()
  {
    return counter_;
  }


  //
  // AllDestroyer class
  //

  template <typename Determinator>
  AllDestroyer<Determinator>::AllDestroyer() throw ()
    : counter_(ReferenceCounting::add_ref(
        AllDestroyer<Determinator>::LoudCounterHolder::Single::
          instance().counter()))
  {
    counter_->increment(&info_);
  }

  template <typename Determinator>
  AllDestroyer<Determinator>::AllDestroyer(const AllDestroyer& another)
    throw ()
    : counter_(another.counter_)
  {
    counter_->increment(&info_);
  }

  template <typename Determinator>
  AllDestroyer<Determinator>::~AllDestroyer() throw ()
  {
    counter_->decrement(&info_);
  }
}

#endif
