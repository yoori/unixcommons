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



#ifndef LOGGER_DISTRIBUTOR_LOGGER_HPP
#define LOGGER_DISTRIBUTOR_LOGGER_HPP

#include <ReferenceCounting/Deque.hpp>

#include <Logger/Logger.hpp>


namespace Logging
{
  /**
   * Selective logger
   * Publishes message only if its severity matches the severity interval
   */
  class SeveritySelectorLogger : public SimpleLoggerHolder
  {
  public:
    /**
     * Constructor
     * @param logger logger to use for messages publishing
     * @param low the low border of severity interval
     * @param high the high border of severity interval
     */
    SeveritySelectorLogger(Logger* logger,
      unsigned long low, unsigned long high = ULONG_MAX) throw ();

    /**
     * Constructor
     * @param logger logger to use for messages publishing
     * @param high the high border of severity interval
     */
    SeveritySelectorLogger(unsigned long high, Logger* logger) throw ();

    /*
     * Returns minimum of high severity bound and stored logger log level
     * @return effective trace level
     */
    virtual
    unsigned long
    log_level() throw ();

    /**
     * Passes log data to the contained logger if severity matches
     * @param text text to be logged
     * @param severity log record severity
     * @param aspect log record aspect
     * @param code log record code
     * @return false if severity does not match or contained logger's
     * result otherwise
     */
    virtual
    bool
    log(const String::SubString& text, unsigned long severity = INFO,
      const char* aspect = 0, const char* code = 0) throw ();

  protected:
    /**
     * Destructor
     */
    virtual
    ~SeveritySelectorLogger() throw ();

  private:
    unsigned long low_;
    unsigned long high_;
  };

  /**
   * Distributor of log messages among a set of [selective] loggers
   */
  class DistributorLogger :
    public Logger,
    public ReferenceCounting::AtomicImpl
  {
  public:
    /**
     * Constructor
     * @param begin begin of loggers to use
     * @param end end of loggers to use
     */
    template <typename InputIterator>
    DistributorLogger(InputIterator begin, InputIterator end)
      throw (eh::Exception);

    /*
     * Gets max logger level of stored loggers
     * @return current trace level
     */
    virtual
    unsigned long
    log_level() throw ();

    /*
     * Sets new trace level to the stored loggers
     * @param value new trace level
     */
    virtual
    void
    log_level(unsigned long value) throw ();

    /**
     * Passes log data to all of the stored loggers
     * @param text text to be logged
     * @param severity log record severity
     * @param aspect log record aspect
     * @param code log record code
     * @return if any contained logger returned true
     */
    virtual
    bool
    log(const String::SubString& text, unsigned long severity = INFO,
      const char* aspect = 0, const char* code = 0) throw ();

  protected:
    /**
     * Destructor
     */
    virtual
    ~DistributorLogger() throw ();

  private:
    typedef ReferenceCounting::Deque<QLogger_var> Loggers;

    Loggers loggers_;
  };
}

namespace Logging
{
  //
  // DistributorLogger
  //

  template <typename InputIterator>
  DistributorLogger::DistributorLogger(InputIterator begin,
    InputIterator end) throw (eh::Exception)
  {
    while (begin != end)
    {
      loggers_.push_back(ReferenceCounting::add_ref(*begin));
      ++begin;
    }
  }
}


#endif
