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



#ifndef GENERICS_THREAD_RUNNER_HPP
#define GENERICS_THREAD_RUNNER_HPP

#include <signal.h>
#include <pthread.h>

#include <algorithm>

#include <Sync/Semaphore.hpp>

#include <ReferenceCounting/ReferenceCounting.hpp>


namespace Generics
{
  class ThreadRunner;

  /**
   * A job performed by ThreadRunner in the threads.
   */
  class ThreadJob : public ReferenceCounting::AtomicImpl
  {
  public:
    /**
     * Work process for the job.
     */
    virtual
    void
    work() throw () = 0;

  protected:
    /**
     * Destructor.
     */
    virtual
    ~ThreadJob() throw ();
  };
  typedef ReferenceCounting::QualPtr<ThreadJob> ThreadJob_var;


  /**
   * Callback for each new thread tuning
   */
  class ThreadCallback : public virtual ReferenceCounting::Interface
  {
  public:
    /**
     * Called in the newly created thread.
     */
    virtual
    void
    on_start() throw ();

    /**
     * Called in the thread going to terminate.
     */
    virtual
    void
    on_stop() throw ();

  protected:
    virtual
    ~ThreadCallback() throw ();
  };
  typedef ReferenceCounting::SmartPtr<ThreadCallback> ThreadCallback_var;

  /**
   * Creates several threads and executes specified job(s) in them.
   */
  class ThreadRunner
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(AlreadyStarted, Exception);
    DECLARE_EXCEPTION(PosixException, Exception);

    /**
     * Options for threads.
     */
    struct Options
    {
      /**
       * Constructor
       * @param stack_size stack size for the thread.
       * @param thread_callback thread tuner callback
       */
      explicit
      Options(size_t stack_size = 0, ThreadCallback* thread_callback = 0)
        throw ();

      // Default stack size for threads
      static const size_t DEFAULT_STACK_SIZE = 1024 * 1024;

      size_t stack_size;
      ThreadCallback_var thread_callback;
    };


    /**
     * Constructor
     * @param job to be executed in all threads.
     * @param number_of_jobs number of jobs to run concurrently.
     * @param options threads options
     */
    ThreadRunner(ThreadJob* job, unsigned number_of_jobs,
      const Options& options = Options())
      throw (eh::Exception, PosixException);

    /**
     * Constructor
     * @param functor functor producing jobs
     * @param number_of_jobs number of jobs to produce and run concurrently
     * @param options threads options
     */
    template <typename Functor>
    ThreadRunner(unsigned number_of_jobs, Functor functor,
      const Options& options = Options())
      throw (eh::Exception, PosixException);

    /**
     * Constructor
     * @param begin beginning of the container with jobs
     * @param end end of the container with jobs
     * @param options threads options
     */
    template <typename ForwardIterator>
    ThreadRunner(ForwardIterator begin, ForwardIterator end,
      const Options& options = Options())
      throw (eh::Exception, PosixException);

    /**
     * Destructor
     * Waits for threads' completion if they are not terminated yet.
     */
    ~ThreadRunner() throw ();

    /**
     * Number of jobs to execute
     * @return number of jobs
     */
    unsigned
    number_of_jobs() const throw ();

    /**
     * Return number of jobs running. Thread unsafe.
     * @return jobs running number
     */
    unsigned
    running() const throw ();

    /**
     * Creates threads and runs the jobs. If creation of a thread fails,
     * no jobs will run. Thread unsafe.
     * @param to_start number of thread to start (0 - all of them)
     */
    void
    start(unsigned to_start = 0)
      throw (AlreadyStarted, PosixException, eh::Exception);

    /**
     * Creates an additional thread if any is left. Thread unsafe.
     */
    void
    start_one() throw (AlreadyStarted, PosixException);

    /**
     * Waits for termination of previously started threads.
     * Thread unsafe.
     */
    void
    wait_for_completion() throw (PosixException);

  private:
    static
    void*
    thread_func_(void* arg) throw ();

    void
    thread_func_(ThreadJob& job) throw ();

    void
    start_one_thread_() throw (PosixException);

    class PThreadAttr
    {
    public:
      explicit
      PThreadAttr(size_t stack_size) throw (PosixException);
      ~PThreadAttr() throw ();
      operator pthread_attr_t*() throw ();

    private:
      pthread_attr_t attr_;
    };

    struct JobInfo
    {
      ThreadRunner* runner;
      ThreadJob_var job;
      pthread_t thread_id;
    };

    PThreadAttr attr_;
    ThreadCallback_var thread_callback_;
    Sync::Semaphore start_semaphore_;
    volatile sig_atomic_t number_running_;
    unsigned number_of_jobs_;
    ArrayAutoPtr<JobInfo> jobs_;
  };
}

namespace Generics
{
  //
  // ThreadRunner class
  //

  inline
  unsigned
  ThreadRunner::number_of_jobs() const throw ()
  {
    return number_of_jobs_;
  }

  inline
  unsigned
  ThreadRunner::running() const throw ()
  {
    return number_running_;
  }

  template <typename Functor>
  ThreadRunner::ThreadRunner(unsigned number_of_jobs, Functor functor,
    const Options& options) throw (eh::Exception, PosixException)
    : attr_(options.stack_size), thread_callback_(options.thread_callback),
      start_semaphore_(0), number_running_(0),
      number_of_jobs_(number_of_jobs), jobs_(number_of_jobs_)
  {
    for (unsigned i = 0; i < number_of_jobs_; i++)
    {
      jobs_[i].runner = this;
      jobs_[i].job = functor();
    }
  }

  template <typename ForwardIterator>
  ThreadRunner::ThreadRunner(ForwardIterator begin, ForwardIterator end,
    const Options& options) throw (eh::Exception, PosixException)
    : attr_(options.stack_size), thread_callback_(options.thread_callback),
      start_semaphore_(0), number_running_(0),
      number_of_jobs_(std::distance(begin, end)), jobs_(number_of_jobs_)
  {
    for (unsigned i = 0; i < number_of_jobs_; i++)
    {
      jobs_[i].runner = this;
      jobs_[i].job = ReferenceCounting::add_ref(*begin++);
    }
  }
}

#endif
