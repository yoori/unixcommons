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





#ifndef GENERICS_STATISTICS_HPP
#define GENERICS_STATISTICS_HPP

#include <iostream>
#include <list>

#include <Sync/SyncPolicy.hpp>

#include <ReferenceCounting/Map.hpp>

#include <Generics/TaskRunner.hpp>


//#define GENERICS_STATISTICS_USE_LATEST_TIMES

namespace Generics
{
  namespace Statistics
  {
    //
    // Statistics interfaces
    //

    class Subject
    {
    public:
      virtual
      ~Subject() throw () = 0;
    };

    class StatSink : public virtual ReferenceCounting::Interface
    {
    public:
      virtual
      void
      consider(const Subject& subject) throw (eh::Exception) = 0;

      virtual
      unsigned
      considered_count() const throw (eh::Exception) = 0;

      virtual
      void
      reset() throw (eh::Exception) = 0;

      virtual
      StatSink*
      clone() throw (eh::Exception) = 0;

      virtual
      void
      dump(std::ostream& ostr) throw (eh::Exception) = 0;
    };
    typedef ReferenceCounting::QualPtr<StatSink> StatSink_var;

    //
    // Statistics dumping policy classes
    //

    class DumpPolicy : public virtual ReferenceCounting::Interface
    {
    public:
      virtual
      bool
      need_dump(StatSink* stat) throw (eh::Exception) = 0;

      virtual
      DumpPolicy*
      clone() throw (eh::Exception) = 0;

      virtual
      void
      dump(StatSink* stat) throw (eh::Exception) = 0;
    };
    typedef ReferenceCounting::QualPtr<DumpPolicy> DumpPolicy_var;


    class NullDumpPolicy :
      public virtual DumpPolicy,
      public virtual ReferenceCounting::DefaultImpl<>
    {
    public:
      virtual
      bool
      need_dump(StatSink* stat) throw (eh::Exception);

      virtual
      DumpPolicy*
      clone() throw (eh::Exception);

      virtual
      void
      dump(StatSink* stat) throw (eh::Exception);

    protected:
      ~NullDumpPolicy() throw ();
    };

    class StreamDumpPolicy :
      public virtual DumpPolicy,
      public virtual ReferenceCounting::AtomicImpl
    {
    public:
      explicit
      StreamDumpPolicy(std::ostream& ostr)
        throw (eh::Exception);

      std::ostream&
      stream() throw (eh::Exception);

      virtual
      void
      dump(StatSink* stat) throw (eh::Exception);

    protected:
      virtual
      ~StreamDumpPolicy() throw ();

      mutable Sync::PosixMutex mutex_;
      std::ostream& ostream_;
    };

    class CountBasedDumpPolicy : public virtual StreamDumpPolicy
    {
    public:
      CountBasedDumpPolicy(std::ostream& ostr, unsigned long long dump_freq)
        throw (eh::Exception);

      virtual
      bool
      need_dump(StatSink* stat) throw (eh::Exception);

      virtual
      DumpPolicy*
      clone() throw (eh::Exception);

    protected:
      virtual
      ~CountBasedDumpPolicy() throw ();

    protected:
      unsigned long long dump_freq_;
    };

    //
    // Dump running policies
    //

    class DumpRunner : public virtual ActiveObject
    {
    public:
      virtual
      void
      execute_dumping(DumpPolicy* policy, StatSink* stat)
        throw (eh::Exception) = 0;

    protected:
      virtual
      ~DumpRunner() throw ();
    };
    typedef ReferenceCounting::QualPtr<DumpRunner> DumpRunner_var;

    class NullDumpRunner :
      public virtual DumpRunner,
      public virtual ReferenceCounting::AtomicImpl
    {
    public:
      NullDumpRunner() throw ();

      virtual
      void
      execute_dumping(DumpPolicy* policy, StatSink* stat)
        throw (eh::Exception);

      virtual
      void
      activate_object()
        throw (AlreadyActive, Exception, eh::Exception);

      virtual
      void
      deactivate_object()
        throw (Exception, eh::Exception);

      virtual
      void
      wait_object() throw (Exception, eh::Exception);

      virtual
      bool
      active() throw (eh::Exception);

    protected:
      virtual
      ~NullDumpRunner() throw ();

    private:
      bool active_;
    };

    class TaskDumpRunner :
      public virtual DumpRunner,
      public virtual ReferenceCounting::AtomicImpl
    {
    public:
      explicit
      TaskDumpRunner(ActiveObjectCallback* callback,
        TaskRunner* task_runner = 0) throw (eh::Exception);

      virtual
      void
      execute_dumping(DumpPolicy* policy, StatSink* stat)
        throw (eh::Exception);

      virtual
      void
      activate_object() throw (AlreadyActive, Exception, eh::Exception);

      virtual
      void
      deactivate_object() throw (Exception, eh::Exception);

      virtual
      void
      wait_object() throw (Exception, eh::Exception);

      virtual
      bool
      active() throw (eh::Exception);

    protected:
      virtual
      ~TaskDumpRunner() throw ();
    public:
      class DumpTask : public virtual TaskImpl
      {
      public:
        DumpTask(StatSink* stat, DumpPolicy* dump_policy)
          throw (eh::Exception);

        virtual
        void
        execute() throw ();

      protected:
        virtual
        ~DumpTask() throw ();

      private:
        StatSink_var stat_;
        DumpPolicy_var dump_policy_;
      };
      typedef ReferenceCounting::QualPtr<DumpTask> DumpTask_var;

    private:
      TaskRunner_var task_runner_;
      ActiveObjectCallback_var callback_;
    };


