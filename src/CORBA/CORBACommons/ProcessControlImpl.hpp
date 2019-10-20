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





#ifndef CORBA_COMMONS_PROCESS_CONTROL_IMPL_HPP
#define CORBA_COMMONS_PROCESS_CONTROL_IMPL_HPP

#include <Generics/ThreadRunner.hpp>

#include <Logger/ActiveObjectCallback.hpp>

#include <CORBACommons/ProcessControl_s.hpp>
#include <CORBACommons/ServantImpl.hpp>


namespace CORBACommons
{
  /**
   * Interface for external shutdown of running ORBs
   */
  class OrbShutdowner : public virtual ::ReferenceCounting::Interface
  {
  public:
    /**
     * Shutdowns controlled ORBs (if any)
     * @param type whether wait for completion of pending CORBA requests
     * or not
     */
    virtual
    void
    shutdown(bool type) throw () = 0;

  protected:
    /**
     * Constructor
     */
    OrbShutdowner() throw ();
    /**
     * Destructor
     */
    virtual
    ~OrbShutdowner() throw ();
  };
  typedef ::ReferenceCounting::QualPtr<OrbShutdowner> OrbShutdowner_var;

  /**
   * One ORB shutdowner
   */
  class SimpleOrbShutdowner :
    public OrbShutdowner,
    public ::ReferenceCounting::AtomicImpl
  {
  public:
    /**
     * Constructor
     * @param orb orb to shutdown
     */
    explicit
    SimpleOrbShutdowner(CORBA::ORB_ptr orb) throw ();
    /**
     * Shutdowns controlled ORB
     * @param type whether wait for completion of pending CORBA requests
     * or not
     */
    virtual
    void
    shutdown(bool type) throw ();

  private:
    /**
     * Destructor
     */
    virtual
    ~SimpleOrbShutdowner() throw ();

    CORBA::ORB_var orb_;
  };

  template <typename Parent>
  class ProcessControlDefault : public ReferenceCounting::ServantImpl<Parent>
  {
  public:
    /**
     * Does nothing. Expected to be overridden in child classes.
     * @param wait_for_completion If true, then wait for completion of
     * pending corba requests passing 1 to CORBA::ORB::shutdown, otherwise
     * terminate process immediately passing 0 to CORBA::ORB::shutdown.
     */
    virtual
    void
    shutdown(CORBA::Boolean wait_for_completion)
      throw (CORBA::SystemException);

    /**
     * Implements process vitality check.
     * @return either AS_READY or AS_ALIVE depending on is_ready_()
     * return value
     */
    virtual
    CORBACommons::IProcessControl::ALIVE_STATUS
    is_alive() throw (CORBA::SystemException);

    /**
     * Provides extended status of the process to the caller
     * @return empty string, may be reimplemented in derived classes
     */
    virtual
    char*
    comment() throw (OutOfMemory);

    /**
     * Performs specific action on remote object
     * @param param_name action name or variable name to set
     * @param param_value additional action data
     * @return action result
     */
    virtual
    char*
    control(const char* param_name, const char* param_value)
      throw (OutOfMemory, ImplementationError);

  protected:
    /*
     * Destructor
     */
    virtual
    ~ProcessControlDefault() throw ();
    /**
     * Called by is_alive implementation to determine AS_ALIVE or AS_READY
     * status
     * @return true now, can be reimplemented in derived classes
     */
    virtual
    bool
    is_ready_() throw ();
  };

