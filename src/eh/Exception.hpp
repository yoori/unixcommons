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





#ifndef EH_EXCEPTION_HPP
#define EH_EXCEPTION_HPP

#include <stdexcept>
#include <string>


#if __GNUC__ == 4 && __GNUC_MINOR__ == 8
#else
#ifndef constexpr
#define constexpr
#endif
#endif

#define DECLARE_EXCEPTION(NAME, BASE) \
  class NAME##_tag_; \
  typedef eh::Composite<NAME##_tag_, BASE> NAME

namespace String
{
  template <typename CharType, typename Traits, typename Checker>
  class BasicSubString;
};

namespace Stream
{
  class Error;
}

namespace eh
{
  /** The standard library exception. */
  typedef std::exception Exception;

  /**
   * A descriptive exception.
   * Extends std::exception to provides textual information describing
   * the exception raised.
   */
  class DescriptiveException : public virtual Exception
  {
  protected:
    /**
     * Parametric constructor.
     * Initializes the exception instance using a null-terminated string.
     *
     * @param description The message to be associated with the exception.
     * @param code Additional code associated with the message.
     */
    explicit
    DescriptiveException(const char* description, const char* code = 0)
      throw ();

    /**
     * Parametric constructor. Initializes the exception instance using
     * a pointer to string and its length
     *
     * @param description The message to be associated with the exception.
     * @param length The message's length
     * @param code Additional code associated with the message.
     */
    DescriptiveException(const char* description, size_t length,
      const char* code = 0) throw ();

    /**
     * Parametric constructor.
     * Initializes the exception instance using an std::string.
     *
     * @param description The message to be associated with the exception.
     * @param code Additional code associated with the message.
     */
    explicit
    DescriptiveException(const std::string& description,
      const char* code = 0) throw ();

    /** Copy initialization constructor. */
    DescriptiveException(const DescriptiveException& exception) throw ();

    /** Assignment operator. */
    DescriptiveException&
    operator =(const DescriptiveException& exception) throw ();

  public:
    /** Destructor. */
    virtual
    ~DescriptiveException() throw ();

    /** Returns the message associated with the exception. */
    virtual
    const char*
    what() const throw ();

    virtual
    const char*
    code() const throw ();

  protected:
    DescriptiveException() throw ();

    void
    copy_string_(const char* src, char* dst, size_t size) throw ();

    void
    init_(const char* description, const char* code) throw ();
    void
    init_(const char* description, size_t length, const char* code)
      throw ();

  protected:
    enum { DESC_EXCEPTION_BUFFER_SIZE = 10 * 1024 };
    enum { CODE_EXCEPTION_BUFFER_SIZE = 64 };
    char description_[DESC_EXCEPTION_BUFFER_SIZE];
    char code_[CODE_EXCEPTION_BUFFER_SIZE];
  };

  /**
   * Template for creating derived exceptions.
   * The base class must implement certain initialization methods
   * (see DescriptiveException::init, for example).
   */
  template <typename Tag, typename Base>
  class Composite : public virtual Base
  {
  public:
    /**
     * Parametric constructor.
     * Initializes the exception instance using a null-terminated string.
     *
     * @param description The message to be associated with the exception.
     * @param code Additional code associated with the message.
     */
    explicit
    Composite(const char* description, const char* code = 0)
      throw ();

    /**
     * Parametric constructor.
     * Initializes the exception instance using
     * a pointer to string and its length
     *
     * @param description The message to be associated with the exception.
     * @param length The message's length
     * @param code Additional code associated with the message.
     */
    Composite(const char* description, size_t length,
      const char* code = 0) throw ();

    /**
     * Parametric constructor.
     * Initializes the exception instance using an std::string.
     *
     * @param description The message to be associated with the exception.
     * @param code Additional code associated with the message.
     */
    explicit
    Composite(const std::string& description, const char* code = 0)
      throw ();

