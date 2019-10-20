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





#include <Stream/BinaryStream.hpp>


namespace Stream
{
  //
  // BinaryStreambuf
  //

  const size_t BinaryStreambuf::BUFFER_SIZE;
  const size_t BinaryStreambuf::PUTBACK_SIZE;

  BinaryStreambuf::BinaryStreambuf(BinaryInputStream* in)
    throw (eh::Exception)
    : in_(in), out_(0)
  {
  }

  BinaryStreambuf::BinaryStreambuf(BinaryOutputStream* out)
    throw (eh::Exception)
    : in_(0), out_(out)
  {
    setp(buffer_, buffer_ + (BUFFER_SIZE - 1));
  }

  std::streamsize
  BinaryStreambuf::showmanyc() throw (eh::Exception)
  {
    return egptr() - gptr();
  }

  BinaryStreambuf::int_type
  BinaryStreambuf::underflow() throw (eh::Exception)
  {
    if (gptr() < egptr())
    {
      return traits_type::to_int_type(*gptr());
    }

    size_t num_putback = gptr() - eback();
    if (num_putback > PUTBACK_SIZE)
    {
      num_putback = PUTBACK_SIZE;
    }

    std::copy(gptr() - num_putback, gptr(),
      buffer_ + (PUTBACK_SIZE - num_putback));

    if (!in_->read(buffer_ + PUTBACK_SIZE, BUFFER_SIZE - PUTBACK_SIZE))
    {
      return traits_type::eof();
    }

    size_t num = in_->gcount();

    setg(buffer_ + (PUTBACK_SIZE - num_putback), buffer_ + PUTBACK_SIZE,
      buffer_ + PUTBACK_SIZE + num);

    return traits_type::to_int_type(*gptr());
  }

  ssize_t
  BinaryStreambuf::flush_buffer() throw (eh::Exception)
  {
    int num = pptr() - pbase();
    if (!out_->write(buffer_, num))
    {
      return -1;
    }
    pbump(-num);
    return num;
  }

  int
  BinaryStreambuf::sync() throw (eh::Exception)
  {
    return flush_buffer();
  }

  BinaryStreambuf::int_type
  BinaryStreambuf::overflow(int_type c) throw (eh::Exception)
  {
    if (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *pptr() = traits_type::to_char_type(c);
      pbump(1);
    }

    if (flush_buffer() < 0)
    {
      // error
      return traits_type::eof();
    }

    return traits_type::to_int_type(c);
  }
} // namespace Stream
