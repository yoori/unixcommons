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

#include <Generics/Network.hpp>


namespace
{
  Generics::Network::IsLocalInterface local;

  enum EXPECTED_RESULT
  {
    ER_NONLOCAL,
    ER_LOCAL,
    ER_INVALID
  };

  void
  check(const char* host_name, EXPECTED_RESULT expected_result)
    throw (eh::Exception)
  {
    std::cout << "Checking " << host_name << std::endl;
    try
    {
      EXPECTED_RESULT result = local.check_host_name(host_name) ?
        ER_LOCAL : ER_NONLOCAL;
      if (result != expected_result)
      {
        std::cerr << "'" << host_name << "' " <<
          (expected_result == ER_LOCAL ? "is" : "isn't") <<
          (expected_result == ER_INVALID ?
            " an address" : " a local address)") <<
          "check failed" << std::endl;
      }
    }
    catch (const eh::Exception& ex)
    {
      if (expected_result != ER_INVALID)
      {
        std::cerr << "'" << host_name << "' unexpected exception: " <<
          ex.what() << std::endl;
      }
      else
      {
        std::cout << "'" << host_name << "' expected exception: " <<
          ex.what() << std::endl;
      }
    }
  }
}

int
main()
{
  try
  {
    check("127.0.0.1", ER_LOCAL);
    check("localhost", ER_LOCAL);
    check("phorm.com", ER_NONLOCAL);
    check("non-existing-hostname", ER_INVALID);
    char hostname[128];
    if (!gethostname(hostname, sizeof(hostname)))
    {
      check(hostname, ER_LOCAL);
    }

    std::cout << "Test complete" << std::endl;
    return 0;
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "Exception: " << ex.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception: " << std::endl;
  }

  return 0;
}
