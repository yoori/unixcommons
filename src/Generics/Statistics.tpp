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



#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>


namespace Generics
{
  namespace Statistics
  {
    //
    // Subject class
    //

    inline
    Subject::~Subject() throw ()
    {
    }


    //
    // NullDumpPolicy class
    //

    inline
    NullDumpPolicy::~NullDumpPolicy() throw ()
    {
    }

    inline
    bool
    NullDumpPolicy::need_dump(StatSink* /*stat*/) throw (eh::Exception)
    {
      return false;
    }

    inline
    DumpPolicy*
    NullDumpPolicy::clone() throw (eh::Exception)
    {
      return new NullDumpPolicy();
    }

    inline
    void
    NullDumpPolicy::dump(StatSink* /*stat*/) throw (eh::Exception)
    {
    }


    //
    // StreamDumpPolicy class
    //

    inline
    StreamDumpPolicy::StreamDumpPolicy(std::ostream& ostr)
      throw (eh::Exception)
      : ostream_(ostr)
    {
    }

    inline
    StreamDumpPolicy::~StreamDumpPolicy() throw ()
    {
    }

    inline
    std::ostream&
    StreamDumpPolicy::stream() throw (eh::Exception)
    {
      return ostream_;
    }

    inline
    void
    StreamDumpPolicy::dump(StatSink* stat) throw (eh::Exception)
    {
      if (stat)
      {
        Sync::PosixGuard guard(mutex_);
        stat->dump(ostream_);
        ostream_ << std::endl;
      }
    }


    //
    // CountBasedDumpPolicy class
    //

    inline
    CountBasedDumpPolicy::CountBasedDumpPolicy(std::ostream& ostr,
      unsigned long long dump_freq) throw (eh::Exception)
      : StreamDumpPolicy(ostr),
        dump_freq_(dump_freq)
    {
    }

    inline
    CountBasedDumpPolicy::~CountBasedDumpPolicy() throw ()
    {
    }

    inline
    bool
    CountBasedDumpPolicy::need_dump(StatSink* stat)
      throw (eh::Exception)
    {
      return (stat->considered_count() % dump_freq_) == 0;
    }

    inline
    DumpPolicy*
    CountBasedDumpPolicy::clone() throw (eh::Exception)
    {
      return new CountBasedDumpPolicy(ostream_, dump_freq_);
    }


    //
    // DumpRunner class

    inline
    DumpRunner::~DumpRunner() throw ()
    {
    }


    //
    // NullDumpRunner
    //

    inline
    NullDumpRunner::NullDumpRunner() throw ()
      : active_(false)
    {
    }

    inline
    NullDumpRunner::~NullDumpRunner() throw ()
    {
    }

    inline
    void
    NullDumpRunner::execute_dumping(DumpPolicy*, StatSink*)
      throw (eh::Exception)
    {
    }

    inline
    void
    NullDumpRunner::activate_object()
      throw (AlreadyActive, Exception, eh::Exception)
    {
      active_ = true;
    }

    inline
    void
    NullDumpRunner::deactivate_object()
      throw (Exception, eh::Exception)
    {
      active_ = false;
    }

    inline
    void
    NullDumpRunner::wait_object() throw (Exception, eh::Exception)
    {
      active_ = false;
    }

    inline
    bool
    NullDumpRunner::active() throw (eh::Exception)
    {
      return active_;
    }


    //
    // TaskNullDumpRunner
    //

    inline
    TaskDumpRunner::TaskDumpRunner(ActiveObjectCallback* callback,
      TaskRunner* task_runner)
      throw (eh::Exception)
      : callback_(ReferenceCounting::add_ref(callback))
    {
      if (callback == 0)
      {
        Stream::Error ostr;
        ostr << FNS << "callback == 0";
        throw Exception(ostr);
      }

      if (task_runner)
      {
        task_runner_ = ReferenceCounting::add_ref(task_runner);
      }
      else
      {
        task_runner_ = new TaskRunner(callback, 1);
      }
    }

    inline
    TaskDumpRunner::~TaskDumpRunner() throw ()
    {
    }

    inline
    void
    TaskDumpRunner::execute_dumping(DumpPolicy* policy, StatSink* stat)
      throw (eh::Exception)
    {
      task_runner_->enqueue_task(DumpTask_var(new DumpTask(stat, policy)));

      sleep(0);
    }

    inline
    void
    TaskDumpRunner::activate_object()
      throw (AlreadyActive, Exception, eh::Exception)
    {
      task_runner_->activate_object();
    }

