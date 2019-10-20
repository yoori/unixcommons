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





#include <Generics/Scheduler.hpp>
#include <Generics/Function.hpp>

//#define BUILD_WITH_DEBUG_MESSAGES
#include "Trace.hpp"


namespace Generics
{
  //
  // Planner::PlannerJob class
  //

  Planner::PlannerJob::PlannerJob(ActiveObjectCallback* callback,
    bool delivery_time_adjustment)
    throw (eh::Exception)
    : SingleJob(callback),
      have_new_events_(false),
      delivery_time_adjustment_(delivery_time_adjustment)
  {
  }

  Planner::PlannerJob::~PlannerJob() throw ()
  {
  }

  void
  Planner::PlannerJob::terminate() throw ()
  {
    have_new_events_ = true;
    new_event_in_schedule_.signal(); // wake the working thread
  }

  void
  Planner::PlannerJob::schedule(Goal* goal, const Time& time)
    throw (InvalidArgument, Exception, eh::Exception)
  {
    if (!goal)
    {
      Stream::Error ostr;
      ostr << FNS << "goal is null";
      throw InvalidArgument(ostr);
    }

    Time tm(time > Time::ZERO ? time : Time::ZERO);

#ifdef BUILD_WITH_DEBUG_MESSAGES
    {
      Stream::Error ostr;
      ostr << "entering " << tm << " ";
      if (tm == Time::ZERO)
      {
        ostr << "now";
      }
      else
      {
        ostr << tm - Time::get_time_of_day();
      }
      trace_message(FNB, ostr.c_str());
    }
#endif

    bool signal;
    {
      /** sch 1: add message into list */
      Sync::PosixGuard guard(mutex());

      TimedList::iterator itor(messages_.end());

      while (itor != messages_.begin())
      {
        --itor;
        if (itor->time() < tm)
        {
          ++itor;
          break;
        }
      }

      signal = (itor == messages_.begin());

      TimedMessage m(tm, goal);
      messages_.insert(itor, m);
      if (signal)
      {
        have_new_events_ = true;
      }
    }
    if (signal)
    {
      /** sch 2: new events into schedule signal */
      trace_message(FNB, "signaling");
      new_event_in_schedule_.signal();
      trace_message(FNB, "signaled");
    }
    trace_message(FNB, "leaving");
  }

  unsigned
  Planner::PlannerJob::unschedule(const Goal* goal)
    throw (eh::Exception)
  {
    unsigned removed = 0;

    {
      Sync::PosixGuard guard(mutex());

      for (TimedList::iterator itor(messages_.begin());
        itor != messages_.end();)
      {
        if (itor->is_goal(goal))
        {
          itor = messages_.erase(itor);
          removed++;
        }
        else
        {
          ++itor;
        }
      }
    }

    return removed;
  }

  void
  Planner::PlannerJob::work() throw ()
  {
    trace_message(FNB, "entering");

    try
    {
      TimedList pending;
      Time abs_time;
      Time cur_time;

      for (;;)
      {
        Time* pabs_time = 0;

        {
          /** svc 1: make list of pending tasks */
          trace_message(FNB, "acquiring lock");
          Sync::PosixGuard guard(mutex());
          trace_message(FNB, "lock acquired");

          if (is_terminating())
          {
            trace_message(FNB, "signaled");
            break;
          }
          cur_time = Time::get_time_of_day();

          while (!messages_.empty())  // pump messages to pending.
          {
            abs_time = messages_.front().time();

            if (delivery_time_adjustment_)
            {
              abs_time = abs_time > delivery_time_shift_ ?
                abs_time - delivery_time_shift_ :
                Time::ZERO;
            }

            // pump all overdue event to pending list.
            //  They will call immediately
            if (abs_time <= cur_time)
            {
              pending.splice(pending.end(), std::move(messages_),
                messages_.begin());
            }
            else
            {
              pabs_time = &abs_time;  // first event in the future
              break;
            }
          }
        } // end data lock

        if (pending.empty())
        {
          /** svc 2: wait semaphore signal */
#ifdef BUILD_WITH_DEBUG_MESSAGES
          {
            Stream::Error ostr;
            ostr << FNS << ": waiting ";

            if (pabs_time == 0)
            {
              ostr << "INFINITE";
            }
            else
            {
              Time tm = *pabs_time - cur_time;
              ostr << tm;
            }

            trace_message(FNB, ostr.c_str());
          }
#endif

          bool new_event_in_schedule = true;
          {
            Sync::ConditionalGuard cond_guard(new_event_in_schedule_,
              mutex());
            while (!have_new_events_)
            {
              try
              {
                new_event_in_schedule = cond_guard.timed_wait(pabs_time);
              }
              catch (const eh::Exception& e)
              {
                callback()->critical(String::SubString(e.what()));
                new_event_in_schedule = false;
              }
              if (!have_new_events_)
              {
                new_event_in_schedule = false;
                break;
              }
            } // while
            if (is_terminating())
            {
              break;
            }
            if (new_event_in_schedule)
            {
              have_new_events_ = false;
            }
          } // Unlock ConditionalGuard condition

          if (new_event_in_schedule)
          {
            if (delivery_time_adjustment_ && pabs_time)
            {
              Time wake_tm(Time::get_time_of_day());
              if (wake_tm > *pabs_time) // if OVERDUE event
              {
                trace_message(FNB, "wake up");
                Time shift = wake_tm - *pabs_time; // OVERDUE event
                delivery_time_shift_ = shift / 2;
              }
            }
            continue;
          }
        } // if (pending.empty())

        /** svc 3: deliver pending tasks */
        while (!pending.empty())
        {
          trace_message(FNB, "deliver message");
          try
          {
            pending.front().deliver();
          }
          catch (const eh::Exception& ex)
          {
            callback()->error(String::SubString(ex.what()));
          }
          trace_message(FNB, "message delivered");
          pending.pop_front();
        }
      }
    }
    catch (const eh::Exception& e)
    {
      Stream::Error ostr;
      ostr << FNS << "eh::Exception caught: " << e.what();

      callback()->critical(ostr.str());
    }

    trace_message(FNB, "leaving");
  }

  void
  Planner::PlannerJob::clear() throw ()
  {
    Sync::PosixGuard guard(mutex());
    messages_.clear();
  }


  //
  // Planner class
  //

  Planner::Planner(ActiveObjectCallback* callback, size_t stack_size,
    bool delivery_time_adjustment) throw (InvalidArgument, eh::Exception)
    : ActiveObjectCommonImpl(
        PlannerJob_var(
          new PlannerJob(callback, delivery_time_adjustment)),
        1, stack_size),
      job_(static_cast<PlannerJob&>(*SINGLE_JOB_))
  {
  }

  Planner::~Planner() throw ()
  {
  }
}
