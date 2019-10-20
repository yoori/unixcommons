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



#ifndef _CORBA_CRASHCALL_APPLICATION_HPP_
#define _CORBA_CRASHCALL_APPLICATION_HPP_

#include <eh/Exception.hpp>

#include <CORBACommons/ProcessControlImpl.hpp>
#include <CORBACommons/CorbaAdapters.hpp>

#include "TestCrash_s.hpp"


namespace CORBATest
{
  class TestCrashImpl :
    virtual public CORBACommons::ReferenceCounting::ServantImpl<
      POA_CORBATest::TestCrash>
  {
  public:
    virtual void
    crash() throw ();

  protected:
    virtual
    ~TestCrashImpl() throw ();
  };
  typedef ReferenceCounting::QualPtr<TestCrashImpl> TestCrashImpl_var;
}

inline
CORBATest::TestCrashImpl::~TestCrashImpl() throw ()
{
}

inline void
CORBATest::TestCrashImpl::crash() throw ()
{
  _exit(1);
}

class Application :
  public CORBACommons::ProcessControlImpl
{
public:
  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

public:
  Application() throw (eh::Exception);

  void
  run(int argc, char* argv[]) throw (Exception, eh::Exception);

protected:
  virtual
  ~Application() throw ();
};

#endif
