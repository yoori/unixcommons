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



// @file UTF8Category/TestUTF8Category.cpp

#include <iostream>

#include <String/UTF8Category.hpp>
#include <String/AsciiStringManip.hpp>

using namespace String;
using namespace String::AsciiStringManip;

namespace
{
  DECLARE_EXCEPTION(TestException, eh::DescriptiveException);
  const char SAMPLE[] = "Example for test 12345h lowerUPPER";
  const Utf8Category* CATEGORIES[] =
  {
    &UNICODE_SPACES, &UNICODE_DIGITS, &UNICODE_LETTERS, &UNICODE_LOWER_LETTERS,
    &UNICODE_UPPER_LETTERS
  };

  CharCategory LOWER("a-z");
  CharCategory UPPER("A-Z");
  const CharCategory* CHAR_CATEGORIES[] =
  {
    &SPACE, &NUMBER, &ALPHA, &LOWER, &UPPER
  };
}

template <typename Categories>
void
rfind_test(const Categories& CATEGORIES) throw (eh::Exception)
{
  const char FUN[] = "rfind_test(): ";
  const char* const FROM = SAMPLE + sizeof(SAMPLE) - 1;
  int STANDARD_OFFSETS[] =
  {
    23, 33, 21, 33, 33, 23, 28, 33, 33, 28
  };

  const char* result;
  for (std::size_t i = 0;
    i < 2 * sizeof(CATEGORIES) / sizeof(CATEGORIES[0]);
    i += 2)
  {
    result = CATEGORIES[i / 2]->rfind_owned(FROM, SAMPLE);
    if (result - SAMPLE != STANDARD_OFFSETS[i])
    {
      std::cerr << FUN << "FAIL, rfind_owned, category number=" << i / 2;
      if (!result)
      {
        std::cerr << ", symbols not found";
      }
      else
      {
        std::cerr << ", awaiting " << STANDARD_OFFSETS[i] << ", fact="
          << result - SAMPLE;
      }
      std::cerr << std::endl;
    }
    result = CATEGORIES[i / 2]->rfind_nonowned(FROM, SAMPLE);
    if (result - SAMPLE != STANDARD_OFFSETS[i + 1])
    {
      std::cerr << FUN << "FAIL, rfind_nonowned, category number=" << i / 2;
        if (!result)
        {
          std::cerr << ", symbols not found";
        }
        else
        {
          std::cerr << ", awaiting " << STANDARD_OFFSETS[i] << ", fact="
            << result - SAMPLE;
        }
        std::cerr << std::endl;
    }
  }
}

void
finishers_test() throw (eh::Exception)
{
  const char FUN[] = "finishers_test(): ";
  const char* const FROM = SAMPLE + sizeof(SAMPLE) - 1;
  const char*
  result = UNICODE_TITLE_LETTERS.rfind_owned(FROM, SAMPLE);
  if (result != FROM)
  {
    std::cerr << FUN << "Fail: incorrect not found value, offset="
      << result - SAMPLE << std::endl;
  }
  result = REGEX_META.rfind_owned(FROM, SAMPLE);
  if (result != FROM)
  {
    std::cerr << FUN << "Fail: incorrect not found value, offset="
      << result - SAMPLE << std::endl;
  }
  Utf8Category unicode_cat("A-Za-z0-9 ");
  result = unicode_cat.rfind_nonowned(FROM, SAMPLE);
  if (result != FROM)
  {
    std::cerr << FUN << "Fail: incorrect not found value, offset="
      << result - SAMPLE << std::endl;
  }
  CharCategory cat("A-Za-z0-9 ");
  result = cat.rfind_nonowned(FROM, SAMPLE);
  if (result != FROM)
  {
    std::cerr << FUN << "Fail: incorrect not found value, offset="
      << result - SAMPLE << std::endl;
  }
}

int
main(int /*argc*/, char* /*argv*/[])
{
  try
  {
    std::cout << "UTF8Category test started.." << std::endl;

    std::cout << "CharCategory test" << std::endl;
    rfind_test(CHAR_CATEGORIES);
    std::cout << "UTF8Category test" << std::endl;
    rfind_test(CATEGORIES);
    finishers_test();
    std::cout << "SUCCESS" << std::endl;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "Exception occurred: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception occurred!" << std::endl;
    throw;
  }

  return 0;
}