    inline
    void
    TaskDumpRunner::deactivate_object()
      throw (Exception, eh::Exception)
    {
      task_runner_->deactivate_object();
    }

    inline
    void
    TaskDumpRunner::wait_object()
      throw (Exception, eh::Exception)
    {
      task_runner_->wait_object();
    }

    inline
    bool
    TaskDumpRunner::active() throw (eh::Exception)
    {
      return task_runner_->active();
    }


    //
    // Collection class
    //

    inline
    bool
    Collection::active() throw (eh::Exception)
    {
      return stat_dumper_->active();
    }

    inline
    Statistics::StatSink*
    Collection::get(const char* id)
      throw (InvalidArgument, StatItemNotFound, Exception, eh::Exception)
    {
      if (id == 0)
      {
        Stream::Error ostr;
        ostr << FNS << "id == 0";
        throw InvalidArgument(ostr);
      }

      ReadGuard_ guard(lock_);

      ItemMap::iterator item = items_.find(id);

      if (item == items_.end())
      {
        Stream::Error ostr;
        ostr << FNS << "item not found for id " << id;
        throw StatItemNotFound(ostr);
      }

      StatSink* stat = item->second;
      stat->add_ref();

      return stat;
    }

    //
    // Collection::Item class
    //

    inline
    Collection::Item::~Item() throw ()
    {
    }

    inline
    Collection::Item::Item(const char* id, StatSink* stat,
      DumpPolicy* dump_policy, DumpRunner* stat_dumper)
      throw (InvalidArgument, eh::Exception)
    {
      if (stat == 0)
      {
        Stream::Error ostr;
        ostr << FNS << "stat == 0";
        throw InvalidArgument(ostr);
      }

      if (id == 0)
      {
        Stream::Error ostr;
        ostr << FNS << "id == 0";
        throw InvalidArgument(ostr);
      }

      if (dump_policy == 0)
      {
        Stream::Error ostr;
        ostr << FNS << "dump_policy == 0";
        throw InvalidArgument(ostr);
      }

      if (stat_dumper == 0)
      {
        Stream::Error ostr;
        ostr << FNS << "stat_dumper == 0";
        throw InvalidArgument(ostr);
      }

      id_ = id;
      stat_ = stat;
      dump_policy_ = dump_policy->clone();
      stat_dumper_ = ReferenceCounting::add_ref(stat_dumper);
    }

    inline
    void
    Collection::Item::consider(const Subject& subject)
      throw (eh::Exception)
    {
      Sync::PosixGuard guard(mutex_);

      stat_->consider(subject);

      if (dump_policy_->need_dump(stat_))
      {
        stat_dumper_->execute_dumping(dump_policy_->clone(), clone_i());
      }
    }

    inline
    unsigned
    Collection::Item::considered_count() const
      throw (eh::Exception)
    {
      return stat_->considered_count();
    }

    inline
    void
    Collection::Item::reset() throw (eh::Exception)
    {
      stat_->considered_count();
    }

    inline
    StatSink*
    Collection::Item::clone()
      throw (eh::Exception)
    {
      Sync::PosixGuard guard(mutex_);
      return clone_i();
    }

    inline
    StatSink*
    Collection::Item::clone_i()
      throw (eh::Exception)
    {
      return new Item(id_.c_str(), stat_->clone(), dump_policy_,
        stat_dumper_);
    }


    //
    // TaskDumpRunner::DumpTask class
    //

    inline
    TaskDumpRunner::DumpTask::DumpTask(StatSink* stat,
      DumpPolicy* dump_policy) throw (eh::Exception)
      : stat_(stat), dump_policy_(dump_policy)
    {
    }

    inline
    TaskDumpRunner::DumpTask::~DumpTask() throw ()
    {
    }

    inline
    void
    TaskDumpRunner::DumpTask::execute() throw ()
    {
      try
      {
        if (dump_policy_)
        {
          dump_policy_->dump(stat_);
        }
      }
      catch(...)
      {
      }
    }


    //
    // DefaultDataProvider class
    //

    template <typename DataType, typename Policy>
    DefaultDataProvider<DataType, Policy>::DefaultDataProvider(
      const Data& data) throw (eh::Exception)
      : data_(data)
    {
    }

    template <typename DataType, typename Policy>
    typename DefaultDataProvider<DataType, Policy>::Policy::Mutex&
    DefaultDataProvider<DataType, Policy>::mutex() const throw ()
    {
      return mutex_;
    }

