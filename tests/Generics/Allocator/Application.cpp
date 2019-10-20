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



#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include <eh/Exception.hpp>
#include <Generics/ArrayAutoPtr.hpp>
#include <Generics/MemBuf.hpp>
#include <Generics/Rand.hpp>
#include <Generics/Time.hpp>

#include <Stream/MemoryStream.hpp>

#include <TestCommons/MTTester.hpp>

using namespace Generics;

namespace
{
  DECLARE_EXCEPTION(TestException, eh::DescriptiveException);

  // changing required additional code modification.
  const std::size_t BUFFERS_AMOUNT = 200;//32*32;
  const std::size_t METERS = 1;

  // Alignment for allocators that support alignment.
  const std::size_t ALLOC_BLOCK_SIZE = 512;

  using namespace Allocator;

  // Using in test for MemBuf creation.
  Base_var current_test_allocator;

  const char* NAME_ALLOCATORS[] =
  {
    "Default",
    "PoolBlocksAllocator",
    "PoolListAllocator",
    "PoolUniversal",
    "Align",
  };

}

class TestStategyGenerator : Uncopyable
{
public:
  struct Memory 
  {
    std::size_t high;
    std::size_t low;
    std::size_t value;
    void
    swap(Memory& right) throw ();
  };

  typedef std::vector<Memory> TestStrategy;
  typedef std::vector<TestStrategy> AllStrategies;

  TestStategyGenerator() throw ();

  void
  generate_test_strategy(std::size_t low,
    std::size_t high,
    std::size_t threads = 1) throw (eh::Exception);

  const AllStrategies&
  get() const throw ();

private:

  /**
   * Create and store random sequence, used for mixing later.
   */
  class RandomOnceAtRun
  {
  public:
    /**
     * Create mixers for one thread (random data using for hashing sequence for
     * one thread.
     */
    RandomOnceAtRun(std::size_t random_data_len) throw ();

    std::size_t
    operator() (std::size_t pos) const throw ();
  private:

    typedef std::vector<std::size_t> RandomData;
    RandomData random_at_once_;
  };

  std::size_t low_;
  std::size_t high_;

  AllStrategies memories_;
};

TestStategyGenerator::TestStategyGenerator() throw ()
  :  low_(0), high_(0)
{
}

void
TestStategyGenerator::Memory::swap(Memory& right) throw ()
{
  std::swap(high, right.high);
  std::swap(low, right.low);
  std::swap(value, right.value);
}

TestStategyGenerator::RandomOnceAtRun::RandomOnceAtRun(
  std::size_t random_data_len) throw ()
{
  random_at_once_.reserve(random_data_len);
  for (std::size_t i = 1; i < random_data_len; ++i)
  {
    random_at_once_.push_back( safe_rand(0, i) );
  }
}

std::size_t
TestStategyGenerator::RandomOnceAtRun::operator() (std::size_t pos) const
  throw ()
{
  return random_at_once_[pos - 2];
}

const TestStategyGenerator::AllStrategies&
TestStategyGenerator::get() const throw ()
{
  return memories_;
}

void
TestStategyGenerator::generate_test_strategy(std::size_t low,
                                             std::size_t high,
                                             std::size_t threads)
  throw (eh::Exception)
{
  low_ = low;
  high_ = high;
  memories_.clear();
  memories_.resize(threads);
  std::size_t thread_strategy_len = BUFFERS_AMOUNT / threads;
  for (std::size_t i = 0; i < thread_strategy_len; ++i)
  {
    Memory m;
    m.low = low;
    m.high = high;
    m.value = low + static_cast<size_t>
      ((high - low) * static_cast<double>(i) / thread_strategy_len);

    for (std::size_t j = 0 ; j < threads; ++j)
    {
      memories_[j].push_back(m);
    }
  }
  // mixing part
  
  for (std::size_t j = 0 ; j < threads; ++j)
  {
    RandomOnceAtRun mixer(thread_strategy_len);
    std::random_shuffle(memories_[j].begin(), memories_[j].end(), mixer);
  }
  std::cout << "Created test scenario length=" << thread_strategy_len
    << " for " << threads << " threads." << std::endl;
}

class MultiThreadPerformanceTest
{
public:
  MultiThreadPerformanceTest(std::size_t meters,
    std::size_t buffers_amount,
    const TestStategyGenerator::AllStrategies& ref)
    throw ();

  void
  operator()() throw (eh::Exception);

  /**
   * Should reset multiplexer before new test cycle.
   */
  void
  reset() throw ();

private:
  const std::size_t METERS_;
  const std::size_t BUFFERS_AMOUNT_;
  const TestStategyGenerator::AllStrategies& STRATEGY_;

  // use it for choosing task data vector by some thread.
  volatile _Atomic_word multiplexor_;
};

MultiThreadPerformanceTest::MultiThreadPerformanceTest(
  std::size_t meters,
  std::size_t threads,
  const TestStategyGenerator::AllStrategies& ref) throw ()
  : METERS_(meters),
    BUFFERS_AMOUNT_(BUFFERS_AMOUNT / threads),
    STRATEGY_(ref),
    multiplexor_(0)
{
}

