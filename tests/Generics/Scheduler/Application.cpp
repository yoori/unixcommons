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





#include <iostream>
#include <sstream>

#include <Generics/Scheduler.hpp>

#include <TestCommons/ActiveObjectCallback.hpp>

#include "Application.hpp"

//#define TRACE
namespace
{
  // This number of events using for PERFORMANCE testing.
  const std::size_t MAX_EVENTS_ON_TEST = 10000;
}

namespace Generics
{
  Application::Application()
    throw (eh::Exception)
      : deviation_grid_(40000),
        deviation_max_(50000000),
        message_count_(100),
        max_sceduling_time_(5),
        min_sceduling_time_(1),
        execution_time_(6),
        deviation_stat_size_(0),
        last_deliver(Generics::Time::ZERO),
        max_gap_(Generics::Time::ZERO),
        max_gap_planed_moment_(Generics::Time::ZERO),
        max_gap_schedule_moment_(Generics::Time::ZERO),
        max_gap_moment_(Generics::Time::ZERO),
        stop_message_time_(Generics::Time::ZERO),
        processed_events_(0)
  {
    srand(time(0));
    callback_ = new TestCommons::ActiveObjectCallbackStreamImpl(
      std::cerr, "Schedule");
//    preschedule_stat_ = new Statistics::Timed();
//    schedule_stat_ = new Statistics::Timed();
  }

  void
  Application::init(int& /*argc*/, char** /*argv*/)
    throw (Exception, eh::Exception)
  {
    Write_Guard_ guard(lock_);

    deviation_stat_size_ = deviation_max_ / deviation_grid_ + 1;

    negative_deviation_.assign(deviation_stat_size_, 0);
    positive_deviation_.assign(deviation_stat_size_, 0);
    max_gap_ = Generics::Time::ZERO;
    max_gap_planed_moment_ = Generics::Time::ZERO;
    max_gap_schedule_moment_ = Generics::Time::ZERO;
    max_gap_moment_ = Generics::Time::ZERO;

    try
    {
      scheduler_ = new Planner(callback_);
      statistics_ = new Statistics::Collection(callback_);

      Statistics::DumpPolicy_var dump_policy(
        new Statistics::CountBasedDumpPolicy(std::cout, 100000));

      statistics_->add("Preschedule",
                       new Statistics::TimedStatSink(),
                       dump_policy.in());

      statistics_->add("Schedule",
                       new Statistics::TimedStatSink(),
                       dump_policy.in());
    }
    catch(const Statistics::Collection::Exception& e)
    {
      Stream::Error ostr;
      ostr << "Application::init: Statistics::Collection::Exception caught. "
        "Description:" << std::endl << e.what();
      throw Exception(ostr);
    }
    catch(const eh::Exception& e)
    {
      Stream::Error ostr;
      ostr << "Application::init: eh::Exception caught. Description:" <<
        std::endl << e.what();

      throw Exception(ostr);
    }

    std::cout << "Messages scheduled: " << message_count_ << std::endl <<
      "Max scheduled time: " << Generics::Time(max_sceduling_time_) <<
      std::endl << "Min scheduled time: " <<
      Generics::Time(min_sceduling_time_) << std::endl;
  }

  /**
   * Schedule continuous creation strategy
   */
  void
  Application::ScheduleMaker(TimeGenerator tg)
    throw (Planner::Exception, eh::Exception)
  {
    start_time_ = Generics::Time::get_time_of_day();
    stop_message_time_ = start_time_ + execution_time_;
    StopMessage_var stop_message(new StopMessage(this));
    scheduler_->schedule(stop_message, stop_message_time_);

    // initial scheduling: this is rather long procedure,
    // but rand_time based on
    // current time and give only future times.
    // I.e. minimum monotonic increase.
    // We should control that rand_time (full_random_time)
    // doesn't exceed the threshold of completing the program.
    {
      PGuard_ guard(schedule_events_lock_);
      Generics::Time tm_event;
      for (unsigned long i = 0; i < message_count_; i++)
      {
        tm_event = (this->*tg)();
        if (tm_event > stop_message_time_)
        {
          continue;
        }
        TimedMessage_var message(new TimedMessage(this, tm_event));
        scheduled_events_.insert(tm_event);
        scheduler_->schedule(
          message->scheduling_time(Generics::Time::get_time_of_day()),
          tm_event);
        // will test 2 simultaneous events, insert one with same time.
        TimedMessage_var message2(new TimedMessage(this, tm_event));
        scheduled_events_.insert(tm_event);
        scheduler_->schedule(
          message2->scheduling_time(Generics::Time::get_time_of_day()),
          tm_event);
      }
    }
  }

