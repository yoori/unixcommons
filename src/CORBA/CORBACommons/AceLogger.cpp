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



#include <ace/Log_Msg_Backend.h>
#include <ace/Log_Record.h>

#include "CorbaAdaptersInternal.hpp"


// If CorbaServerAdapter and CorbaClientAdapter don't provide
// Logging::Logger, TAO and ACE log messages will be ignored.
// Undef this to put logs into std::cerr, if no logger is
// provided through CorbaClientAdapter or CorbaClientServer.
// If defined, ACE and TAO logs can be put only in user loggers.
#define SILENT_WITHOUT_CUSTOM_LOGGER


namespace
{
  //
  // ACELoggerHook class
  //

  class ACELoggerHook :
    public ACE_Log_Msg_Backend,
    private Generics::Uncopyable
  {
  public:
    /**
     * Constructor hooks ACE_Log_Msg mechanism to special backend.
     * This backend will be used later for redirect TAO logs into
     * Phorm logger.
     * We should do this here because backends mode should be set before
     * TAO create own threads with own ACE loggers. After backend set
     * spawning threads will inherit this logging option.
     */
    ACELoggerHook() throw ();

    /**
     * Open the back end object. Perform any actions needed to prepare
     * the object for later logging operations.
     *
     * @param logger_key  The character string passed to ACE_Log_Msg::open().
     *                    If the @c LOGGER logging destination is not being
     *                    used, any string can be passed through to the
     *                    back end.
     * @return 0 for success.
     * @return -1 for failure.
     */
    virtual
    int
    open(const ACE_TCHAR* logger_key) throw ();

    /**
     * Reset the backend.  If ACE_Log_Msg is reopened during execution, this
     * hook will be called. This method should perform any needed cleanup
     * activity (similar to close()) because this object won't be reopened
     * if the new open call does not specify use of this back end being reset.
     * @return Currently ignored, but to be safe, return 0 for success;
     *         -1 for failure.
     */
    virtual
    int
    reset() throw ();

    /// Close the backend completely.
    virtual
    int
    close() throw ();

    /**
     * Process a log record.
     * @param log_record   The ACE_Log_Record to process.
     * @return -1 for failure; else it is customarily the number of bytes
     *         processed, but can also be 0 to signify success.
     */
    virtual
    ssize_t
    log(ACE_Log_Record& log_record) throw ();

    /**
     * Remove logger from internal counting map.
     * @param this pointer to logger used as key for logger count
     */
    void
    remove_logger(Logging::Logger* logger) throw ();

    /**
     * Memorize phorm logger, there delegate ACE_Log_Msg calls.
     * @param pointer to new or already count logger
     * used as key for logger count.
     */
    void
    add_logger(Logging::Logger* logger) throw (eh::Exception);

  private:
    /// Convert ACE message types to Phorm logger types.
    Logging::Logger::Severity
    convert_severity(ACE_UINT32 ace_severity) const throw ();

    struct LoggerCounter
    {
      explicit
      LoggerCounter(Logging::Logger* logger = 0) throw ();
      LoggerCounter(LoggerCounter&&) throw ();

      Logging::QLogger_var logger;
      unsigned count;
    };

    typedef ReferenceCounting::Map<Logging::Logger*, LoggerCounter>
      UsedLoggers;
    UsedLoggers loggers_;

    typedef Sync::PosixRWLock Mutex_;
    typedef Sync::PosixRGuard ReadGuard_;
    typedef Sync::PosixWGuard WriteGuard_;
    Mutex_ lock_;
  };

  ACELoggerHook::LoggerCounter::LoggerCounter(Logging::Logger* logger)
    throw ()
    : logger(ReferenceCounting::add_ref(logger)), count(0)
  {
  }

  ACELoggerHook::LoggerCounter::LoggerCounter(LoggerCounter&& l)
    throw ()
    : logger(std::move(l.logger)), count(l.count)
  {
  }

  // This variable should be created before any object that
  // logs ACE or TAO events.
  ACELoggerHook* ace_logger_replacement = new ACELoggerHook;

  //
  // ACELoggerHook implementation
  //

  ACELoggerHook::ACELoggerHook() throw ()
  {
    ACE_Log_Msg::instance()->msg_backend(this);
    ACE_Log_Msg::instance()->clr_flags(ACE_Log_Msg::STDERR);
    ACE_Log_Msg::instance()->set_flags(ACE_Log_Msg::CUSTOM |
      ACE_Log_Msg::LOGGER);
  }

  int
  ACELoggerHook::open(const ACE_TCHAR* /*logger_key*/) throw ()
  {
    return 0;
  }

  int
  ACELoggerHook::reset() throw ()
  {
    return 0;
  }

  int
  ACELoggerHook::close() throw ()
  {
    return 0;
  }

  void
  ACELoggerHook::remove_logger(Logging::Logger* logger) throw ()
  {
    Logging::Logger_var logger_find(ReferenceCounting::add_ref(logger));
    WriteGuard_ guard(lock_);
    UsedLoggers::iterator it = loggers_.find(logger_find);
    if (it != loggers_.end())
    {
      if (!--it->second.count)
      {
        loggers_.erase(it);
      }
    }
  }

  void
  ACELoggerHook::add_logger(Logging::Logger* logger) throw (eh::Exception)
  {
    LoggerCounter lc(logger);
    WriteGuard_ guard(lock_);
    loggers_.insert(UsedLoggers::value_type(logger, std::move(lc))).first->
      second.count++;
  }

  ssize_t
  ACELoggerHook::log(ACE_Log_Record& log_record) throw ()
  {
    ReadGuard_ guard(lock_);
    if (!loggers_.empty())
    {
      const Logging::Logger::Severity SEVERITY =
        convert_severity(log_record.type());
      for (UsedLoggers::iterator it(loggers_.begin());
        it != loggers_.end(); ++it)
      {
        it->first->log(String::SubString(log_record.msg_data(),
          log_record.msg_data_len()), SEVERITY);
      }
    }
#ifndef SILENT_WITHOUT_CUSTOM_LOGGER
    else
    {
      std::cerr << log_record.msg_data();
    }
#endif
    return 0; // success (-1 if error)
  }

  Logging::Logger::Severity
  ACELoggerHook::convert_severity(ACE_UINT32 ace_severity) const throw ()
  {
    switch (ace_severity)
    {
    case LM_TRACE:
      return Logging::Logger::TRACE;
    case LM_DEBUG:
      return Logging::Logger::DEBUG;
    case LM_NOTICE:
      return Logging::Logger::NOTICE;
    case LM_WARNING:
      return Logging::Logger::WARNING;
    case LM_ERROR:
      return Logging::Logger::ERROR;
    case LM_CRITICAL:
      return Logging::Logger::CRITICAL;
    case LM_ALERT:
      return Logging::Logger::ALERT;
    case LM_EMERGENCY:
      return Logging::Logger::EMERGENCY;
    default:
      // LM_INFO, LM_SHUTDOWN, LM_STARTUP, LM_MAX, etc
      // return as INFO severity
      return Logging::Logger::INFO; 
    }
  }
}


namespace CORBACommons
{
  namespace AceLogger
  {
    void
    add_logger(Logging::Logger* logger) throw (eh::Exception)
    {
      ace_logger_replacement->add_logger(logger);
    }

    void
    remove_logger(Logging::Logger* logger) throw ()
    {
      ace_logger_replacement->remove_logger(logger);
    }
  }
}
