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
#include <string>
#include <memory>

#include <unistd.h>
#include <sys/stat.h>

#include <Generics/Time.hpp>

#include <Stream/GzipStreams.hpp>
#include <Stream/BzlibStreams.hpp>


const size_t LS = (1 << 7) + 1;
const size_t LN = (1 << 17) - 2;
const size_t IN = 3;


struct Data
{
  char data[LN][LS];
};
std::unique_ptr<Data> data;

const char* tmp = getenv("TEST_TMP_DIR");
std::string root(tmp ? tmp : ".");

void
write_stream(std::ostream& ostr) throw (eh::Exception)
{
  Generics::Timer t;
  Generics::CPUTimer c;

  t.start();
  c.start();

  for (size_t i = 0; i < LN; i++)
  {
    ostr.write(data->data[i], LS) << "\n";
  }

  t.stop();
  c.stop();

  std::cout << "\t Real: " << t.elapsed_time() << " CPU: " <<
    c.elapsed_time() << "\n";
}

template <typename Stream>
void
write_stream_n(const char* filename) throw (eh::Exception)
{
  std::cout << "Write\n";
  for (size_t i = 0; i < IN; i++)
  {
    Stream ostr(filename);
    write_stream(ostr);
    if (i != IN - 1)
    {
      unlink(filename);
    }
  }
}

void
read_stream(std::istream& istr) throw (eh::Exception)
{
  Generics::Timer t;
  Generics::CPUTimer c;

  t.start();
  c.start();

  size_t i = 0;
  for (std::string str; std::getline(istr, str); i++)
  {
    if (str != std::string(data->data[i], LS))
    {
      (std::cerr << i << ":\n" << str << "\n").
        write(data->data[i], LS) << "\n";
      abort();
    }
  }

  if (i != LN)
  {
    std::cerr << i << "\n";
    abort();
  }

  t.stop();
  c.stop();

  std::cout << "\t Real: " << t.elapsed_time() << " CPU: " <<
    c.elapsed_time() << "\n";
}

template <typename Stream>
void
read_stream_n(const char* filename) throw (eh::Exception)
{
  struct stat st;
  stat(filename, &st);
  std::cout << "Read " << (st.st_size >> 20) << "\n";
  for (size_t i = 0; i < IN; i++)
  {
    Stream istr(filename);
    read_stream(istr);
  }
  unlink(filename);
}

template <typename OStream, typename IStream>
void
test_stream(const char* filename) throw (eh::Exception)
{
  std::cout << filename << "\n";
  const std::string& file = root + "/" + filename;
  write_stream_n<OStream>(file.c_str());
  read_stream_n<IStream>(file.c_str());
}

int
main()
{
  static const String::SubString CHARS("0123456789+.-@;");
  data.reset(new Data);
  for (size_t i = 0; i < LN; i++)
  {
    for (size_t j = 0; j < LS; j++)
    {
      data->data[i][j] = CHARS[rand() % CHARS.size()];
    }
  }

  test_stream<std::ofstream, std::ifstream>("test.txt");
  test_stream<Stream::GzipOutStream, Stream::GzipInStream>("test.txt.gz");
  test_stream<Stream::BzlibOutStream, Stream::BzlibInStream>("test.txt.bz2");
}