  /**
   * Schedule creation strategy breaks with sleeps
   */
  inline
  void
  sleep_msc(unsigned int msec) throw ()
  {
    timespec ts;
    ts.tv_sec = msec / 1000;
    msec %= 1000;
    ts.tv_nsec = msec * 1000000; // to nanoseconds
    nanosleep(&ts, 0);
  }

  void
  Application::SchedulePortionMaker(TimeGenerator tg)
    throw (Planner::Exception, eh::Exception)
  {
    start_time_ = Generics::Time::get_time_of_day();
    stop_message_time_ = start_time_ + execution_time_;
    StopMessage_var stop_message(new StopMessage(this));
    scheduler_->schedule(stop_message, stop_message_time_);

    // initial scheduling: this is rather long procedure,
    // but rand_time based on
    // current time and give only future times.
    // I.e. minimum monotonic increase.
    // We should control that rand_time (full_random_time)
    // doesn't exceed the threshold of completing the program.
    {
      PGuard_ guard(schedule_events_lock_);
      Generics::Time tm_event;
      for (unsigned long i = 0; i < message_count_; i++)
      {
        tm_event = (this->*tg)();
        if (tm_event > stop_message_time_)
        {
          continue;
        }
        TimedMessage_var message(new TimedMessage(this, tm_event));
        scheduled_events_.insert(tm_event);
        scheduler_->schedule(
          message->scheduling_time(Generics::Time::get_time_of_day()),
          tm_event);
        // will test 2 simultaneous events, insert one with same time.
        TimedMessage_var message2(new TimedMessage(this, tm_event));
        scheduled_events_.insert(tm_event);
        scheduler_->schedule(
          message2->scheduling_time(Generics::Time::get_time_of_day()),
          tm_event);
        sleep_msc(2);
      }
    }
  }

  /**
   * Special simple schedule scenario that emulate UCS97 situation
   */
  void
  Application::ScheduleMakerUCS97(TimeGenerator)
    throw (Planner::Exception, eh::Exception)
  {
    start_time_ = Generics::Time::get_time_of_day();
    stop_message_time_ = start_time_ + execution_time_;
    StopMessage_var stop_message(new StopMessage(this));
    scheduler_->schedule(stop_message, stop_message_time_);

    // initial scheduling: this is rather long procedure,
    // but rand_time based on
    // current time and give only future times.
    // I.e. minimum monotonic increase.
    // We should control that rand_time (full_random_time)
    // doesn't exceed the threshold of completing the program.
    {
      PGuard_ guard(schedule_events_lock_);
      Generics::Time tm_event = start_time_ + 10;
      if (tm_event > stop_message_time_)
      {
        return;
      }
      TimedMessage_var message(new TimedMessage(this, tm_event));
      scheduled_events_.insert(tm_event);
      scheduler_->schedule(
        message->scheduling_time(Generics::Time::get_time_of_day()),
        tm_event);
      sleep(1);   // <-- Trouble UCS-97 reason
      tm_event = start_time_ + 2;
      if (tm_event > stop_message_time_)
      {
        return;
      }
      // will test 2 simultaneous events, insert one with same time.
      TimedMessage_var message2(new TimedMessage(this, tm_event));
      scheduled_events_.insert(tm_event);
      scheduler_->schedule(
        message2->scheduling_time(Generics::Time::get_time_of_day()),
        tm_event);
    }
  }

  void
  Application::run(Scenarist make_schedule, TimeGenerator tg)
    throw (InvalidOperationOrder, Exception, eh::Exception)
  {
    if (scheduler_.in() == 0)
    {
      throw InvalidOperationOrder("Application::run: call init() first");
    }

    std::cout << std::endl << "Running test ..." << std::endl;

    try
    {
      statistics_->activate_object();
      scheduler_->activate_object();

      try
      {
        (this->*make_schedule)(tg);
        std::cout << "All scheduled for "
          << Generics::Time::get_time_of_day() - start_time_ << std::endl;

        scheduler_->wait_object();
        statistics_->wait_object();
      }
      catch (...)
      {
        stop();
        scheduler_->wait_object();
        statistics_->wait_object();
        throw;
      }

      stop_time_ = Generics::Time::get_time_of_day();
    }
    catch(const Statistics::Collection::Exception& e)
    {
      Stream::Error ostr;
      ostr << "Application::run: Statistics::Collection::Exception caught. "
        "Description:" << std::endl << e.what();
      throw Exception(ostr);
    }
    catch(const eh::Exception& e)
    {
      Stream::Error ostr;
      ostr << "Application::run: eh::Exception caught. Description:" <<
        std::endl << e.what();
      throw Exception(ostr);
    }

    print_results();
  }

