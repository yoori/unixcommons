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

#include <iostream>
#include <pthread.h>
#include "Application.hpp"

// Consumer - Producer test
// Testing base functional

using Sync::Conditional;
using Sync::ConditionalGuard;

ConsumerProducer::ThreadContext::ThreadContext(
  ConsumerProducer *this_ptr_val) throw()
  : this_ptr(this_ptr_val), work_done_stat(0)
{
}

ConsumerProducer::ConsumerProducer(std::size_t max_item_count,
  std::size_t producer_threads_count)
  throw(Conditional::Exception)
  : MAX_ITEM_COUNT_(max_item_count), next_value_(0), ready_number_(0)
{
  // we must avoid reallocations, to ensure
  // the delivery of the correct context object into the stream.
  // Deliver reference on context per se.
  threads_.reserve(producer_threads_count+1);

  ThreadContext thread_context(this);

//  For creation producer thread at first.
//  Uncomment, shift cycle +1, comment after cycle.
//    threads_.push_back(thread_context);
//    ThreadsContainer::reference stored_context = threads_.at(0);
//    pthread_create(&stored_context.thread, 0, consumer, this);

  for (std::size_t i = 0; i < producer_threads_count; ++i)
  {
    threads_.push_back(thread_context);
    ThreadsContainer::reference stored_context = threads_.at(i);
    pthread_create(&stored_context.thread, 0, producer, &stored_context);
  }

  threads_.push_back(thread_context);
  ThreadsContainer::reference stored_context =
    threads_.at(producer_threads_count);
  pthread_create(&stored_context.thread, 0, consumer, this);
}

ConsumerProducer::~ConsumerProducer() throw()
{
  for(std::size_t i = 0; !threads_.empty(); threads_.pop_back())
  {
    ThreadsContainer::reference stored_context = threads_.back();
    pthread_join(stored_context.thread, 0);
    std::cout << ++i << " done "
      << stored_context.work_done_stat << " products." << std::endl;
  }
  std::cout << std::endl;
}

void
ConsumerProducer::producer(std::size_t &work_stat)
  throw(Conditional::Exception)
{
  for(;;)
  {
    { // Produce
      Sync::PosixGuard guard(mutex_);
      if (buffer_.size() >= MAX_ITEM_COUNT_)
      {
        return; // array full, we're done all.
      }
      buffer_.push_back(next_value_);
      ++next_value_;
      ++work_stat;
    }

    {
    ConditionalGuard condition(cond_);
    if (ready_number_ == 0)
    {
      cond_.signal();  // consumer, please, wake up - begin or continue work
    }
    ++ready_number_;
    }
  }
}

void *
ConsumerProducer::producer(void *arg) throw()
{
  try
  {
    ThreadContext &thread_context = *static_cast<ThreadContext*>(arg);
    thread_context.this_ptr->producer(thread_context.work_done_stat);
  }
  catch (...)
  {
  }
  return 0;
}

void
ConsumerProducer::consumer()
  throw(Conditional::Exception)
{
  for(std::size_t i = 0; i < MAX_ITEM_COUNT_; ++i)
  {
    {
      ConditionalGuard condition(cond_);
      while(ready_number_ == 0)
      {
        condition.wait();
      }
      --ready_number_;
    }

    {
      Sync::PosixGuard guard(mutex_);
      if (buffer_[i] != i)  // test SUCCESS criteria
      {
        std::cerr << "buffer[" << i << ']' << buffer_[i] << std::endl;
      }
    }
  }
  std::cout << "All consumed. Consumed " << buffer_.size()
    << " elements." << std::endl;
}

void *
ConsumerProducer::consumer(void *arg) throw()
{
  try
  {
    static_cast<ConsumerProducer*>(arg)->consumer();
  }
  catch (...)
  {
  }
  return 0;
}

int
main(int /*argc*/, char** /*argv*/)
{
  std::cout << "Conditional variable tests started.." << std::endl;
  {
    ConsumerProducer cp(1000, 1);
  }
  {
    ConsumerProducer cp(10000, 10);
  }

  return 0;
}