    //
    // Statistics collection classes
    //

    class Collection :
      public virtual ActiveObject,
      public virtual ReferenceCounting::AtomicImpl
    {
    public:
      DECLARE_EXCEPTION(Exception, ActiveObject::Exception);
      DECLARE_EXCEPTION(InvalidArgument, Exception);
      DECLARE_EXCEPTION(StatItemNotFound, Exception);

      explicit
      Collection(DumpRunner* dump_runner)
        throw (InvalidArgument, Exception, eh::Exception);

      /* Collector c-tor for compatibility
       * use TaskDumpRunner strategy for stat dumping
       */
      explicit
      Collection(ActiveObjectCallback* callback)
        throw (InvalidArgument, Exception, eh::Exception);

      void
      add(const char* id, Statistics::StatSink* stat,
        DumpPolicy* policy)
        throw (InvalidArgument, Exception, eh::Exception);

      Statistics::StatSink*
      get(const char* id)
        throw (InvalidArgument, StatItemNotFound, Exception, eh::Exception);

      void
      dump(std::ostream& ostr) throw (Exception, eh::Exception);

      virtual
      void
      activate_object()
        throw (AlreadyActive, Exception, eh::Exception);

      virtual
      void
      deactivate_object() throw (Exception, eh::Exception);

      virtual
      void
      wait_object() throw (Exception, eh::Exception);

      virtual
      bool
      active() throw (eh::Exception);

    protected:
      virtual
      ~Collection() throw ();

      class Item :
        public virtual Statistics::StatSink,
        public virtual ReferenceCounting::AtomicImpl
      {
      public:
        DECLARE_EXCEPTION(Exception, ActiveObject::Exception);
        DECLARE_EXCEPTION(InvalidArgument, Exception);

        Item(const char* id, StatSink* stat, DumpPolicy* dump_policy,
          DumpRunner* stat_dumper)
          throw (InvalidArgument, eh::Exception);

        virtual
        void
        consider(const Subject& subject) throw (eh::Exception);

        virtual
        unsigned
        considered_count() const throw (eh::Exception);

        virtual
        void
        reset() throw (eh::Exception);

        virtual
        StatSink*
        clone() throw (eh::Exception);

        virtual
        void
        dump(std::ostream& ostr) throw (eh::Exception);

      protected:
        virtual
        ~Item() throw ();

        virtual
        StatSink*
        clone_i() throw (eh::Exception);

        static
        std::string
        current_time() throw (eh::Exception);

      protected:
        mutable Sync::PosixMutex mutex_;
        std::string id_;
        StatSink_var stat_;
        DumpPolicy_var dump_policy_;
        DumpRunner_var stat_dumper_;
      };
      typedef ReferenceCounting::QualPtr<Item> Item_var;

      typedef ReferenceCounting::Map<std::string, Item_var> ItemMap;
      typedef std::list<std::string> IdList;

      typedef Sync::PosixRWLock Mutex_;
      typedef Sync::PosixRGuard ReadGuard_;
      typedef Sync::PosixWGuard WriteGuard_;

      mutable Mutex_ lock_;

      DumpRunner_var stat_dumper_;
      ItemMap items_;
    };
    typedef ReferenceCounting::QualPtr<Collection> Collection_var;



    template <typename DataType,
      typename SyncPolicy = Sync::Policy::PosixThread>
    class DefaultDataProvider
    {
    public:
      typedef DataType Data;
      typedef SyncPolicy Policy;

      explicit
      DefaultDataProvider(const Data& data = Data())
        throw (eh::Exception);

      typename Policy::Mutex&
      mutex() const throw ();
      Data&
      get() throw ();
      const Data&
      get() const throw ();
      void
      set() throw ();

    private:
      mutable typename Policy::Mutex mutex_;
      Data data_;
    };


    //
    // Timed instance statistics implementation classes
    //

    class TimedSubject : public Subject
    {
    public:
      explicit
      TimedSubject(const Time& time)
        throw (eh::Exception);

      virtual
      ~TimedSubject() throw ();

      const Time&
      time() const throw (eh::Exception);

      void
      time(const Time& src) throw (eh::Exception);

    protected:
      Time time_;
    };

    struct TimedStatData
    {
      struct Data
      {
        Data() throw (eh::Exception);

