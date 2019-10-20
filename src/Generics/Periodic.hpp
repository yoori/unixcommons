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



#ifndef GENERICS_PERIODIC_HPP
#define GENERICS_PERIODIC_HPP

#include <ReferenceCounting/Vector.hpp>

#include <Generics/ActiveObject.hpp>


namespace Generics
{
  /**
   * Task for Periodic. Aggregates period it must be started every.
   * Can be used separately with "run" and "stop" calls.
   */
  class PeriodicTask : public ReferenceCounting::AtomicImpl
  {
  public:
    /**
     * Constructor
     * @param period desired time interval between actions
     */
    PeriodicTask(const Generics::Time& period)
      throw (eh::Exception);

    /**
     * Changes desired interval between actions. Doesn't force it.
     * @param period new desired time interval
     */
    void
    set_period(const Generics::Time& period) throw ();

    /**
     * Wait time calculator. Depending on elapsed time it calculates
     * time to wait.
     * @param elapsed elapsed time
     * @return time to wait, usually "period - elapsed"
     */
    virtual
    Generics::Time
    wait_period(const Generics::Time& elapsed) const throw ();

    /**
     * Action function to execute.
     */
    virtual
    void
    task(bool forced) throw (eh::Exception) = 0;

    /**
     * Notifies to break the wait cycle and to try to execute the action.
     */
    void
    enforce_start() throw (eh::Exception);


    /**
     * Runs the action once with error reporting.
     * @param callback callback to use for error reporting
     * @param forced it will be passed to task function
     */
    void
    run_once(ActiveObjectCallback* callback, bool forced) throw ();

    /**
     * Runs the main cycle with action execution and wait.
     * Can be used for separate (without Periodic) functionality usage.
     */
    void
    run(ActiveObjectCallback* callback) throw ();

    /**
     * Notifies to break the main cycle
     */
    void
    stop() throw (eh::Exception);

  protected:
    /**
     * Destructor
     */
    virtual
    ~PeriodicTask() throw ();

  protected:
    mutable Sync::PosixMutex mutex_;
    Generics::Time period_;

  private:
    Sync::Conditional cond_;
    volatile sig_atomic_t quit_, start_;
  };
  typedef ReferenceCounting::QualPtr<PeriodicTask> PeriodicTask_var;

  /**
   * Aggregator of PeriodicTasks.
   * Each task has a separate thread assigned.
   */
  class PeriodicRunner :
    public ActiveObject,
    public ReferenceCounting::AtomicImpl
  {
  public:
    typedef ActiveObject::Exception Exception;
    typedef ActiveObject::NotSupported NotSupported;
    typedef ActiveObject::AlreadyActive AlreadyActive;
    typedef ActiveObject::InvalidArgument InvalidArgument;

    /**
     * Constructor
     * @param callback error callback
     * @param stack_size desired threads' stack size
     */
    PeriodicRunner(ActiveObjectCallback* callback,
      std::size_t stack_size = 0)
      throw (eh::Exception);


    /**
     * Runs a task once and adds it.
     * @param task task to execute and add
     * @param silent if no task execution exception must be thrown out
     * @param run if the task must be run
     */
    void
    add_task(PeriodicTask* task, bool silent = true, bool run = true)
      throw (eh::Exception);

    /**
     * Create threads running tasks.
     */
    virtual
    void
    activate_object() throw (AlreadyActive, Exception, eh::Exception);

    /**
     * Initiate stopping of threads.
     */
    virtual
    void
    deactivate_object() throw (Exception, eh::Exception);

    /**
     * Waits for threads to be finished.
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

    /**
     * Clears the aggregated tasks.
     */
    virtual
    void
    clear() throw (eh::Exception);

    /**
     * Informs all tasks to break the wait cycle and to execute.
     */
    void
    enforce_start_all() throw (eh::Exception);


  protected:
    /**
     * Destructor
     */
    virtual
    ~PeriodicRunner() throw ();

  private:
    class PeriodicJob : public ThreadJob
    {
    public:
      PeriodicJob(ActiveObjectCallback* callback, PeriodicTask* task)
        throw ();

      virtual
      void
      work() throw ();

      void
      signal(void (PeriodicTask::*signal)()) throw (eh::Exception);

    protected:
      virtual
      ~PeriodicJob() throw ();

    private:
      ActiveObjectCallback_var callback_;
      PeriodicTask_var task_;
    };
    typedef ReferenceCounting::QualPtr<PeriodicJob> PeriodicJob_var;

    typedef ReferenceCounting::Vector<PeriodicJob_var> PeriodicJobs;

    void
    signal_all_(void (PeriodicTask::*signal)()) throw (eh::Exception);


    Sync::PosixMutex work_mutex_;
    Sync::PosixMutex termination_mutex_;
    ActiveObjectCallback_var callback_;
    volatile sig_atomic_t active_state_;
    size_t stack_size_;
    PeriodicJobs jobs_;
    std::unique_ptr<ThreadRunner> thread_runner_;
  };
  typedef ReferenceCounting::QualPtr<PeriodicRunner> PeriodicRunner_var;
}

#endif
