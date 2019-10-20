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





#ifndef STREAM_BINARY_STREAM_HPP
#define STREAM_BINARY_STREAM_HPP

#include <ostream>
#include <istream>

#include <sys/types.h>

#include <eh/Exception.hpp>

#include <Generics/Uncopyable.hpp>


namespace Stream
{
  /**
   * Base class for BinaryInputStream and BinaryOutputStream.
   * Encapsulates stream state and few typedefs.
   */
  class StreamBase : private Generics::Uncopyable
  {
  public:
    typedef int int_type;
    typedef char char_type;
    typedef unsigned long streamsize;

    /**
     * Construct object in good state
     */
    StreamBase() throw ();

    /**
     * Empty virtual destructor
     */
    virtual
    ~StreamBase() throw ();

    /**
     * Indicates if the stream is still good
     * @return The operator returns a null pointer only if fail()
     */
    operator const void*() const throw ();

    /**
     * Indicates if the stream is not bad
     * @return returns fail()
     */
    bool
    operator !() const throw ();

    /**
     * Reads the state of bits for flags
     * @return The stored stream state information
     */
    std::ios_base::iostate
    rdstate() const throw ();

    /**
     * Clears all error flags
     * @param state The flags you want to set after clearing all flags,
     * optional
     */
    void
    clear(std::ios_base::iostate state = std::ios_base::goodbit)
      throw (eh::Exception);

    /**
     * Sets additional flags
     * @param state Additional flags to set
     */
    void
    setstate(std::ios_base::iostate state)
      throw (eh::Exception);

    /**
     * Indicates the state of rdstate() & std::ios_base::goodbit
     * @return true if rdstate() == goodbit (no state flags are set),
     * otherwise, false
     */
    bool
    good() const throw ();

    /**
     * Indicates if the end of a stream has been reached
     * @return true if the end of the stream has been reached,
     *  false otherwise
     */
    bool
    eof() const throw ();

    /**
     * Indicates the status of
     * rdstate() & (std::ios_base::failbit | std::ios_base::badbit)
     * @return true if rdstate & (failbit | badbit) is nonzero,
     * otherwise false
     */
    bool
    fail() const throw ();

    /**
     * Indicates the state of rdstate() & std::ios_base::badbit
     * @return true if rdstate & badbit is nonzero; otherwise false
     */
    bool
    bad() const throw ();

    /**
     * Indicates which exceptions will be thrown by the stream
     * @return The flags that are currently specified to thrown
     * an exception for the stream
     */
    std::ios_base::iostate
    exceptions() const throw ();

    /**
     * Set new exceptions mask
     * @param except The flags that you want to throw an exception
     */
    void
    exceptions(std::ios_base::iostate except) throw (eh::Exception);

  protected:
    std::ios_base::iostate state_;
    std::ios_base::iostate exceptions_;
  };

  /**
   * Interface for binary input.
   */
  class BinaryInputStream : public StreamBase
  {
  public:
    /**
     * Constructor initialize the extraction count to zero
     */
    BinaryInputStream() throw ();

    /**
     * Get the number of characters read during the last unformatted input
     * @return The number of characters extracted by the last unformatted
     * input member function called for the object.
     */
    streamsize
    gcount() const throw ();

    // TODO: Do we need this?
    // virtual
    // int_type
    // get() throw (eh::Exception);

    /**
     * Function extracts the int_type element meta. If meta compares equal
     * to traits_type::eof, the function calls setstate(failbit). Otherwise,
     * it stores traits_type::to_char_type(meta) in c.
     * @param c Character to store result
     * @return Reference on self
     */
    virtual
    BinaryInputStream&
    get(char_type& c) throw (eh::Exception);

    /**
     * Reads a specified number of characters from the stream and stores them
     * in an array
     * @param s The array in which to read the characters
     * @param n The number of characters to read
     */
    virtual
    BinaryInputStream&
    read(char_type* s, streamsize n) throw (eh::Exception) = 0;

    // TODO: do we need these?
    // BinaryInputStream&
    // ignore(streamsize n = 1) throw (eh::Exception);
    // int_type
    // peek() throw (eh::Exception);
    // BinaryInputStream&
    // putback(char_type c) throw (eh::Exception);
    // BinaryInputStream&
    // unget() throw (eh::Exception);

  protected:
    streamsize gcount_;
  };

  /**
   * Interface for binary output.
   */
  class BinaryOutputStream : public StreamBase
  {
  public:
    /**
     * The unformatted output function inserts the element c in a stream
     * @param c A character
     * @return Reference to self
     */
    virtual
    BinaryOutputStream&
    put(char_type c) throw (eh::Exception);

    /**
     * Put characters in a stream
     * @param s Characters to put into the stream
     * @param n Count of characters to put into the stream
     * @return Reference on self
     */
    virtual
    BinaryOutputStream&
    write(const char_type* s, streamsize n) throw (eh::Exception) = 0;
  };

