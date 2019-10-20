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

#include <Logger/Logger.hpp>

#include <TestCommons/MTTester.hpp>

class NullLoggerHolder
{
public:
  NullLoggerHolder() throw (eh::Exception);

protected:
  Logging::QLogger_var logger_;
};

NullLoggerHolder::NullLoggerHolder() throw (eh::Exception)
  : logger_(new Logging::Null::Logger)
{
}

class Logger1 : private NullLoggerHolder
{
public:
  void
  operator ()() throw (eh::Exception);

private:
  void
  test_log_(unsigned level, void* mark) throw (eh::Exception);
};

class Logger2 : private NullLoggerHolder
{
public:
  void
  operator ()() throw (eh::Exception);

private:
  void
  test_log_(unsigned level, void* mark) throw (eh::Exception);
};

class Logger3 : private NullLoggerHolder
{
public:
  void
  operator ()() throw (eh::Exception);

private:
  void
  test_log_(unsigned level, void* mark) throw (eh::Exception);
};

void
print_mark(char c, void* mark1, void* mark2) throw (eh::Exception)
{
  std::cout << c << ' ' << (reinterpret_cast<size_t>(mark2) -
    reinterpret_cast<size_t>(mark1)) << std::endl;
}

void
Logger1::operator ()() throw (eh::Exception)
{
  int d;
  test_log_(3, &d);
}

void
Logger1::test_log_(unsigned level, void* mark) throw (eh::Exception)
{
  {
    int a;
    print_mark('A', &a, mark);
  }

  {
  logger_->stream<Logging::Logger::DEFAULT_BUFFER_SIZE>(
    Logging::Logger::INFO) << "";
  logger_->stream<Logging::Logger::DEFAULT_BUFFER_SIZE>(
    Logging::Logger::INFO) << "";
  logger_->stream<Logging::Logger::DEFAULT_BUFFER_SIZE>(
    Logging::Logger::INFO) << "";
  logger_->stream<Logging::Logger::DEFAULT_BUFFER_SIZE>(
    Logging::Logger::INFO) << "";
  logger_->stream<Logging::Logger::DEFAULT_BUFFER_SIZE>(
    Logging::Logger::INFO) << "";
  logger_->stream<Logging::Logger::DEFAULT_BUFFER_SIZE>(
    Logging::Logger::INFO) << "";
  logger_->stream<Logging::Logger::DEFAULT_BUFFER_SIZE>(
    Logging::Logger::INFO) << "";
  /*logger_->stream<Logging::Logger::DEFAULT_BUFFER_SIZE>(
    Logging::Logger::INFO) << "";*/
  }

  {
    int b;
    print_mark('B', &b, mark);
  }

  if (level)
  {
    test_log_(level - 1, &level);
  }

  {
    int c;
    print_mark('C', &c, mark);
  }
}

void
Logger2::operator ()() throw (eh::Exception)
{
  int d;
  test_log_(16, &d);
}

void
Logger2::test_log_(unsigned level, void* mark) throw (eh::Exception)
{
  {
    int a;
    print_mark('A', &a, mark);
  }

  Logging::Logger::DBuffer buffer;

  logger_->stream(buffer, Logging::Logger::INFO) << "";
  logger_->stream(buffer, Logging::Logger::INFO) << "";
  logger_->stream(buffer, Logging::Logger::INFO) << "";
  logger_->stream(buffer, Logging::Logger::INFO) << "";
  logger_->stream(buffer, Logging::Logger::INFO) << "";
  logger_->stream(buffer, Logging::Logger::INFO) << "";
  logger_->stream(buffer, Logging::Logger::INFO) << "";
  logger_->stream(buffer, Logging::Logger::INFO) << "";

  {
    int b;
    print_mark('B', &b, mark);
  }

  if (level)
  {
    test_log_(level - 1, &level);
  }

  {
    int c;
    print_mark('C', &c, mark);
  }
}

void
Logger3::operator ()() throw (eh::Exception)
{
  int d;
  test_log_(70, &d);
}

void
Logger3::test_log_(unsigned level, void* mark) throw (eh::Exception)
{
  {
    int a;
    print_mark('A', &a, mark);
  }

  {
  logger_->sstream(Logging::Logger::INFO) << "";
  logger_->sstream(Logging::Logger::INFO) << "";
  logger_->sstream(Logging::Logger::INFO) << "";
  logger_->sstream(Logging::Logger::INFO) << "";
  logger_->sstream(Logging::Logger::INFO) << "";
  logger_->sstream(Logging::Logger::INFO) << "";
  logger_->sstream(Logging::Logger::INFO) << "";
  logger_->sstream(Logging::Logger::INFO) << "";
  }

  {
    int b;
    print_mark('B', &b, mark);
  }

  if (level)
  {
    test_log_(level - 1, &level);
  }

  {
    int c;
    print_mark('C', &c, mark);
  }
}

template <typename Logger>
void
test() throw (eh::Exception)
{
  Logger logger;
  TestCommons::MTTester<Logger&> tester(logger, 1);
  tester.run(1, 1, 1);
}

int
main()
{
  test<Logger1>();
  test<Logger2>();
  test<Logger3>();

  return 0;
}