  void
  Application::StopMessage::deliver()
    throw (eh::Exception)
  {
    app_->stop();
  }

  void
  Application::TimedMessage::deliver()
    throw (eh::Exception)
  {
    app_->deliver_message(this);
  }

  void
  Application::deliver_message(Application::TimedMessage* timed_message)
    throw ()
  {
#ifdef TRACE
    {
      std::cerr << "Application::deliver_message: entering" << std::endl;
    }
#endif

    try
    {
      Generics::Timer timer;
      timer.start();

      // Real execution time of current event
      Generics::Time cur_tm = Generics::Time::get_time_of_day();

      // Planned time of execution
      Time msg_time(timed_message->time());
      // At this moment event was schedule into Schedule
      Time msg_schedule_moment = timed_message->scheduling_time();
      Time message_lag = cur_tm - msg_time;
      if (max_gap_ < message_lag)
      {
        max_gap_ = message_lag;
        max_gap_planed_moment_ = msg_time;
        max_gap_schedule_moment_ = msg_schedule_moment;
        max_gap_moment_ = cur_tm;
      }
      {
        PGuard_ guard(schedule_events_lock_);
        Schedule::iterator it = scheduled_events_.lower_bound(msg_time);
        if (it != scheduled_events_.begin())
        {
          std::cerr << "Not first event!" << std::endl;
          std::size_t i = 0;
          for (Schedule::iterator cit = scheduled_events_.begin();
            cit != it;
            ++cit, ++i)
          {
            std::cerr << i << "=" << *cit << std::endl;
          }
          std::cerr << "it=" << *it << "\tmessage_time="
                    << msg_time << std::endl;

        }

        if (*it == msg_time)
        {
          scheduled_events_.erase(it);
          ++processed_events_;
        }
        else
        {
          std::cerr << "Improperly scheduled events occurred"
                    << std::endl;
          return;
        }
      }

      // At now, should not happened past events
      // Overdue events are equivalent to the current time events
      // order of their processing is not defined
      // There is no priority between overdue and upcoming event.
      // The order of implementation of overdue events is not defined.
      // Arbitrariness.
      // Therefore, the order is guaranteed only
      // for the future sequence of events.
      if (msg_schedule_moment <= msg_time)
      {
        if (msg_time < last_deliver)
        {
          // nearest time event happened after long-term event.
          std::cerr << "Invalid sequence of delivered messages: msg_time="
                    << msg_time.get_local_time() << " and last_deliver="
                    << last_deliver.get_local_time() << std::endl;
        }
        last_deliver = msg_time;
      }

      if (msg_time > cur_tm)
      {
        consider_deviation(msg_time, cur_tm, &negative_deviation_[0]);
      }
      else if (msg_time < cur_tm)
      {
        consider_deviation(cur_tm, msg_time, &positive_deviation_[0]);
      }

      timed_message->time(rand_time());

      {
        timer.stop();

        Generics::Statistics::StatSink_var stat(
          statistics_->get("Preschedule"));

        stat->consider(Statistics::TimedSubject(timer.elapsed_time()));
      }

      timer.start();
      // re-planning event, if we have time.
      if (timed_message->time()+1 < stop_message_time_)
      {
        scheduled_events_.insert(timed_message->time());
        scheduler_->schedule(timed_message, timed_message->time());
      }

      {
        timer.stop();

        Generics::Statistics::StatSink_var stat(
          statistics_->get("Schedule"));

        stat->consider(Statistics::TimedSubject(timer.elapsed_time()));
      }
    }
    catch(const eh::Exception& e)
    {
      try
      {
        stop();

        std::cerr << "Application::deliver_message: "
          "eh::Exception exception caught. "
          "Description:" << std::endl << e.what() << std::endl;
      }
      catch(...)
      {
      }
    }

#ifdef TRACE
    {
      std::cerr << "Application::deliver_message: leaving" << std::endl;
    }
#endif

  }

  void
  Application::consider_deviation(const Generics::Time& tm1,
                                  const Generics::Time& tm2,
                                  unsigned long* deviation)
    throw (eh::Exception)
  {
    Generics::Time tm = tm1 - tm2;

    unsigned long long usec = (unsigned long long)1000000 * tm.tv_sec +
      tm.tv_usec;

    Write_Guard_ guard(lock_);

    unsigned long slot = usec / deviation_grid_;

    deviation[slot >= deviation_stat_size_ ?
              (deviation_stat_size_ - 1) : slot]++;
  }

