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





#ifndef GENERICS_TASK_RUNNER_HPP
#define GENERICS_TASK_RUNNER_HPP

#include <Sync/Semaphore.hpp>

#include <ReferenceCounting/Deque.hpp>

#include <Generics/Scheduler.hpp>


namespace Generics
{
  /**
   * General Task to be processed by TaskRunner.
   */
  class Task : public virtual ReferenceCounting::Interface
  {
  public:
    /**
     * Method is called by TaskRunner when the object's order arrives.
     */
    virtual
    void
    execute() throw (eh::Exception) = 0;

  protected:
    virtual
    ~Task() throw ();
  };
  typedef ReferenceCounting::QualPtr<Task> Task_var;

  /**
   * Performs tasks in several threads simultaneously.
   */
  class TaskRunner : public ActiveObjectCommonImpl
  {
  public:
    DECLARE_EXCEPTION(Exception, ActiveObject::Exception);
    DECLARE_EXCEPTION(Overflow, Exception);
    DECLARE_EXCEPTION(NotActive, Exception);

    /**
     * Constructor
     * @param callback not null callback is called on errors
     * @param threads_number number of working threads
     * @param stack_size their stack sizes
     * @param max_pending_tasks maximum task queue length
     * @param start_threads initial number of threads to start (0 - all)
     */
    TaskRunner(ActiveObjectCallback* callback,
      unsigned threads_number, size_t stack_size = 0,
      unsigned max_pending_tasks = 0,
      unsigned start_threads = 0)
      throw (InvalidArgument, Exception, eh::Exception);

    /**
     * Enqueues a task
     * @param task task to enqueue. Number of references is not increased
     * @param timeout maximal absolute wait time before fail on mutex lock
     * until the task is put in the queue. NULL timeout means no wait.
     * If you put limitations on the size of the queue, and it's full,
     * method waits for the release up to timeout
     */
    void
    enqueue_task(Task* task, const Time* timeout = 0)
      throw (InvalidArgument, Overflow, NotActive, eh::Exception);

    /**
     * Returns number of tasks recently being enqueued
     * This number does not have much meaning in MT environment
     * @return number of tasks enqueued
     */
    unsigned
    task_count() const throw ();

    /**
     * Waits for the moment task queue is empty and returns control.
     * In MT environment tasks can be added at the very same moment of
     * return of control.
     */
    void
    wait_for_queue_exhausting() throw (eh::Exception);

    /**
     * Clear task queue
     */
    virtual
    void
    clear() throw (eh::Exception);

  protected:
    virtual
    ~TaskRunner() throw ();

  private:
    class TaskRunnerJob : public SingleJob
    {
    public:
      TaskRunnerJob(ActiveObjectCallback* callback,
        unsigned number_of_threads, unsigned max_pending_tasks)
        throw (eh::Exception);

      virtual
      void
      work() throw ();

      virtual
      void
      started(unsigned threads) throw ();

      virtual
      void
      terminate() throw ();

      void
      enqueue_task(Task* task, const Time* timeout,
        ThreadRunner& thread_runner)
        throw (InvalidArgument, Overflow, NotActive, eh::Exception);

      unsigned
      task_count() const throw ();

      void
      wait_for_queue_exhausting() throw (eh::Exception);

      void
      clear() throw (eh::Exception);

      void
      add_thread(ThreadRunner& thread_runner) throw ();

    protected:
      virtual
      ~TaskRunnerJob() throw ();

    private:
      typedef ReferenceCounting::Deque<Task_var> Tasks;

      const unsigned NUMBER_OF_THREADS_;
      unsigned number_of_unused_threads_;
      Tasks tasks_;
      Sync::Semaphore new_task_;
      Sync::Semaphore not_full_;
      const bool LIMITED_;
    };
    typedef ReferenceCounting::FixedPtr<TaskRunnerJob> TaskRunnerJob_var;

    TaskRunnerJob& job_;
  };
  typedef ReferenceCounting::QualPtr<TaskRunner> TaskRunner_var;
  typedef ReferenceCounting::FixedPtr<TaskRunner> FixedTaskRunner_var;

