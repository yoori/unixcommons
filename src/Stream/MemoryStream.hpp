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



// @file Stream/MemoryStream.hpp
#ifndef STREAM_MEMORYSTREAM_HPP
#define STREAM_MEMORYSTREAM_HPP

#include <streambuf>
#include <istream>
#include <ostream>

#include <sys/param.h>

#include <String/SubString.hpp>


namespace Stream
{
  /**
   * These classes are designed to be "less costly" versions of ostringstream
   * and istringstream in terms of allocations and copying.
   * Based on Andrey Makarov's work.
   */
  namespace MemoryStream
  {
    /**
     * Input memory buffer
     * Using supplied memory region as a stream content
     * No allocations are performed
     */
    template <typename Elem, typename Traits>
    class InputMemoryBuffer : public std::basic_streambuf<Elem, Traits>
    {
    public:
      typedef typename Traits::int_type Int;
      typedef typename Traits::pos_type Position;
      typedef typename Traits::off_type Offset;

      typedef Elem* Pointer;
      typedef const Elem* ConstPointer;
      typedef size_t Size;

      /**
       * Constructor
       * @param ptr address of memory region
       * @param size size of memory region
       */
      InputMemoryBuffer(Pointer ptr, Size size)
        throw (eh::Exception);

      /**
       * @return The pointer to data not read yet
       */
      ConstPointer
      data() const throw ();

      /**
       * @return The size of data not read yet
       */
      Size
      size() const throw ();

    protected:
      virtual
      Position
      seekoff(Offset off, std::ios_base::seekdir way,
        std::ios_base::openmode which) throw (eh::Exception);

      virtual
      Position
      seekpos(Position pos, std::ios_base::openmode which)
        throw (eh::Exception);

      virtual
      Int
      underflow() throw ();
    };

    /**
     * Output memory buffer
     * Can preallocate memory region of desired size
     */
    template <typename Elem, typename Traits, typename Allocator,
      typename AllocatorInitializer = Allocator>
    class OutputMemoryBuffer : public std::basic_streambuf<Elem, Traits>
    {
    public:
      typedef typename Traits::int_type Int;
      typedef typename Traits::pos_type Position;
      typedef typename Traits::off_type Offset;

      typedef typename Allocator::pointer Pointer;
      typedef typename Allocator::const_pointer ConstPointer;
      typedef typename Allocator::size_type Size;

      /**
       * Constructor
       * @param initial_size preallocated region size
       * @param args allocator initializer
       */
      OutputMemoryBuffer(Size initial_size = 0,
        const AllocatorInitializer& allocator_initializer =
          AllocatorInitializer()) throw (eh::Exception);

      /**
       * Destructor
       * Frees allocated memory region
       */
      virtual
      ~OutputMemoryBuffer() throw ();

      /**
       * @return The pointer to filled data
       */
      ConstPointer
      data() const throw ();

      /**
       * @return The size of filled data
       */
      Size
      size() const throw ();

    protected:
      virtual
      Position
      seekoff(Offset off, std::ios_base::seekdir way,
        std::ios_base::openmode which) throw (eh::Exception);

      virtual
      Position
      seekpos(Position pos, std::ios_base::openmode which)
        throw (eh::Exception);

      virtual
      Int
      overflow(Int c = Traits::eof()) throw (eh::Exception);

    private:
      /**
       * Extends allocated memory region
       * @return whether or not extension was successful
       */
      bool
      extend() throw (eh::Exception);

      Allocator allocator_;
      Offset max_offset_;
    };

    /**
     * Initializer of MemoryBuffer
     * Required because of order of construction
     */
    template <typename Buffer>
    class MemoryBufferHolder
    {
    public:
      typedef typename Buffer::char_type Elem;

      typedef typename String::BasicSubString<const Elem,
        String::CharTraits<typename std::remove_const<Elem>::type>,
          String::CheckerNone<Elem> > SubString;

      /**
       * Constructor
       * Constructs memory buffer without parameters
       */
      MemoryBufferHolder() throw (eh::Exception);

      /**
       * Constructor
       * Constructs memory buffer with one parameter
       * @param var parameter for buffer's constructor
       */
      template <typename T>
      MemoryBufferHolder(T var) throw (eh::Exception);

