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





#include <TestCommons/MTTester.hpp>

/// Multi-thread executable functor
struct Tester
{
  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
  void
  operator ()() throw (Exception);
};

void
Tester::operator ()() throw (Exception)
{
  throw Exception("Something wrong");
}


int
main() throw ()
{
  dup2(STDOUT_FILENO, STDERR_FILENO);

  try
  {
    std::cout << "MTTester logging exceptions test" << std::endl;

    Tester test;
    TestCommons::MTTester<Tester&> mt_tester(test, 5);

    mt_tester.run(20, 0, 20);
    std::cout << "SUCCESS" << std::endl;
    return 0;
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "unknown exception" << std::endl;
  }

  return -1;
}
