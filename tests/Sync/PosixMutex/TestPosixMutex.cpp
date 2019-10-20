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



// @file PosixMutex/TestPosixMutex.cpp
#include <iostream>
#include <memory>

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <Generics/MMap.hpp>
#include <Sync/SyncPolicy.hpp>

//#define DEBUG

/**
 * Class encapsulate test data and code
 */
template <typename TestingPolicy>
class MutexTester
{
public:

  /**
   * Test the mutex and the lock/unlock ability
   * Two concurrency threads do select operation between two numbers.
   * SUCCESS if isn't race condition in results.
   */
  void
  do_lock_test() throw ();

private:
  typedef typename TestingPolicy::Mutex TestingMutex;
  typedef std::unique_ptr<TestingMutex> SmartMutex;
  /**
   * Context object transmits data to the thread
   */
  struct ThreadContext
  {
    /**
     * Initialization is comfortably through constructor
     */
    ThreadContext(MutexTester *this_ptr_val,
      SmartMutex& mutex_selector,
      SmartMutex& mutex_relese) throw();
    /// Access to sharable data from enclosing class MutexTester
    MutexTester* this_ptr;
    /// ID of the created thread
    pthread_t thread;
    /// Mutex to be locked on start
    SmartMutex& mutex;
    /// Mutex to be unlocked on finish
    SmartMutex& mutex_to_release;
    /// Value to be saved into shared variable as thread work result
    std::size_t result_value;
  };

  /**
   * Cycle with pairs of random numbers
   */
  void
  do_lock_test_() throw ();

  /**
   * Select numbers by ordering thread actions
   * If second = true function must return beta value,
   * If second = false function must return alpha value.
   */
  std::size_t
  select_(std::size_t alpha, std::size_t beta, bool second) throw ();

  /**
   * Working code of concurrency threads
   */
  void
  rival_(const ThreadContext& context) throw ();

  class Active
  {
  public:
    /**
     * Save context in object
     */
    Active(ThreadContext& context) throw ();

    virtual void
    activate() throw () = 0;

    virtual void
    join() throw () = 0;

    /**
     * Save number that Actor will push to shared memory
     */
    void
    set_number(std::size_t number) throw ();

    /**
     * virtual empty destructor
     */
    virtual
    ~Active() throw ();
  protected:
    ThreadContext& context_;
  };
  typedef std::unique_ptr<Active> SmartActive;

  /**
   * Do thread C++ object to do auto cleanup
   */
  class Thread : public Active
  {
  public:
    /**
     * Save context in object
     */
    Thread(ThreadContext& context) throw ();

    /**
     * Start thread with stored context
     */
    virtual void
    activate() throw ();

    /**
     * do join to created thread
     */
    virtual void
    join() throw ();

    /**
     * do join to created thread
     */
    virtual
    ~Thread() throw ();
  protected:
    bool active_state_;
  };

  struct Process : public Active
  {
    /**
     * Save context in object
     */
    Process(ThreadContext& context) throw ();

    /**
     * Start process with stored context
     */
    virtual void
    activate() throw ();

    /**
     * do join to created process
     */
    virtual void
    join() throw ();

  private:
    pid_t cpid_;
  };


  /**
   * Mediator to C++ from C-functions
   */
  static void*
  rival_(void* arg) throw ();

  SmartMutex mutex1_;
  SmartMutex mutex2_;
  SmartActive actor1_;
  SmartActive actor2_;

  /// Here concurrent thread put results of its work
  /// For concurrent processes variable may be in shared memory
  std::unique_ptr<std::size_t> shared_value_;

};

