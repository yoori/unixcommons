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



#include <iostream>
#include <iomanip>
#include <list>

#include <Generics/TAlloc.hpp>
#include <Generics/Time.hpp>
#include <Generics/GnuHashTable.hpp>

#include <Stream/MemoryStream.hpp>

#include <TestCommons/MTTester.hpp>

#define TA_

#ifdef TA
#include </home/konstantin_sadov/work/server/UserInfoSvcs/UserInfoCommons/Allocator.hpp>
#endif


const int CYCLES = 1000;
const int THREADS = 8;
const int OPS = 1000;

#ifdef TA
#define TYPE(Type) \
  struct Type##1 : private Type::allocator_type::buffer_type, public Type \
  { \
    Type##1() : Type(typename Type::allocator_type(this)) \
    { \
    } \
  }; \
  Type##1
#else
#define TYPE(Type) \
  Type
#endif

template <typename Elem, typename Alloc>
struct Test1
{
  void
  operator ()() const throw (eh::Exception)
  {
    typedef std::list<Elem, Alloc> List;
    TYPE(List) l;

    for (int i = 0; i < CYCLES; i++)
    {
      l.push_back(Elem());
    }
  }
};

template <typename Elem, typename Alloc>
struct Test2
{
  void
  operator ()() const throw (eh::Exception)
  {
    typedef std::list<Elem, Alloc> List;
    TYPE(List) l;

    for (int i = 0; i < CYCLES; i++)
    {
      for (int j = 0; j < 10; j++)
      {
        l.push_back(Elem());
      }
      l.erase(l.begin());
    }
  }
};

template <typename Elem, typename Alloc>
struct Test3
{
  void
  operator ()() const throw (eh::Exception)
  {
    typedef Generics::GnuHashTable<Generics::NumericHashAdapter<int>,
      Elem, Alloc> Hash;
    Hash h;

    for (int i = 0; i < CYCLES; i++)
    {
      h[i] = Elem();
    }
  }
};

template <typename Elem, typename Alloc>
struct Test4
{
  void
  operator ()() const throw (eh::Exception)
  {
    typedef Generics::GnuHashSet<Generics::NumericHashAdapter<int>, Alloc>
      Hash;
    Hash h;

    for (int i = 0; i < CYCLES; i++)
    {
      h.insert(i);
    }
  }
};

struct Elem1
{
  uint64_t data;
};

struct Elem2
{
  uint64_t data[2];
};

struct Elem3
{
  uint64_t data[4];
};

template <template <typename, typename> class Test, typename Elem,
  typename Alloc>
void
test1(const char* description) throw (eh::Exception)
{
  typedef Test<Elem, Alloc> Functor;

  std::cout << "  " << description << "... " << std::flush;
  Generics::Timer timer;
  TestCommons::MTTester<Functor> tester(Functor(), THREADS);
  timer.start();
  tester.run(THREADS * 2, 0, OPS);
  timer.stop();
  std::cout << timer.elapsed_time() << std::endl;
}

template <template <typename, typename> class Test, typename Elem>
void
test2(const char* description) throw (eh::Exception)
{
  std::cout << " " << description << "\n";
#ifndef TA
  test1<Test, Elem, std::allocator<Elem> >(
    "std            ");
  test1<Test, Elem, Generics::TAlloc::AllocOnly<Elem, 64, true> >(
    "AllocOnly    64");
  test1<Test, Elem, Generics::TAlloc::AllocOnly<Elem, 1024, true> >(
    "AllocOnly  1024");
  test1<Test, Elem, Generics::TAlloc::Aggregated<Elem, 64, true> >(
    "Aggregated   64");
  test1<Test, Elem, Generics::TAlloc::Aggregated<Elem, 1024, true> >(
    "Aggregated 1024");
#if 0
  test1<Test, Elem, Generics::TAlloc::GlobalPool<Elem, 64> >(
    "GlobalPool   64");
  test1<Test, Elem, Generics::TAlloc::GlobalPool<Elem, 1024> >(
    "GlobalPool 1024");
#endif
  test1<Test, Elem, Generics::TAlloc::ThreadPool<Elem, 64, true> >(
    "ThreadPool   64");
  test1<Test, Elem, Generics::TAlloc::ThreadPool<Elem, 1024, true> >(
    "ThreadPool 1024");
#else
  test1<Test, Elem,
    AdServer::UserInfoSvcs::PagedBufferAllocator<Elem, 8192> >(
    "PagedBuffer8192");
#endif
  std::cout << "\n";
}

template <template <typename, typename> class Test>
void
test3(const char* description) throw (eh::Exception)
{
  std::cout << description << std::endl;
  test2<Test, Elem1>("8");
  test2<Test, Elem2>("16");
  test2<Test, Elem3>("32");
}

int
main()
{
  try
  {
    test3<Test1>("list push_back");
    test3<Test2>("list push_back x10 + erase");
    test3<Test3>("unordered_map insert");
    test3<Test4>("unordered_set insert");
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
