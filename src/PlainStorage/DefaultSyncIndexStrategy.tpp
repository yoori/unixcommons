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



// @file PlainStorage/DefaultSyncIndexStrategy.tpp
#ifndef PLAINSTORAGE_DEFAULTSYNCINDEXSTRATEGY_TPP
#define PLAINSTORAGE_DEFAULTSYNCINDEXSTRATEGY_TPP

#include <eh/Exception.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>


namespace PlainStorage
{
  template <typename Key, typename KeyAccessor>
  DefaultSyncIndexStrategy<Key, KeyAccessor>::
  DefaultSyncIndexStrategy(
    WriteBlockFileAdapter* write_block_file_adapter,
    BaseBlockAllocator* block_allocator,
    BlockIndex descr_block_index)
    throw (typename BaseType::Exception)
    : write_block_file_adapter_(write_block_file_adapter),
      block_allocator_(block_allocator)
  {
    try
    {
      descr_block_ = 
        write_block_file_adapter_->get_block(descr_block_index);
      
      typedef const GenericField FirstKeysIndexBlock;
      BlockIndex first_keys_block_index = 
        static_cast<FirstKeysIndexBlock*>(descr_block_->content())->value();

      if (first_keys_block_index == 0)
      {
        first_keys_block_index = block_allocator_->allocate();

        first_keys_block_ = 
          write_block_file_adapter_->get_block(first_keys_block_index);

        sync_();
      }
      else
      { 
        first_keys_block_ = 
          write_block_file_adapter_->get_block(first_keys_block_index);
      }
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't create DefaultSyncIndexStrategy. "
        "Caught eh::Exception: " << ex.what();
      throw typename BaseType::Exception(ostr);
    }
  }

  template <typename Key, typename KeyAccessor>
  bool
  DefaultSyncIndexStrategy<Key, KeyAccessor>::
  save_(
    WriteBlockFileAdapter::WriteBlockStruct_var& write_block,
    const Key& key, 
    BlockIndex first_data_block,
    DefaultSyncIndexStrategy<Key, KeyAccessor>::KeyAddition& 
      key_addition)
    throw (eh::Exception, typename BaseType::FileFormatError)
  {
    WriteGuard_ lock(lock_);

    // get user data size
    unsigned long used_size = write_block->size();
    // get size of all available data for user in block
    unsigned long all_data_size = write_block->available_size();

    KeyAccessor key_accessor;
    const unsigned long SIZE_OF_KEY = key_accessor.size(key);
    
    BlockIndex old_first_keys_block_index = 
      first_keys_block_->index();

    // if not used available memory < space need to save Key
    if (all_data_size - used_size <
      SIZE_OF_KEY + KeyHeader::KEY_HEADER_SIZE)
    {
      // allocate additional block and insert in front of Data blocks chain
      // that store keys - it is creating Keys chain
      BlockIndex new_keys_block_index = block_allocator_->allocate();
      
      // write_block IS REFERENCE ON first_keys_block_
      write_block = 
        write_block_file_adapter_->get_block(new_keys_block_index);
      
      write_block->next_index(old_first_keys_block_index);

      all_data_size = write_block->available_size();
      used_size = 0;

      if (all_data_size < SIZE_OF_KEY + KeyHeader::KEY_HEADER_SIZE)
      {
        Stream::Error ostr;
        ostr << FNS << "Key size > size of block of file";
        throw typename BaseType::FileFormatError(ostr);
      }
    }

    key_addition.block_index = write_block->index();
    key_addition.block_offset = used_size;

    write_block->size(used_size + KeyHeader::KEY_HEADER_SIZE + SIZE_OF_KEY);

    KeyHeader& ex_pos = *reinterpret_cast<KeyHeader*>(
        static_cast<char*>(first_keys_block_->content()) + used_size);

    ex_pos.key_size() = KeyHeader::KEY_HEADER_SIZE + SIZE_OF_KEY;
    ex_pos.data_block_index() = first_data_block;
    ex_pos.mark() = KeyHeader::MARK_VALID;

    key_accessor.save(key, ex_pos.key_value(), SIZE_OF_KEY);

    return old_first_keys_block_index != write_block->index();
  }

  template <typename Key, typename KeyAccessor>
  void
  DefaultSyncIndexStrategy<Key, KeyAccessor>::sync_() 
    throw (eh::Exception)
  {
    ReadGuard_ lock(lock_);
    
    typedef GenericField FirstKeysIndexBlock;
    static_cast<FirstKeysIndexBlock*>(descr_block_->content())->value() =
      first_keys_block_->index();
  }

  template <typename Key, typename KeyAccessor>
  void
  DefaultSyncIndexStrategy<Key, KeyAccessor>::
  insert(
    const Key& key, 
    BlockIndex first_data_block,
    DefaultSyncIndexStrategy<Key, KeyAccessor>::KeyAddition& 
      key_addition)
    throw (eh::Exception)
  {
    if (save_(first_keys_block_, key, first_data_block, key_addition))
    {
      sync_();
    }
  }

