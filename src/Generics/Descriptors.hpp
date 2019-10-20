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



#ifndef GENERICS_DESCRIPTORS_HPP
#define GENERICS_DESCRIPTORS_HPP

#include <unistd.h>
#include <fcntl.h>

#include <eh/Errno.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>


namespace Generics
{
  /**
   * Pipe operations
   */
  class Pipe
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(Errno, Exception);
    DECLARE_EXCEPTION(ConnectionClosed, Exception);

    /**
     * Creates pipe
     */
    Pipe() throw (eh::Exception, Exception);
    /**
     * Closes both ends of the pipe
     */
    ~Pipe() throw ();

    /**
     * Read descriptor
     * @return read descriptor
     */
    int
    read_descriptor() const throw ();
    /**
     * Write descriptor
     * @return write descriptor
     */
    int
    write_descriptor() const throw ();

    /**
     * Performs a single read operation from the pipe
     * @param buf buffer for read data
     * @param size maximum read size
     * @return see read(2)
     */
    ssize_t
    read(void* buf, size_t size) throw ();

    /**
     * Tries to read the exact amount of data.
     * Throws an exception if fails.
     * @param buf buffer for read data
     * @param size read size
     */
    void
    read_n(void* buf, size_t size) throw (Exception);

    /**
     * Performs a single write operation into the pipe
     * @param buf buffer with write data
     * @param size buffer size
     * @return see write(2)
     */
    ssize_t
    write(const void* buf, size_t size) throw ();

    /**
     * Tries to write the exact amount of data.
     * Throws an exception if fails.
     * @param buf buffer with write data
     * @param size write size
     */
    void
    write_n(const void* buf, size_t size) throw (Exception);

    /**
     * Writes a single character into the pipe ignoring EINTRs
     * @param ch character to write
     * @return see write(2)
     */
    ssize_t
    signal(char ch = '\0') throw ();

  protected:
    template <typename Functor>
    void
    act_n_(Functor func, int fd, void* buf, ssize_t size) throw (Exception);

  private:
    int pipe_[2];
  };

  /**
   * Pipe with non-blocking read end.
   */
  class NonBlockingReadPipe : protected Pipe
  {
  public:
    using Pipe::Exception;
    using Pipe::Errno;
    using Pipe::ConnectionClosed;

    /**
     * Unblocks the read end
     */
    NonBlockingReadPipe() throw (eh::Exception, Exception);

    using Pipe::read_descriptor;
    using Pipe::write_descriptor;
    using Pipe::read;
    using Pipe::write;
    using Pipe::write_n;
    using Pipe::signal;
  };

  /**
   * Descriptor to /dev/null
   */
  class DevNull : private Uncopyable
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    DevNull() throw (eh::Exception, Exception);
    ~DevNull() throw ();

    int
    fd() throw ();

  private:
    int fd_;
  };

  /**
   * Sets FD_CLOEXEC on the specified descriptor
   * @param fd descriptor to tune
   * @return 0 for success, negative for fcntl error
   */
  int
  set_cloexec(int fd) throw ();
}

//
// INLINES
//

namespace Generics
{
  //
  // Pipe class
  //

  inline
  Pipe::Pipe() throw (eh::Exception, Exception)
  {
    if (pipe(pipe_) < 0)
    {
      eh::throw_errno_exception<Exception>(FNE, "failed to create pipe");
    }
  }

  inline
  Pipe::~Pipe() throw ()
  {
    close(pipe_[1]);
    close(pipe_[0]);
  }

  inline
  int
  Pipe::read_descriptor() const throw ()
  {
    return pipe_[0];
  }

  inline
  int
  Pipe::write_descriptor() const throw ()
  {
    return pipe_[1];
  }

  inline
  ssize_t
  Pipe::read(void* buf, size_t size) throw ()
  {
    return ::read(read_descriptor(), buf, size);
  }

  template <typename Functor>
  void
  Pipe::act_n_(Functor func, int fd, void* buf, ssize_t size)
    throw (Exception)
  {
    do
    {
      ssize_t res = func(fd, buf, size);

      if (res < 0)
      {
        if (errno == EINTR)
        {
          continue;
        }

        eh::throw_errno_exception<Errno>(FNE, "operation failed");
      }

      if (!res)
      {
        Stream::Error ostr;
        ostr << FNS << "other end of the pipe is closed";
        throw ConnectionClosed(ostr);
      }


      size -= res;
      buf = static_cast<char*>(buf) + res;
    }
    while (size);
  }

  inline
  void
  Pipe::read_n(void* buf, size_t size) throw (Exception)
  {
    act_n_(::read, read_descriptor(), buf, size);
  }

  inline
  ssize_t
  Pipe::write(const void* buf, size_t size) throw ()
  {
    return ::write(write_descriptor(), buf, size);
  }

  inline
  void
  Pipe::write_n(const void* buf, size_t size) throw (Exception)
  {
    act_n_(::write, write_descriptor(), const_cast<void*>(buf), size);
  }

  inline
  ssize_t
  Pipe::signal(char ch) throw ()
  {
    ssize_t result;
    while ((result = write(&ch, 1)) < 0 && errno == EINTR)
    {
    }
    return result;
  }


  //
  // NonBlockingPipe class
  //

  inline
  NonBlockingReadPipe::NonBlockingReadPipe()
    throw (eh::Exception, Exception)
  {
    int flags = fcntl(read_descriptor(), F_GETFL);
    if (flags == -1 ||
      fcntl(read_descriptor(), F_SETFL, flags | O_NONBLOCK) == -1)
    {
      eh::throw_errno_exception<Exception>(FNE, "fcntl failure");
    }
  }


  //
  // DevNull class
  //

  inline
  DevNull::DevNull() throw (eh::Exception, Exception)
  {
    fd_ = open("/dev/null", O_RDWR);
    if (fd_ < 0)
    {
      eh::throw_errno_exception<Exception>(FNE, "Failed to open /dev/null");
    }
  }

  inline
  DevNull::~DevNull() throw ()
  {
    close(fd_);
  }

  inline
  int
  DevNull::fd() throw ()
  {
    return fd_;
  }


  //

  inline
  int
  set_cloexec(int fd) throw ()
  {
    int flags = fcntl(fd, F_GETFD);
    return flags < 0 ? flags : fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
  }
}

#endif
