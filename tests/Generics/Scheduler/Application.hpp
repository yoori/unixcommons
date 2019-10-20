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





#ifndef SCHEDULER_TEST_INCLUDED
#define SCHEDULER_TEST_INCLUDED

#include <vector>
#include <set>

#include <eh/Exception.hpp>
#include <Generics/Scheduler.hpp>
#include <Generics/Statistics.hpp>

namespace Generics
{
  class Application
  {
  public:

    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(InvalidOperationOrder, Exception);

  public:

    Application() throw (eh::Exception);

    void init(int& argc, char** argv)
      throw (Exception, eh::Exception);

    typedef Generics::Time (Application::*TimeGenerator)() const;
    typedef void (Application::*Scenarist)(TimeGenerator);

    void run(Scenarist make_schedule, TimeGenerator tg)
      throw (InvalidOperationOrder, Exception, eh::Exception);

    void stop() throw (Exception, eh::Exception);

    Generics::Time
    rand_time() const throw (eh::Exception);

    Generics::Time
    full_random_time() const throw (eh::Exception);

    Generics::Time
    compact_time_series() const throw (eh::Exception);

    void
    set_test_execution_time(int nt) throw ();

    /**
     * Scheduling strategies
     */
    void
    ScheduleMaker(TimeGenerator tg)
      throw (Planner::Exception, eh::Exception);

    void
    SchedulePortionMaker(TimeGenerator tg)
      throw (Planner::Exception, eh::Exception);

    void
    ScheduleMakerUCS97(TimeGenerator tg)
      throw (Planner::Exception, eh::Exception);

    void
    set_message_count(unsigned long new_value) throw ();

  private:

    void consider_deviation(const Generics::Time& tm1,
                            const Generics::Time& tm2,
                            unsigned long* deviation)
      throw (eh::Exception);

    void print_results() throw (eh::Exception);

  private:

    class Message : public Goal,
      public ReferenceCounting::AtomicImpl
    {
    public:
      Message(Application* app);
    protected:
      Application* app_;
    };

    class StopMessage : public Message
    {
    public:
      StopMessage(Application* app);

      virtual void
      deliver() throw (eh::Exception);
    };
    typedef ReferenceCounting::QualPtr<StopMessage> StopMessage_var;

    class TimedMessage : public Message
    {
    public:
      TimedMessage(Application* app, const Generics::Time& tm)
        throw (eh::Exception);

      Generics::Time
      time() const throw (eh::Exception);

      void
      time(const Generics::Time& tm)
        throw (eh::Exception);

      Generics::Time
      scheduling_time() const throw (eh::Exception);

      TimedMessage*
      scheduling_time(const Generics::Time& tm)
        throw (eh::Exception);

      virtual void
      deliver() throw (eh::Exception);

    private:
      Generics::Time time_;
      // At this time message was scheduled into Scheduler
      Generics::Time push_time_;
    };
    typedef ReferenceCounting::QualPtr<TimedMessage> TimedMessage_var;

    void
    deliver_message(TimedMessage* timed_message)
      throw ();

  private:

    typedef Sync::PosixRWLock Mutex_;
    typedef Sync::PosixRGuard Read_Guard_;
    typedef Sync::PosixWGuard Write_Guard_;

    bool
    is_test_successfull_(std::string& error_description) const
      throw ();

    mutable Mutex_ lock_;

    Planner_var scheduler_;

    unsigned long  deviation_grid_; // usec
    unsigned long  deviation_max_;  // usec
    unsigned long  message_count_;
    unsigned long  max_sceduling_time_; // sec
    unsigned long  min_sceduling_time_; // sec
    Generics::Time execution_time_;

    Generics::Time start_time_;
    Generics::Time stop_time_;
    unsigned long  deviation_stat_size_;
    std::vector<unsigned long> negative_deviation_;
    std::vector<unsigned long> positive_deviation_;

    Time last_deliver;

//    Statistics::Timed_var preschedule_stat_;
//    Statistics::Timed_var schedule_stat_;
    Generics::ActiveObjectCallback_var callback_;
    Statistics::Collection_var statistics_;
    typedef Sync::PosixMutex PMutex_;
    typedef Sync::PosixGuard PGuard_;
    PMutex_        schedule_events_lock_;

    typedef std::multiset<Generics::Time> Schedule;
    Schedule        scheduled_events_;
    Generics::Time  max_gap_;
    Generics::Time  max_gap_planed_moment_;
    Generics::Time  max_gap_schedule_moment_;
    Generics::Time  max_gap_moment_;
    Generics::Time  stop_message_time_;
    std::size_t     processed_events_;
  };
}

/////////////////////////////////////////////////////////////////////////////
// Inlines
/////////////////////////////////////////////////////////////////////////////

namespace Generics
{

  //
  // Application::Message class
  //

  inline
  Application::Message::Message(Application* app)
    : app_(app)
  {
  }

  //
  // Application::StopMessage class
  //

  inline
  Application::StopMessage::StopMessage(Application* app)
    : Application::Message(app)
  {
  }

  //
  // Application::TimedMessage class
  //

  inline
  Application::TimedMessage::TimedMessage(Application* app,
    const Generics::Time& time)
    throw (eh::Exception)
    : Application::Message(app),
      time_(time)
  {
  }

  inline
  Generics::Time
  Application::TimedMessage::scheduling_time() const
    throw (eh::Exception)
  {
    return push_time_;
  }

  inline
  Application::TimedMessage*
  Application::TimedMessage::scheduling_time(const Generics::Time& time)
    throw (eh::Exception)
  {
    push_time_ = time;
    return this;
  }

  inline
  Generics::Time
  Application::TimedMessage::time() const
    throw (eh::Exception)
  {
    return time_;
  }

  inline
  void
  Application::TimedMessage::time(const Generics::Time& time)
    throw (eh::Exception)
  {
    time_ = time;
  }

}

#endif // SCHEDULER_TEST_INCLUDED
