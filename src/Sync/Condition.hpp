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



// Condition.hpp
#ifndef SYNC_CONDITION_HPP
#define SYNC_CONDITION_HPP

#include <Sync/PosixLock.hpp>

#include <Generics/Time.hpp>


namespace Sync
{
  /**
   * @class Conditional
   *
   * @brief Conditional variable wrapper, which allows threads
   * to block until shared data changes state.
   *
   * A condition variable enables threads to atomically block and
   * test the condition under the protection of a mutual exclusion
   * lock (mutex) until the condition is satisfied.  That is,
   * the mutex must have been held by the thread before calling
   * wait or signal on the condition.  If the condition is false,
   * a thread blocks on a condition variable and atomically
   * releases the mutex that is waiting for the condition to
   * change.  If another thread changes the condition, it may wake
   * up waiting threads by signaling the associated condition
   * variable.  The waiting threads, upon awakening, reacquire the
   * mutex and re-evaluate the condition.
   */
  class Conditional : private Generics::Uncopyable
  {
  public:
    // Can be raised if system API errors occurred
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    /**
     * Constructor
     */
    constexpr
    Conditional() throw ();

    /**
     * Destructor
     */
    ~Conditional() throw ();

    // Lock accessors.

    /**
     * Block on condition.
     * @param mutex
     * Wait functions shall block on a condition variable. It
     * shall be called with mutex locked by the calling thread
     * or undefined behavior results.
     */
    void
    wait(pthread_mutex_t& mutex) throw (Exception, eh::Exception);

    /**
     * Block on condition, or until absolute time-of-day has passed.
     * Wait functions shall block on a condition variable.
     * @param mutex Method shall be called with mutex locked by the
     * calling thread or undefined behavior results.
     * @param time pointer to absolute time or time interval in
     * dependency of third parameter. If pointer = 0 use blocking wait()
     * semantics.
     * This is useful if we choose time interval and sometime need
     * infinity waiting.
     * @param time_is_relative = true time parameter should be time interval.
     * Implementation add this time interval to current system time.
     * @return false if timeout.
     */
    bool
    timed_wait(pthread_mutex_t& mutex,
      const Generics::Time* time,
      bool time_is_relative = false)
      throw (Exception, eh::Exception);

    /**
     * Signal one waiting thread. This method shall unblock at least one
     * of the threads that are blocked on Conditional
     * (if any threads are blocked on this).
     */
    void
    signal() throw (Exception, eh::Exception);

    /**
     * Signal *all* waiting threads. This method shall unblock all threads
     * currently blocked on Conditional
     */
    void
    broadcast() throw (Exception, eh::Exception);

  private:
    pthread_cond_t cond_;
  };

  /**
   * @class Condition
   *
   * @brief Condition is Conditional with mutex
   */
  class Condition :
    public virtual Conditional,
    public PosixMutex
  {
  public:
    // Lock accessors.

    /**
     * Block on condition.
     * Wait functions shall block on a condition variable. It
     * shall be called when internal_mutex locked by the calling thread
     * or undefined behavior results.
     */
    void
    wait() throw (Exception, eh::Exception);

    /**
     * Block on condition, or until absolute time-of-day has passed.
     * Wait functions shall block on a condition variable. It
     * shall be called with mutex locked by the calling thread
     * or undefined behavior results.
     * @param time pointer to absolute time or time interval in dependency
     * of second parameter. If pointer = 0 use blocking wait() semantics.
     * This is useful if we choose time interval and sometime need
     * infinity waiting.
     * @param time_is_relative = true time parameter should be time interval.
     * Implementation add this time interval to current system time.
     * @return bool: false if timeout.
     */
    bool
    timed_wait(const Generics::Time* time,
      bool time_is_relative = false)
      throw (Exception, eh::Exception);
  };

  /**
   * @class ConditionalGuard
   *
   * @brief ConditionalGuard is useful guard that locks associated with
   * Conditional mutex in constructor and unlock in destructor
   * And it will delegate calls to used Conditional while created.
   */

  class ConditionalGuard : private PosixGuard
  {
  public:
    /**
     * Constructor use Condition.
     * @param condition Lock internal_mutex of condition.
     * Methods calls will be delegate to condition.
     */
    explicit
    ConditionalGuard(Condition& condition)
      throw ();

    /**
     * Constructor with Conditional and mutex.
     * @param conditional methods calls will be delegate to this object
     * @param mutex lock mutex for conditional using
     */
    ConditionalGuard(Conditional& conditional, pthread_mutex_t& mutex)
      throw ();

    /**
     * Destructor unlocks mutex that locked by constructor.
     */

    /**
     * Block on condition. Delegate call to conditional.
     */
    void
    wait() throw (Conditional::Exception, eh::Exception);

    /**
     * Block on condition or until absolute time-of-day has passed.
     * Delegate call to conditional.
     * @param time pointer to absolute time or time interval in dependency
     * of second parameter. If pointer = 0 use blocking wait() semantics.
     * This is useful if we choose time interval and sometime need
     * infinity waiting.
     * @param time_is_relative if = true time parameter should be time
     * interval.
     * Implementation add this time interval to current system time.
     */
    bool
    timed_wait(const Generics::Time* time,
      bool time_is_relative = false)
      throw (Conditional::Exception, eh::Exception);

  private:
    Conditional& conditional_;
    pthread_mutex_t& mutex_;
  };
}

//
// INLINES
//

namespace Sync
{
  //
  // class Conditional
  //

  inline
  constexpr
  Conditional::Conditional() throw ()
    : cond_ PTHREAD_COND_INITIALIZER
  {
  }
}

#endif
