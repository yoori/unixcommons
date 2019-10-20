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



#ifndef CORBA_COMMONS_TEST_INT_IMPL_HPP
#define CORBA_COMMONS_TEST_INT_IMPL_HPP

#include <eh/Exception.hpp>
#include <Sync/PosixLock.hpp>

#include <TestInt_s.hpp>

#include <CORBACommons/ServantImpl.hpp>
#include <CORBACommons/StatsImpl.hpp>

namespace CORBATest
{
  class TestIntImpl :
    virtual public
      CORBACommons::ReferenceCounting::ServantImpl<POA_CORBATest::TestInt>,
    private CORBACommons::ProcessStatsImpl
  {
  public:
    struct Callback
    {
      virtual
      ~Callback() throw ()
      {
      }
      virtual void
      error(const char*) throw () = 0;
    };

    TestIntImpl(int seq3 = 3000, int seq2 = 15, int size = 1000) throw ();

    virtual
    ~TestIntImpl() throw ();

    virtual void
    test(const OctetSeq& in_seq) throw ();

    virtual void
    oneway_test(const OctetSeq& in_seq) throw ();

    virtual Seq3*
    memory_test() throw (eh::Exception);

    virtual void
    print_memory(CORBA::Boolean full) throw ();

    volatile _Atomic_word received_requests;

  private:
    unsigned timeout_;
    int seq3_, seq2_, size_;
  };

  typedef ReferenceCounting::QualPtr<TestIntImpl> TestIntImpl_var;
}

#endif
