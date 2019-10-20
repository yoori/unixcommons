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



// @file Stream/MMapStream.hpp
#ifndef STREAM_MMAPSTREAM_HPP
#define STREAM_MMAPSTREAM_HPP

#include <Stream/MemoryStream.hpp>
#include <Generics/MMap.hpp>


namespace Stream
{
  namespace MemoryStream
  {
    /**
     * Input stream based on memory mapped file
     */
    template <typename Elem, typename Traits = std::char_traits<Elem> >
    class MMapStream :
      private Generics::MMapFile,
      public InputMemoryStream<Elem, Traits>
    {
    public:
      using Generics::MMapFile::Exception;

      /**
       * Constructor
       * @param filename file to open
       * @param size size to map (zero - from offset till the end)
       * @param offset starting offset in file
       */
      explicit
      MMapStream(const char* filename, size_t size = 0, off_t offset = 0)
        throw (eh::Exception, Exception);
    };
  }

  typedef MemoryStream::MMapStream<char> FileParser;
  typedef MemoryStream::MMapStream<wchar_t> WFileParser;
}

//
// Implementation
//

namespace Stream
{
  namespace MemoryStream
  {
    //
    // MMapParser class
    //

    template <typename Elem, typename Traits>
    MMapStream<Elem, Traits>::MMapStream(const char* filename,
      size_t size, off_t offset) throw (eh::Exception, Exception)
      : Generics::MMapFile(filename, offset, size),
        InputMemoryStream<Elem, Traits>(
          static_cast<const Elem*>(this->memory()),
          this->length() / sizeof(Elem))
    {
    }
  }
}

#endif