void
MultiThreadPerformanceTest::operator()() throw (eh::Exception)
{
  std::size_t my_strategy = __gnu_cxx::__exchange_and_add(&multiplexor_, 1);
  const TestStategyGenerator::TestStrategy& buffer_sizes =
    STRATEGY_[my_strategy];
  for (std::size_t j = 0; j < METERS_; ++j)
  {
    for (std::size_t i = 0; i < BUFFERS_AMOUNT_; ++i)
    {
      Generics::MemBuf tmp(buffer_sizes[i].value,
        current_test_allocator.in());
      Generics::MemBuf tmp2(buffer_sizes[i].value + 377,
        current_test_allocator.in());
      tmp = std::move(tmp2);
      tmp2 = Generics::MemBuf(tmp);
      Generics::MemBuf tmp3(tmp);
    }
  }
}

void
MultiThreadPerformanceTest::reset() throw ()
{
  multiplexor_ = 0;
}

void
do_performance_test(std::size_t threads,
                    std::size_t low,
                    std::size_t high,
                    TestStategyGenerator& strategist) throw (eh::Exception)
{
  std::cout << "\n\tSTART performance metering for " << threads
    << " threads." << std::endl;

  Generics::Allocator::Base_var ALLOCATORS[] =
  {
    Generics::Allocator::Base_var(new Default),
    Generics::Allocator::Base_var(new ConstSizeArray(100, 1024 * 1024)),
    Generics::Allocator::Base_var(new VarSizeList(64 * 1024, 100)),
    Generics::Allocator::Base_var(new Universal),
    Generics::Allocator::Base_var(new Align)
  };

  strategist.generate_test_strategy(low, high, threads);
  MultiThreadPerformanceTest mtt(METERS, threads, strategist.get());

  TestCommons::MTTester<MultiThreadPerformanceTest&>
    mt_tester(mtt, threads);

  std::cout << "LOW=" << low << ", HIGH=" << high
    << std::endl;

  CPUTimer timer;

  std::cout << std::setfill(' ');
  std::cout.setf(std::ios::left);
  std::cout.width(28);
  std::cout << "Allocator" << '|';
  std::cout.width(10);
  std::cout << "Time" << std::endl;

  Time fake_allocators_time[4];

  for (std::size_t i = 0;
    i < sizeof(ALLOCATORS) / sizeof(ALLOCATORS[0]);
    ++i)
  {
    current_test_allocator = ALLOCATORS[i];
    std::cout.width(28);
    std::cout << std::setfill(' ') //<< std::setiosflags(std::ios::left)
      << NAME_ALLOCATORS[i] << '|';
    mtt.reset();
    timer.start();
    mt_tester.run(threads, 0, threads);
    timer.stop();

    Time current_duration;
    if (i < 16)
    {
      current_duration = timer.elapsed_time() - fake_allocators_time[i % 4];
    }
    std::cout << current_duration <<std::endl;

    std::cout << "Cached: " << current_test_allocator->cached() <<
      " Detailed: ";
    current_test_allocator->print_cached(std::cout);
    std::cout << std::endl;

    ALLOCATORS[i].reset();
  }

}

void
collect_statistics() throw (eh::Exception)
{
  TestStategyGenerator  strategist;

  struct Task
  {
    std::size_t low;
    std::size_t high;
  };
  
  Task tasks[] =
  {
    {8 * 1024, 16 * 1024}, {8 * 1024, 32 * 1024},
    {8 * 1024, 64 * 1024},{16 * 1024, 32 * 1024}, {64 * 1024, 256 * 1024},

/*    {64 * 1024, 128 * 1024},
    {64 * 1024, 64 * 1024},
    {64 * 1024, 128 * 1024},
    {64 * 1024, 256 * 1024},
    {64 * 1024, 512 * 1024},
    {64 * 1024, 1024 * 1024},
    {128 * 1024, 128 * 1024},
    {128 * 1024, 256 * 1024},
    {128 * 1024, 512 * 1024},
    {128 * 1024, 1024 * 1024},
    {256 * 1024, 256 * 1024},
    {256 * 1024, 512 * 1024},
    {256 * 1024, 1024 * 1024},*/

  };

  for (std::size_t i = 0; i < sizeof(tasks) / sizeof(tasks[0]); ++i)
  {
    do_performance_test(1, tasks[i].low, tasks[i].high, strategist);
    do_performance_test(4, tasks[i].low, tasks[i].high, strategist);
    do_performance_test(8, tasks[i].low, tasks[i].high, strategist);
    do_performance_test(16, tasks[i].low, tasks[i].high, strategist);
  }
}

int
main()
{
  std::cout << "MemBuf test started" << std::endl;
  
  try
  {
    std::cout << "Count of elemental test to perform " << BUFFERS_AMOUNT
      << std::endl;

    std::cout << "Test passes " << METERS << std::endl;

    collect_statistics();
    
    std::cout << "Test complete" << std::endl;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "FAIL:" << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "unknown exception" << std::endl;
  }

  return 0;
}