      /**
       * Constructor
       * Constructs memory buffer with two parameters
       * @param var1 the first parameter for buffer's constructor
       * @param var2 the second parameter for buffer's constructor
       */
      template <typename T1, typename T2>
      MemoryBufferHolder(T1 var1, T2 var2) throw (eh::Exception);

      /**
       * Return SubString based on this buffer. Buffer can be without
       * zero at the end.
       * @return SubString spreads the buffer
       */
      SubString
      str() const throw (eh::Exception);

      /**
       * Templatized version of string which allow get SubString
       * with any suitable Traits and Checker types
       * @return SubString spreads the buffer
       */
      template <typename Traits, typename Checker>
      String::BasicSubString<const Elem, Traits, Checker>
      str() const throw (eh::Exception);

    protected:
      /**
       * @return pointer to holding buffer
       */
      Buffer*
      buffer() throw ();

      /**
       * @return pointer to holding buffer
       */
      const Buffer*
      buffer() const throw ();

    private:
      Buffer buffer_;
    };

    /**
     * Input memory stream. Uses InputMemoryBuffer for data access.
     */
    template <typename Elem, typename Traits = std::char_traits<Elem> >
    class InputMemoryStream :
      public MemoryBufferHolder<InputMemoryBuffer<Elem, Traits> >,
      public std::basic_istream<Elem, Traits>
    {
    private:
      typedef MemoryBufferHolder<InputMemoryBuffer<Elem, Traits> > Holder;
      typedef std::basic_istream<Elem, Traits> Stream;

    public:
      typedef Elem* Pointer;
      typedef const Elem* ConstPointer;
      typedef size_t Size;

      /**
       * Constructor
       * Passes data and Traits::length(data) to InputMemoryBlock's
       * constructor
       * @param data address of memory region
       */
      InputMemoryStream(ConstPointer data)
        throw (eh::Exception);

      /**
       * Constructor
       * Passes parameters to InputMemoryBlock's constructor
       * @param data address of memory region
       * @param size size of memory region
       */
      InputMemoryStream(ConstPointer data, Size size)
        throw (eh::Exception);

      /**
       * Constructor
       * Passes str.data() and str.size() to InputMemoryBlock's constructor
       * @param str memory region, should not be temporal
       */
      template <typename Char, typename STraits, typename Checker>
      InputMemoryStream(
        const String::BasicSubString<Char, STraits, Checker>& str)
        throw (eh::Exception);

      /**
       * Constructor
       * Passes str.data() and str.size() to InputMemoryBlock's constructor
       * @param str memory region, should not be temporal
       */
      template <typename Char, typename STraits, typename Alloc>
      InputMemoryStream(const std::basic_string<Char, STraits, Alloc>& str)
        throw (eh::Exception);
    };

    /**
     * Output memory stream. Uses OutputMemoryBuffer for data access.
     */
    template <typename Elem, typename Traits = std::char_traits<Elem>,
      typename Allocator = std::allocator<Elem>,
      typename AllocatorInitializer = Allocator, const size_t SIZE = 0>
    class OutputMemoryStream :
      public MemoryBufferHolder<
        OutputMemoryBuffer<Elem, Traits, Allocator, AllocatorInitializer> >,
      public std::basic_ostream<Elem, Traits>
    {
    private:
      typedef MemoryBufferHolder<
        OutputMemoryBuffer<Elem, Traits, Allocator, AllocatorInitializer> >
          Holder;
      typedef std::basic_ostream<Elem, Traits> Stream;

    public:
      /**
       * Constructor
       * Passes parameters to OutputMemoryBlock's constructor
       * @param initial_size preallocated region size
       * @param allocator_initializer allocator initializer
       */
      explicit
      OutputMemoryStream(typename Allocator::size_type initial_size = SIZE,
        const AllocatorInitializer& allocator_initializer =
          AllocatorInitializer()) throw (eh::Exception);
    };

    namespace Allocator
    {
      template <typename Elem, const size_t SIZE, typename Buffer,
        typename BufferInitializer = Buffer>
      class Simple : public std::allocator<Elem>
      {
      public:
        typedef std::allocator<Elem> Allocator;

