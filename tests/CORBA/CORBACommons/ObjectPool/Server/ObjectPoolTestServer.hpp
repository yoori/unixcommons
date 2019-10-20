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



// @file Server/ObjectPoolTestServer.hpp
#ifndef CORBA_OBJECT_POOL_TEST_SERVER_HPP
#define CORBA_OBJECT_POOL_TEST_SERVER_HPP

#include <eh/Exception.hpp>

#include <CORBACommons/ProcessControlImpl.hpp>
#include <CORBACommons/CorbaAdapters.hpp>

#include "Simple_s.hpp"

namespace CORBATest
{

  class TestObjectPoolImpl :
    public CORBACommons::ReferenceCounting::ServantImpl<
      POA_CORBATest::TestObjectPool>
  {
    static volatile _Atomic_word stat_counter_;
    volatile _Atomic_word counter_;
    std::size_t my_number_;
  public:
    TestObjectPoolImpl() throw ();

    virtual ::CORBA::Long
    square(::CORBA::Long num) throw ();

    virtual ::CORBA::Long
    root(::CORBA::Long num) throw ();

    virtual CORBA::Long
    get_calling_number() throw ();

    virtual void
    up() throw ();

  protected:
    virtual
    ~TestObjectPoolImpl() throw ();
  };
  typedef ReferenceCounting::QualPtr<
    TestObjectPoolImpl> TestObjectPoolImpl_var;


  class PoolObjectImpl :
    public CORBACommons::ReferenceCounting::ServantImpl<
      POA_CORBATest::PoolObject>
  {
  public:
    virtual ::CORBA::Long
    is_base() throw ();

  protected:
    virtual
    ~PoolObjectImpl() throw ();
  };
  typedef ReferenceCounting::QualPtr<
    PoolObjectImpl> PoolObjectImpl_var;

}

class Application :
  public CORBACommons::ProcessControlImpl
{
public:
  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

  Application() throw (eh::Exception);
  virtual
  ~Application() throw () {}

  /**
   * Method up shutdown CORBA server and we starting new server
   * for the second time, but with with Cuatro object already.
   * @param after_up true if server should support Cuatro object
   */
  void
  run(int argc, char* argv[], std::size_t before_up = 1)
    throw (Exception, eh::Exception);

  void
  create_names(std::size_t port, std::size_t count = 3)
    throw (eh::Exception);

  static CORBACommons::OrbShutdowner_var shuter;
private:
  typedef std::vector<std::string> ObjectNames;
  ObjectNames servants;
};

//////////////////////////////////////////////////////////////////////////
// Inlines implementations
//////////////////////////////////////////////////////////////////////////

namespace CORBATest
{
  //
  // TestObjectPoolImpl class
  //

  inline
  TestObjectPoolImpl::TestObjectPoolImpl() throw () :
    counter_(0)
  {
    my_number_ = __gnu_cxx::__exchange_and_add(&stat_counter_, 1);
  }

  inline
  TestObjectPoolImpl::~TestObjectPoolImpl() throw ()
  {
  }

  inline ::CORBA::Long
  TestObjectPoolImpl::square(::CORBA::Long num) throw ()
  {
    __gnu_cxx::__atomic_add(&counter_, 1);
    return num * num;
  }

  inline ::CORBA::Long
  TestObjectPoolImpl::root(::CORBA::Long num) throw ()
  {
    __gnu_cxx::__atomic_add(&counter_, 1);
    return static_cast<long>(sqrt(num));
  }

  inline ::CORBA::Long
  TestObjectPoolImpl::get_calling_number() throw ()
  {
    ::CORBA::Long old = counter_;
    counter_ = 0;
    return old;
  }

  inline void
  TestObjectPoolImpl::up() throw ()
  {
    try
    {
      if (Application::shuter.in())
      {
        try
        {
          std::cout << "Shutting DOWN" << std::endl;
          Application::shuter->shutdown(false);
          std::cout << "Shut DOWN" << std::endl;
        }
        catch (...)
        {
        }
      }
    }
    catch (...)
    {
      // nothing to do for now
    }
  }

  inline
  PoolObjectImpl::~PoolObjectImpl() throw ()
  {
  }

  inline ::CORBA::Long
  PoolObjectImpl::is_base() throw ()
  {
    return 12345;
  }


}

#endif  // CORBA_OBJECT_POOL_TEST_SERVER_HPP
