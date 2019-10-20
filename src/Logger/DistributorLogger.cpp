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



#include <Logger/DistributorLogger.hpp>


namespace Logging
{
  //
  // DistributorLogger class
  //

  DistributorLogger::~DistributorLogger() throw ()
  {
  }

  unsigned long
  DistributorLogger::log_level() throw ()
  {
    unsigned long level = 0;
    for (Loggers::iterator it(loggers_.begin());
      it != loggers_.end(); ++it)
    {
      unsigned long value = (*it)->log_level();
      if (value > level)
      {
        level = value;
      }
    }
    return level;
  }

  void
  DistributorLogger::log_level(unsigned long value) throw ()
  {
    for (Loggers::iterator it(loggers_.begin());
      it != loggers_.end(); ++it)
    {
      (*it)->log_level(value);
    }
  }

  bool
  DistributorLogger::log(const String::SubString& text,
    unsigned long severity, const char* aspect, const char* code) throw ()
  {
    bool result = false;
    for (Loggers::iterator it(loggers_.begin());
      it != loggers_.end(); ++it)
    {
      if ((*it)->log(text, severity, aspect, code))
      {
        result = true;
      }
    }
    return result;
  }


  //
  // SeveritySelectorLogger class
  //

  SeveritySelectorLogger::SeveritySelectorLogger(
    Logger* logger, unsigned long low, unsigned long high) throw ()
    : SimpleLoggerHolder(logger), low_(low), high_(high)
  {
  }

  SeveritySelectorLogger::SeveritySelectorLogger(
    unsigned long high, Logger* logger) throw ()
    : SimpleLoggerHolder(logger), low_(0), high_(high)
  {
  }

  SeveritySelectorLogger::~SeveritySelectorLogger() throw ()
  {
  }

  unsigned long
  SeveritySelectorLogger::log_level() throw ()
  {
    unsigned long level = SimpleLoggerHolder::log_level();
    return level > high_ ? high_ : level;
  }

  bool
  SeveritySelectorLogger::log(const String::SubString& text,
    unsigned long severity, const char* aspect, const char* code) throw ()
  {
    if (severity >= low_ && severity <= high_)
    {
      return SimpleLoggerHolder::log(text, severity, aspect, code);
    }
    return false;
  }
}
