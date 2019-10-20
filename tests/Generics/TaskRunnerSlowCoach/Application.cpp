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



// Application.cpp

#include <ctime>
#include <TestCommons/MTTester.hpp>

#include "Application.hpp"

using namespace Generics;

class TestEmptyTask : public Generics::TaskImpl
{
  virtual void
  execute() throw ();
};

void
TestEmptyTask::execute() throw ()
{
  // empty task
}

struct TasksSpreader
{
  TasksSpreader(
    TaskRunner_var task_runner,
    const Time& next_time)
    throw (eh::Exception);

  void
  operator()() throw (eh::Exception);

  std::size_t
  get_count() const throw ();

private:
  Generics::TaskRunner_var task_runner_;
  const Time NEXT_TIME_;
  volatile _Atomic_word tasks_counter_;
};

TasksSpreader::TasksSpreader(
  TaskRunner_var task_runner,
  const Time& next_time)
  throw (eh::Exception)
  : task_runner_(task_runner),
    NEXT_TIME_(next_time),
    tasks_counter_(0)
{
}

void
TasksSpreader::operator()() throw (eh::Exception)
{
  __gnu_cxx::__atomic_add(&tasks_counter_, 1);
  task_runner_->enqueue_task(Generics::Task_var(new TestEmptyTask),
                        &NEXT_TIME_);
}

std::size_t
TasksSpreader::get_count() const throw ()
{
  return tasks_counter_;
}

void
TestTasker::do_test() throw (eh::Exception)
{
  try
  {
    struct TestParams
    {
      const std::size_t THREADS_AMOUNT;
      const std::size_t QUEUE_LIMIT;
      const Time DURATION;
    };

    //    const std::size_t TASKS_AMOUNT = 10;
    const TestParams TEST_PARAMS[] =
    {
      {2, 5, Time(1)},
    };
    // + 10 for detection waitlocks.
    std::size_t wait_lock_gap = 10;
    for (std::size_t j = 0;
      j < sizeof(TEST_PARAMS) /
      sizeof(TEST_PARAMS[0]);
    ++j)
    {
      spawn_tasker_(TEST_PARAMS[j].THREADS_AMOUNT,
        TEST_PARAMS[j].QUEUE_LIMIT);

      Time now = Time::get_time_of_day();
      const Generics::Time NEXT_TIME(
        now + TEST_PARAMS[j].DURATION + wait_lock_gap);

      TasksSpreader tasks_spreader(task_runner_, NEXT_TIME);

      std::cout << "Original duration="
        << TEST_PARAMS[j].DURATION.tv_sec << std::endl;

      TestCommons::MTTester<TasksSpreader&> mt_tester(
        tasks_spreader, 5);

      CPUTimer timer;
      timer.start();
      mt_tester.run(1, TEST_PARAMS[j].DURATION.tv_sec);
      timer.stop();
      // Test fail if we have overflows and 
      // execution time  > 1 + wait_lock_gap seconds
      // because queueing threads are locking on enqueue task.

      std::cout << "Start time = " << now.get_local_time() << std::endl;
      std::cout << "Put " << tasks_spreader.get_count() << " tasks." << std::endl;
      std::cout << "Acquire " 
        << timer.elapsed_time() << std::endl;

    } // for
  }
  catch (...)
  {
    std::cerr << "Unknown exception" << std::endl;
    throw;
  }
}

int
main()
{
  std::cout << "TaskRunner performance tests started.." << std::endl;
  try
  {
    TestTasker tasker;
    tasker.do_test();
    std::cout << "Test complete" << std::endl;
  }
  catch (...)
  {
    std::cerr << "FAIL: unknown exception raised" << std::endl;
  }

  return 0;
}
