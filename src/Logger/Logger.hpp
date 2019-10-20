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





#ifndef LOGGER_LOGGER_HPP
#define LOGGER_LOGGER_HPP

#include <cstdarg>
#include <signal.h>

#include <ReferenceCounting/ReferenceCounting.hpp>

#include <Generics/Time.hpp>
#include <Generics/ThreadBuffer.hpp>
#include <Generics/ArrayAutoPtr.hpp>


/**
 * Common namespace for all logging related classes
 */
namespace Logging
{
  DECLARE_EXCEPTION(LoggerException, eh::DescriptiveException);

  class BasicLogger;
  class StreamLogger;
  class Logger;

  /**
   * Declares key logger interface.
   */
  class BaseLogger : public virtual ReferenceCounting::Interface
  {
  public:
    DECLARE_EXCEPTION(Exception, LoggerException);

    /**
     * Logger records severities.
     */
    enum Severity
    {
      EMERGENCY = 0,
      ALERT = 1,
      CRITICAL = 2,
      ERROR = 3,
      WARNING = 4,
      NOTICE = 5,
      INFO = 6,
      DEBUG = 7,
      TRACE = 8
    };

    /**
     * Gets logger trace level.
     * @return current trace level
     */
    virtual
    unsigned long
    log_level() throw () = 0;

    /**
     * Sets logger trace level.
     * Records with severity value higher than trace
     * level should not be logged.
     * @param value new log level.
     */
    virtual
    void
    log_level(unsigned long value) throw () = 0;

    /**
     * Logs text with severity, aspect and code specified.
     * @param text text to be logged
     * @param severity log record severity
     * @param aspect log record aspect
     * @param code log record code
     * @return success status
     */
    virtual
    bool
    log(const String::SubString& text, unsigned long severity = INFO,
      const char* aspect = 0, const char* code = 0) throw () = 0;

  protected:
    /**
     * Destructor
     */
    virtual
    ~BaseLogger() throw ();

  private:
    BaseLogger() throw ();

    friend class BasicLogger;
  };

  /**
   * Supplies simple usage for BaseLogger interface
   */
  class BasicLogger : public BaseLogger
  {
  public:
    using BaseLogger::log;

    /**
     * Logs formatted text with severity, aspect and code specified.
     * If enough memory is not available it tries to log only format string.
     * @param severity log record severity
     * @param aspect log record aspect
     * @param code log record code
     * @param format format string (printf-like)
     * @return success status
     */
    bool
    log(unsigned long severity, const char* aspect, const char* code,
      const char* format, ...) throw ()
      __attribute__((format(printf, 5, 6)));

    /**
     * Logs text with EMERGENCY severity, specified aspect and code.
     * @param text log record text
     * @param aspect log record aspect
     * @param code log record code
     * @return success status
     */
    bool
    emergency(const String::SubString& text, const char* aspect = 0,
      const char* code = 0)
      throw ();

    /**
     * Logs text with ALERT severity, specified aspect and code.
     * @param text log record text
     * @param aspect log record aspect
     * @param code log record code
     * @return success status
     */
    bool
    alert(const String::SubString& text, const char* aspect = 0,
      const char* code = 0)
      throw ();

    /**
     * Logs text with CRITICAL severity, specified aspect and code.
     * @param text log record text
     * @param aspect log record aspect
     * @param code log record code
     * @return success status
     */
    bool
    critical(const String::SubString& text, const char* aspect = 0,
      const char* code = 0)
      throw ();

    /**
     * Logs text with ERROR severity, specified aspect and code.
     * @param text log record text
     * @param aspect log record aspect
     * @param code log record code
     * @return success status
     */
    bool
    error(const String::SubString& text, const char* aspect = 0,
      const char* code = 0)
      throw ();

    /**
     * Logs text with WARNING severity, specified aspect and code.
     * @param text log record text
     * @param aspect log record aspect
     * @param code log record code
     * @return success status
     */
    bool
    warning(const String::SubString& text, const char* aspect = 0,
      const char* code = 0)
      throw ();

