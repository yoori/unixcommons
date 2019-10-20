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



// @file AsciiStringManipTest.cpp
//

#include <iostream>
#include <String/AsciiStringManip.hpp>
#include <String/UTF8IsProperty.hpp>
#include <Generics/Rand.hpp>

void
check_flatten() throw (eh::Exception);

class TestFlattenCaseGenerator
{
public:
  void
  generate() throw (eh::Exception);

  void
  check() throw (eh::Exception);

  static void
  checking(const std::string& res,
           const std::string& src,
           const char *standard)
    throw (eh::Exception);

private:
  std::string input_;
  std::string standard_;
};

void
check_random_flatten() throw (eh::Exception);

void
check_compare_caseless() throw (eh::Exception);

//
// Test body below
//
int
main()
{
  try
  {
    std::cout << "AsciiStringManip test started.." << std::endl;

    check_flatten();
    check_random_flatten();
    check_compare_caseless();
    std::cout << "SUCCESS" << std::endl;
  }
  catch (eh::Exception& e)
  {
    std::cout << "\nFAIL: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "\nFAIL: unknown exception" << std::endl;
    throw;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////
// Implementations

using namespace String::AsciiStringManip;

void
TestFlattenCaseGenerator::generate() throw (eh::Exception)
{
  standard_.clear();
  input_.clear();
  bool put_space_before = false;
  for (std::size_t i = Generics::safe_integral_rand(10); i != 0; --i)
  {
    if (Generics::safe_integral_rand(1))
    {
      input_ += " ";
      if (!put_space_before)
      {
        standard_ += " ";
        put_space_before = true;
      }
    }
    else
    {
      const char STR[] = "acbnp439hf1-34djc,12394i1293ier1923ie=23ie";
      char put_it = STR[Generics::safe_rand(sizeof(STR)-1)];
      input_ += put_it;
      standard_ += put_it;
      put_space_before = false;
    }
  }
}

void
TestFlattenCaseGenerator::check() throw (eh::Exception)
{
  std::string dest;
  String::AsciiStringManip::flatten(dest, input_);
  if (dest != standard_)
  {
    std::cerr << "flatten functional doesn't work: input=" << input_ << std::endl
      << "result=" << dest << std::endl << "standard="
      << standard_ << std::endl;
  }
}

void
TestFlattenCaseGenerator::checking(const std::string& res,
                                   const std::string& src,
                                   const char *standard)
  throw (eh::Exception)
{
  if (res != standard)
  {
    std::cerr << "flatten functional trouble:\n"
      << "Source: " << src << std::endl
      << "Result: " << res << std::endl
      << "Standard: " << standard << std::endl;
  }
}

void
check_random_flatten() throw (eh::Exception)
{
  TestFlattenCaseGenerator checker;
  for (std::size_t i = 0; i < 100; ++i)
  {
    checker.generate();
    checker.check();
  }
}

void
check_flatten() throw (eh::Exception)
{
  std::string dest;

  std::string src("Test   \t  _Rpl\t  . ");
  String::AsciiStringManip::flatten(
    dest, src, String::SubString("R", 1));
  std::cout << "Source: " << src << std::endl;
  std::cout << "Result: " << dest << std::endl;
  TestFlattenCaseGenerator::checking(dest, src, "TestR_RplR.R");

  String::AsciiStringManip::flatten(dest, src);
  TestFlattenCaseGenerator::checking(dest, src, "Test _Rpl . ");
}

void
check_compare_caseless() throw (eh::Exception)
{
  const String::SubString S1("A\0A", 4);
  const String::SubString S2("a\0b", 4);
  const String::SubString S3("a\0a", 4);
  
  const char FUN[] = "check_compare_caseless(): ";
  std::cout << FUN << "started" << std::endl;
  
  if (String::AsciiStringManip::Caseless(S1).compare(S2) >= 0)
  {
    std::cerr << FUN << "fail 1" << std::endl;
  }
  if (String::AsciiStringManip::Caseless(S1).compare(S3) != 0)
  {
    std::cerr << FUN << "fail 2" << std::endl;
  }
  if (String::AsciiStringManip::Caseless(S2).compare(S3) <= 0)
  {
    std::cerr << FUN << "fail 3" << std::endl;
  }
}