    template <typename DataType, typename Policy>
    typename DefaultDataProvider<DataType, Policy>::Data&
    DefaultDataProvider<DataType, Policy>::get() throw ()
    {
      return data_;
    }

    template <typename DataType, typename Policy>
    const typename DefaultDataProvider<DataType, Policy>::Data&
    DefaultDataProvider<DataType, Policy>::get() const throw ()
    {
      return data_;
    }

    template <typename DataType, typename Policy>
    void
    DefaultDataProvider<DataType, Policy>::set() throw ()
    {
    }


    //
    // TimedSubject class
    //

    inline
    TimedSubject::TimedSubject(const Time& time)
      throw (eh::Exception)
      : time_(time)
    {
    }

    inline
    TimedSubject::~TimedSubject() throw ()
    {
    }

    inline
    const Time&
    TimedSubject::time() const throw (eh::Exception)
    {
      return time_;
    }

    inline
    void
    TimedSubject::time(const Time& src)
      throw (eh::Exception)
    {
      time_ = src;
    }


    //
    // TimedStatSinkData::Data class
    //

    inline
    TimedStatData::Data::Data() throw (eh::Exception)
      : count(0)
    {
    }


    //
    // TimedStatSinkTempl class
    //

    template <typename DataProvider>
    TimedStatSinkTempl<DataProvider>::TimedStatSinkTempl()
      throw (eh::Exception)
    {
    }

    template <typename DataProvider>
    template <typename T>
    TimedStatSinkTempl<DataProvider>::TimedStatSinkTempl(T data)
      throw (eh::Exception)
      : provider_(data)
    {
    }

    template <typename DataProvider>
    TimedStatSinkTempl<DataProvider>::TimedStatSinkTempl(const Data& data)
      throw (eh::Exception)
      : provider_(data)
    {
    }

    template <typename DataProvider>
    TimedStatSinkTempl<DataProvider>::~TimedStatSinkTempl() throw ()
    {
    }

