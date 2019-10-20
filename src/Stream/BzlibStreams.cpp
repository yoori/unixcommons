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





#include <bzlib.h>

#include <eh/Errno.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>
#include <Stream/BzlibStreams.hpp>


namespace
{
  template <typename InvalidArgument, typename IOError, const bool READ>
  class FileHandleAdapter : public Stream::File::IO
  {
  public:
    FileHandleAdapter(const char* file_name)
      throw (InvalidArgument, eh::Exception);

    ~FileHandleAdapter() throw ();

    size_t
    read(void* buf, size_t size) throw (eh::Exception);

    void
    write(const void* buf, size_t size) throw (eh::Exception);

  private:
    FILE* handle_;
    BZFILE* bzlib2_handle_;
    bool stream_end_;
  };

  template <typename InvalidArgument, typename IOError, const bool READ>
  FileHandleAdapter<InvalidArgument, IOError, READ>::FileHandleAdapter(
    const char* file_name) throw (InvalidArgument, eh::Exception)
    : stream_end_(false)
  {
    if (!file_name)
    {
      Stream::Error ostr;
      ostr << FNS << "file_name is NULL";
      throw InvalidArgument(ostr);
    }

    const char* mode = READ ? "rb" : "wb";
    handle_ = fopen(file_name, mode);
    if (!handle_)
    {
      eh::throw_errno_exception<InvalidArgument>(FNE,
        "Cannot open file: \"", file_name, "\" with mode=\"",
        mode, "\"");
    }

    int bz_error = BZ_OK;
    bzlib2_handle_ = READ ?
      ::BZ2_bzReadOpen(&bz_error, handle_, 0, 0, 0, 0) :
      ::BZ2_bzWriteOpen(&bz_error, handle_, 1, 0, 0);
    if (bzlib2_handle_ == 0 || bz_error != BZ_OK)
    {
      fclose(handle_);
      Stream::Error ostr;
      ostr << FNS << "BZ2_bzOpen failed. Error code: " << bz_error <<
        ". Returned handle=0x" << std::hex << bzlib2_handle_;
      throw InvalidArgument(ostr);
    }
  }

  template <typename InvalidArgument, typename IOError, const bool READ>
  FileHandleAdapter<InvalidArgument, IOError, READ>::~FileHandleAdapter()
    throw ()
  {
    int bz_error = BZ_OK;
    if (READ)
    {
      ::BZ2_bzReadClose(&bz_error, bzlib2_handle_);
    }
    else
    {
      ::BZ2_bzWriteClose(&bz_error, bzlib2_handle_, 0, 0, 0);
    }

    fclose(handle_);
  }

  template <typename InvalidArgument, typename IOError, const bool READ>
  size_t
  FileHandleAdapter<InvalidArgument, IOError, READ>::read(void* buf,
    size_t size) throw (eh::Exception)
  {
    if (stream_end_)
    {
      return 0;
    }

    int bz_error = BZ_OK;
    int bytes_read = ::BZ2_bzRead(&bz_error, bzlib2_handle_, buf, size);
    if (bz_error != BZ_OK && bz_error != BZ_STREAM_END)
    {
      Stream::Error ostr;
      ostr << FNS << "::BZ2_bzRead has returned error. Error code = " <<
        bz_error;
      throw IOError(ostr);
    }
    if (bz_error == BZ_STREAM_END)
    {
      stream_end_ = true;
    }
    return bytes_read;
  }

  template <typename InvalidArgument, typename IOError, const bool READ>
  void
  FileHandleAdapter<InvalidArgument, IOError, READ>::write(const void* buf,
    size_t size) throw (eh::Exception)
  {
    int bz_error = BZ_OK;
    ::BZ2_bzWrite(&bz_error, bzlib2_handle_, const_cast<void*>(buf), size);
    if (bz_error != BZ_OK)
    {
      Stream::Error ostr;
      ostr << FNS << "::BZ2_bzWrite has returned error. Error code = " <<
        bz_error;
      throw IOError(ostr);
    }
  }
}

namespace Stream
{
  BzlibInStream::BzlibInStream(const char* bzip_file_name,
    size_t buffer_size, size_t put_back_size)
    throw (eh::Exception)
    : std::basic_istream<char, std::char_traits<char> >(0),
      buf_(new FileHandleAdapter<
        File::InStreamBuf::InvalidArgument,
        File::InStreamBuf::Underflow, true>(bzip_file_name),
        buffer_size, put_back_size)
  {
    init(&buf_);
  }

  BzlibOutStream::BzlibOutStream(const char* bzip_file_name,
    size_t buffer_size)
    throw (eh::Exception)
    : std::basic_ostream<char, std::char_traits<char> >(0),
      buf_(new FileHandleAdapter<
        File::OutStreamBuf::InvalidArgument,
        File::OutStreamBuf::Overflow, false>(bzip_file_name), buffer_size)
  {
    init(&buf_);
  }
}
