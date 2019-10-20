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

#include <unistd.h>

#include <ReferenceCounting/ReferenceCounting.hpp>

#include <Generics/LastPtr.hpp>


class Obj : public Generics::Last<ReferenceCounting::AtomicImpl>
{
public:
  void
  func() throw ();

protected:
  virtual
  ~Obj() throw ();
};

Obj::~Obj() throw ()
{
  std::cout << FNS << std::endl;
}

void
Obj::func() throw ()
{
  std::cout << FNS << std::endl;
}

typedef ReferenceCounting::QualPtr<Obj> ObjPtr;

typedef Generics::LastPtr<Obj> ObjLastPtr;

void
test0() throw (eh::Exception)
{
  ObjPtr ptr(new Obj);
  ptr->func();
}

void
test1() throw (eh::Exception)
{
  ObjPtr ptr(new Obj);
  ptr->func();
  ObjLastPtr lptr(ptr.retn());
  lptr->func();
}

void*
reseter(void* arg)
{
  sleep(2);
  static_cast<ObjPtr*>(arg)->reset();
  return 0;
}

void
test2() throw (eh::Exception)
{
  ObjPtr ptr(new Obj);
  ObjPtr ptr2(ptr);
  pthread_t tid;
  pthread_create(&tid, 0, reseter, &ptr2);
  {
    Generics::Timer timer;
    timer.start();
    ObjLastPtr lptr(ptr.retn());
    timer.stop();
    lptr->func();
    std::cout << timer.elapsed_time() << std::endl;
  }
  pthread_join(tid, 0);
}

int
main()
{
  try
  {
    test0();
    test1();
    test2();
    return 0;
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "eh::Exception: " << ex.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception" << std::endl;
  }

  return -1;
}