  void
  Application::stop() throw (Exception, eh::Exception)
  {
#ifdef TRACE
    {
      std::cerr << "Application::stop: entering" << std::endl;
    }
#endif

    try
    {
      scheduler_->deactivate_object();
      statistics_->deactivate_object();
    }
    catch(const Statistics::Collection::Exception& e)
    {
      Stream::Error ostr;
      ostr << "Application::stop: Statistics::Collection::Exception caught. "
        "Description:" << std::endl << e.what();
      throw Exception(ostr);
    }
    catch(const eh::Exception& e)
    {
      Stream::Error ostr;
      ostr << "Application::stop: eh::Exception caught. Description:" <<
        std::endl << e.what();
      throw Exception(ostr);
    }
#ifdef TRACE
    {
      std::cerr << "Application::stop: leaving" << std::endl;
    }
#endif
  }

  /**
   * This function must give random FUTURE timestamp.
   * Minimum is monotonic increasing
   */
  Generics::Time
  Application::rand_time() const throw (eh::Exception)
  {
    Read_Guard_ guard(lock_);

    // + 1 to make a more uniform distribution
    unsigned long tm = min_sceduling_time_ +
      static_cast<long>((max_sceduling_time_ - min_sceduling_time_ + 1) *
      static_cast<double>(rand()) / RAND_MAX);
    if (tm > max_sceduling_time_)
    {
      tm = max_sceduling_time_;
    }

    Time time(Time::get_time_of_day());
    std::size_t current_msc = time.tv_usec;
    if (tm != max_sceduling_time_)
    { // randomizing microseconds
      current_msc +=
        static_cast<std::size_t>((Time::USEC_MAX - current_msc) *
                                 static_cast<double>(rand()) / RAND_MAX);
    }

    return Time(time.tv_sec + tm, current_msc);
  }

  /**
   * This function give random time from some interval
   * We will check what happen with past events into schedule.
   */
  Generics::Time
  Application::full_random_time() const throw (eh::Exception)
  {
    Read_Guard_ guard(lock_);

    // + 1 to make a more uniform distribution
    unsigned long tm = min_sceduling_time_ +
      static_cast<long>((max_sceduling_time_ - min_sceduling_time_ + 1) *
      static_cast<double>(rand()) / RAND_MAX);
    if (tm > max_sceduling_time_)
    {
      tm = max_sceduling_time_;
    }

    static Time time(Time::get_time_of_day()); //FIXME
    std::size_t current_msc = time.tv_usec;
    if (tm != max_sceduling_time_)
    { // randomizing microseconds
      current_msc += static_cast<std::size_t>(Time::USEC_MAX *
        static_cast<double>(rand()) / RAND_MAX);
      current_msc %= 1000000;
    }

    return Time(time.tv_sec + tm, current_msc);
  }

  Generics::Time
  Application::compact_time_series() const throw (eh::Exception)
  {
    Read_Guard_ guard(lock_);

    static Time time(Time::get_time_of_day()+1); //FIXME
    time += Time(0, 1);

    return time;
  }

  bool
  Application::is_test_successfull_(std::string& error_description) const
    throw ()
  {
    error_description.clear();
    Generics::Time epsilon(0, 500000);
    if (!scheduled_events_.empty())
    {
      error_description = "Failed because has undelivered events";
      return false;
    }
    if (message_count_ != MAX_EVENTS_ON_TEST && max_gap_ > epsilon)
    {
      if (max_gap_planed_moment_ < max_gap_schedule_moment_ && // overdue AND
          // but right away
          max_gap_moment_ - max_gap_schedule_moment_ < epsilon)
     {
          return true;
     }
     std::ostringstream ostr;
     ostr << "Failed because maximum overdue for event >= "
       << epsilon << ". But we are awaiting <";
     error_description = ostr.str();
     return false;
    }
    return true;
  }

