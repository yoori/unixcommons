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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include <PlainStorage/BlockFileAdapter.hpp>

int
main()
{
  try
  {
    {
      PlainStorage::WriteBlockFileAdapter
        write_block_file_adapter(
          "test.out",
          64*1024,
          PlainStorage::WriteBlockFileAdapter::OT_OPEN_OR_CREATE);

      PlainStorage::WriteBlockFileAdapter::WriteBlockStruct_var
        first_block = write_block_file_adapter.get_block(0);

      first_block->size(100);
      memset(first_block->content(), 'A', 100);

      PlainStorage::WriteBlockFileAdapter::WriteBlockStruct_var
        second_block = write_block_file_adapter.get_block(1);

      second_block->size(100);
      memset(second_block->content(), 'A', 100);

      first_block->next_index(second_block->index());

      PlainStorage::WriteBlockFileAdapter::ReadBlockStruct_var
        read_second_block = write_block_file_adapter.get_read_block(1);

      std::cout << "read size of block #2:"
                << read_second_block->size() << std::endl;

      std::cout << "first symbol: '"
                << *(const char*)read_second_block->read_content()
                << "'" << std::endl;
    }

    {
      unsigned long first_index = 0;
      unsigned long second_index = 0xF0000;
      
      /* big files test */
      {
        PlainStorage::WriteBlockFileAdapter
          write_block_file_adapter(
            "bigtest.out",
            64*1024,
            PlainStorage::WriteBlockFileAdapter::OT_OPEN_OR_CREATE);

        PlainStorage::WriteBlockFileAdapter::WriteBlockStruct_var
          first_block = write_block_file_adapter.get_block(first_index);

        first_block->size(100);
        memset(first_block->content(), 'A', 100);

        PlainStorage::WriteBlockFileAdapter::WriteBlockStruct_var
          second_block = write_block_file_adapter.get_block(second_index);

        second_block->size(100);
        memset(second_block->content(), 'A', 100);
      }
      
      {
        PlainStorage::WriteBlockFileAdapter
          write_block_file_adapter(
            "bigtest.out",
            64*1024,
            PlainStorage::WriteBlockFileAdapter::OT_OPEN_OR_CREATE);

        PlainStorage::WriteBlockFileAdapter::WriteBlockStruct_var
          first_block = write_block_file_adapter.get_block(first_index);

        PlainStorage::WriteBlockFileAdapter::WriteBlockStruct_var
          second_block = write_block_file_adapter.get_block(second_index);

        std::cout << "read size of block (" << first_index << "):"
                  << first_block->size() << std::endl;

        std::cout << "first symbol: '"
                  << *(const char*)first_block->read_content()
                  << "'" << std::endl;

        std::cout << "read size of block (" << second_index << "):"
                  << second_block->size() << std::endl;

        std::cout << "first symbol: '"
                  << *(const char*)second_block->read_content()
                  << "'" << std::endl;        
      }
    }
    
    {
      /* reading */
    }
  }
  catch(const eh::Exception& ex)
  {
    std::cout << "Caught exception: " << ex.what() << std::endl;
  }
  
  return 0;
}

