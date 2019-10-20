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
#include <sstream>
#include <stdlib.h>

#include <Stream/MemoryStream.hpp>


void
test_output(std::string& result) throw (eh::Exception)
{
  std::ostringstream ostr;
  Stream::MemoryStream::OutputMemoryStream<char> omem;
  int symbols = 0;

  for (int i = 0; i < 1000; i++)
  {
    std::string out;
    for (int j = rand() % 100; j >= 0; j--)
    {
      out.push_back(rand () % 254 + 1);
      symbols++;
    }
    ostr << out;
    omem << out;
    if (ostr.str() != omem.str())
    {
      std::cerr << "Failure after output of '" << out << "'" << std::endl;
      return;
    }
  }

  result = ostr.str();

  std::cout << symbols << " symbols sent to output" << std::endl;
}

void
test_input(const char* result) throw (eh::Exception)
{
  std::istringstream istr(result);
  Stream::Parser imem(result);
  std::string in1, in2;
  int readings = 0;

  while (istr >> in1)
  {
    imem >> in2;
    if (in1 != in2)
    {
      std::cerr << "Failure reading of '" << in1 << "' vs '" << in2 <<
        "'" << std::endl;
      return;
    }
    readings++;
  }
  if (imem.str().size())
  {
    std::cerr << "Invalid finish state" << std::endl;
    return;
  }
  std::cout << readings << " readings complete" << std::endl;
}

int
main()
{
  std::string result;
  test_output(result);
  test_input(result.c_str());

  return 0;
}