        /**
         * Constructor without parameters
         */
        Simple() throw ();

        /**
         * Constructor with buffer_ init value
         * @param buffer_initializer initializer for buffer_
         */
        Simple(BufferInitializer buffer_initializer) throw ();

        /**
         * Allocation function
         * Allows to allocate SIZE bytes one time in a row
         * @param size should be equal to SIZE
         * @return pointer to size_ternal buffer
         */
        typename Allocator::pointer
        allocate(typename Allocator::size_type size, const void* = 0)
          throw ();

        /**
         * Deallocation function
         * Deallocates previously allocated memory
         * @param ptr should be equal to the pointer returned by allocate()
         * @param size should be equal to SIZE
         */
        void
        deallocate(typename Allocator::pointer ptr,
          typename Allocator::size_type size) throw ();

      private:
        Buffer buffer_;
        bool allocated_;
      };

      /**
       * Simple buffer allocator
       * Allows a single allocation on preallocated buffer
       */
      template <typename Elem, const size_t SIZE>
      class SimpleBuffer : public Simple<Elem, SIZE, Elem*>
      {
      public:
        /**
         * Constructor
         * @param buffer preallocated buffer of size not less than SIZE
         */
        explicit
        SimpleBuffer(Elem* buffer) throw ();
      };

      template <typename Elem, const size_t SIZE, typename Initializer>
      class ArrayBuffer
      {
      public:
        explicit
        ArrayBuffer(Initializer initializer = Initializer()) throw ();

        operator Elem*() throw ();

      private:
        Elem buffer_[SIZE];
      };

      /**
       * Simple stack allocator
       * Required for disuse of heap for OutputStream
       */
      template <typename Elem, const size_t SIZE>
      class SimpleStack :
        public Simple<Elem, SIZE, ArrayBuffer<Elem, SIZE, size_t>, size_t>
      {
      public:
        /**
         * Constructor
         */
        explicit
        SimpleStack(size_t allocator_initializer) throw ();
      };
    }
  }


  // Input memory streams working on external buffer
  typedef MemoryStream::InputMemoryStream<char> Parser;
  typedef MemoryStream::InputMemoryStream<wchar_t> WParser;

  // Dynamic memory output stream with preallocation
  typedef MemoryStream::OutputMemoryStream<char> Dynamic;


  /**
   * Output memory stream working on external memory buffer of size
   * not less than SIZE. No more than SIZE-1 chars are written size_to
   * the buffer and buffer is always zero terminated after the
   * destruction of the stream.
   *
   * Example:
   * char buf[10];
   * {
   *   Buffer<5> ostr(buf);
   *   ostr << something;
   *   // buf IS NOT required to be nul-terminated here
   * }
   * // buf IS nul-terminated here. strlen(buf) <= 4.
   */
  template <const size_t SIZE>
  class Buffer :
    public MemoryStream::OutputMemoryStream<char, std::char_traits<char>,
      MemoryStream::Allocator::SimpleBuffer<char, SIZE>,
      MemoryStream::Allocator::SimpleBuffer<char, SIZE>, SIZE - 1>
  {
  private:
    typedef MemoryStream::Allocator::SimpleBuffer<char, SIZE> Allocator;

  public:
    /**
     * Constructor
     * @param buffer buffer to make output to of size not less than SIZE
     */
    explicit
    Buffer(char* buffer) throw ();

    /**
     * Destructor
     * Appends nul-terminating character to the buffer
     */
    ~Buffer() throw ();
  };

  /**
   * Output memory stream holding memory buffer of size SIZE+1.
   */
  template <const size_t SIZE>
  class Stack :
    public MemoryStream::OutputMemoryStream<char, std::char_traits<char>,
      MemoryStream::Allocator::SimpleStack<char, SIZE + 1>, size_t, SIZE>
  {
  };

  /**
   * Default class for throwing DescriptiveException successors
   */
  class Error : public Stack<sizeof(eh::DescriptiveException)>
  {
  };

  /**
   * Helper class for forming of file names in preallocated buffers of
   * enough size
   */
  typedef Buffer<MAXPATHLEN> FileName;
}

#include <Stream/MemoryStream.tpp>

#endif
