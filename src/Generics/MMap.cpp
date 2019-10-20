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



// @file Generics/MMap.cpp
#include <unistd.h>

#include <limits>

#include <eh/Errno.hpp>

#include <Generics/MMap.hpp>
#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>


namespace Generics
{
  //
  // MMap class
  //

  void
  MMap::map_(int fd, void* preferrable_address, size_t size, off_t offset,
    int mmap_prot, int mmap_flags) throw (eh::Exception, Exception)
  {
    off_t file_size = size;
    if (fd >= 0)
    {
      file_size = lseek(fd, 0, SEEK_END);
      if (file_size == static_cast<off_t>(-1))
      {
        eh::throw_errno_exception<Exception>(FNE,
          "Failed to determine size of file");
      }

      if (offset + static_cast<off_t>(size) > file_size)
      {
        Stream::Error ostr;
        ostr << FNS << "Map window of offset " << offset << " and size " <<
          size << " exceeds file's size of " << file_size;
        throw Exception(ostr);
      }

      file_size -= offset;
    }

    length_ = size;
    if (!length_)
    {
      if (std::numeric_limits<ssize_t>::max() < file_size)
      {
        Stream::Error ostr;
        ostr << FNS << "requested map length " << file_size <<
          " is too large";
        throw Exception(ostr);
      }
      length_ = static_cast<ssize_t>(file_size);
    }

    memory_ = mmap(preferrable_address, length_, mmap_prot, mmap_flags, fd,
      offset);
    if (memory_ == MAP_FAILED)
    {
      memory_ = 0;
      eh::throw_errno_exception<Exception>(FNE, "mmap failed");
    }
  }

  MMap::MMap() throw ()
    : memory_(0), length_(0)
  {
  }

  MMap::MMap(int fd, size_t size, off_t offset, int mmap_prot,
    int mmap_flags) throw (eh::Exception, Exception)
  {
    map_(fd, 0, size, offset, mmap_prot, mmap_flags);
  }

  MMap::MMap(void* preferrable_address, std::size_t size)
    throw (eh::Exception, Exception)
  {
    map_(-1, preferrable_address, size, 0, PROT_READ | PROT_WRITE,
      MAP_SHARED | MAP_ANONYMOUS);
  }

  MMap::~MMap() throw ()
  {
    munmap(memory_, length_);
  }

  void*
  MMap::memory() const throw ()
  {
    return memory_;
  }

  size_t
  MMap::length() const throw ()
  {
    return length_;
  }


  //
  // MMap class
  //

  MMapFile::MMapFile(const char* filename, size_t size, off_t offset,
    int flags, int mmap_prot, int mmap_flags)
    throw (eh::Exception, Exception)
  {
    if (offset < 0)
    {
      Stream::Error ostr;
      ostr << FNS << "offset is negative";
      throw Exception(ostr);
    }

    fd_ = open(filename, flags, 0666);
    if (fd_ < 0)
    {
      eh::throw_errno_exception<Exception>(FNE, "Failed to open file '",
        filename, "'");
    }

    try
    {
      map_(fd_, 0, size, offset, mmap_prot, mmap_flags);
    }
    catch (...)
    {
      close(fd_);
      throw;
    }
  }

  MMapFile::MMapFile(int fd, size_t size, off_t offset,
    int mmap_prot, int mmap_flags) throw (eh::Exception, Exception)
    : fd_(fd)
  {
    if (fd_ < 0)
    {
      Stream::Error ostr;
      ostr << FNS << "invalid file descriptor";
      throw Exception(ostr);
    }

    try
    {
      if (offset < 0)
      {
        Stream::Error ostr;
        ostr << FNS << "offset is negative";
        throw Exception(ostr);
      }

      map_(fd_, 0, size, offset, mmap_prot, mmap_flags);
    }
    catch (...)
    {
      close(fd_);
      throw;
    }
  }

  MMapFile::~MMapFile() throw ()
  {
    close(fd_);
  }

  int
  MMapFile::file_descriptor() const throw ()
  {
    return fd_;
  }
}
