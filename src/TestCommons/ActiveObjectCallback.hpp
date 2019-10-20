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



#ifndef CHECKCOMMONS_ACTIVEOBJECTCALLBACK_HPP
#define CHECKCOMMONS_ACTIVEOBJECTCALLBACK_HPP

#include <Logger/ActiveObjectCallback.hpp>
#include <Logger/StreamLogger.hpp>

namespace TestCommons
{
  struct LoggerHolder
  {
    explicit
    LoggerHolder(Logging::Logger* logger) throw ();

    Logging::FLogger_var logger;
  };

  class ActiveObjectCallbackStreamImpl :
    private LoggerHolder,
    public Logging::ActiveObjectCallbackImpl
  {
  public:
    explicit
    ActiveObjectCallbackStreamImpl(std::ostream& output_stream,
      const char* message_prefix = "ActiveObject",
      const char* aspect = 0, const char* code = 0) throw (eh::Exception);

  protected:
    virtual
    ~ActiveObjectCallbackStreamImpl() throw ();
  };
}

namespace TestCommons
{
  //
  // LoggerHolder class
  //

  inline
  LoggerHolder::LoggerHolder(Logging::Logger* logger) throw ()
    : logger(ReferenceCounting::add_ref(logger))
  {
  }


  //
  // ActiveObjectCallbackStreamImpl class
  //

  inline
  ActiveObjectCallbackStreamImpl::ActiveObjectCallbackStreamImpl(
    std::ostream& output_stream, const char* message_prefix,
    const char* aspect, const char* code) throw (eh::Exception)
    : LoggerHolder(Logging::Logger_var(new Logging::OStream::Logger(
        Logging::OStream::Config(output_stream)))),
      Logging::ActiveObjectCallbackImpl(LoggerHolder::logger, message_prefix,
        aspect, code)
  {
  }

  inline
  ActiveObjectCallbackStreamImpl::~ActiveObjectCallbackStreamImpl() throw ()
  {
  }
}

#endif


