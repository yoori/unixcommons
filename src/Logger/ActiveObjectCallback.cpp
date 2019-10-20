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

#include <Logger/ActiveObjectCallback.hpp>


namespace Logging
{
  //
  // ActiveObjectCallback class
  //

  void
  ActiveObjectCallbackImpl::report_error(Severity severity,
    const String::SubString& description,
    const char* error_code)
    throw ()
  {
    unsigned long log_level = 0;
    const char* str_severity = "UNKNOWN";

    switch (severity)
    {
    case CRITICAL_ERROR:
      log_level = Logging::Logger::EMERGENCY;
      str_severity = "CRITICAL_ERROR";
      break;

    case ERROR:
      log_level = Logging::Logger::CRITICAL;
      str_severity = "ERROR";
      break;

    case WARNING:
      log_level = Logging::Logger::WARNING;
      str_severity = "WARNING";
      break;
    }

    Stream::Error ostr;
    ostr << message_prefix() << " " << str_severity << "(" << severity <<
      ") report:" << description;

    Logging::Logger* current_logger = logger();
    if (current_logger)
    {
      current_logger->log(ostr.str(), log_level, aspect(), code(error_code));
    }
    else
    {
      std::cerr << ostr.str() << std::endl;
    }
  }

  Logger*
  ActiveObjectCallbackImpl::logger() const throw ()
  {
    return logger_;
  }

  const char*
  ActiveObjectCallbackImpl::message_prefix() const throw ()
  {
    return message_prefix_;
  }

  const char*
  ActiveObjectCallbackImpl::aspect() const throw ()
  {
    return aspect_;
  }

  const char*
  ActiveObjectCallbackImpl::code(const char* error_code) const throw ()
  {
    return error_code ? error_code : code_;
  }
}