    /**
     * Logs text with NOTICE severity, specified aspect and code.
     * @param text log record text
     * @param aspect log record aspect
     * @param code log record code
     * @return success status
     */
    bool
    notice(const String::SubString& text, const char* aspect = 0,
      const char* code = 0)
      throw ();

    /**
     * Logs text with INFO severity, specified aspect and code.
     * @param text log record text
     * @param aspect log record aspect
     * @param code log record code
     * @return success status
     */
    bool
    info(const String::SubString& text, const char* aspect = 0,
      const char* code = 0)
      throw ();

    /**
     * Logs text with DEBUG severity, specified aspect and code.
     * @param text log record text
     * @param aspect log record aspect
     * @param code log record code
     * @return success status
     */
    bool
    debug(const String::SubString& text, const char* aspect = 0,
      const char* code = 0)
      throw ();

    /**
     * Logs text with TRACE or higher severity, specified aspect and code.
     * @param text log record text
     * @param aspect log record aspect
     * @param trace_level increase of TRACE severity
     * @param code log record code
     * @return success status
     */
    bool
    trace(const String::SubString& text, const char* aspect = 0,
      unsigned long trace_level = 0, const char* code = 0) throw ();

  protected:
    /**
     * Destructor
     */
    virtual
    ~BasicLogger() throw ();

  private:
    BasicLogger() throw ();

    friend class StreamLogger;
  };

  /**
   * Appends functions for stream-like usage
   */
  class StreamLogger : public BasicLogger
  {
  public:
    /**X
     */
    template <const size_t SIZE>
    struct StackWrapper : public Stream::Stack<SIZE>
    {
      StackWrapper(size_t size) throw (eh::Exception);
    };

    /**X
     */
    template <typename Stream, typename Initializer>
    class Wrapper : private Generics::Uncopyable
    {
    private:
      friend class StreamLogger;

      Wrapper(BasicLogger* logger, unsigned long severity, const char* aspect,
        const char* code, Initializer initializer)
        throw (eh::Exception);
      Wrapper(Wrapper<Stream, Initializer>&& wrapper)
        throw (eh::Exception);

    public:
      ~Wrapper() throw ();

      std::ostream&
      operator ()() throw ();

      template <typename Object>
      std::ostream&
      operator <<(const Object& object) throw (eh::Exception);

    private:
      BasicLogger* logger_;
      unsigned long severity_;
      const char* aspect_;
      const char* code_;
      Initializer initializer_;
      Stream ostr_;
    };

    static const size_t DEFAULT_BUFFER_SIZE = 32 * 1024;

    /**X
     */
    typedef Wrapper<Stream::MemoryStream::OutputMemoryStream<char>, size_t>
      WrapperAlloc;

    /**X
     */
    typedef Wrapper<StackWrapper<DEFAULT_BUFFER_SIZE>, size_t>
      WrapperStack;

    /**
     */
    template <const size_t SIZE>
    class Buffer
    {
    private:
      friend class StreamLogger;

      char buffer_[SIZE];
    };
    typedef Buffer<DEFAULT_BUFFER_SIZE> DBuffer;


    /**
     * Creates stream-like object allowing to use stream operations
     * for composition of log message. Logs this message with specified
     * severity, aspect and code.
     * An exception can be thrown during composition.
     * @param severity log record severity
     * @param aspect log record aspect (it should not be a pointer to a
     * temporal object)
     * @param code log record code (it should not be a pointer to a temporal
     * object)
     * @param initial_size initial size for memory stream object
     * @return stream-like object
     */
    WrapperAlloc
    stream(unsigned long severity, const char* aspect = 0,
      const char* code = 0, size_t initial_size = 8192) throw (eh::Exception);

