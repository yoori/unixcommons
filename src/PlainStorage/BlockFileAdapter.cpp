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



// @file PlainStorage/BlockFileAdapter.cpp

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>

#include <fstream>
#include <iostream>
#include <limits>

#include <eh/Errno.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>

#include "BlockFileAdapter.hpp"


namespace PlainStorage
{
  namespace
  {
    const std::size_t SYSTEM_PAGE_SIZE =
      ::getpagesize();

    struct BlockFileAdapterContext
    {
      BlockFileAdapterContext(int & file_desc_val,
        ReadBlockFileAdapter::FileOffset& file_size_val,
        std::size_t& map_page_size_val,
        std::size_t block_size_val) throw ();

      int& file_desc;
      ReadBlockFileAdapter::FileOffset& file_size;
      std::size_t& map_page_size;
      std::size_t block_size;
    };

    BlockFileAdapterContext::BlockFileAdapterContext(
      int & file_desc_val,
      ReadBlockFileAdapter::FileOffset& file_size_val,
      std::size_t& map_page_size_val,
      std::size_t block_size_val)
      throw ()
      : file_desc(file_desc_val),
        file_size(file_size_val),
        map_page_size(map_page_size_val),
        block_size(block_size_val)
    {
    }

    /**
     * @return opened file descriptor
     */
    void
    open_file(const char* filename,
      int flags, mode_t mode, BlockFileAdapterContext& context)
      throw (ReadBlockFileAdapter::PosixException, eh::Exception)
    {
      context.file_desc = ::open(filename, flags, mode);
      if (context.file_desc == -1)
      {
        eh::throw_errno_exception<ReadBlockFileAdapter::PosixException>(
          FNE, "Can't open file '", filename, "'");
      }
  
      struct stat f_stat;
  
      if (::fstat(context.file_desc, &f_stat))
      {
        eh::throw_errno_exception<ReadBlockFileAdapter::PosixException>(
          FNE, "Can't do fstat at '", filename, "'");
      }
  
      context.file_size = f_stat.st_size;
  
      // calculate size of file part that portions will be read
      // block_size_ cannot be zero, rounding up block_size_ to a multiply
      // of SYSTEM_PAGE_SIZE_
      context.map_page_size = context.block_size - 1 +
        SYSTEM_PAGE_SIZE -
        (context.block_size - 1) % SYSTEM_PAGE_SIZE;
    }

    void*
    resolve_block(BlockIndex block_index,
      int file_desc,
      int prot,
      std::size_t map_page_size)
      throw (ReadBlockFileAdapter::PosixException, eh::Exception)
    {
      void* mem_ptr =
        ::mmap(0,
                 map_page_size,
                 prot,
                 MAP_SHARED,
                 file_desc,
                 static_cast<off64_t>(block_index) * map_page_size);
  
      if (mem_ptr == 0 || mem_ptr == MAP_FAILED)
      {
        int error = errno;
        char pos[64];
        snprintf(pos, sizeof(pos), "%llu",
          static_cast<unsigned long long>(block_index) * map_page_size);
        char size[32];
        snprintf(size, sizeof(size), "%lu",
          static_cast<unsigned long>(map_page_size));
        eh::throw_errno_exception<ReadBlockFileAdapter::PosixException>(
          error, FNE, "Can't map to memory file block: "
          "pos = ", pos, ", size = ", size);
      }
  
      return mem_ptr;
    }

    void
    unresolve_block(void* mem_ptr, std::size_t map_page_size)
      throw (ReadBlockFileAdapter::PosixException, eh::Exception)
    {
      if (::munmap(mem_ptr, map_page_size) == -1)
      {
        eh::throw_errno_exception<ReadBlockFileAdapter::PosixException>(
          FNE, "Can't unmap file block.");
      }
    }
        
  } // namespace

  //
  // ReadBlockFileAdapter::ReadBlockStruct
  //

  ReadBlockFileAdapter::ReadBlockStruct::ReadBlockStruct(
    ReadBlockFileAdapter* block_file_adapter,
    BlockIndex block_index,
    bool resolve_content)
    throw (eh::Exception)
    : read_block_file_adapter_(block_file_adapter),
      block_index_(block_index),
      content_(0)
  {
    if (resolve_content)
    {
      content_ = static_cast<BlockHeader*>(
        read_block_file_adapter_->read_resolve_block_(block_index_));
    }
  }

  ReadBlockFileAdapter::ReadBlockStruct::~ReadBlockStruct()
    throw ()
  {
    if (content_)
    {
      try
      {
        read_block_file_adapter_->read_unresolve_block_(content_);
      }
      catch (const eh::Exception& ex)
      {
        std::cerr << FNS << "Caught eh::Exception: " << ex.what()
          << std::endl;
      }
    }
  }

  ReadBlockFileAdapter::ReadBlockStruct*
  ReadBlockFileAdapter::ReadBlockStruct::read_next()
    throw (eh::Exception)
  {
    BlockIndex ind = next_index();
    if (!ind)
    {
      return 0;
    }

    return new ReadBlockStruct(read_block_file_adapter_, ind);
  }

  //
  // WriteBlockFileAdapter::WriteBlockStruct
  //

