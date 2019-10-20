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
#include <fstream>
#include <stdlib.h>

#include <Stream/MemoryStream.hpp>
#include <Stream/MMapStream.hpp>


void
create_output(const char* filename) throw (eh::Exception)
{
  std::ofstream out(filename);
  int symbols = 0;

  for (int i = 0; i < (1 << 20); i++)
  {
    out << static_cast<char>(rand () % 254 + 1);
    symbols++;
  }

  std::cout << symbols << " symbols sent to output" << std::endl;
}

void
test_input(const char* filename) throw (eh::Exception)
{
  std::ifstream in(filename);
  Stream::FileParser parser(filename);
  std::string in1, in2;
  int readings = 0;

  while (in >> in1)
  {
    parser >> in2;
    if (in1 != in2)
    {
      std::cerr << "Failure reading of '" << in1 << "' vs '" << in2 <<
        "'" << std::endl;
      return;
    }
    readings++;
  }
  if (parser.str().size())
  {
    std::cerr << "Invalid finish state" << std::endl;
    return;
  }
  std::cout << readings << " readings complete" << std::endl;
}

int
main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage:\n" << argv[0] << " temporal_file_name\n";
    return -1;
  }
  create_output(argv[1]);
  test_input(argv[1]);

  return 0;
}
