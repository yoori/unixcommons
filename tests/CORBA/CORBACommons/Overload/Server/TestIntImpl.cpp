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



#include <malloc.h>

#include <Generics/Time.hpp>
#include <Generics/Rand.hpp>
#include <Generics/Proc.hpp>

#include <TestCommons/Memory.hpp>

#include "TestIntImpl.hpp"


namespace CORBATest
{
  TestIntImpl::TestIntImpl(int seq3, int seq2, int size) throw ()
    : CORBACommons::ProcessStatsImpl(Generics::Values_var(
        new Generics::Values).in()),
      received_requests(0), timeout_(0),
      seq3_(seq3), seq2_(seq2), size_(size)
  {
    {
      const char* timeout = getenv("ORB_TIMEOUT");
      if (timeout)
      {
        timeout_ = atoi(timeout);
      }
    }
    Generics::Values& st = stats();
    st.set("name", "TestInt");
    st.set("total fee", 0.0);
    st.set("received_requests", 0ul);
    st.set("failed_requests", 0l);
    st.set_as_string("start time",
      Generics::Time::get_time_of_day().get_gm_time());
  }

  TestIntImpl::~TestIntImpl() throw ()
  {
  }

  void
  TestIntImpl::test(const OctetSeq& in_seq) throw ()
  {
    stats().add("received_requests", 1ul);

    CORBA::ULong PARAM_LEN = in_seq.length();

    for (CORBA::ULong i = 0; i < PARAM_LEN; ++i)
    {
      if (in_seq[i] != i % 256)
      {
        Stream::Dynamic ostr(4096);
        ostr << "Invalid array of length " << PARAM_LEN << " at element " <<
          i << "\n";
        std::cerr << ostr.str();
        for (CORBA::ULong j = 0; j < PARAM_LEN; ++j)
        {
          if (i == j)
          {
            ostr << " *" << static_cast<unsigned>(in_seq[j]) << "*";
          }
          else
          {
            ostr << " " << static_cast<unsigned>(in_seq[j]);
          }
        }
        ostr << "\n";
        std::cerr << ostr.str();
      }
    }

    __gnu_cxx::__atomic_add(&received_requests, 1);

    if (timeout_)
    {
      unsigned wait_time = timeout_ * Generics::Time::USEC_MAX;
      wait_time += Generics::safe_rand(wait_time / 4) - wait_time / 8;
      Generics::Time wait(wait_time / Generics::Time::USEC_MAX,
        wait_time % Generics::Time::USEC_MAX);
      select(0, 0, 0, 0, &wait);
    }
  }

  void
  TestIntImpl::oneway_test(const OctetSeq& in_seq) throw ()
  {
    test(in_seq);
  }

  Seq3*
  TestIntImpl::memory_test() throw (eh::Exception)
  {
    print_memory(false);

    Seq3_var seq3(new Seq3);
    seq3->length(seq3_);
    for (int i = 0; i < seq3_; i++)
    {
      Seq2& seq2 = seq3->operator[](i);
      seq2.length(seq2_);
      for (int j = 0; j < seq2_; j++)
      {
        seq2[j].length(size_);
      }
    }

    print_memory(false);

    return seq3._retn();
  }

  void
  TestIntImpl::print_memory(CORBA::Boolean full) throw ()
  {
    unsigned long vsize, rss;

    Generics::Proc::memory_status(vsize, rss);

    struct mallinfo info = mallinfo();
    std::cout << "vsize " << (vsize >> 20) << " rss " << (rss >> 20) <<
      " allocated " << (info.uordblks >> 20) << " free " <<
      (info.fordblks >> 20) << std::endl;

    if (full)
    {
      TestCommons::print_mallinfo(std::cout, &info);
    }
  }
}
