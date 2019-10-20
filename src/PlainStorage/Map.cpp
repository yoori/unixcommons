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



// @file PlainStorage/Map.cpp
#include <cstring>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>

#include "Map.hpp"


namespace PlainStorage
{
  //
  // PlainReader class
  //

  unsigned long
  PlainReader::read_i_(void* buf, unsigned long buf_size) const
    throw (eh::Exception, ReadFailed)
  {
    if (data_size_ > buf_size)
    {
      return 0;
    }

    try
    {
      WriteBlockFileAdapter::ReadBlockStruct_var
        read_cur = read_block_file_adapter_->get_block(FIRST_BLOCK_INDEX_);

      unsigned long buf_offset = 0;

      while (read_cur.in())
      {
        if (buf_offset + read_cur->size() > buf_size)
        {
          /* assert */
          Stream::Error ostr;
          ostr << "In reading exceed buffer. "
            << "Buffer size: " << buf_size
            << ", data size: " << data_size_
            << ", buffer offset: " << buf_offset
            << ", current block size: " << read_cur->size()
            << ".";
          throw BufferExhausted(ostr);
        }

        memcpy(static_cast<char*>(buf) + buf_offset,
               read_cur->read_content(), read_cur->size());

        buf_offset += read_cur->size();

        read_cur = read_cur->read_next();
      }
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't do reading. Caught eh::Exception: " << ex.what();
      throw ReadFailed(ostr);
    }

    return data_size_;
  }

  //
  // PlainWriter class
  //

  void
  PlainWriter::write_i_(const void* buf, unsigned long size)
    throw (eh::Exception, WriteFailed)
  {
    try
    {
      WriteBlockFileAdapter::WriteBlockStruct_var dealloc_cur;
      WriteBlockFileAdapter::WriteBlockStruct_var
        write_cur = write_block_file_adapter_->get_block(FIRST_BLOCK_INDEX_);

      if (size != 0)
      {
        unsigned int in_buf_offset = 0;

        while (in_buf_offset < size)
        {
          unsigned int size_to_write = write_cur->available_size();

          if (size_to_write > size - in_buf_offset)
          {
            size_to_write = size - in_buf_offset;
          }

          write_cur->size(size_to_write);

          memcpy(write_cur->content(),
            static_cast<const char*>(buf) + in_buf_offset, size_to_write);

          in_buf_offset += size_to_write;

          if (in_buf_offset < size)
          {
            if (write_cur->next_index() == 0)
            {
              // Write more data that can be stored in exists blocks
              // allocate new block and insert into end of chain of Data blocks
              BlockIndex new_block =
                block_allocator_->allocate();

              write_cur->next_index(new_block);

              write_cur = write_cur->next();

              write_cur->next_index(0);
            }
            else
            {
              /* use exist blocks */
              write_cur = write_cur->next();
            }
          }
        }
      }
      else /* size == 0 */
      {
        write_cur->size(0);
      }

      dealloc_cur = write_cur->next();
      write_cur->next_index(0);
      data_size_ = size;

      // deallocate excess
      while (dealloc_cur.in())
      {
        BlockIndex dealloc_index = dealloc_cur->index();
        dealloc_cur = dealloc_cur->next();
        block_allocator_->deallocate(dealloc_index);
      }
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't do writing. Caught eh::Exception: " << ex.what();
      throw WriteFailed(ostr);
    }
  }

  //
  // DefaultBlockAllocator class
  //

  DefaultBlockAllocator::DefaultBlockAllocator(
    WriteBlockFileAdapter* write_block_file_adapter,
    BlockIndex first_description_block)
    throw (eh::Exception)
    : write_block_file_adapter_(write_block_file_adapter),
      first_free_block_(0)
  {
    block_allocator_description_ =
      write_block_file_adapter_->get_block(first_description_block);

    first_free_block_ = static_cast<ConstAllocatorIndex*>(
      block_allocator_description_->content())->value();
  }

  DefaultBlockAllocator::~DefaultBlockAllocator()
    throw ()
  {
    sync_();
  }

  void
  DefaultBlockAllocator::sync_()
    throw ()
  {
    static_cast<AllocatorIndex*>(
      block_allocator_description_->content())->value() =
      first_free_block_;
  }

  BlockIndex
  DefaultBlockAllocator::allocate()
    throw (eh::Exception, AllocationFailed)
  {
    const std::size_t ALLOCATE_PORTION = 10;
    try
    {
      WriteGuard_ lock(lock_);

      if (first_free_block_ == 0)
      {
        // resizing of file
        BlockIndex max_block_index =
          write_block_file_adapter_->max_block_index();

        WriteBlockFileAdapter::WriteBlockStruct_var
          last_block = write_block_file_adapter_->get_block(
            max_block_index + ALLOCATE_PORTION - 1);

        last_block->size(0);
        last_block->next_index(0);

        WriteBlockFileAdapter::WriteBlockStruct_var
          cur_block = write_block_file_adapter_->get_block(
            max_block_index);

        first_free_block_ = cur_block->index();

        for (BlockIndex i = max_block_index;
             i < max_block_index + ALLOCATE_PORTION;
             ++i)
        {
          cur_block->size(0);
          cur_block->next_index(i + 1);
          cur_block = cur_block->next();
        }
      }

      BlockIndex first_free_index = first_free_block_;

      WriteBlockFileAdapter::WriteBlockStruct_var allocated_block =
        write_block_file_adapter_->get_block(first_free_index);

      BlockIndex new_first_free_index = allocated_block->next_index();

      allocated_block->next_index(0);

      first_free_block_ = new_first_free_index;

      sync_();

      return allocated_block->index();
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't allocate block. Caught eh::Exception: " <<
        ex.what();
      throw AllocationFailed(ostr);
    }
  }

  void
  DefaultBlockAllocator::deallocate(BlockIndex block_to_free)
    throw (eh::Exception, DeallocationFailed)
  {
    try
    {
      WriteGuard_ lock(lock_);

      BlockIndex first_free_block = first_free_block_;

      WriteBlockFileAdapter::WriteBlockStruct_var deallocated_block =
        write_block_file_adapter_->get_block(block_to_free);

      deallocated_block->next_index(first_free_block);

      first_free_block_ = block_to_free;

      sync_();
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't deallocate block. Caught eh::Exception: " <<
        ex.what();
      throw DeallocationFailed(ostr);
    }
  }
}
