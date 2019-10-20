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





#include <Generics/TaskRunner.hpp>

//#define BUILD_WITH_DEBUG_MESSAGES
#include "Trace.hpp"


namespace Generics
{
  //
  // TaskRunner::TaskRunnerJob class
  //

  TaskRunner::TaskRunnerJob::TaskRunnerJob(ActiveObjectCallback* callback,
    unsigned number_of_threads, unsigned max_pending_tasks)
    throw (eh::Exception)
    : SingleJob(callback),
      NUMBER_OF_THREADS_(number_of_threads), number_of_unused_threads_(0),
      new_task_(0),
      not_full_(std::min<unsigned>(max_pending_tasks, SEM_VALUE_MAX)),
      LIMITED_(max_pending_tasks)
  {
  }

  TaskRunner::TaskRunnerJob::~TaskRunnerJob() throw ()
  {
  }

  void
  TaskRunner::TaskRunnerJob::started(unsigned threads) throw ()
  {
    number_of_unused_threads_ = threads;
  }

  void
  TaskRunner::TaskRunnerJob::clear() throw (eh::Exception)
  {
    Sync::PosixGuard guard(mutex());
    if (LIMITED_)
    {
      for (size_t i = tasks_.size(); i; i--)
      {
        not_full_.release();
      }
    }
    tasks_.clear();
  }

  void
  TaskRunner::TaskRunnerJob::enqueue_task(Task* task,
    const Time* timeout, ThreadRunner& thread_runner)
    throw (InvalidArgument, Overflow, NotActive, eh::Exception)
  {
    if (!task)
    {
      Stream::Error ostr;
      ostr << FNS << "task is NULL";
      throw InvalidArgument(ostr);
    }

    // Producer
    if (LIMITED_)
    {
      if (!(timeout ? not_full_.timed_acquire(timeout) :
        not_full_.try_acquire()))
      {
        Stream::Error ostr;
        ostr << FNS << "TaskRunner overflow";
        throw Overflow(ostr);
      }
    }

    {
      Sync::PosixGuard guard(mutex());
      try
      {
        tasks_.push_back(Task_var(ReferenceCounting::add_ref(task)));
      }
      catch (...)
      {
        if (LIMITED_)
        {
          not_full_.release();
        }
        throw;
      }
      add_thread(thread_runner);
    }

    // Wake any working thread
    new_task_.release();
  }

  void
  TaskRunner::TaskRunnerJob::wait_for_queue_exhausting()
    throw (eh::Exception)
  {
    for (;;)
    {
      {
        Sync::PosixGuard guard(mutex());
        if (tasks_.empty())
        {
          return;
        }
      }
      Generics::Time wait(0, 300000);
      select(0, 0, 0, 0, &wait);
    }
  }

  void
  TaskRunner::TaskRunnerJob::work() throw ()
  {
    try
    {
      for (;;)
      {
        Task_var task;
        {
          new_task_.acquire();
          Sync::PosixGuard guard(mutex());
          if (is_terminating())
          {
            break;
          }
          if (tasks_.empty())
          {
            continue;
          }
          task = tasks_.front();
          tasks_.pop_front();
          number_of_unused_threads_--;
        }
        // Tell any blocked thread that the queue is ready for a "new item"
        if (LIMITED_)
        {
          not_full_.release();
        }
        try
        {
          task->execute();
        }
        catch (const eh::Exception& ex)
        {
          callback()->error(String::SubString(ex.what()));
        }

        Sync::PosixGuard guard(mutex());
        number_of_unused_threads_++;
      }
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "eh::Exception: " << ex.what();
      callback()->critical(ostr.str());
    }
  }

  void
  TaskRunner::TaskRunnerJob::add_thread(ThreadRunner& thread_runner)
    throw ()
  {
    if (!thread_runner.running() ||
      thread_runner.running() == thread_runner.number_of_jobs())
    {
      return;
    }

    if (tasks_.size() <= number_of_unused_threads_)
    {
      return;
    }

    try
    {
      thread_runner.start_one();
      number_of_unused_threads_++;
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "eh::Exception: " << ex.what();
      callback()->warning(ostr.str());
    }
  }

  void
  TaskRunner::TaskRunnerJob::terminate() throw ()
  {
    for (unsigned long i = NUMBER_OF_THREADS_; i; i--)
    {
      new_task_.release();
    }
  }


  //
  // TaskRunner class
  //

  TaskRunner::TaskRunner(ActiveObjectCallback* callback,
    unsigned threads_number, size_t stack_size,
    unsigned max_pending_tasks, unsigned start_threads)
    throw (InvalidArgument, Exception, eh::Exception)
    : ActiveObjectCommonImpl(
        TaskRunnerJob_var(new TaskRunnerJob(callback, threads_number,
          max_pending_tasks)),
        threads_number, stack_size, start_threads),
      job_(static_cast<TaskRunnerJob&>(*SINGLE_JOB_))
  {
  }

  TaskRunner::~TaskRunner() throw ()
  {
  }
}
