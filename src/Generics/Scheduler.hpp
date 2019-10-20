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





#ifndef GENERICS_SCHEDULER_HPP
#define GENERICS_SCHEDULER_HPP

#include <ReferenceCounting/List.hpp>

#include <Generics/ActiveObject.hpp>


namespace Generics
{
  class Goal : public virtual ReferenceCounting::Interface
  {
  public:
    /**
     * Callback function to be called from the scheduler
     */
    virtual
    void
    deliver() throw (eh::Exception) = 0;
  };
  typedef ReferenceCounting::QualPtr<Goal> Goal_var;

  class Planner : public ActiveObjectCommonImpl
  {
  public:
    DECLARE_EXCEPTION(Exception, ActiveObject::Exception);

    /**
     * Constructor
     * @param callback Reference countable callback object to be called
     * for errors
     * @param stack_size stack size for working thread
     * @param delivery_time_adjustment Should delivery_time_shift_ be used
     * for messages' time shift
     */
    Planner(ActiveObjectCallback* callback,
      size_t stack_size = 0, bool delivery_time_adjustment = false)
      throw (InvalidArgument, eh::Exception);

    /**
     * Adds goal to the queue. Goal's reference counter is incremented.
     * On error it is unchanged (and object will be freed in the caller).
     * @param goal Object to enqueue
     * @param time Timestamp to match
     */
    void
    schedule(Goal* goal, const Time& time)
      throw (InvalidArgument, Exception, eh::Exception);

    /**
     * Tries to remove goal from the queue.
     * @param goal Object to remove
     * @return number of entries removed
     */
    unsigned
    unschedule(const Goal* goal)
      throw (eh::Exception);

    /**
     * Clearance of messages' queue
     */
    virtual
    void
    clear() throw (eh::Exception);

  protected:
    /**
     * Destructor
     * Decreases all unmatched messages' reference counters
     */
    virtual
    ~Planner() throw ();

  private:
    class PlannerJob : public SingleJob
    {
    public:
      PlannerJob(ActiveObjectCallback* callback,
        bool delivery_time_adjustment) throw (eh::Exception);

      virtual
      void
      work() throw ();

      virtual
      void
      terminate() throw ();

      void
      schedule(Goal* goal, const Time& time)
        throw (InvalidArgument, Exception, eh::Exception);

      unsigned
      unschedule(const Goal* goal)
        throw (eh::Exception);

      void
      clear() throw ();

    protected:
      virtual
      ~PlannerJob() throw ();

      /**
       * Element of messages' queue. Composition of Message and
       * associated Time.
       */
      class TimedMessage
      {
      public:
        TimedMessage() throw ();

        TimedMessage(TimedMessage&) = default;

        /**
         * Constructor
         * @param time Associated time
         * @param goal Shared ownership on goal
         */
        TimedMessage(const Time& time, Goal* goal) throw ();

        /**
         * Holding time
         * @return Associated time
         */
        const Time&
        time() const throw ();

        /**
         * Calls deliver() on owned goal
         */
        void
        deliver() throw (eh::Exception);

        /**
         * Checks if it holds the goal
         * @param goal goal to check against
         * @return true if they coincide
         */
        bool
        is_goal(const Goal* goal) const throw ();

      private:
        Time time_;
        Goal_var goal_;
      };
      typedef ReferenceCounting::List<TimedMessage> TimedList;

      mutable Sync::Conditional new_event_in_schedule_;
      bool have_new_events_;  // Predicate for condition!

      TimedList messages_;
      bool delivery_time_adjustment_;
      Time delivery_time_shift_;
    };
    typedef ReferenceCounting::FixedPtr<PlannerJob> PlannerJob_var;

    PlannerJob& job_;
  };
  typedef ReferenceCounting::QualPtr<Planner> Planner_var;
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace Generics
{
  //
  // Planner::TimedMessage class
  //

  inline
  Planner::PlannerJob::TimedMessage::TimedMessage() throw ()
  {
  }

  inline
  Planner::PlannerJob::TimedMessage::TimedMessage(
    const Time& time, Goal* goal) throw ()
    : time_(time), goal_(ReferenceCounting::add_ref(goal))
  {
  }

  inline
  const Time&
  Planner::PlannerJob::TimedMessage::time() const throw ()
  {
    return time_;
  }

  inline
  void
  Planner::PlannerJob::TimedMessage::deliver() throw (eh::Exception)
  {
    goal_->deliver();
  }

  inline
  bool
  Planner::PlannerJob::TimedMessage::is_goal(const Goal* goal) const
    throw ()
  {
    return goal == goal_;
  }


  //
  // Planner class
  //

  inline
  void
  Planner::schedule(Goal* goal, const Time& time)
    throw (InvalidArgument, Exception, eh::Exception)
  {
    job_.schedule(goal, time);
  }

  inline
  unsigned
  Planner::unschedule(const Goal* goal)
    throw (eh::Exception)
  {
    return job_.unschedule(goal);
  }

  inline
  void
  Planner::clear() throw (eh::Exception)
  {
    job_.clear();
  }
}

#endif