  void
  Application::print_results() throw (eh::Exception)
  {
    std::cout << "*** Test Results ***" << std::endl << std::endl;

    if (start_time_ == Generics::Time::ZERO
        || stop_time_ == Generics::Time::ZERO)
    {
      std::cerr << "Test failed" << std::endl;
      return;
    }

    Generics::Time real_execution_time = stop_time_ - start_time_;

    std::cout << "Execution time: " << real_execution_time << std::endl <<
      std::endl;

    std::string possible_err_dsc;
    (is_test_successfull_(possible_err_dsc) ? std::cout : std::cerr)
      << possible_err_dsc
      << "\nUndelivered events: " << scheduled_events_.size() << std::endl
      << "Processed events: " << processed_events_ << std::endl
      << "Maximum gap " << max_gap_ << std::endl
      << "Maximum gap planned time " << max_gap_planed_moment_
      << "\nMaximum gap scheduling time " << max_gap_schedule_moment_
      << "\nMaximum gap processing moment " << max_gap_moment_
      << std::endl;
    statistics_->dump(std::cout);

    long bound = deviation_stat_size_ - 1;

    for (; bound >= 0 && !negative_deviation_[bound] &&
          !positive_deviation_[bound]; bound--);

    bound++;

    if (bound > 0)
    {
      std::cout << "Delay +dev -dev" << std::endl;

      for (unsigned long i = 0; i < (unsigned long)bound; i++)
      {
        if (!positive_deviation_[i] && !negative_deviation_[i])
        {
          continue;
        }

        if (i == deviation_stat_size_ - 1)
        {
          std::cout << ">" << deviation_max_ << "  ";
        }
        else
        {
          std::cout << (i + 1) * deviation_grid_ << "   ";
        }

        std::cout << positive_deviation_[i] << "  " <<
          negative_deviation_[i] << std::endl;
      }

    }
  }
  void
  Application::set_test_execution_time(const int nt) throw ()
  {
    execution_time_ = Generics::Time(nt);
  }
  void
  Application::set_message_count(unsigned long new_value) throw ()
  {
    message_count_ = new_value;
  }

} // namespace Generics


int
main(int argc, char** argv)
{
  using namespace Generics;

  try
  {
    Generics::Application app;
    const char STR[] = "\t\tTIME ON TASK ";

    app.set_test_execution_time(11);

    std::cout << std::endl << STR << "\t\tUCS 97" << std::endl;
    app.init(argc, argv);
    app.run(&Application::ScheduleMakerUCS97,
      &Application::rand_time);

    // General reason of UCS-97 breaking scheduling.
    // Check it with portion scheduling scenario.
    std::cout << std::endl << STR << "\t\tGENERAL PORTION SCHEDULER"
              << std::endl;
    app.init(argc, argv);
    app.run(&Application::SchedulePortionMaker,
      &Application::rand_time);

    // General reason of UCS-97 breaking scheduling.
    // Check it with portion scheduling scenario.
    std::cout << std::endl << STR << "\t\tPERFORMANCE SCHEDULER TEST"
              << std::endl;
    app.init(argc, argv);
    app.set_message_count(MAX_EVENTS_ON_TEST);
    app.run(&Application::SchedulePortionMaker,
      &Application::rand_time);
    std::cout << "\t\tREGRESSION TEST FINISHED" << std::endl;
    app.set_message_count(100);

    // Additional tests

    int times_for_all_checks[] = {0, 1};
    for (std::size_t i = 0; i < sizeof(times_for_all_checks)/sizeof(int);
      ++i)
    {
      app.set_test_execution_time(times_for_all_checks[i]);
      std::cout << std::endl << STR << times_for_all_checks[i]
                << " seconds" << std::endl
                << "\t\tRAND_TIME START" << std::endl;
      app.init(argc, argv);
      app.run(&Application::SchedulePortionMaker,
              &Application::rand_time);

      //
      std::cout << std::endl << STR << times_for_all_checks[i]
                << " seconds" << std::endl
        << "\t\tFULL_RANDOM_TIME START" << std::endl;
      app.init(argc, argv);
      app.run(&Application::ScheduleMaker,
              &Application::full_random_time);

      //
      std::cout << std::endl << STR << times_for_all_checks[i]
                << " seconds" << std::endl
                << "\t\tCOMPACT_TIME_SERIES START" << std::endl;
      app.init(argc, argv);
      app.run(&Application::ScheduleMaker,
              &Application::compact_time_series);
    }

    int times[] = {2, 6, 30};
    for (std::size_t i = 0; i < sizeof(times)/sizeof(int); ++i)
    {
      std::cout << std::endl << STR << times[i]
                << " seconds" << std::endl;
      app.set_test_execution_time(times[i]);
      app.init(argc, argv);
      app.run(&Application::ScheduleMaker,
              &Application::rand_time);
    }

  }
  catch(const Application::Exception& e)
  {
    std::cerr << "main: Application::Exception exception caught. "
      "Description:" << std::endl << e.what() << std::endl;
  }
  catch(const eh::Exception& e)
  {
    std::cerr << "main: eh::Exception exception caught. "
      "Description:" << std::endl << e.what() << std::endl;
  }

  return 0;
}
