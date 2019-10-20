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
#include "Application.hpp"

const Generics::Time TINFINITY(Generics::Time::get_time_of_day() + 100000);

class TestTask1s : public Generics::TaskImpl
{
  virtual void
  execute() throw ();
};

void
TestTask1s::execute() throw ()
{
  sleep(1);
  std::cout << "task1s done" << std::endl;
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
      const std::size_t PLANNED_OVERFLOW;
    };
    
    const std::size_t TASKS_AMOUNT = 10;
    const TestParams TEST_PARAMS[] =
    {
      {1, 1, TASKS_AMOUNT - 2}, {1, 5, TASKS_AMOUNT - 6},
      {5, 1, TASKS_AMOUNT - 6}, {1, 0, 0}, {5, 0, 0}
    };
    std::size_t overflows_counter;

    for (std::size_t j = 0;
      j < sizeof(TEST_PARAMS) /
          sizeof(TEST_PARAMS[0]);
      ++j)
    {
      overflows_counter = 0;
      spawn_tasker_(TEST_PARAMS[j].THREADS_AMOUNT,
        TEST_PARAMS[j].QUEUE_LIMIT);
      const Generics::Time NEXT_TIME(
        Generics::Time::get_time_of_day() + Generics::Time(0, 500000));
      for ( std::size_t i = 0; i < TASKS_AMOUNT; ++i)
      {
        try
        {
          // throw exception if overdue task put.
          task_runner_->enqueue_task(Generics::Task_var(new TestTask1s),
            &NEXT_TIME);
          std::cout << "enqueued " << i << std::endl; 
        }
        catch (const Generics::TaskRunner::Overflow& e)
        {
          // check FAIL. Thresholds not pass!
          const Generics::Time NOW = Generics::Time::get_time_of_day();
          (NOW + Generics::Time(0,100000) < NEXT_TIME ?
            std::cerr << "Timeout wasn't reached, time lag="
            << NEXT_TIME - NOW << ", overflows amount=" 
            << overflows_counter + 1 << std::endl
            << "NEXT_TIME=" << NEXT_TIME << ", NOW="
            << NOW << std::endl
            : std::cout)
            << " Phase " << j << ": overflowed " << i << " " << e.what()
            << std::endl;
          ++overflows_counter;
        }
      }
      std::cout << "Phase " << j << ": "
        << overflows_counter << " overflows." << std::endl;
      if (TEST_PARAMS[j].PLANNED_OVERFLOW != overflows_counter)
      {
        std::cerr << 
          "FAIL: Incorrect occurred overflows number " << overflows_counter
          << ", expected " << TEST_PARAMS[j].PLANNED_OVERFLOW <<
          "TaskRunner threads=" << TEST_PARAMS[j].THREADS_AMOUNT
          << " queue limit=" << TEST_PARAMS[j].QUEUE_LIMIT << std::endl
          << std::endl;
      }

      // Infinity timeout tests
      std::cout << "Infinity part" << std::endl;
      overflows_counter = 0;
      for ( std::size_t i = 0; i < TASKS_AMOUNT; ++i)
      {
        try
        {
          task_runner_->enqueue_task(Generics::Task_var(new TestTask1s),
            &TINFINITY);
          std::cout << "enqueued " << i << std::endl; 
        }
        catch (const Generics::TaskRunner::Overflow& e)
        {
          std::cout << "Overflowed " << i << " " << e.what() << std::endl;
          ++overflows_counter;
        }
      }
      if (overflows_counter)
      {
        std::cerr << "Infinity Phase " << j
          << " FAIL: were " << overflows_counter
          << " overflows. "
          << "But overflow impossible when infinity awaiting queue."
          << " Must be zero." << std::endl;
      }
      overflows_counter = 0;
    } // for
  }
  catch (...)
  {
    std::cerr << "Unknown exception" << std::endl;
    throw;
  }
}

void
TestTasker::do_release_queue_test() throw (eh::Exception)
{
  std::cout << "Test queue releasing" << std::endl;
  spawn_tasker_(1, 1);
  Generics::Timer timer;
  const Generics::Time NEXT_TIME = Generics::Time::get_time_of_day() + 1;
  // fill queue
  timer.start();
  task_runner_->enqueue_task(Generics::Task_var(new TestTask1s),
    &NEXT_TIME);
  task_runner_->enqueue_task(Generics::Task_var(new TestTask1s),
    &TINFINITY);
  timer.stop();
  timer.start();
  // Infinity blocking until queue released
  timer.start();
  task_runner_->enqueue_task(Generics::Task_var(new TestTask1s),
    &TINFINITY);
  timer.stop();
  (timer.elapsed_time() < Generics::Time(0,900000) ||
    timer.elapsed_time() > Generics::Time(1,500000) ?
      std::cerr << "FAIL: " : std::cout << "Result: ")
    << "Release waiting for " << timer.elapsed_time()
    << std::endl;
}

int
main()
{
  std::cout << "TaskRunner performance tests started.." << std::endl;
  try
  {
    TestTasker tasker;
    tasker.do_test();
    tasker.do_release_queue_test();
    std::cout << "SUCCESS" << std::endl;
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "FAIL: " << ex.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "FAIL: unknown exception raised" << std::endl;
  }

  return 0;
}