  /**
   * Buffer class that allows to use standart STL streams on top of
   * binary streams
   */
  class BinaryStreambuf :
    public std::basic_streambuf<char, std::char_traits<char> >
  {
  public:
    /**
     * Construct buffer work with input stream
     */
    BinaryStreambuf(BinaryInputStream* in) throw (eh::Exception);
 
    /**
     * Construct buffer work with output stream
     */
    BinaryStreambuf(BinaryOutputStream* out) throw (eh::Exception);

  protected:
    //
    // read functions
    //

    virtual
    std::streamsize
    showmanyc() throw (eh::Exception);

    virtual
    int_type
    underflow() throw (eh::Exception);

    // virtual
    // std::streamsize
    // xsgetn(char_type* s, std::streamsize n) throw (eh::Exception);

    //
    // write functions
    //

    ssize_t
    flush_buffer() throw (eh::Exception);

    virtual
    int
    sync() throw (eh::Exception);

    virtual
    int_type
    overflow(int_type c = traits_type::eof()) throw (eh::Exception);
    // virtual
    // std::streamsize
    // xsputn(const char_type* s, std::streamsize n) throw (eh::Exception);

  protected:
    static const size_t BUFFER_SIZE = 1024;
    static const size_t PUTBACK_SIZE = 20;

    BinaryInputStream* in_;
    BinaryOutputStream* out_;
    char buffer_[BUFFER_SIZE];
  };

  /**
   * Adapter class to input text data from binary stream
   */
  class BinaryStreamReader : public std::istream
  {
  public:
    /**
     * Constructor call base class constructor with zero
     */
    BinaryStreamReader(BinaryInputStream* in) throw (eh::Exception);

  protected:
    BinaryStreambuf buf_;
  };

  /**
   * Adapter class to output text data to binary stream
   */
  class BinaryStreamWriter : public std::ostream
  {
  public:
    /**
     * Constructor call base class constructor with zero
     */
    BinaryStreamWriter(BinaryOutputStream* out) throw (eh::Exception);

  protected:
    BinaryStreambuf buf_;
  };
} // namespace Stream

//
// INLINES
//

namespace Stream
{
  //
  // StreamBase class
  //

  inline
  StreamBase::StreamBase() throw ()
    : state_(std::ios_base::goodbit), exceptions_(std::ios_base::goodbit)
  {
  }

  inline
  StreamBase::~StreamBase() throw ()
  {
  }

  inline
  StreamBase::operator const void*() const throw ()
  {
    return fail() ? 0 : static_cast<const void*>(this);
  }

  inline
  bool
  StreamBase::operator !() const throw ()
  {
    return fail();
  }

  inline
  std::ios_base::iostate
  StreamBase::rdstate() const throw ()
  {
    return state_;
  }

  inline
  void
  StreamBase::clear(std::ios_base::iostate state) throw (eh::Exception)
  {
    state_ = state;
    if (exceptions_ & state_)
    {
      throw std::ios_base::failure("");
    }
  }

  inline
  void
  StreamBase::setstate(std::ios_base::iostate state) throw (eh::Exception)
  {
    clear(rdstate() | state);
  }

  inline
  bool
  StreamBase::good() const throw ()
  {
    return !state_;
  }

  inline
  bool
  StreamBase::eof() const throw ()
  {
    return state_ & std::ios_base::eofbit;
  }

  inline
  bool
  StreamBase::fail() const throw ()
  {
    return state_ & (std::ios_base::failbit | std::ios_base::badbit);
  }

  inline
  bool
  StreamBase::bad() const throw ()
  {
    return state_ & std::ios_base::badbit;
  }

  inline
  std::ios_base::iostate
  StreamBase::exceptions() const throw ()
  {
    return exceptions_;
  }

  inline
  void
  StreamBase::exceptions(std::ios_base::iostate except) throw (eh::Exception)
  {
    exceptions_ = except;
    clear(rdstate());
  }


  //
  // BinaryInputStream class
  //

  inline
  BinaryInputStream::BinaryInputStream() throw ()
    : gcount_(0)
  {
  }

  inline
  BinaryInputStream::streamsize
  BinaryInputStream::gcount() const throw ()
  {
    return gcount_;
  }

  inline
  BinaryInputStream&
  BinaryInputStream::get(char_type& c) throw (eh::Exception)
  {
    return read(&c, 1);
  }


  //
  // BinaryOutputStream class
  //

  inline
  BinaryOutputStream&
  BinaryOutputStream::put(char_type c) throw (eh::Exception)
  {
    return write(&c, 1);
  }


  //
  // BinaryStreamReader class
  //

  inline
  BinaryStreamReader::BinaryStreamReader(BinaryInputStream* in)
    throw (eh::Exception)
    : std::istream(0), buf_(in)
  {
    init(&buf_);
  }


  //
  // BinaryStreamWriter class
  //

  inline
  BinaryStreamWriter::BinaryStreamWriter(BinaryOutputStream* out)
    throw (eh::Exception)
    : std::ostream(0), buf_(out)
  {
    init(&buf_);
  }
}

#endif
