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
#include <signal.h>
//#include <sys/wait.h>
#include "client.hpp"

#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <dlfcn.h>

int
main(int argc, char** argv)
{
  if (argc < 3)
  {
    return 1;
  }

  try
  {
    std::cout << "Client started" << std::endl;
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    const char* ior = argv[1];
    CORBA::Object_var obj = orb->string_to_object(ior);
    ::Test::Echo_var echoref = ::Test::Echo::_narrow(obj.in());
    if (CORBA::is_nil(echoref))
    {
      std::cerr << "Can't narrow reference to type Echo (or it was nil)."
                << std::endl;
      return 1;
    }

    const char* ior_out = argv[2];
    CORBA::Object_var obj_out = orb->string_to_object(ior_out);
    ::Test::OutVal_var outref = ::Test::OutVal::_narrow(obj_out.in());
    if (CORBA::is_nil(outref))
    {
      std::cerr << "Can't narrow reference to type Echo (or it was nil)."
                << std::endl;
      return 1;
    }

    Test::B b;
    std::cout << "Client call server OutVal" << std::endl;
    outref->test(b); // call server

    Test::AType_var values = new Test::AType;
    const size_t N = 3;
    values->length(N);
    
    for (std::size_t i = 0; i < N; ++i)
    {
      values[i].aa = 0xFFFFF;
      values[i].bb = 0xFFFFF;
    }
/*    for (std::size_t i = 0; i < N-1; ++i)
    {
      values[i].aa = i;
      values[i].bb = -i;
    }*/
    std::cout << "And now, try use server..." << std::endl
      << "client sizeof=" << sizeof(Test::AType) << std::endl;

    for (std::size_t i =0; i < 5;++i)
    {
       echoref->echoString(values);  // call server...
    }

    orb->destroy();

    kill(getppid(), SIGINT);

    return 0;
  }
  catch (const CORBA::Exception& ex)
  {
    std::cerr << "CORBA::Exception in client:" << ex;
  }
  catch (const std::exception& e)
  {
    std::cerr << "eh::Exception on client:" << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception" << std::endl;
    throw;
  }
  return 1;
}
