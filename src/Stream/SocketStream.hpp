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





#ifndef STREAM_SOCKET_STREAM_HPP
#define STREAM_SOCKET_STREAM_HPP

#include <memory>
#include <streambuf>
#include <istream>

#include <ace/SOCK_Stream.h>

#include <Generics/Time.hpp>
#include <Generics/ArrayAutoPtr.hpp>


namespace Stream
{
  /**
   * Buffer with std::streambuf interface with able receive/send buffered
   * data from/to socket. Buffered send is not implemented yet.
   */
  class SocketStreambuf :
    public std::basic_streambuf<char, std::char_traits<char> >
  {
  public:
    /**
     * Constructor
     * @param sock_stream Socked prepared to do system calls recv or send
     * @param mode Binary mask with need modes: send, receive data.
     * @param send_timeout Do not use 
     * @param recv_timeout Time to do system call recv()
     */
    explicit
    SocketStreambuf(ACE_SOCK_Stream& sock_stream,
      std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out,
      const Generics::Time* send_timeout = 0,
      const Generics::Time* recv_timeout = 0);

    /**
     * Empty virtual destructor
     */
    virtual
    ~SocketStreambuf();

    /**
     * Continuously increasing the counter of received bytes
     * @return The number of received bytes 
     */
    size_t
    bytes_received() const throw ();

  protected:
    //
    // read functions
    //

    virtual
    std::streamsize
    showmanyc();

    virtual
    int_type
    underflow();

    // virtual
    // std::streamsize
    // xsgetn(char_type* s, std::streamsize n);

    //
    // write functions
    //

    // virtual
    // int
    // sync();

    // virtual
    // int_type
    // overflow(int_type c = traits_type::eof());

    // virtual
    // std::streamsize
    // xsputn(const char_type* s, std::streamsize n);

  protected:
    static const size_t IN_BUFFER_SIZE = 1024;
    static const size_t OUT_BUFFER_SIZE = 1024;
    static const size_t PUTBACK_SIZE = 20;

    ACE_SOCK_Stream& sock_stream_;
    std::unique_ptr<ACE_Time_Value> send_timeout_;
    std::unique_ptr<ACE_Time_Value> recv_timeout_;
    Generics::ArrayChar in_buffer_;
    Generics::ArrayChar out_buffer_;

    size_t bytes_sent_;
    size_t bytes_received_;
  };

  /**
   * Stream with std::istream interface able to read data from
   * socket and buffering it 
   */
  class SocketInStream :
    public std::basic_istream<char, std::char_traits<char> >
  {
  public:
    /**
     * Constructor
     * @param sock_stream Socket ready to do system call recv()
     * @param recv_timeout Time to do system call recv()
     */
    explicit
    SocketInStream(ACE_SOCK_Stream& sock_stream,
      const Generics::Time* recv_timeout = 0);

    /**
     * Continuously increasing the counter of received bytes
     * @return The number of receved bytes 
     */
    size_t
    bytes_received() const throw ();

  protected:
    SocketStreambuf buf_;
  };
}

//
// INLINES
//

namespace Stream
{
  //
  // SocketStreambuf class
  //

  size_t
  SocketStreambuf::bytes_received() const throw ()
  {
    return bytes_received_;
  }

  //
  // SocketInStream class
  //

  size_t
  SocketInStream::bytes_received() const throw ()
  {
    return buf_.bytes_received();
  }
}

#endif