    /**
     * Creates stream-like object allowing to use stream operations
     * for composition of log message. Logs this message with specified
     * severity, aspect and code.
     * No allocations are performed in the stream itself. Stream is created
     * on stack and contains the buffer of size SIZE. No more than SIZE-1
     * bytes can be written into the stream. Make sure you have the rest
     * of the stack large enough to contain the entire stream.
     * @param severity log record severity
     * @param aspect log record aspect (it should not be a pointer to a
     * temporal object)
     * @param code log record code (it should not be a pointer to a temporal
     * object)
     * @return stream-like object
     */
    template <const size_t SIZE>
    Wrapper<StackWrapper<SIZE>, size_t>
    stream(unsigned long severity, const char* aspect = 0,
      const char* code = 0) throw (eh::Exception);


    /**
     * Creates stream-like object allowing to use stream operations
     * for composition of log message. Logs this message with specified
     * severity, aspect and code.
     * No allocations are performed in the stream itself. Stream is created
     * using passed buffer object.
     * @param buffer memory for log message
     * @param severity log record severity
     * @param aspect log record aspect (it should not be a pointer to a
     * temporal object)
     * @param code log record code (it should not be a pointer to a temporal
     * object)
     * @return stream-like object
     */
    template <const size_t SIZE>
    Wrapper<Stream::Buffer<SIZE>, char*>
    stream(Buffer<SIZE>& buffer, unsigned long severity,
      const char* aspect = 0, const char* code = 0) throw (eh::Exception);


    /**
     * Creates stream-like object allowing to use stream operations
     * for composition of log message. Logs this message with specified
     * severity, aspect and code.
     * No allocations are performed in the stream itself. Stream is created
     * using TLS buffer of size DEFAULT_STACK_STREAM_SIZE.
     * No more than DEFAULT_STACK_STREAM_SIZE-1
     * bytes can be written into the stream. Make sure you have the rest
     * of the stack large enough to contain the entire stream.
     * @param severity log record severity
     * @param aspect log record aspect (it should not be a pointer to a
     * temporal object)
     * @param code log record code (it should not be a pointer to a temporal
     * object)
     * @return stream-like object
     */
    Wrapper<Stream::Buffer<StreamLogger::DEFAULT_BUFFER_SIZE>, char*>
    sstream(unsigned long severity, const char* aspect = 0,
      const char* code = 0) throw (eh::Exception);

  protected:
    virtual
    ~StreamLogger() throw ();

  private:
    StreamLogger() throw ();

    friend class Logger;


    typedef Generics::ThreadBuffer<Logger, DEFAULT_BUFFER_SIZE, 100>
      ThreadBuffer;

    static ThreadBuffer thread_buffer_;
  };

  /**
   * Base class for all of the loggers
   */
  class Logger : public StreamLogger
  {
  protected:
    /**
     * Destructor
     */
    virtual
    ~Logger() throw ();
  };
  typedef ReferenceCounting::SmartPtr<Logger> Logger_var;
  typedef ReferenceCounting::QualPtr<Logger> QLogger_var;
  typedef ReferenceCounting::FixedPtr<Logger> FLogger_var;