int
main()
{
  try
  {
    std::cout << "Mutex test started.." << std::endl;
    {
      MutexTester<Sync::Policy::PosixThread> tester;
      tester.do_lock_test();
    }
    std::cout << "Spinlock test started.." << std::endl;
    {
      MutexTester<Sync::Policy::PosixSpinThread> tester;
      tester.do_lock_test();
    }
    std::cout << "SUCCESS" << std::endl;
  }
  catch (...)
  {
    std::cerr << "\nFAIL: unknown exception" << std::endl;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////
// Implementations

//
// MutexTester<TestingPolicy> class
//

// Test body below
template <typename TestingPolicy>
void
MutexTester<TestingPolicy>::do_lock_test() throw ()
{
  shared_value_.reset(new std::size_t);
  //case 1
  mutex1_.reset(new TestingMutex);
  mutex2_.reset(new TestingMutex);
  ThreadContext thread_one(this, mutex1_, mutex2_);
  ThreadContext thread_two(this, mutex2_, mutex1_);
  actor1_.reset(new Thread(thread_one));
  actor2_.reset(new Thread(thread_two));
  do_lock_test_();
  //case 2
  mutex1_.reset(new TestingMutex(PTHREAD_PROCESS_SHARED));
  do_lock_test_();
  //case 3
  mutex2_.reset(new TestingMutex(PTHREAD_PROCESS_SHARED));
  do_lock_test_();
  //case 4
  mutex1_.reset(new TestingMutex);
  do_lock_test_();

  //case 5 - test for mutexes shared between processes
  actor1_.reset(new Process(thread_one));
  actor2_.reset(new Process(thread_two));
  // Share two mutex and variable between processes
  Generics::MMap shared_memory(static_cast<void*>(0),
    sizeof(TestingMutex[2]) + sizeof(std::size_t));
  TestingMutex* ptr =
    static_cast<TestingMutex*>(shared_memory.memory());
  mutex1_.reset(new (ptr) TestingMutex(PTHREAD_PROCESS_SHARED));
  mutex2_.reset(new (++ptr) TestingMutex(PTHREAD_PROCESS_SHARED));
  shared_value_.reset(new (++ptr) std::size_t);
  do_lock_test_();

  // cannot free shared memory through delete
  mutex1_.release();
  mutex2_.release();
  shared_value_.release();
}

template <typename TestingPolicy>
void
MutexTester<TestingPolicy>::do_lock_test_() throw ()
{
  for (std::size_t i = 0; i < 100; ++i)
  {
    std::size_t alpha = rand();
    std::size_t beta;
    do
    {
      beta = rand();
    }
    while (alpha == beta);
#ifdef DEBUG
    std::cout << "Alpha=" << alpha << ", Beta=" << beta
      << ", select_1=" << select_(alpha, beta, false)
      << ", select_2=" << select_(alpha, beta, true)
      << std::endl;
#endif
    if (select_(alpha, beta, false) != alpha ||
      select_(alpha, beta, true) != beta)
    {
      std::cerr << "Threads sync failed via mutex, "
        "race condition detected" << std::endl;
    }
  }
}

template <typename TestingPolicy>
std::size_t
MutexTester<TestingPolicy>::select_(std::size_t alpha,
  std::size_t beta, bool second)
  throw ()
{
  mutex1_->lock();
  mutex2_->lock();
  {

    actor1_->set_number(alpha);
    actor2_->set_number(beta);
    actor1_->activate();
    actor2_->activate();
    if (second)
    {
      // allow put value to first thread, and second thread overwrite
      // this result by own later (when first release mutex)
      mutex1_->unlock();
    }
    else
    {
      mutex2_->unlock();
    }
    actor1_->join();
    actor2_->join();
  }
  return *shared_value_;    
}

template <typename TestingPolicy>
void
MutexTester<TestingPolicy>::rival_(const ThreadContext& context) throw ()
{
  {
    typename TestingPolicy::WriteGuard lock(*context.mutex);
    *shared_value_ = context.result_value;
  }
  context.mutex_to_release->unlock();
}

template <typename TestingPolicy>
void*
MutexTester<TestingPolicy>::rival_(void* arg) throw ()
{
  try
  {
    ThreadContext &thread_context = *static_cast<ThreadContext*>(arg);
    thread_context.this_ptr->rival_(thread_context);
  }
  catch (...)
  {
  }
  return 0;
}

//
// MutexTester<TestingPolicy>::ThreadContext struct
//

template <typename TestingPolicy>
MutexTester<TestingPolicy>::ThreadContext::ThreadContext(
  MutexTester *this_ptr_val,
  SmartMutex& mutex_selector,
  SmartMutex& mutex_relese)
  throw ()
  : this_ptr(this_ptr_val),
    mutex(mutex_selector),
    mutex_to_release(mutex_relese),
    result_value(rand())
{
}

//
// MutexTester<TestingPolicy>::Active class
//
template <typename TestingPolicy>
MutexTester<TestingPolicy>::Active::Active(ThreadContext& context) throw ()
  : context_(context)
{
}

template <typename TestingPolicy>
void
MutexTester<TestingPolicy>::Active::set_number(std::size_t number) throw ()
{
  context_.result_value = number;
}

template <typename TestingPolicy>
MutexTester<TestingPolicy>::Active::~Active() throw ()
{
}

//
// MutexTester::Thread class
//

template <typename TestingPolicy>
MutexTester<TestingPolicy>::Thread::Thread(ThreadContext& context) throw ()
  : Active(context),
    active_state_(false)
{
}

template <typename TestingPolicy>
void
MutexTester<TestingPolicy>::Thread::activate() throw ()
{
  ThreadContext& ref = MutexTester<TestingPolicy>::Active::context_;
  if (pthread_create(&ref.thread,
    0, rival_, &ref))
  {
    std::cerr << "Create thread fail" << std::endl;
    exit(EXIT_FAILURE);
  }
  active_state_ = true;
}

template <typename TestingPolicy>
void
MutexTester<TestingPolicy>::Thread::join() throw ()
{
  pthread_join(MutexTester<TestingPolicy>::Active::context_.thread, 0);
  active_state_ = false;
}

template <typename TestingPolicy>
MutexTester<TestingPolicy>::Thread::~Thread() throw ()
{
  if (active_state_)
  {
    join();
  }
}

//
// MutexTester<TestingPolicy>::Process class
//

template <typename TestingPolicy>
MutexTester<TestingPolicy>::Process::Process(ThreadContext& context)
  throw ()
  : Active(context)
{
}

template <typename TestingPolicy>
void
MutexTester<TestingPolicy>::Process::activate() throw ()
{
  cpid_ = fork();
  if (cpid_ == -1)
  {
    std::cerr << "fork fail" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (cpid_ == 0)  // Child process
  {
    MutexTester<TestingPolicy>::Active::context_.this_ptr->rival_(
      MutexTester<TestingPolicy>::Active::context_);
    _exit(EXIT_SUCCESS);
  }
}

template <typename TestingPolicy>
void
MutexTester<TestingPolicy>::Process::join() throw ()
{
  // wait for child termination
  int status;
  pid_t wait_result;
  do 
  {
    wait_result = waitpid(cpid_, &status, WUNTRACED | WCONTINUED);
    if (wait_result == -1)
    {
      std::cerr << "Wait fail" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  while(!WIFEXITED(status) && !WIFSIGNALED(status));
}
