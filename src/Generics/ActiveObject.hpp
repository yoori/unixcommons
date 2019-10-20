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





#ifndef GENERICS_ACTIVE_OBJECT_HPP
#define GENERICS_ACTIVE_OBJECT_HPP

#include <Sync/Condition.hpp>

#include <Generics/Singleton.hpp>
#include <Generics/ThreadRunner.hpp>


namespace Generics
{
  /**
   * Reference countable callback for report errors.
   */
  class ActiveObjectCallback : public ThreadCallback
  {
  public:
    enum Severity
    {
      CRITICAL_ERROR = 0,
      ERROR = 1,
      WARNING = 2
    };

    virtual
    void
    report_error(Severity severity, const String::SubString& description,
      const char* error_code = 0) throw () = 0;

    void
    critical(const String::SubString& description,
      const char* error_code = 0) throw ();

    void
    error(const String::SubString& description,
      const char* error_code = 0) throw ();

    void
    warning(const String::SubString& description,
      const char* error_code = 0) throw ();

  protected:
    virtual
    ~ActiveObjectCallback() throw ();
  };
  typedef ReferenceCounting::QualPtr<ActiveObjectCallback>
    ActiveObjectCallback_var;
  typedef ReferenceCounting::FixedPtr<ActiveObjectCallback>
    FixedActiveObjectCallback_var;

  class ActiveObject :
    public virtual ReferenceCounting::Interface,
    private AllDestroyer<ActiveObject>
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(NotSupported, Exception);
    DECLARE_EXCEPTION(AlreadyActive, Exception);
    DECLARE_EXCEPTION(InvalidArgument, Exception);

  public:
    virtual
    void
    activate_object()
      throw (AlreadyActive, Exception, eh::Exception) = 0;

    virtual
    void
    deactivate_object()
      throw (Exception, eh::Exception) = 0;

    virtual
    void
    wait_object()
      throw (Exception, eh::Exception) = 0;

    virtual
    bool
    active()
      throw (eh::Exception) = 0;

    virtual
    void
    clear() throw (eh::Exception);

  public:
    static const char PRINTABLE_NAME[];

  protected:
    virtual
    ~ActiveObject() throw ();

    enum ACTIVE_STATE
    {
      AS_ACTIVE,
      AS_DEACTIVATING,
      AS_NOT_ACTIVE
    };
  };
  typedef ReferenceCounting::QualPtr<ActiveObject>
    ActiveObject_var;


  /**
   * SimpleActiveObject implements expected ActiveObject state machine
   * and provides callbacks for additional state changing operations.
   * Base for many ActiveObjects.
   */
  class SimpleActiveObject : public virtual ActiveObject
  {
  public:
    SimpleActiveObject() throw (eh::Exception);

    virtual
    void
    activate_object() throw (AlreadyActive, Exception, eh::Exception);

    virtual
    void
    deactivate_object() throw (Exception, eh::Exception);

    virtual
    void
    wait_object() throw (Exception, eh::Exception);

    virtual
    bool
    active() throw (eh::Exception);

  protected:
    virtual
    ~SimpleActiveObject() throw ();

    virtual
    void
    activate_object_() throw (Exception, eh::Exception);

    virtual
    void
    deactivate_object_() throw (Exception, eh::Exception);

    virtual
    bool
    wait_more_() throw (Exception, eh::Exception);

    virtual
    void
    wait_object_() throw (Exception, eh::Exception);

    Sync::Condition cond_;
    volatile sig_atomic_t state_;
  };


  /**
   * General implementation Active Object logic by default.
   * May be supplement with special logic in concrete Active Object
   * through virtual methods override (of SingleJob descendand).
   */
  class ActiveObjectCommonImpl :
    public virtual ActiveObject,
    public virtual ReferenceCounting::AtomicImpl
  {
  public:
    typedef ActiveObject::Exception Exception;
    typedef ActiveObject::NotSupported NotSupported;
    typedef ActiveObject::AlreadyActive AlreadyActive;
    typedef ActiveObject::InvalidArgument InvalidArgument;

    /**
     * Start threads that will perform SingleJob
     */
    virtual
    void
    activate_object()
      throw (AlreadyActive, Exception, eh::Exception);

    /**
     * Initiate stopping of Active object
     * Acquires mutex and informs SingleJob
     */
    virtual
    void
    deactivate_object()
      throw (Exception, eh::Exception);

    /**
     * Waits for deactivation completion
     * Acquires mutex and waits for threads completion
     */
    virtual
    void
    wait_object() throw (Exception, eh::Exception);

    /**
     * Current status
     * @return Returns true if active and not going to deactivate
     */
    virtual
    bool
    active() throw (eh::Exception);

  protected:
    /**
     * ActiveObjectCommonImpl expects the only object will be a job for
     * all ThreadRunner's threads. This object must be a descendant of
     * this class.
     */
    class SingleJob : public ThreadJob
    {
    public:
      typedef ActiveObject::Exception Exception;
      typedef ActiveObject::NotSupported NotSupported;
      typedef ActiveObject::AlreadyActive AlreadyActive;
      typedef ActiveObject::InvalidArgument InvalidArgument;

      /**
       * Constructor
       * @param callback callback to be called for error reporting
       */
      explicit
      SingleJob(ActiveObjectCallback* callback)
        throw (InvalidArgument, eh::Exception);

      /**
       * Stored callback
       * @return stored callback
       */
      ActiveObjectCallback_var
      callback() throw ();

      /**
       * Mutex for operations synchronizations
       * @return stored mutex
       */
      Sync::PosixMutex&
      mutex() const throw ();

      virtual
      void
      started(unsigned threads) throw ();

      void
      make_terminate() throw ();

      void
      terminated() throw ();

      bool
      is_terminating() throw ();

      /**
       * Function must inform the object to stop jobs to work.
       */
      virtual
      void
      terminate() throw () = 0;

    protected:
      /**
       * Destructor
       */
      virtual
      ~SingleJob() throw ();

    private:
      mutable Sync::PosixMutex mutex_;
      ActiveObjectCallback_var callback_;
      volatile sig_atomic_t terminating_;
    };
    typedef ReferenceCounting::FixedPtr<SingleJob> SingleJob_var;

    /**
     * Constructor
     * Initializes SINGLE_JOB_ with the provided job and
     * creates ThreadRunner.
     * @param job job to execute in threads
     * @param threads_number number of threads to execute the job in
     * @param stack_size stack size for threads
     * @param start_threads initial number of threads to start (0 - all)
     */
    explicit
    ActiveObjectCommonImpl(SingleJob* job,
      unsigned threads_number = 1, size_t stack_size = 0,
      unsigned start_threads = 0) throw (InvalidArgument);

    /**
     * Destructor
     */
    virtual
    ~ActiveObjectCommonImpl() throw ();

    /**
     * @return the same mutex SINGLE_JOB_->mutex() returns
     */
    Sync::PosixMutex&
    mutex_() const throw ();


    SingleJob_var SINGLE_JOB_;
    ThreadRunner thread_runner_;

  private:
    unsigned start_threads_;

    mutable Sync::PosixMutex termination_mutex_;
    Sync::PosixMutex& work_mutex_;

    volatile sig_atomic_t active_state_;
  };
} // namespace Generics

