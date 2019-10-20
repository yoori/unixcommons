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



#ifndef CHECKCOMMONS_MTTESTER
#define CHECKCOMMONS_MTTESTER

#include <vector>

#include <sys/wait.h>

#include <eh/Exception.hpp>

#include <ReferenceCounting/ReferenceCounting.hpp>

#include <Stream/MemoryStream.hpp>

#include <Sync/PosixLock.hpp>
#include <Sync/Semaphore.hpp>

#include <Generics/TaskRunner.hpp>
#include <Generics/Function.hpp>

#include <Logger/StreamLogger.hpp>
#include <TestCommons/ActiveObjectCallback.hpp>

//#define BUILD_WITH_DEBUG_MESSAGES
#include <Generics/Trace.hpp>


namespace TestCommons
{
  class MTTasker
  {
  public:
    MTTasker(int threads) throw (eh::Exception);

    MTTasker(Generics::TaskRunner* task_runner)
      throw (eh::Exception);

    virtual
    ~MTTasker() throw ();

    void
    enqueue(Generics::Task* task) throw (eh::Exception);

    bool
    enqueue_conditionally(Generics::Task* task) throw (eh::Exception);

    void
    report_error(const String::SubString& message) throw ();

    void
    start(int limit, Sync::Semaphore* semaphore)
      throw (eh::Exception);

    void
    stop() throw (eh::Exception);

  private:
    Sync::PosixMutex mutex_;
    bool own_task_runner_;
    int limit_;
    Sync::Semaphore* semaphore_;
    Logging::ActiveObjectCallbackImpl_var callback_;
    Generics::TaskRunner_var task_runner_;
  };

  template <typename Functor>
  class MTTester
  {
  public:
    MTTester(Functor functor, int threads)
      throw (eh::Exception);

    MTTester(Functor functor, Generics::TaskRunner_var task_runner)
      throw (eh::Exception);

    void
    run(int tasks, time_t interval, int limit = -1) throw (eh::Exception);

  private:
    class FunctorTask :
      public Generics::Task,
      public ReferenceCounting::AtomicImpl
    {
    public:
      FunctorTask(Functor functor, MTTasker& tasker)
        throw (eh::Exception);

      virtual
      void
      execute() throw ();

    protected:
      virtual
      ~FunctorTask() throw ();

    private:
      Functor functor_;
      MTTasker& tasker_;
    };

    MTTasker tasker_;
    Functor functor_;
  };

  template <typename Functor>
  bool
  mp_test(Functor functor, int processes) throw (eh::Exception);
}

namespace TestCommons
{

  //
  // MTTasker class
  //

  inline
  MTTasker::MTTasker(int threads) throw (eh::Exception)
    : own_task_runner_(true),
      limit_(0),
      semaphore_(0),
      callback_(new ActiveObjectCallbackStreamImpl(std::cerr,
        "MTTasker")),
      task_runner_(new Generics::TaskRunner(callback_, threads))
  {
  }

  inline
  MTTasker::MTTasker(Generics::TaskRunner* task_runner)
    throw (eh::Exception)
    : own_task_runner_(false),
      limit_(0),
      semaphore_(0),
      callback_(new ActiveObjectCallbackStreamImpl(std::cerr,
        "MTTasker")),
      task_runner_(ReferenceCounting::add_ref(task_runner))
  {
  }

  inline
  MTTasker::~MTTasker() throw ()
  {
  }

  inline
  void
  MTTasker::enqueue(Generics::Task* task) throw (eh::Exception)
  {
    task_runner_->enqueue_task(task);
  }

  inline
  bool
  MTTasker::enqueue_conditionally(Generics::Task* task) throw (eh::Exception)
  {
#ifdef BUILD_WITH_DEBUG_MESSAGES
    const char FUN[] = "MTTasker::enqueue_conditionally(): ";
#endif
    Sync::PosixGuard guard(mutex_);
    trace_message(FUN, "enter");
    if (!limit_)
    {
      trace_message(FUN, "!limit return false");
      return false;
    }
    if (limit_ > 0)
    {
      trace_message(FUN, limit_);
      --limit_;
      if (!limit_)
      {
        trace_message(FUN, "!limit_");
        if (semaphore_)
        {
          trace_message(FUN, "MTTasker::enqueue");
          semaphore_->release();
          semaphore_ = 0;
        }
        return false;
      }
    }
    trace_message(FUN, "enqueue_task");
    enqueue(task);
    return true;
  }

  inline
  void
  MTTasker::report_error(const String::SubString& message) throw ()
  {
    callback_->error(message);
  }

  inline
  void
  MTTasker::start(int limit, Sync::Semaphore* semaphore)
    throw (eh::Exception)
  {
    limit_ = limit;
    semaphore_ = semaphore;
    if (own_task_runner_)
    {
      task_runner_->activate_object();
    }
  }

  inline
  void
  MTTasker::stop() throw (eh::Exception)
  {
    {
      Sync::PosixGuard guard(mutex_);
      limit_ = 0;
      semaphore_ = 0;
    }
    if (own_task_runner_)
    {
      task_runner_->deactivate_object();
      task_runner_->wait_object();
    }
  }

  //
  // MTTester::FunctorTask class
  //

  template <typename Functor>
  MTTester<Functor>::FunctorTask::FunctorTask(
    Functor functor, MTTasker& tasker)
    throw (eh::Exception)
    : functor_(functor), tasker_(tasker)
  {
  }

  template <typename Functor>
  MTTester<Functor>::FunctorTask::~FunctorTask() throw ()
  {
  }

  template <typename Functor>
  void
  MTTester<Functor>::FunctorTask::execute() throw ()
  {
    try
    {
      if (tasker_.enqueue_conditionally(this))
      {
        functor_();
      }
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Exception " << ex.what();
      tasker_.report_error(ostr.str());
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "Unknown exception";
      tasker_.report_error(ostr.str());
    }
  }


  //
  // MTTester class
  //

  template <typename Functor>
  MTTester<Functor>::MTTester(Functor functor, int threads)
    throw (eh::Exception)
    : tasker_(threads), functor_(functor)
  {
  }

  template <typename Functor>
  MTTester<Functor>::MTTester(Functor functor,
    Generics::TaskRunner_var task_runner)
    throw (eh::Exception)
    : tasker_(task_runner), functor_(functor)
  {
  }

  template <typename Functor>
  void
  MTTester<Functor>::run(int tasks, time_t interval, int limit)
    throw (eh::Exception)
  {
    Sync::Semaphore semaphore(0);
    tasker_.start(limit > 0 ? limit + 1: limit, &semaphore);
    {
      Generics::Task_var task(new FunctorTask(functor_, tasker_));
      while (tasks-- > 0)
      {
        tasker_.enqueue(task);
      }
    }
    if (interval)
    {
      sleep(interval);
    }
    if (limit > 0)
    {
      semaphore.acquire();
    }
    tasker_.stop();
  }


  template <typename Functor>
  bool
  mp_test(Functor functor, int processes) throw (eh::Exception)
  {
    bool result = true;

    typedef std::vector<pid_t> Children;
    Children children;
    children.reserve(processes);

    for (int i = 0; i < processes; i++)
    {
      pid_t pid = fork();
      if (pid < 0)
      {
        result = false;
        break;
      }

      if (!pid)
      {
        functor();
        exit(0);
      }

      children.push_back(pid);
    }

    for (Children::const_iterator itor(children.begin());
      itor != children.end(); ++itor)
    {
      waitpid(*itor, 0, 0);
    }

    return result;
  }
}

#endif