    /**
     * Parametric constructor.
     * Initializes the exception instance using an String::SubString
     *
     * @param description The message to be associated with the exception.
     * @param code Additional code associated with the message.
     */
    template <typename CharType, typename Traits, typename Checker>
    explicit
    Composite(
      const String::BasicSubString<CharType, Traits, Checker>& description,
      const char* code = 0) throw ();

    /**
     * Parametric constructor.
     * Initializes the exception instance using Stream::Error.
     *
     * @param stream The message to be associated with the exception.
     * @param code Additional code associated with the message.
     */
    explicit
    Composite(const Stream::Error& stream, const char* code = 0) throw ();

  protected:
    Composite() throw ();
  };

  const char*
  code(const Exception& ex) throw ();
}

//
// Implementation
//

namespace eh
{
  //
  // DescriptiveException class
  //

  inline
  DescriptiveException::DescriptiveException() throw ()
  {
    init_(0, 0);
  }

  inline
  DescriptiveException::DescriptiveException(const char* description,
    const char* code) throw ()
  {
    init_(description, code);
  }

  inline
  DescriptiveException::DescriptiveException(const char* description,
    size_t length, const char* code) throw ()
  {
    init_(description, length, code);
  }

  inline
  DescriptiveException::DescriptiveException(const std::string& description,
    const char* code)
    throw ()
  {
    init_(description.data(), description.size(), code);
  }

  inline
  DescriptiveException::~DescriptiveException() throw ()
  {
    std::fill(description_, description_ + sizeof(description_), 0);
    std::fill(code_, code_ + sizeof(code_), 0);
  }

  inline
  DescriptiveException::DescriptiveException(
    const DescriptiveException& exception) throw ()
    : Exception()
  {
    init_(exception.description_, exception.code_);
  }

  inline
  DescriptiveException&
  DescriptiveException::operator =(const DescriptiveException& exception)
    throw ()
  {
    init_(exception.description_, exception.code_);

    return *this;
  }

  inline
  const char*
  DescriptiveException::what() const throw ()
  {
    return description_;
  }

  inline
  const char*
  DescriptiveException::code() const throw ()
  {
    return code_;
  }

  inline
  void
  DescriptiveException::copy_string_(const char* src, char* dst, size_t size)
    throw ()
  {
    if (src)
    {
      char* end = dst + size - 1;
      while ((*dst++ = *src++) && dst < end)
      {
      }
      *end = '\0';
    }
    else
    {
      *dst = '\0';
    }
  }

  inline
  void
  DescriptiveException::init_(const char* description, const char* code)
    throw ()
  {
    copy_string_(description, description_, DESC_EXCEPTION_BUFFER_SIZE);
    copy_string_(code, code_, CODE_EXCEPTION_BUFFER_SIZE);
  }

  inline
  void
  DescriptiveException::init_(const char* description, size_t size,
    const char* code) throw ()
  {
    if (size)
    {
      if (size > DESC_EXCEPTION_BUFFER_SIZE - 1)
      {
        size = DESC_EXCEPTION_BUFFER_SIZE - 1;
      }
      std::copy(description, description + size, description_);
    }
    description_[size] = '\0';

    copy_string_(code, code_, CODE_EXCEPTION_BUFFER_SIZE);
  }


  //
  // Composite class
  //

  template <typename Tag, typename Base>
  Composite<Tag, Base>::Composite() throw ()
  {
  }

  template <typename Tag, typename Base>
  Composite<Tag, Base>::Composite(const char* description, const char* code)
    throw ()
  {
    Base::init_(description, code);
  }

  template <typename Tag, typename Base>
  Composite<Tag, Base>::Composite(const char* description, size_t size,
    const char* code)
    throw ()
  {
    Base::init_(description, size, code);
  }

  template <typename Tag, typename Base>
  Composite<Tag, Base>::Composite(const std::string& description,
    const char* code) throw ()
  {
    Base::init_(description.data(), description.size(), code);
  }


  inline
  const char*
  code(const Exception& ex) throw ()
  {
    if (const DescriptiveException* de =
      dynamic_cast<const DescriptiveException*>(&ex))
    {
      return de->code();
    }
    return 0;
  }
} // namespace eh

#endif