  /**
   * Simple class proxy for Logger
   * holds own logger, tranship calls to held logger
   * Immutable
   */
  class SimpleLoggerHolder :
    public Logger,
    public ReferenceCounting::AtomicImpl
  {
  public:
    /*
     * Construct holder with logger to hold
     * @param logger logger to hold
     */
    explicit
    SimpleLoggerHolder(Logger* logger) throw ();

    /**
     * Gets logger trace level.
     * @return current trace level
     */
    virtual
    unsigned long
    log_level() throw ();

    /**
     * Sets logger trace level.
     * Records with severity value higher than trace
     * level should not be logged.
     * @param value new log level.
     */
    virtual
    void
    log_level(unsigned long value) throw ();

    /**
     * Logs text with severity, aspect and code specified.
     * @param text text to be logged
     * @param severity log record severity
     * @param aspect log record aspect
     * @param code log record code
     * @return success status
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
    ~SimpleLoggerHolder() throw ();

    mutable QLogger_var logger_;
  };

  /*
   * Class proxy for Logger
   * Holds own logger, tranship calls to held logger
   * Thread safe
   */
  class LoggerHolder : public SimpleLoggerHolder
  {
  public:
    /*
     * Construct holder with logger to hold
     * @param logger logger to hold
     */
    explicit
    LoggerHolder(Logger* logger = 0) throw ();

    /*
     * Set in held logger
     * @param logger logger to hold
     */
    void
    logger(Logger* logger) throw ();

    /**
     * Gets logger trace level.
     * @return current trace level
     */
    virtual
    unsigned long
    log_level() throw ();
    /**
     * Sets logger trace level.
     * Records with severity value higher than trace
     * level should not be logged.
     * @param value new log level.
     */
    virtual
    void
    log_level(unsigned long value) throw ();

    /**
     * Logs text with severity, aspect and code specified.
     * @param text text to be logged
     * @param severity log record severity
     * @param aspect log record aspect
     * @param code log record code
     * @return success status
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
    ~LoggerHolder() throw ();

  private:
    /*
     * Check - if logger is held here
     * @return true if logger present
     */
    bool
    has_logger_() throw ();

    Sync::PosixSpinLock mutex_;
    volatile sig_atomic_t log_level_;
  };
  typedef ReferenceCounting::QualPtr<LoggerHolder> LoggerHolder_var;

  /**
   * Logger uses another logger and puts predefined aspect and/or error_code
   * if required.
   */
  class LoggerDefaultHolder : public LoggerHolder
  {
  public:
    /**
     * Constructor
     * @param logger logger to hold
     * @param aspect aspect to use if unspecified in log() call
     * @param code error code to use if unspecified in log() call
     */
    explicit
    LoggerDefaultHolder(Logger* logger = 0, const char* aspect = 0,
      const char* code = 0) throw (eh::Exception);

    /**
     * Logs text with severity, aspect and code specified.
     * @param text text to be logged
     * @param severity log record severity
     * @param aspect log record aspect
     * @param code log record code
     * @return success status
     */
    virtual
    bool
    log(const String::SubString& text, unsigned long severity = INFO,
      const char* aspect = 0, const char* code = 0) throw ();

  protected:
    virtual
    ~LoggerDefaultHolder() throw () = default;

    std::string aspect_;
    std::string code_;
  };

  /**
   * Log record to be passed to Formatter and Handler
   */
  struct LogRecord
  {
    String::SubString text; /**X Text to log. */
    unsigned long severity;  /**X Log record severity. */
    String::SubString aspect; /**X Log record aspect. */
    String::SubString code; /**X Error code. */
    Generics::Time time; /**X Time log record produced. */
    Generics::Time::TimeZone time_zone; /**X Preferred time zone for logging */
  };

  /**
   * Log backend interface. Responsible for placing log record into
   * corresponding media: file, stream, network connection, ...
   */
  class Handler : public virtual ReferenceCounting::Interface
  {
  public:
    DECLARE_EXCEPTION(Exception, LoggerException);

    /**
     * Places record into corresponding media.
     * @param record log record to publish
     */
    virtual
    void
    publish(const LogRecord& record)
      throw (Exception, eh::Exception) = 0;

  protected:
    /**
     * Destructor
     */
    virtual
    ~Handler() throw ();
  };
  typedef ReferenceCounting::QualPtr<Handler> Handler_var;

  /**
   * Log record formatter. Responsible for converting
   * log record into plain text, possibly prepending initial line with
   * additional information: time, severity, aspect, ...
   */
  class Formatter : public virtual ReferenceCounting::Interface
  {
  public:
    DECLARE_EXCEPTION(Exception, LoggerException);

    /**
     * Converts record into text string.
     * @param record log record to format
     * @return formatted text string
     */
    Generics::ArrayChar
    format(const LogRecord& record) const
      throw (Exception, eh::Exception);

    /**
     * Calculated required size for record to format
     * @param record log record to format
     * @return memory size to use
     */
    virtual
    size_t
    required_size(const LogRecord& record) const
      throw (Exception, eh::Exception);

    /**
     * Formats record into external memory
     * @param record log record to format
     * @param buf external memory start
     * @param size external memory size
     * @return whether the record has been formatted or not
     */
    virtual
    bool
    format(const LogRecord& record, char* buf, size_t size) const
      throw (Exception, eh::Exception);

  protected:
    /**
     * Destructor.
     */
    virtual
    ~Formatter() throw ();
  };
  typedef ReferenceCounting::ConstPtr<Formatter> Formatter_var;


  /**
   * Wrapper for Formatter
   * Calls on of format() functions depending on availability of
   * preallocated memory
   */
  class FormatWrapper
  {
  public:
    /**
     * Result type of for format() call.
     * May contain allocated array of chars to free.
     */
    class Result
    {
    public:
      /**
       * Constructor
       * @param ptr pointer to formatted message
       * @param buf potentially allocated buffer to free
       */
      Result(const char* ptr, Generics::ArrayChar&& buf) throw ();

      /**
       * Move constructor. Moves content of result into the
       * constructed object
       * @param result source object
       */
      Result(Result&& result) throw ();

      /**
       * Returns pointer to formatted message
       * @return formatted message or NULL if formatting error occurred
       */
      const char*
      get() const throw ();

    private:
      const char* ptr_;
      Generics::ArrayChar buf_;
    };

    /**
     * Constructor
     * @param formatter formatter to use
     * @param size buffer to preallocate. zero - don't use preallocation,
     * allocate buffer on each call to format()
     */
    FormatWrapper(const Formatter* formatter, size_t size)
      throw (eh::Exception);

    /**
     * Format log record
     * @param record log record to format
     * @return formatted string in Result
     */
    Result
    format(const LogRecord& record) const throw (eh::Exception);

  private:
    /**
     * Creates default (simple) formatter if the one is not passed
     */
    static
    Formatter_var
    create_default_formatter_() throw (eh::Exception);

    const Formatter_var FORMATTER_;
    const size_t ALLOCATED_;
    const Generics::ArrayChar BUFFER_;
  };


  namespace Null
  {
    /**
     * Logger null implementation (i.e. no logging).
     */
    class Logger :
      public ::Logging::Logger,
      public ReferenceCounting::AtomicImpl
    {
    public:
      /**
       * Gets logger trace level
       * @return zero
       */
      virtual
      unsigned long
      log_level() throw ();

      /**X
       * Does nothing
       * @param value new log level
       */
      virtual
      void
      log_level(unsigned long value) throw ();

      /**
       * Ignores passed log record information
       * @param text text to be logged
       * @param severity log record severity
       * @param aspect log record aspect
       * @param code log record code
       * @return true
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
      ~Logger() throw ();
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace Logging
{
  //
  // BaseLogger class
  //

  inline
  BaseLogger::BaseLogger() throw ()
  {
  }

  inline
  BaseLogger::~BaseLogger() throw ()
  {
  }


  //
  // BasicLogger class
  //

  inline
  BasicLogger::BasicLogger() throw ()
  {
  }

  inline
  BasicLogger::~BasicLogger() throw ()
  {
  }

  inline
  bool
  BasicLogger::emergency(const String::SubString& text, const char* aspect,
    const char* code) throw ()
  {
    return log(text, EMERGENCY, aspect, code);
  }

  inline
  bool
  BasicLogger::alert(const String::SubString& text, const char* aspect,
    const char* code)
    throw ()
  {
    return log(text, ALERT, aspect, code);
  }

  inline
  bool
  BasicLogger::critical(const String::SubString& text, const char* aspect,
    const char* code) throw ()
  {
    return log(text, CRITICAL, aspect, code);
  }

  inline
  bool
  BasicLogger::error(const String::SubString& text, const char* aspect,
    const char* code)
    throw ()
  {
    return log(text, ERROR, aspect, code);
  }

  inline
  bool
  BasicLogger::warning(const String::SubString& text, const char* aspect,
    const char* code)
    throw ()
  {
    return log(text, WARNING, aspect, code);
  }

  inline
  bool
  BasicLogger::notice(const String::SubString& text, const char* aspect,
    const char* code)
    throw ()
  {
    return log(text, NOTICE, aspect, code);
  }

  inline
  bool
  BasicLogger::info(const String::SubString& text, const char* aspect,
    const char* code)
    throw ()
  {
    return log(text, INFO, aspect, code);
  }

  inline
  bool
  BasicLogger::debug(const String::SubString& text, const char* aspect,
    const char* code)
    throw ()
  {
    return log(text, DEBUG, aspect, code);
  }

  inline
  bool
  BasicLogger::trace(const String::SubString& text, const char* aspect,
    unsigned long trace_level, const char* code) throw ()
  {
    return log(text, TRACE + trace_level, aspect, code);
  }

  inline
  bool
  BasicLogger::log(unsigned long severity, const char* aspect,
    const char* code, const char* format, ...) throw ()
  {
    int n;
    char* text = 0;
    std::va_list ap;
    va_start(ap, format);
    n = vasprintf(&text, format, ap);
    va_end(ap);

    if (n == -1)
    {
      return log(String::SubString(format), severity, aspect, code);
    }
    else
    {
      bool ret = log(String::SubString(text, n), severity, aspect, code);
      free(text);
      return ret;
    }
  }


  //
  // StreamLogger::StackWrapper class
  //

  template <const size_t SIZE>
  StreamLogger::StackWrapper<SIZE>::StackWrapper(size_t /*size*/)
    throw (eh::Exception)
  {
  }


  //
  // StreamLogger::Wrapper class
  //

  template <typename Stream, typename Initializer>
  StreamLogger::Wrapper<Stream, Initializer>::Wrapper(
    BasicLogger* logger, unsigned long severity, const char* aspect,
    const char* code, Initializer initializer) throw (eh::Exception)
    : logger_(logger), severity_(severity), aspect_(aspect),
      code_(code), initializer_(initializer), ostr_(initializer)
  {
  }

  template <typename Stream, typename Initializer>
  StreamLogger::Wrapper<Stream, Initializer>::Wrapper(
    Wrapper<Stream, Initializer>&& wrapper)
    throw (eh::Exception)
    : Generics::Uncopyable(),
      logger_(wrapper.logger_), severity_(wrapper.severity_),
      aspect_(wrapper.aspect_), code_(wrapper.code_),
      ostr_(wrapper.initializer_)
  {
    wrapper.logger_ = 0;
  }

  template <typename Stream, typename Initializer>
  StreamLogger::Wrapper<Stream, Initializer>::~Wrapper() throw ()
  {
    if (!logger_)
    {
      return;
    }

    try
    {
      logger_->log(ostr_.str(), severity_, aspect_, code_);
    }
    catch (...)
    {
      ::Stream::Error ostr;
      ostr << FNS << "Failed to log";
      logger_->critical(ostr.str());
    }
  }

  template <typename Stream, typename Initializer>
  std::ostream&
  StreamLogger::Wrapper<Stream, Initializer>::operator ()() throw ()
  {
    return ostr_;
  }

  template <typename Stream, typename Initializer>
  template <typename Object>
  std::ostream&
  StreamLogger::Wrapper<Stream, Initializer>::operator <<(
    const Object& object) throw (eh::Exception)
  {
    return ostr_ << object;
  }


  //
  // StreamLogger class
  //

  inline
  StreamLogger::StreamLogger() throw ()
  {
  }

  inline
  StreamLogger::~StreamLogger() throw ()
  {
  }

  inline
  StreamLogger::WrapperAlloc
  StreamLogger::stream(unsigned long severity, const char* aspect,
    const char* code, size_t initial_size) throw (eh::Exception)
  {
    return WrapperAlloc(this, severity, aspect, code, initial_size);
  }

  template <const size_t SIZE>
  StreamLogger::Wrapper<StreamLogger::StackWrapper<SIZE>, size_t>
  StreamLogger::stream(unsigned long severity, const char* aspect,
    const char* code) throw (eh::Exception)
  {
    return Wrapper<StackWrapper<SIZE>, size_t>(this, severity, aspect,
      code, SIZE);
  }

  template <const size_t SIZE>
  StreamLogger::Wrapper<Stream::Buffer<SIZE>, char*>
  StreamLogger::stream(Buffer<SIZE>& buffer, unsigned long severity,
    const char* aspect, const char* code) throw (eh::Exception)
  {
    return Wrapper<Stream::Buffer<SIZE>, char*>(this, severity, aspect,
      code, buffer.buffer_);
  }

  inline
  StreamLogger::Wrapper<Stream::Buffer<
    StreamLogger::DEFAULT_BUFFER_SIZE>, char*>
  StreamLogger::sstream(unsigned long severity, const char* aspect,
    const char* code) throw (eh::Exception)
  {
    return Wrapper<Stream::Buffer<DEFAULT_BUFFER_SIZE>, char*>(this,
      severity, aspect, code, thread_buffer_.get_buffer());
  }


  //
  // Logger class
  //

  inline
  Logger::~Logger() throw ()
  {
  }


  //
  // Handler class
  //

  inline
  Handler::~Handler() throw ()
  {
  }


  //
  // Formatter class
  //

  inline
  Formatter::~Formatter() throw ()
  {
  }

  inline
  Generics::ArrayChar
  Formatter::format(const LogRecord& record) const
    throw (Exception, eh::Exception)
  {
    size_t size = required_size(record);
    Generics::ArrayChar buffer(size);
#ifndef NDEBUG
    bool result = format(record, buffer.get(), size);
    assert(result);
#else
    format(record, buffer.get(), size);
#endif
    return buffer;
  }

  inline
  size_t
  Formatter::required_size(const LogRecord& /*record*/) const
    throw (Exception, eh::Exception)
  {
    return 0;
  }

  inline
  bool
  Formatter::format(const LogRecord& /*record*/, char* /*buf*/,
    size_t /*size*/) const throw (Exception, eh::Exception)
  {
    return false;
  }


  //
  // FormatWrapper::Result class
  //

  inline
  FormatWrapper::Result::Result(const char* ptr, Generics::ArrayChar&& buf)
    throw ()
    : ptr_(ptr), buf_(std::move(buf))
  {
  }

  inline
  FormatWrapper::Result::Result(Result&& result) throw ()
    : ptr_(result.ptr_), buf_(std::move(result.buf_))
  {
  }

  inline
  const char*
  FormatWrapper::Result::get() const throw ()
  {
    return ptr_;
  }


  //
  // FormatWrapper class
  //

  inline
  FormatWrapper::FormatWrapper(const Formatter* formatter, size_t size)
    throw (eh::Exception)
    : FORMATTER_(ReferenceCounting::add_ref(formatter ? formatter :
        create_default_formatter_().in())), ALLOCATED_(size), BUFFER_(size)
  {
  }

  inline
  FormatWrapper::Result
  FormatWrapper::format(const LogRecord& record) const throw (eh::Exception)
  {
    if (!ALLOCATED_)
    {
      Generics::ArrayChar result(FORMATTER_->format(record));
      const char* ptr = result.get();
      return Result(ptr, std::move(result));
    }

    return Result(FORMATTER_->format(record, BUFFER_.get(), ALLOCATED_) ?
      BUFFER_.get() : 0, Generics::ArrayChar());
  }


  namespace Null
  {
    //
    // Logger class
    //

    inline
    Logger::~Logger() throw ()
    {
    }

    inline
    unsigned long
    Logger::log_level() throw ()
    {
      return 0;
    }

    inline
    void
    Logger::log_level(unsigned long /*level*/) throw ()
    {
    }

    inline
    bool
    Logger::log(const String::SubString& /*text*/, unsigned long /*severity*/,
      const char* /*aspect*/, const char* /*code*/) throw ()
    {
      return true;
    }
  }

  //
  // LoggerHolder class
  //

  inline
  bool
  LoggerHolder::has_logger_() throw ()
  {
    return logger_ != 0;
  }
}

#endif