  /**
   * Implements CORBA servant class for process control.Object of this class
   * being registered with POA of custom CORBA server process takes care
   * about signalling ORB to leave message loop.
   */
  class ProcessControlImpl :
    public ProcessControlDefault<POA_CORBACommons::IProcessControl>
  {
  public:
    /**
     * ProcessControlImpl base exception class
     */
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    /**
     * InvalidArgument exception class.
     */
    DECLARE_EXCEPTION(InvalidArgument, Exception);

  public:
    /**
     * Construct ProcessControlImpl object.
     * @param shutdowner Pointer to shutdowner which allows to terminate
     * ORBs which receive signal to escape message loop.
     * @param shutdowner shutdowner to call by external request
     */
    explicit
    ProcessControlImpl(OrbShutdowner* shutdowner = 0)
      throw (InvalidArgument, Exception, eh::Exception);

    /**
     * Waits until object fully stops.
     */
    void
    wait() throw (eh::Exception);

    /**
     * Shutdowns a process calling CORBA::ORB::shutdown on ORB passed as a
     * parameter to constructor.
     * @param wait_for_completion If true, then wait for completion of
     * pending corba requests passing 1 to CORBA::ORB::shutdown, otherwise
     * terminate process immediately passing 0 to CORBA::ORB::shutdown.
     */
    virtual
    void
    shutdown(CORBA::Boolean wait_for_completion)
      throw (CORBA::SystemException);

  protected:
    /**
     * Destructor
     */
    virtual
    ~ProcessControlImpl() throw ();

  private:
    class ShutdownJob : public Generics::ThreadJob
    {
    public:
      explicit
      ShutdownJob(OrbShutdowner_var& shutdowner) throw ();

      virtual
      void
      work() throw ();

      void
      wake(bool shutdown) throw ();

    protected:
      virtual
      ~ShutdownJob() throw ();

    private:
      OrbShutdowner_var& shutdowner_;
      bool shutdown_;
      Sync::Semaphore sem_;
    };
    typedef ::ReferenceCounting::FixedPtr<ShutdownJob> ShutdownJob_var;

  protected:
    OrbShutdowner_var shutdowner_;

  private:
    ShutdownJob_var job_;
    Generics::ThreadRunner thread_runner_;
  };

  /**
   * Mix of LoggerCallbackHolder for ActiveObjectCallback
   * and ProcessControlImpl
   */
  class ProcessControlWithLogger :
    private Logging::LoggerCallbackHolder,
    public ProcessControlImpl
  {
  public:
    /**
     * Construct ProcessControlWithLogger
     * @param logger is initial logger will be used
     * @param message_prefix for callback
     * @param aspect for callback
     * @param code for callback
     * @param shutdowner for ProcessControlImpl
     */
    explicit
    ProcessControlWithLogger(Logging::Logger* logger = 0,
      const char* message_prefix = "ProcessControlWithLogger",
      const char* aspect = 0,
      const char* code = 0,
      OrbShutdowner* shutdowner = 0)
      throw (InvalidArgument, Exception, eh::Exception);

    /*
     * Share access methods
     */
    using Logging::LoggerCallbackHolder::callback;
    using Logging::LoggerCallbackHolder::logger;
  protected:
    /**
     * Destructor
     */
    virtual
    ~ProcessControlWithLogger() throw ();
  };
  typedef ::ReferenceCounting::QualPtr<ProcessControlWithLogger>
    ProcessControlWithLogger_var;
}

//
// INLINES
//

namespace CORBACommons
{
  //
  // ProcessControlDefault class
  //

  template <typename Parent>
  ProcessControlDefault<Parent>::~ProcessControlDefault()
    throw ()
  {
  }

  template <typename Parent>
  void
  ProcessControlDefault<Parent>::shutdown(
    CORBA::Boolean /*wait_for_completion*/)
    throw (CORBA::SystemException)
  {
  }

  template <typename Parent>
  CORBACommons::IProcessControl::ALIVE_STATUS
  ProcessControlDefault<Parent>::is_alive() throw (CORBA::SystemException)
  {
    return is_ready_() ? CORBACommons::IProcessControl::AS_READY :
      CORBACommons::IProcessControl::AS_ALIVE;
  }

  template <typename Parent>
  bool
  ProcessControlDefault<Parent>::is_ready_() throw ()
  {
    return true;
  }

  template <typename Parent>
  char*
  ProcessControlDefault<Parent>::comment() throw (OutOfMemory)
  {
    return 0;
  }

  template <typename Parent>
  char*
  ProcessControlDefault<Parent>::control(const char* /*param_name*/,
    const char* /*param_value*/)
    throw (OutOfMemory, ImplementationError)
  {
    return 0;
  }


  //
  // ProcessControlWithLogger class
  //

  inline
  ProcessControlWithLogger::ProcessControlWithLogger(
    Logging::Logger* logger, const char* message_prefix,
    const char* aspect, const char* code, OrbShutdowner* shutdowner)
    throw (InvalidArgument, Exception, eh::Exception)
    : Logging::LoggerCallbackHolder(logger, message_prefix, aspect, code),
      ProcessControlImpl(shutdowner)
  {
  }
  
  inline
  ProcessControlWithLogger::~ProcessControlWithLogger()
    throw ()
  {
  }
}

#endif
