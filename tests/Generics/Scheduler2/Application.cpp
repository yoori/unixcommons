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



#include <ext/atomicity.h>

#include <iostream>

#include <TestCommons/ActiveObjectCallback.hpp>

#include "Application.hpp"

namespace Generics
{
/*
  struct PrintMessage:
    public Generics::Goal,
    public ReferenceCounting::AtomicImpl
  {
    PrintMessage(const char* msg_val): msg(msg_val) {}

    virtual ~PrintMessage() throw() {}

    virtual void deliver() throw ()
    {
      if(!msg.empty())
      {
        std::cout << Generics::Time::get_time_of_day() << ": message: " << msg << " delivered." << std::endl;
      }
    }

    std::string msg;
  };
*/
  /**/
  Application::Application() throw(eh::Exception)
  {
    try
    {
      callback_ = new TestCommons::ActiveObjectCallbackStreamImpl(
        std::cerr, "Scheduler2");
      scheduler1_ = new Planner(callback_);
//    scheduler2_ = new TestPlanner(this);
    }
    catch(const eh::Exception& e)
    {
      Stream::Error ostr;
      ostr << "Application::init: eh::Exception caught. Description:" <<
        std::endl << e.what();
      throw Exception(ostr);
    }
  }

  struct CheckMessage:
    public Generics::Goal,
    public ReferenceCounting::DefaultImpl<>
  {
    CheckMessage(unsigned long deliver_count)
      : deliver_count_(deliver_count)
    {}

    virtual void deliver() throw ()
    {
      __gnu_cxx::__atomic_add(&deliver_count_, -1);
    }

    bool done()
    {
      return deliver_count_ == 0;
    }

  private:
    volatile _Atomic_word deliver_count_;
  };

  typedef ReferenceCounting::QualPtr<CheckMessage> CheckMessage_var;

  void
  Application::run()
    throw (Exception, eh::Exception)
  {
    try
    {
      std::cout << std::endl << "Running test ..." << std::endl;

      scheduler1_->activate_object();
      CheckMessage_var msg1(new CheckMessage(1));
      CheckMessage_var msg2(new CheckMessage(1));
      try
      {
        Generics::Time start_time = Generics::Time::get_time_of_day();

        {
          std::cout
            << Generics::Time::get_time_of_day()
            << ": Bug scenarios: 1 sec and must appear message ..." << std::endl;

          scheduler1_->schedule(msg1, start_time + 30);
          sleep(1);
          scheduler1_->schedule(msg2, start_time + 10);
        }

        sleep(11);
        if(!msg2->done())
        {
          throw Exception("Msg 2 didn't delivered.");
        }

        std::cout << "Msg 2 delivered." << std::endl;

        sleep(20);
        if(!msg1->done())
        {
          throw Exception("Msg 1 didn't delivered.");
        }

        std::cout << "Msg 1 delivered." << std::endl;

        scheduler1_->deactivate_object();
        scheduler1_->wait_object();
      }
      catch (...)
      {
        scheduler1_->deactivate_object();
        scheduler1_->wait_object();
        throw;
      }
    }
    catch(const eh::Exception& e)
    {
      Stream::Error ostr;
      ostr << "Application::run: eh::Exception caught. Description:" <<
        std::endl << e.what();
      throw Exception(ostr);
    }
  }
}

int
main() throw ()
{
  int result = 1;

  try
  {
    Generics::Application app;
    app.run();
    result = 0;
  }
  catch(const Generics::Application::Exception& e)
  {
    std::cerr << "main: Generics::Application::Exception exception caught. "
      "Description:" << std::endl << e.what() << std::endl;
  }
  catch(const eh::Exception& e)
  {
    std::cerr << "main: eh::Exception exception caught. "
      "Description:" << std::endl << e.what() << std::endl;
  }

  return result;
}