  // ReadBlockStruct does not allocate shared memory because
  // third parameter=false 
  WriteBlockFileAdapter::WriteBlockStruct::WriteBlockStruct(
    WriteBlockFileAdapter* block_file_adapter,
    BlockIndex block_index)
    throw (eh::Exception)
    : ReadBlockStruct(block_file_adapter, block_index, false),
      write_block_file_adapter_(block_file_adapter)
  {
    bool need_to_init;

    content_ = static_cast<BlockHeader*>(
      write_block_file_adapter_->write_resolve_block_(
        block_index_, need_to_init));

    if (need_to_init)
    {
      /* initialize new block responsible attributes */
      next_index(0);
      size(0);
    }
  }

  WriteBlockFileAdapter::WriteBlockStruct::~WriteBlockStruct()
    throw ()
  {
    if (content_)
    {
      try
      {
        write_block_file_adapter_->write_unresolve_block_(content_);
        content_ = 0;
      }
      catch (const eh::Exception& ex)
      {
        std::cerr << FNS << "Caught eh::Exception: " << ex.what()
          << std::endl;
      }
    }
  }

  WriteBlockFileAdapter::WriteBlockStruct*
  WriteBlockFileAdapter::WriteBlockStruct::next()
    throw (eh::Exception)
  {
    BlockIndex ind = ReadBlockStruct::next_index();

    if (ind == 0)
    {
      return 0;
    }

    return new WriteBlockStruct(write_block_file_adapter_, ind);
  }

  //
  // ReadBlockFileAdapter
  //

  const std::size_t ReadBlockFileAdapter::SYSTEM_PAGE_SIZE_ =
    SYSTEM_PAGE_SIZE;

  ReadBlockFileAdapter::~ReadBlockFileAdapter()
    throw ()
  {
    if (file_desc_ != -1)
    {
      ::close(file_desc_);
    }
  }

  void
  ReadBlockFileAdapter::open_file_(const char* filename)
    throw (PosixException, eh::Exception)
  {
    BlockFileAdapterContext context(
      file_desc_, file_size_, map_page_size_, block_size_);
    open_file(filename, O_RDONLY, 0, context);
  }

  void*
  ReadBlockFileAdapter::read_resolve_block_(BlockIndex block_index)
    throw (PosixException, eh::Exception)
  {
    return resolve_block(block_index, file_desc_, PROT_READ, map_page_size_);
  }

  void
  WriteBlockFileAdapter::open_file_(const char* filename, OpenType open_type)
    throw (BadParam, PosixException, eh::Exception)
  {
    BlockFileAdapterContext context(
      file_desc_, file_size_, map_page_size_, block_size_);
    
    if (open_type == OT_OPEN)
    {
      open_file(filename, O_RDWR, 0, context);
    }
    else if (open_type == OT_OPEN_OR_CREATE)
    {
      open_file(filename, O_RDWR | O_CREAT, S_IWRITE | S_IREAD, context);
    }
    else
    {
      Stream::Error ostr;
      ostr << FNS << "Not defined open mode";
      throw ReadBlockFileAdapter::BadParam(ostr);
    }
  }

  void
  ReadBlockFileAdapter::read_unresolve_block_(void* mem_ptr)
    throw (PosixException, eh::Exception)
  {
    unresolve_block(mem_ptr, map_page_size_);
  }

  void*
  WriteBlockFileAdapter::write_resolve_block_(
    BlockIndex block_index,
    bool& need_to_init)
    throw (PosixException, eh::Exception)
  {
    need_to_init = false;

    if (block_index >= size_file_())
    {
      resize_file_(block_index + 1);
      need_to_init = true;
    }

    return resolve_block(block_index, file_desc_, PROT_READ | PROT_WRITE,
      map_page_size_);
  }

  void
  WriteBlockFileAdapter::write_unresolve_block_(void* mem_ptr)
    throw (PosixException, eh::Exception)
  {
    if (::munmap(mem_ptr, map_page_size_) == -1)
    {
      eh::throw_errno_exception<PosixException>(
        "WriteBlockFileAdapter::write_unresolve_block_(): "
        "Can't unmap file block.");
    }
  }

  void
  WriteBlockFileAdapter::resize_file_(
    BlockIndex new_size_in_blocks)
    throw (FileOpenFailure, PosixException, eh::Exception)
  {
    if (file_desc_ == -1)
    {
      Stream::Error ostr;
      ostr << FNS << "File not opened.";
      throw FileOpenFailure(ostr);
    }

    off64_t max_off = std::numeric_limits<off64_t>::max();

    if (max_off && static_cast<off64_t>(new_size_in_blocks) >
      max_off / map_page_size_)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't resize file. "
        "Requested size greater then maximum possible offset: "
        "requested size in blocks is "
        << new_size_in_blocks << " > " << max_off / map_page_size_ << ".";
      throw ResizeFailure(ostr);
    }

    off64_t new_size =
      static_cast<off64_t>(new_size_in_blocks) * map_page_size_;

    if (::ftruncate(file_desc_, new_size) != 0)
    {
      eh::throw_errno_exception<PosixException>(FNE,
        "Can't resize file. Requested size is ", new_size);
    }

    file_size_ = new_size;
  }
} // namespace PlainStorage
