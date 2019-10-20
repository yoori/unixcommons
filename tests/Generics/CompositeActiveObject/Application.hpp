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



// Application.hpp

/**
 * We are testing container for active objects
 * Functional test scenario:
 *  1. Use two usual active objects TaskRunner + Scheduler
 *  2. Simultaneously do N times next procedure:
 *    random choice one of following
 *    *) Container->add_active(TaskRunner)
 *    *) Container->add_inactive(Scheduler)
 *    *) turn Container state. If Active -> Inactive,
 *       if Inactive -> Active.
 *   This set of operations do in the random order
 *  give us all combinations of test cases.
 *  3. Container->deactivate, Container->wait. Test done.
 */
#ifndef _TEST_APPLICATION_TASK_RUNNER_HPP_INCLUDED_
#define _TEST_APPLICATION_TASK_RUNNER_HPP_INCLUDED_ 

#include <Generics/TaskRunner.hpp>
#include <Generics/CompositeActiveObject.hpp>

class CompositeActiveObjectImpl :
  public Generics::CompositeActiveObject,
  public virtual ReferenceCounting::AtomicImpl
{
};

class TestComposeActors
{
public:
  DECLARE_EXCEPTION(TestFailed, eh::DescriptiveException);

  virtual
  ~TestComposeActors() throw ();

  void
  do_test() throw (eh::Exception, TestFailed);

  void
  do_negative_test() throw (eh::Exception, TestFailed);

  void
  do_wait_test() throw (eh::Exception, TestFailed);

private:
  Generics::TaskRunner_var task_runner_;
  Generics::CompositeActiveObject_var active_objects_;
};

//
// class FailingActiveObjectImpl
//  Failed all active operations instead status check active()
//
// Use for negative test. Scenario:
// 1. Add ActiveObject to Composite
// 2. Add FailActiveObject, 4 cases:
//   Composite | FailActiveObject |
//    Passive  | Passive          | OK
//    Passive  | Active           | FAIL
//    Active   | Passive          | FAIL
//    Active   | Active           | OK
// 3. Add ActiveObject to Composite, its Active now.
// 4. Check demands on behavior 
//      Composite->deactivate() FAIL
//      FailActiveObject->permit(); Composite->deactivate() OK
//      Composite->wait() FAIL
//      FailActiveObject->permit(); Composite->wait() OK
//      Composite->activate() FAIL
//      FailActiveObject->permit(); // final, definitive
//      Composite->activate() OK
//      Composite->deactivate() Composite->wait() OK
//      Composite->activate() OK
//      Composite->deactivate() OK
//      Composite->deactivate() OK
//      Composite->wait() OK
//

class FailActiveObjectImpl : public Generics::ActiveObject,
  public virtual ReferenceCounting::AtomicImpl
{
public:
  FailActiveObjectImpl() throw ();

  virtual void
  activate_object()
    throw (ActiveObject::AlreadyActive, Exception, eh::Exception);

  virtual void
  deactivate_object()
    throw (Exception, eh::Exception);

  virtual void
  wait_object() throw (Exception, eh::Exception);

  virtual bool
  active() throw (eh::Exception);

  void
  permit_work(bool new_status) throw ();

  void
  set_active(bool new_status) throw ();
protected:
  virtual
  ~FailActiveObjectImpl() throw ();
private:
  bool permit_pass_;
  bool active_;
};

typedef ReferenceCounting::QualPtr<FailActiveObjectImpl>
  FailActiveObject_var;

class Waiter
{
public:
  Waiter(Generics::CompositeActiveObject* active_object, bool add_child)
    throw ();
  void
  operator ()() throw (eh::Exception);

private:
  ReferenceCounting::FixedPtr<Generics::CompositeActiveObject>
    ACTIVE_OBJECT_;
  const bool ADD_CHILD_;
  volatile _Atomic_word order_;
};

//////////////////////////////////////////////////////////////////////////
// Implementations

//
// class FailActiveObjectImpl
//

FailActiveObjectImpl::FailActiveObjectImpl() throw ()
  : permit_pass_(false), active_(false)
{
}

FailActiveObjectImpl::~FailActiveObjectImpl() throw ()
{
}

void
FailActiveObjectImpl::activate_object()
  throw (ActiveObject::AlreadyActive, Exception, eh::Exception)
{
  if (!permit_pass_)
  {
    throw Exception("Negative test fail");
  }
  active_ = true;
}

void
FailActiveObjectImpl::deactivate_object()
  throw (Exception, eh::Exception)
{
  if (!permit_pass_)
  {
    throw Exception("Negative test fail");
  }
}

void
FailActiveObjectImpl::wait_object() throw (Exception, eh::Exception)
{
  if (!permit_pass_)
  {
    throw Exception("Negative test fail");
  }
  active_ = false;
}

bool
FailActiveObjectImpl::active() throw (eh::Exception)
{
  return active_;
}

void
FailActiveObjectImpl::permit_work(bool new_status) throw ()
{
  permit_pass_ = new_status;
}

void
FailActiveObjectImpl::set_active(bool new_status) throw ()
{
  active_ = new_status;
}

//////////////////////////////////////////////////////////////////////////

TestComposeActors::~TestComposeActors() throw ()
{
  if (task_runner_.in())
  {
    task_runner_->deactivate_object();
    task_runner_->wait_object();
  }
}

#endif  // _TEST_APPLICATION_TASK_RUNNER_HPP_INCLUDED_
