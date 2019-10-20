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
#include <vector>

#include <Generics/Rand.hpp>

#include <Logger/StreamLogger.hpp>
#include <Logger/ActiveObjectCallback.hpp>

#include <TestCommons/MTTester.hpp>

#include "Application.hpp"


namespace
{
  Logging::FLogger_var logger(new Logging::OStream::Logger(
    Logging::OStream::Config(std::cerr)));
  Generics::ActiveObjectCallback_var callback(
    new Logging::ActiveObjectCallbackImpl(logger));

  const String::SubString MSG_UNKNOWN_EXCEP("Unknown exception");
  const String::SubString MSG_FAILED_TO_WAIT("Failed to wait in wait test");
}

class WorkGenerator
{
public:
  WorkGenerator() throw (eh::Exception);

  void
  operator()() throw (eh::Exception);

  void
  stop() throw (eh::Exception);
private:
  typedef Sync::PosixMutex Mutex_;
  typedef Sync::PosixGuard Guard_;

  Mutex_ mutex_;
  Generics::CompositeActiveObject_var active_objects_composite_;
};

void
WorkGenerator::stop() throw (eh::Exception)
{
  std::cout << "WorkGenerator::stop() " << std::endl;
  Guard_ guard(mutex_);
  active_objects_composite_->clear_children();
}

WorkGenerator::WorkGenerator() throw (eh::Exception)
  : active_objects_composite_(new CompositeActiveObjectImpl)
{
}

void
WorkGenerator::operator()() throw (eh::Exception)
{
  switch(Generics::safe_integral_rand(2))
  {
    case 0 :  // Add active
      {
        Generics::TaskRunner_var tasker(
          new Generics::TaskRunner(callback, 5, 0, 2));
        tasker->activate_object();
        active_objects_composite_->add_child_object(tasker);
      }
      break;
    case 1 :  // Add inactive
      {
        Generics::Planner_var scheduler(
          new Generics::Planner(callback));

        active_objects_composite_->add_child_object(scheduler, true);
      }
      break;
    case 2 :  // Switch state
    case 3 :
      {
        Guard_ guard(mutex_);
        if (active_objects_composite_->active())
        {
          active_objects_composite_->deactivate_object();
          active_objects_composite_->wait_object();
        }
        else
        {
          active_objects_composite_->activate_object();
        }
      }
  }
}

void
TestComposeActors::do_test() throw (eh::Exception, TestFailed)
{
  try
  {
    WorkGenerator worker;
    TestCommons::MTTester<WorkGenerator&> mt_tester(
      worker, 10);

    mt_tester.run(100, 0, 100);
    worker.stop();
    std::cout << "Add functional test SUCCESS" << std::endl;

  }
  catch (...)
  {
    logger->error(MSG_UNKNOWN_EXCEP);
    throw;
  }
}

void
TestComposeActors::do_negative_test() throw (eh::Exception, TestFailed)
{
  using namespace Generics;
  try
  {
    Generics::CompositeActiveObject_var active_objects_composite(
      new CompositeActiveObjectImpl);

    Generics::TaskRunner_var tasker(
      new Generics::TaskRunner(callback, 5, 0, 2));
    active_objects_composite->add_child_object(tasker);
    
    FailActiveObject_var looser(new FailActiveObjectImpl);
    active_objects_composite->add_child_object(looser); // OK
    looser->set_active(true);
    try
    {
      active_objects_composite->add_child_object(looser);
      throw TestFailed("Successfully add inconsistent state object. "
        "Inactive composite, Active object");
    }
    catch (const ActiveObject::Exception&)
    {
    }
    looser->permit_work(true);
    FailActiveObject_var looser2(new FailActiveObjectImpl);
    active_objects_composite->activate_object();
    try
    {
      active_objects_composite->add_child_object(looser2);
      throw TestFailed("Successfully add inconsistent state object. "
        "Active composite, inactive object");
    }
    catch (const ActiveObject::Exception&)
    {
    }
    FailActiveObject_var looser3(new FailActiveObjectImpl);
    looser2->permit_work(true);
    looser3->set_active(true);
    active_objects_composite->add_child_object(looser3);
    try
    {
      active_objects_composite->deactivate_object();
      throw TestFailed("Deactivate not all objects");
    }
    catch (const ActiveObject::Exception&)
    {
    }
    looser3->permit_work(true);
    active_objects_composite->deactivate_object();
    looser3->permit_work(false);
    try
    {
      active_objects_composite->wait_object();
      throw TestFailed("Wait not all objects");
    }
    catch (const ActiveObject::Exception&)
    {
    }
    looser3->permit_work(true);
    active_objects_composite->wait_object();
    looser3->permit_work(false);
    try
    {
      active_objects_composite->activate_object();
      throw TestFailed("Activate not all objects");
    }
    catch (const ActiveObject::Exception&)
    {
    }
    looser3->permit_work(true);
    std::cout << "Finish checks after exceptions" << std::endl;
    active_objects_composite->activate_object();
    active_objects_composite->deactivate_object();
    active_objects_composite->wait_object();
    active_objects_composite->activate_object();
    active_objects_composite->deactivate_object();
    active_objects_composite->deactivate_object();
    active_objects_composite->wait_object();
  }
  catch (const ActiveObject::Exception& e)
  {
    throw TestFailed(e.what());
  }
  catch (const eh::Exception& e)
  {
    Stream::Error ostr;
    ostr << "unexpected exception" << e.what();
    logger->error(ostr.str());
    throw;
  }

}

Waiter::Waiter(Generics::CompositeActiveObject* active_object,
  bool add_child) throw ()
  : ACTIVE_OBJECT_(ReferenceCounting::add_ref(active_object)),
    ADD_CHILD_(add_child), order_(0)
{
}

void
Waiter::operator ()() throw (eh::Exception)
{
  if (__gnu_cxx::__exchange_and_add(&order_, 1))
  {
    Generics::Timer timer;
    timer.start();
    ACTIVE_OBJECT_->wait_object();
    timer.stop();
    if (timer.elapsed_time().tv_sec < 5)
    {
      logger->error(MSG_FAILED_TO_WAIT);
    }
  }
  else
  {
    sleep(10);
    if (ADD_CHILD_)
    {
      ACTIVE_OBJECT_->add_child_object(Generics::ActiveObject_var(
        new Generics::Planner(callback)));
    }
    ACTIVE_OBJECT_->deactivate_object();
  }
}

void
TestComposeActors::do_wait_test() throw (eh::Exception, TestFailed)
{
  try
  {
    Generics::CompositeActiveObject_var active_object(
      new CompositeActiveObjectImpl);
    for (int i = 0; i < 3; i++)
    {
      active_object->activate_object();
      Waiter waiter(active_object, i);
      TestCommons::MTTester<Waiter&> tester(waiter, 3);
      tester.run(3, 0, 3);
    }
  }
  catch (const Generics::ActiveObject::Exception& ex)
  {
    throw TestFailed(ex.what());
  }
  catch (const eh::Exception& ex)
  {
    Stream::Error ostr;
    ostr << "unexpected exception" << ex.what();
    logger->error(ostr.str());
    throw;
  }
}

int
main()
{
  std::cout << "CompositeActiveObject functional test started.."
    << std::endl;
  try
  {
    TestComposeActors tester;
    tester.do_test();
    tester.do_negative_test();
    tester.do_wait_test();
    std::cout << "SUCCESS" << std::endl;
  }
  catch (const TestComposeActors::TestFailed& e)
  {
    std::cerr << "FAIL: " << e.what() << std::endl;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "FAIL std: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "FAIL: unknown exception raised" << std::endl;
  }

  return 0;
}