        Time max_time;
        Time min_time;
        Time total_time;
        unsigned count;
      };

      Data cur;
#ifdef GENERICS_STATISTICS_USE_LATEST_TIMES
      Data latest;
#endif
    };

    template <typename DataProvider>
    class TimedStatSinkTempl :
      public virtual StatSink,
      public virtual ReferenceCounting::AtomicImpl
    {
    public:
      DECLARE_EXCEPTION(Exception, ActiveObject::Exception);
      DECLARE_EXCEPTION(InvalidArgument, Exception);

      typedef typename DataProvider::Data Data;

      struct Stat : public Data::Data
      {
        Time avg_time;
      };

      TimedStatSinkTempl() throw (eh::Exception);
      template <typename T>
      explicit
      TimedStatSinkTempl(T data) throw (eh::Exception);


      virtual
      void
      consider(const Subject& subject) throw (InvalidArgument, eh::Exception);

      virtual
      unsigned
      considered_count() const throw (eh::Exception);

      virtual
      void
      reset() throw (eh::Exception);

      virtual
      StatSink*
      clone() throw (eh::Exception);

      virtual
      void
      dump(std::ostream& ostr) throw (eh::Exception);

      typename Data::Data
      data() const throw (eh::Exception);

      static
      Stat
      stat(const typename Data::Data& data) throw (eh::Exception);

      Time
      max_time() const throw (eh::Exception);
      Time
      min_time() const throw (eh::Exception);
      Time
      total_time() const throw (eh::Exception);
      Time
      average_time() const throw (eh::Exception);

    protected:
      typedef typename DataProvider::Policy Policy;

      explicit
      TimedStatSinkTempl(const Data& data)
        throw (eh::Exception);

      virtual
      ~TimedStatSinkTempl() throw ();

      static
      Time
      average_time_(const typename Data::Data& data)
        throw (eh::Exception);

      static
      void
      consider_(typename Data::Data& data, const Time& time)
        throw (InvalidArgument, eh::Exception);

    protected:
      DataProvider provider_;
    };

    typedef TimedStatSinkTempl<DefaultDataProvider<TimedStatData> >
      TimedStatSink;
    typedef ReferenceCounting::QualPtr<TimedStatSink> TimedStatSink_var;


    //
    // Measurable instance statistics implementation classes
    //

    template <typename DataType>
    class MeasurableSubject : public Statistics::Subject
    {
    public:
      explicit
      MeasurableSubject(DataType value) throw (eh::Exception);

      DataType
      value() const throw ();

    protected:
      DataType value_;
    };

    template <typename DataType, typename CounterType = unsigned>
    struct MeasurableStatData
    {
      MeasurableStatData() throw (eh::Exception);

      DataType max_value;
      DataType min_value;
      DataType sum_value;
      CounterType meterings_count;
    };

    template <typename DataType, typename DataDataType>
    struct MeasurableStatStat : public DataDataType
    {
      MeasurableStatStat() throw (eh::Exception);

      DataType avg_value;
    };

    template <typename DataType,
      typename DataProvider =
        DefaultDataProvider<MeasurableStatData<DataType> >,
      typename StatType =
        MeasurableStatStat<DataType, typename DataProvider::Data> >
    class MeasurableStatSink :
      public virtual Statistics::StatSink,
      public virtual ReferenceCounting::AtomicImpl
    {
    public:
      DECLARE_EXCEPTION(Exception, ActiveObject::Exception);
      DECLARE_EXCEPTION(InvalidArgument, Exception);

      typedef typename DataProvider::Data Data;

      MeasurableStatSink() throw (eh::Exception);
      template <typename Init>
      explicit
      MeasurableStatSink(Init value) throw (eh::Exception);

      virtual
      void
      consider(const Statistics::Subject& subject)
        throw (InvalidArgument, eh::Exception);

      virtual
      unsigned
      considered_count() const
        throw (eh::Exception);

      virtual
      void
      reset() throw (eh::Exception);

      virtual
      Statistics::StatSink*
      clone() throw (eh::Exception);

      virtual
      void
      dump(std::ostream& ostr) throw (eh::Exception);

      Data
      data() const throw (eh::Exception);

      static
      StatType
      stat(const Data& data) throw (eh::Exception);

      DataType
      max_value() const throw (eh::Exception);
      DataType
      min_value() const throw (eh::Exception);
      DataType
      average_value() const throw (eh::Exception);

    protected:
      typedef typename DataProvider::Policy Policy;

      explicit
      MeasurableStatSink(const Data& data) throw (eh::Exception);

      virtual
      ~MeasurableStatSink() throw ();

      static
      DataType
      average_value_(const Data& data) throw (eh::Exception);

    protected:
      DataProvider provider_;
    };
  }
}

#include <Generics/Statistics.tpp>

inline
std::ostream&
operator <<(std::ostream& ostr, Generics::Statistics::StatSink& stat)
  throw (eh::Exception)
{
  stat.dump(ostr);
  return ostr;
}

#endif