  template <typename Key, typename KeyAccessor>
  void
  DefaultSyncIndexStrategy<Key, KeyAccessor>::
  update(
    const Key& key, 
    BlockIndex first_data_block,
    const DefaultSyncIndexStrategy<Key, KeyAccessor>::KeyAddition& 
      key_addition)
    throw (eh::Exception)
  {
    WriteBlockFileAdapter::WriteBlockStruct_var key_block = 
      write_block_file_adapter_->get_block(
        key_addition.block_index);

    KeyHeader& key_pos = 
      *static_cast<KeyHeader*>(
        static_cast<char*>(key_block->content()) +
          key_addition.block_offset);

    key_pos.data_block_index() = first_data_block;    
  }

  template <typename Key, typename KeyAccessor>
  void 
  DefaultSyncIndexStrategy<Key, KeyAccessor>::erase(
    const Key& /*key*/, 
    const DefaultSyncIndexStrategy<Key, KeyAccessor>::KeyAddition&
      key_addition)
    throw (eh::Exception)
  {
    WriteBlockFileAdapter::WriteBlockStruct_var key_block = 
      write_block_file_adapter_->get_block(
        key_addition.block_index);

    const unsigned long SIZE = key_block->size();

    KeyHeader& key_pos = *reinterpret_cast<KeyHeader*>(
      static_cast<char*>(key_block->content()) +
        key_addition.block_offset);

    if (SIZE == key_addition.block_offset + key_pos.key_size())
    {
      // last key in block
      key_block->size(key_addition.block_offset);
    }
    else
    { 
      key_pos.mark() = KeyHeader::MARK_DELETED;
    }
  }

  template <typename Key, typename KeyAccessor>
  void 
  DefaultSyncIndexStrategy<Key, KeyAccessor>::load(
    DefaultSyncIndexStrategy<Key, KeyAccessor>::IndexLoadCallback*
      index_load_callback)
    throw (eh::Exception, typename BaseType::LoadIndexFail)
  {
    BlockIndex block_index = 0;
    unsigned long in_block_offset = 0;
    try
    {
      // loading index on start
      ReadBlockFileAdapter::ReadBlockStruct_var block_cur =
        ReferenceCounting::add_ref(first_keys_block_);

      KeyAccessor key_accessor;

      while (block_cur.in())
      {
        unsigned long sz = block_cur->size();
        block_index = block_cur->index();
        in_block_offset = 0;
        const char* pos =
          static_cast<const char*>(block_cur->read_content());

        while (in_block_offset < sz)
        {
          const KeyHeader& keyhead = *reinterpret_cast<const KeyHeader*>(pos);

          if (keyhead.mark() != KeyHeader::MARK_DELETED)
          {
            Key new_key;
            KeyAddition new_key_addition;

            key_accessor.load(keyhead.key_value(),
              keyhead.get_key_body_size(), new_key);

            new_key_addition.block_index = block_cur->index();
            new_key_addition.block_offset = in_block_offset;

            index_load_callback->load_key(
              new_key, keyhead.data_block_index(), new_key_addition);
          }

          pos += keyhead.key_size();
          in_block_offset += keyhead.key_size();
        }

        block_cur = block_cur->read_next();
      }
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't load index. "
        "Block #" << block_index << ", offset=" << in_block_offset <<
        ". Caught eh::Exception: " << ex.what();
      throw typename BaseType::LoadIndexFail(ostr); 
    }
  }

  template <typename Key, typename KeyAccessor>
  bool 
  DefaultSyncIndexStrategy<Key, KeyAccessor>::begin_saving()
    throw (eh::Exception)
  {
    return false;
  }

  template <typename Key, typename KeyAccessor>
  void 
  DefaultSyncIndexStrategy<Key, KeyAccessor>::save(
    const Key& /*key*/,
    BlockIndex /*first_data_block*/,
    const KeyAddition& /*key_addition*/)
    throw (eh::Exception)
  {
  }

  template <typename Key, typename KeyAccessor>
  void 
  DefaultSyncIndexStrategy<Key, KeyAccessor>::end_saving()
    throw (eh::Exception)
  {
    // erase marked as deleted keys
/*
    ReadBlockFileAdapter::ReadBlockStruct_var 
      block_cur = ReferenceCounting::add_ref(first_keys_block_);
    
    KeyAccessor key_accessor;

    BlockIndex new_keys_index = block_allocator_->allocate();
    
    WriteBlockFileAdapter::WriteBlockStruct_var 
      new_first_keys_block =
        write_block_file_adapter_->get_block(new_keys_index);

    while (block_cur.in())
    {
      unsigned long sz = block_cur->size();
      unsigned long in_block_offset = 0;
      const KeyHeader* cur_exkey =
        static_cast<const KeyHeader*>(block_cur->read_content());

      while (in_block_offset < sz)
      {
        Key new_key;
        KeyAddition new_key_addition;
        unsigned long key_size = cur_exkey->get_key_body_size();

        const void* one_key = cur_exkey->key_value();
        key_accessor.load(one_key, key_size, new_key);

        new_key_addition.block_index = block_cur->index();
        new_key_addition.block_offset = in_block_offset;

        in_block_offset += cur_exkey->key_size();
        cur_exkey = reinterpret_cast<const KeyHeader*>(
          static_cast<const char*>(one_key) + cur_exkey->key_size());
      }

      block_cur = block_cur->read_next();
    }
*/
  }
}

#endif // PLAINSTORAGE_DEFAULTSYNCINDEXSTRATEGY_TPP