//////////////////////////////////////////////////////////////////////////
//  Inlines
//////////////////////////////////////////////////////////////////////////
namespace Generics
{
  //
  // ActiveObjectCallback class
  //

  inline
  ActiveObjectCallback::~ActiveObjectCallback() throw ()
  {
  }

  inline
  void
  ActiveObjectCallback::critical(const String::SubString& description,
    const char* error_code)
    throw ()
  {
    report_error(CRITICAL_ERROR, description, error_code);
  }

  inline
  void
  ActiveObjectCallback::error(const String::SubString& description,
    const char* error_code)
    throw ()
  {
    report_error(ERROR, description, error_code);
  }

  inline
  void
  ActiveObjectCallback::warning(const String::SubString& description,
    const char* error_code)
    throw ()
  {
    report_error(WARNING, description, error_code);
  }


  //
  // SimpleActiveObject class
  //

  inline
  SimpleActiveObject::SimpleActiveObject() throw (eh::Exception)
    : state_(AS_NOT_ACTIVE)
  {
  }


  //
  // class ActiveObjectCommonImpl
  //

  inline
  Sync::PosixMutex&
  ActiveObjectCommonImpl::mutex_() const throw ()
  {
    return work_mutex_;
  }


  //
  // ActiveObject class
  //

  inline
  ActiveObject::~ActiveObject() throw ()
  {
  }

  inline
  void
  ActiveObject::clear() throw (eh::Exception)
  {
  }


  //
  // ActiveObjectCommonImpl::SingleJob class
  //

  inline
  ActiveObjectCommonImpl::SingleJob::SingleJob(
    ActiveObjectCallback* callback)
    throw (InvalidArgument, eh::Exception)
    : callback_(ReferenceCounting::add_ref(callback)),
      terminating_(false)
  {
    if (!callback)
    {
      Stream::Error ostr;
      ostr << FNS << "callback == 0";
      throw InvalidArgument(ostr);
    }
  }

  inline
  ActiveObjectCommonImpl::SingleJob::~SingleJob() throw ()
  {
  }

  inline
  ActiveObjectCallback_var
  ActiveObjectCommonImpl::SingleJob::callback() throw ()
  {
    return callback_;
  }

  inline
  Sync::PosixMutex&
  ActiveObjectCommonImpl::SingleJob::mutex() const throw ()
  {
    return mutex_;
  }

  inline
  void
  ActiveObjectCommonImpl::SingleJob::started(unsigned /*threads*/) throw ()
  {
  }

  inline
  void
  ActiveObjectCommonImpl::SingleJob::make_terminate() throw ()
  {
    terminating_ = true;
    terminate();
  }

  inline
  void
  ActiveObjectCommonImpl::SingleJob::terminated() throw ()
  {
    terminating_ = false;
  }

  inline
  bool
  ActiveObjectCommonImpl::SingleJob::is_terminating() throw ()
  {
    return terminating_;
  }
}

#endif
