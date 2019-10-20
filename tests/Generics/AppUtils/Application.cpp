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

#include <Generics/AppUtils.hpp>

using namespace Generics::AppUtils;

template<typename _TEST_CASE, const char* _ARGV[]>
bool test_case()
{
  Args parser;
  _TEST_CASE test_case(parser);
  size_t argc = 0;
  for(const char** argcur = _ARGV; *argcur; ++argcur)
  {
    ++argc;
  };

  parser.parse(argc, _ARGV);
  return test_case.check();
}

const char* ARGV1[] = {
  "--test=1",
  "--test2=2",
  "--test3=A",
  0
};

const char* ARGV2[] = {
  "-t11",
  "-t22",
  "-t3",
  "3",
  0
};

const char* ARGV3[] = {
  "-t1t2t32",
  0
};

struct TestCase1
{
  TestCase1(Args& parser)
  {
    parser.add(equal_name("test"), test);
    parser.add(equal_name("test2"), test2);
    parser.add(equal_name("test3"), test3);
  }

  bool check()
  {
    return
      test.installed() && *test == 1 &&
      test2.installed() && *test2 == 2 &&
      test2.installed() && *test3 == 'A';
  }

  Option<unsigned long> test;
  Option<unsigned long> test2;
  Option<char> test3;
};

struct TestCase2
{
  TestCase2(Args& parser)
  {
    parser.add(short_name("t1"), test);
    parser.add(short_name("t2"), test2);
    parser.add(short_name("t3"), test3);
  }

  bool check()
  {
    return
      test.installed() && *test == 1 &&
      test2.installed() && *test2 == 2 &&
      test3.installed() && *test3 == 3;
  }

  Option<unsigned long> test;
  Option<unsigned long> test2;
  Option<unsigned long> test3;
};

struct TestCase3
{
  TestCase3(Args& parser)
  {
    parser.add(short_name("t1"), test);
    parser.add(short_name("t2"), test2);
    parser.add(short_name("t3"), test3);
  }

  bool check()
  {
    return
      test.enabled() &&
      test2.enabled() &&
      test3.installed() && *test3 == 2;
  }

  CheckOption test;
  CheckOption test2;
  Option<unsigned long> test3;
};

int main(/*int argc, char* argv[]*/)
{
  int ret = 0;

  try
  {
    if(test_case<TestCase1, ARGV1>())
    {
      std::cout << "Case #1 success." << std::endl;
    }
    else
    {
      ret = 1;
      std::cerr << "Case #1 failed." << std::endl;
    }

    if(test_case<TestCase2, ARGV2>())
    {
      std::cout << "Case #2 success." << std::endl;
    }
    else
    {
      ret = 2;
      std::cerr << "Case #2 failed." << std::endl;
    }

    if(test_case<TestCase3, ARGV3>())
    {
      std::cout << "Case #3 success." << std::endl;
    }
    else
    {
      ret = 3;
      std::cerr << "Case #3 failed." << std::endl;
    }

    return ret;
  }
  catch(const Generics::AppUtils::Exception& ex)
  {
    std::cout << "Caught Exception: " << ex.what() << std::endl;
  }

  return 0;
}