  /**
   * Task with specified RC implementation
   */
  class TaskImpl :
    public virtual Task,
    public virtual ReferenceCounting::AtomicImpl
  {
  protected:
    /**
     * Destructor
     */
    virtual
    ~TaskImpl() throw ();
  };

  /**
   * Should be put into the Planner.
   * When time arrives, it puts itself into TaskRunner.
   */
  class TaskGoal :
    public Goal,
    public Task,
    public ReferenceCounting::AtomicImpl
  {
  public:
    /**
     * Constructor
     * @param task_runner TaskRunner to put the object into.
     */
    explicit
    TaskGoal(TaskRunner* task_runner) throw (eh::Exception);

    /**
     * Implementation of Goal::deliver.
     * Puts the object into the TaskRunner.
     */
    virtual
    void
    deliver() throw (eh::Exception);

  protected:
    /**
     * Destructor
     */
    virtual
    ~TaskGoal() throw ();

  private:
    TaskRunner_var task_runner_;
  };

  /**
   * Reusable version of TaskGoal
   */
  class GoalTask :
    public Goal,
    public Task,
    public ReferenceCounting::AtomicImpl
  {
  public:
    /**
     * Constructor
     * After the object construction call deliver() to put the object into the
     * TaskRunner or schedule_() to put it into the Planner.
     * @param planner Planner to put the object into.
     * @param task_runner TaskRunner to put the object into.
     * or in Planner otherwise
     */
    GoalTask(Planner* planner, TaskRunner* task_runner)
      throw (eh::Exception);

    /**
     * Implementation of Goal::deliver.
     * Puts the object into the TaskRunner.
     */
    virtual
    void
    deliver() throw (eh::Exception);

    /**
     * Put the object into the Planner. Call this in execute().
     * @param time time of putting the object into the TaskRunner
     */
    void
    schedule(const Time& time) throw (eh::Exception);

  protected:
    /**
     * Destructor
     */
    virtual
    ~GoalTask() throw ();

  private:
    Planner_var planner_;
    TaskRunner_var task_runner_;
  };
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace Generics
{
  //
  // Task class
  //

  inline
  Task::~Task() throw ()
  {
  }


  //
  // TaskImpl class
  //

  inline
  TaskImpl::~TaskImpl() throw ()
  {
  }


  //
  // TaskGoal class
  //

  inline
  TaskGoal::TaskGoal(TaskRunner* task_runner)
    throw (eh::Exception)
    : task_runner_(ReferenceCounting::add_ref(task_runner))
  {
  }

  inline
  TaskGoal::~TaskGoal() throw ()
  {
  }

  inline
  void
  TaskGoal::deliver() throw (eh::Exception)
  {
    task_runner_->enqueue_task(this);
  }


  //
  // GoalTask class
  //

  inline
  GoalTask::GoalTask(Planner* planner, TaskRunner* task_runner)
    throw (eh::Exception)
    : planner_(ReferenceCounting::add_ref(planner)),
      task_runner_(ReferenceCounting::add_ref(task_runner))
  {
  }

  inline
  GoalTask::~GoalTask() throw ()
  {
  }

  inline
  void
  GoalTask::deliver() throw (eh::Exception)
  {
    task_runner_->enqueue_task(this);
  }

  inline
  void
  GoalTask::schedule(const Time& when) throw (eh::Exception)
  {
    planner_->schedule(this, when);
  }


  //
  // TaskRunner::TaskRunnerJob class
  //

  inline
  unsigned
  TaskRunner::TaskRunnerJob::task_count() const throw ()
  {
    Sync::PosixGuard guard(mutex());
    return tasks_.size();
  }


  //
  // TaskRunner class
  //

  inline
  void
  TaskRunner::enqueue_task(Task* task, const Time* timeout)
    throw (InvalidArgument, Overflow, NotActive, eh::Exception)
  {
    job_.enqueue_task(task, timeout, thread_runner_);
  }

  inline
  unsigned
  TaskRunner::task_count() const throw ()
  {
    return job_.task_count();
  }

  inline
  void
  TaskRunner::wait_for_queue_exhausting() throw (eh::Exception)
  {
    job_.wait_for_queue_exhausting();
  }

  inline
  void
  TaskRunner::clear() throw (eh::Exception)
  {
    job_.clear();
  }
}

#endif