    template <typename DataProvider>
    StatSink*
    TimedStatSinkTempl<DataProvider>::clone()
      throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());
      return new TimedStatSinkTempl<DataProvider>(provider_.get());
    }

    template <typename DataProvider>
    void
    TimedStatSinkTempl<DataProvider>::reset() throw (eh::Exception)
    {
      typename Policy::WriteGuard guard(provider_.mutex());

      Data& data = provider_.get();

      data.cur = typename Data::Data();

#ifdef GENERICS_STATISTICS_USE_LATEST_TIMES
      data.latest = Data::Data();
#endif

      provider_.set();
    }

    template <typename DataProvider>
    void
    TimedStatSinkTempl<DataProvider>::consider_(typename Data::Data& data,
      const Time& time) throw (InvalidArgument, eh::Exception)
    {
      if (!data.count)
      {
        data.max_time = time;
        data.min_time = time;
      }
      else
      {
        if (data.max_time < time)
        {
          data.max_time = time;
        }
        if (data.min_time > time)
        {
          data.min_time = time;
        }
      }

      data.total_time += time;
      data.count++;
    }

    template <typename DataProvider>
    void
    TimedStatSinkTempl<DataProvider>::consider(const Subject& subject)
      throw (InvalidArgument, eh::Exception)
    {
      typename Policy::WriteGuard guard(provider_.mutex());

      const TimedSubject* timed_subject =
        dynamic_cast<const TimedSubject*>(&subject);

      if (timed_subject == 0)
      {
        Stream::Error ostr;
        ostr << FNS << "subject is not of TimedSubject type";
        throw InvalidArgument(ostr);
      }

      const Time& time = timed_subject->time();

      Data& data = provider_.get();

      consider_(data.cur, time);

#ifdef GENERICS_STATISTICS_USE_LATEST_TIMES
      consider_(data.latest, time);
#endif

      provider_.set();
    }

    template <typename DataProvider>
    typename TimedStatSinkTempl<DataProvider>::Data::Data
    TimedStatSinkTempl<DataProvider>::data() const
      throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());
      return provider_.get().cur;
    }

    template <typename DataProvider>
    Time
    TimedStatSinkTempl<DataProvider>::average_time_(
      const typename Data::Data& data) throw (eh::Exception)
    {
      return data.count ? data.total_time / data.count : Time();
    }

    template <typename DataProvider>
    typename TimedStatSinkTempl<DataProvider>::Stat
    TimedStatSinkTempl<DataProvider>::stat(const typename Data::Data& data)
      throw (eh::Exception)
    {
      typename TimedStatSinkTempl<DataProvider>::Stat stat;
      static_cast<typename Data::Data&>(stat) = data;
      stat.avg_time = average_time_(stat);
      return stat;
    }

    template <typename DataProvider>
    Time
    TimedStatSinkTempl<DataProvider>::max_time() const
      throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());
      return provider_.get().cur.max_time;
    }

    template <typename DataProvider>
    Time
    TimedStatSinkTempl<DataProvider>::min_time() const
      throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());
      return provider_.get().cur.min_time;
    }

    template <typename DataProvider>
    Time
    TimedStatSinkTempl<DataProvider>::total_time() const
      throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());
      return provider_.get().cur.total_time;
    }

    template <typename DataProvider>
    Time
    TimedStatSinkTempl<DataProvider>::average_time() const
      throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());
      return average_time_(provider_.get().cur);
    }

    template <typename DataProvider>
    unsigned
    TimedStatSinkTempl<DataProvider>::considered_count() const
      throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());
      return provider_.get().cur.count;
    }

    template <typename DataProvider>
    void
    TimedStatSinkTempl<DataProvider>::dump(std::ostream& ostr)
      throw (eh::Exception)
    {
#ifdef GENERICS_STATISTICS_USE_LATEST_TIMES
      typename Policy::WriteGuard guard(provider_.mutex());
      Data& data = provider_.get();
#else
      typename Policy::ReadGuard guard(provider_.mutex());
      const Data& data = provider_.get();
#endif

      const Stat& cur = stat(data.cur);
#ifdef GENERICS_STATISTICS_USE_LATEST_TIMES
      const Stat& latest = stat(data.latest);
#endif
      ostr << "Total time meterings: " << cur.count;
#ifdef GENERICS_STATISTICS_USE_LATEST_TIMES
      ostr << " / " << latest.count;
#endif
      ostr << std::endl;

      ostr << "Ttl time: " << cur.total_time;
#ifdef GENERICS_STATISTICS_USE_LATEST_TIMES
      ostr << " / " << latest.total_time;
#endif

      ostr << std::endl << "Max time: " << cur.max_time;
#ifdef GENERICS_STATISTICS_USE_LATEST_TIMES
      ostr << " / " << latest.max_time;
#endif

      ostr << std::endl << "Min time: " << cur.min_time;
#ifdef GENERICS_STATISTICS_USE_LATEST_TIMES
      ostr << " / " << latest.min_time;
#endif

      ostr << std::endl << "Avg time: " << cur.avg_time;
#ifdef GENERICS_STATISTICS_USE_LATEST_TIMES
      ostr << " / " << latest.avg_time;
#endif

      ostr << std::endl;

#ifdef GENERICS_STATISTICS_USE_LATEST_TIMES
      data.latest = Data::Data();
      provider_.set()
#endif
    }


    //
    // MeasurableSubject class
    //

    template <typename DataType>
    MeasurableSubject<DataType>::MeasurableSubject(DataType value)
      throw (eh::Exception)
      : value_(value)
    {
    }

    template <typename DataType>
    DataType
    MeasurableSubject<DataType>::value() const throw ()
    {
      return value_;
    }


    //
    // MeasurableStatData class
    //

    template <typename DataType, typename CounterType>
    MeasurableStatData<DataType, CounterType>::MeasurableStatData()
      throw (eh::Exception)
      : max_value(DataType()), min_value(DataType()), sum_value(DataType()),
        meterings_count(CounterType())
    {
    }


    //
    // MeasurableStatStat class
    //

    template <typename DataType, typename DataDataType>
    MeasurableStatStat<DataType, DataDataType>::MeasurableStatStat()
      throw (eh::Exception)
      : DataDataType(), avg_value(DataType())
    {
    }


    //
    // MeasurableStatSink class
    //

    template <typename DataType, typename DataProvider, typename StatType>
    MeasurableStatSink<DataType, DataProvider, StatType>::
      MeasurableStatSink()
      throw (eh::Exception)
    {
    }

    template <typename DataType, typename DataProvider, typename StatType>
    template <typename Init>
    MeasurableStatSink<DataType, DataProvider, StatType>::
      MeasurableStatSink(Init value) throw (eh::Exception)
      : provider_(value)
    {
    }

    template <typename DataType, typename DataProvider, typename StatType>
    MeasurableStatSink<DataType, DataProvider, StatType>::
      MeasurableStatSink(const Data& data) throw (eh::Exception)
      : provider_(data)
    {
    }

    template <typename DataType, typename DataProvider, typename StatType>
    MeasurableStatSink<DataType, DataProvider, StatType>::
      ~MeasurableStatSink() throw ()
    {
    }

    template <typename DataType, typename DataProvider, typename StatType>
    Statistics::StatSink*
    MeasurableStatSink<DataType, DataProvider, StatType>::clone()
      throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());
      return new MeasurableStatSink<DataType, DataProvider, StatType>(
        provider_.get());
    }

    template <typename DataType, typename DataProvider, typename StatType>
    void
    MeasurableStatSink<DataType, DataProvider, StatType>::reset()
      throw (eh::Exception)
    {
      typename Policy::WriteGuard guard(provider_.mutex());
      provider_.get() = Data();
      provider_.set();
    }

    template <typename DataType, typename DataProvider, typename StatType>
    void
    MeasurableStatSink<DataType, DataProvider, StatType>::consider(
      const Statistics::Subject& subject)
      throw (InvalidArgument, eh::Exception)
    {
      typename Policy::WriteGuard guard(provider_.mutex());

      const MeasurableSubject<DataType>* measurable_subject =
        dynamic_cast<const MeasurableSubject<DataType>*>(&subject);

      if (measurable_subject == 0)
      {
        Stream::Error ostr;
        ostr << FNS << "subject is of invalid type";
        throw InvalidArgument(ostr);
      }

      DataType val = measurable_subject->value();

      Data& data = provider_.get();

      if (!data.meterings_count)
      {
        data.max_value = val;
        data.min_value = val;
      }
      else
      {
        if (data.max_value < val)
        {
          data.max_value = val;
        }
        if (data.min_value > val)
        {
          data.min_value = val;
        }
      }

      data.sum_value = data.sum_value + val;
      data.meterings_count++;

      provider_.set();
    }

    template <typename DataType, typename DataProvider, typename StatType>
    typename MeasurableStatSink<DataType, DataProvider, StatType>::Data
    MeasurableStatSink<DataType, DataProvider, StatType>::data() const
      throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());
      return provider_.get();
    }

    template <typename DataType, typename DataProvider, typename StatType>
    DataType
    MeasurableStatSink<DataType, DataProvider, StatType>::average_value_(
      const Data& data) throw (eh::Exception)
    {
      return data.meterings_count ?
        data.sum_value / data.meterings_count : 0;
    }

    template <typename DataType, typename DataProvider, typename StatType>
    StatType
    MeasurableStatSink<DataType, DataProvider, StatType>::stat(
      const Data& data) throw (eh::Exception)
    {
      StatType stat;
      stat.max_value = data.max_value;
      stat.min_value = data.min_value;
      stat.sum_value = data.sum_value;
      stat.meterings_count = data.meterings_count;
      stat.avg_value = average_value_(data);
      return stat;
    }

    template <typename DataType, typename DataProvider, typename StatType>
    DataType
    MeasurableStatSink<DataType, DataProvider, StatType>::max_value() const
      throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());
      return provider_.get().max_value;
    }

    template <typename DataType, typename DataProvider, typename StatType>
    DataType
    MeasurableStatSink<DataType, DataProvider, StatType>::min_value() const
      throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());
      return provider_.get().min_value;
    }

    template <typename DataType, typename DataProvider, typename StatType>
    DataType
    MeasurableStatSink<DataType, DataProvider, StatType>::
      average_value() const throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());
      return average_value(provider_.get());
    }

    template <typename DataType, typename DataProvider, typename StatType>
    unsigned
    MeasurableStatSink<DataType, DataProvider, StatType>::
      considered_count() const throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());
      return provider_.get().meterings_count;
    }

    template <typename DataType, typename DataProvider, typename StatType>
    void
    MeasurableStatSink<DataType, DataProvider, StatType>::dump(
      std::ostream& ostr) throw (eh::Exception)
    {
      typename Policy::ReadGuard guard(provider_.mutex());

      const StatType& data = stat(provider_.get());

      ostr << "Total meterings: " << data.meterings_count << std::endl <<
        "Max : " << data.max_value << std::endl <<
        "Min : " << data.min_value << std::endl <<
        "Avg : " << data.avg_value << std::endl;
    }
  }
}
