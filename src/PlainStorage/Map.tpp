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



// @file PlainStorage/Map.tpp
#ifndef PLAINSTORAGE_MAP_TPP
#define PLAINSTORAGE_MAP_TPP

#include <eh/Exception.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>


namespace PlainStorage
{

  //
  // BaseBlockAllocator interface
  //

  inline
  BaseBlockAllocator::~BaseBlockAllocator() throw ()
  {
  }

  //
  // PlainReader class
  //

  inline
  PlainReader::~PlainReader() throw ()
  {
  }

  //
  // PlainTransaction class
  //

  template <typename LockPolicy>
  PlainTransaction<LockPolicy>::PlainTransaction(
    typename LockPolicy::PlainActor* plain_actor) throw (eh::Exception)
    : LockPolicy(plain_actor)
  {
  }

  template <typename LockPolicy>
  PlainTransaction<LockPolicy>::~PlainTransaction() throw ()
  {
  }

  template <typename LockPolicy>
  unsigned long
  PlainTransaction<LockPolicy>::size() const
    throw ()
  {
    return LockPolicy::plain_actor_->size_i_();
  }

  template <typename LockPolicy>
  bool
  PlainTransaction<LockPolicy>::read(void* buf, unsigned long buf_size) const
    throw (eh::Exception)
  {
    return LockPolicy::plain_actor_->read_i_(buf, buf_size);
  }


//////////////////////////////////////////////////////////////////////////
// Inlines implementation

  //
  // Write class
  //

  inline
  Write::Write(PlainActor* plain_actor) throw ()
    : plain_actor_(plain_actor)
  {
    plain_actor_->write_lock_();
  }

  inline
  Write::~Write() throw ()
  {
    plain_actor_->unlock_();
  }

  inline
  void
  Write::write(const void* buf, unsigned long buf_size)
    throw (eh::Exception)
  {
    plain_actor_->write_i_(buf, buf_size);
  }

  //
  // Read class
  //

  inline
  Read::Read(PlainActor* plain_actor) throw ()
    : plain_actor_(plain_actor)
  {
    plain_actor_->read_lock_();
  }

  inline
  Read::~Read() throw ()
  {
    plain_actor_->unlock_();
  }

  //
  // PlainReader class
  //

  inline
  PlainReader::PlainReader(
    ReadBlockFileAdapter* read_block_file_adapter,
    unsigned long first_block_index,
    unsigned long data_size)
    throw (eh::Exception)
    : read_block_file_adapter_(read_block_file_adapter),
      FIRST_BLOCK_INDEX_(first_block_index),
      data_size_(data_size)
  {
  }

  inline
  unsigned long
  PlainReader::size() const throw ()
  {
    ReadGuard_ lock(lock_);
    return size_i_();
  }

  inline
  unsigned long
  PlainReader::size_i_() const throw ()
  {
    return data_size_;
  }

  inline
  unsigned long
  PlainReader::read(void* buf, unsigned long buf_size) const
    throw (eh::Exception)
  {
    ReadGuard_ lock(lock_);
    return read_i_(buf, buf_size);
  }

  inline
  PlainReadOnlyTransaction*
  PlainReader::create_readonly_transaction() throw (eh::Exception)
  {
    return new PlainReadOnlyTransaction(this);
  }

  inline
  BlockIndex
  PlainReader::index() const throw ()
  {
    return FIRST_BLOCK_INDEX_;
  }

  inline
  void
  PlainReader::read_lock_() throw ()
  {
    lock_.lock_read();
  }

  inline
  void
  PlainReader::unlock_() throw ()
  {
    lock_.unlock();
  }

  //
  // PlainWriter class
  //

  inline
  PlainWriter::PlainWriter(
    WriteBlockFileAdapter* write_block_file_adapter,
    BaseBlockAllocator* block_allocator,
    unsigned long first_block_index,
    unsigned long data_size)
    throw (eh::Exception)
    : PlainReader(
        write_block_file_adapter,
        (first_block_index ? first_block_index :
          (block_allocator ? block_allocator->allocate() : 0)),
        data_size),
      write_block_file_adapter_(write_block_file_adapter),
      block_allocator_(block_allocator)
  {
//    assert (block_allocator_.get() && FIRST_BLOCK_INDEX_); // or throw InvalidParam exception

/*    if (first_block_index == 0)
    {
      first_block_index_ = block_allocator_->allocate();
    }*/
  }

  inline
  PlainWriter::~PlainWriter() throw ()
  {
  }

  inline
  PlainReadWriteTransaction*
  PlainWriter::create_readwrite_transaction() throw (eh::Exception)
  {
    return new PlainReadWriteTransaction(this);
  }

  inline
  void
  PlainWriter::write(const void* buf, unsigned long buf_size)
    throw (eh::Exception)
  {
    WriteGuard_ lock(lock_);
    write_i_(buf, buf_size);
  }

  inline
  void
  PlainWriter::write_lock_() throw ()
  {
    lock_.lock_write();
  }

  //
  // DefaultReadIndexAccessor<Key> class
  //

  template <typename Key>
  void
  DefaultReadIndexAccessor<Key>::load(const void* buf,
    unsigned long size, Key& out)
    throw (eh::Exception)
  {
    out = std::string(static_cast<const char*>(buf), size);
  }

  //
  // DefaultWriteIndexAccessor<Key> class
  //

  template <typename Key>
  DefaultWriteIndexAccessor<Key>::DefaultWriteIndexAccessor() throw ()
  {
  }

  template <typename Key>
  unsigned long
  DefaultWriteIndexAccessor<Key>::size(const Key& in) throw (eh::Exception)
  {
    return in.size();
  }

  template <typename Key>
  void
  DefaultWriteIndexAccessor<Key>::save(const Key& in,
    void* buf, unsigned long size)
    throw (eh::Exception)
  {
    memcpy(buf, in.data(), std::min(size, in.length()));
  }

  //
  // SyncIndexStrategy<Key>::IndexLoadCallback interface
  //

  template <typename Key>
  SyncIndexStrategy::IndexLoadCallback<Key>::~IndexLoadCallback() throw ()
  {
  }

  //
  // SyncIndexStrategy::GenericField class
  //

  inline
  SyncIndexStrategy::FieldType&
  SyncIndexStrategy::GenericField::value() throw ()
  {
    return data_;
  }

  inline
  SyncIndexStrategy::FieldType
  SyncIndexStrategy::GenericField::value() const throw ()
  {
    return data_;
  }

  //
  // SyncIndexStrategy::FileHeader class
  //

  inline
  SyncIndexStrategy::FieldType&
  SyncIndexStrategy::FileHeader::allocator_index() throw ()
  {
    return data_[FH_FIRST_ALLOCATOR_DESC_BLOCK];
  }

  inline
  SyncIndexStrategy::FieldType
  SyncIndexStrategy::FileHeader::allocator_index() const throw ()
  {
    return data_[FH_FIRST_ALLOCATOR_DESC_BLOCK];
  }

  inline
  SyncIndexStrategy::FieldType&
  SyncIndexStrategy::FileHeader::first_index_block() throw ()
  {
    return data_[FH_FIRST_INDEX_DESC_BLOCK];
  }

  inline
  SyncIndexStrategy::FieldType
  SyncIndexStrategy::FileHeader::first_index_block() const throw ()
  {
    return data_[FH_FIRST_INDEX_DESC_BLOCK];
  }

  //
  // SyncIndexStrategy::KeyHeader class
  //

  inline
  SyncIndexStrategy::FieldType&
  SyncIndexStrategy::KeyHeader::key_size() throw ()
  {
    return data_[KH_KEYSIZE];
  }

  inline
  SyncIndexStrategy::FieldType
  SyncIndexStrategy::KeyHeader::key_size() const throw ()
  {
    return data_[KH_KEYSIZE];
  }

  inline
  unsigned long
  SyncIndexStrategy::KeyHeader::get_key_body_size() const throw ()
  {
    return data_[KH_KEYSIZE] - KEY_HEADER_SIZE;
  }

  inline
  SyncIndexStrategy::FieldType&
  SyncIndexStrategy::KeyHeader::data_block_index() throw ()
  {
    return data_[KH_DATABLOCK];
  }

  inline
  SyncIndexStrategy::FieldType
  SyncIndexStrategy::KeyHeader::data_block_index() const throw ()
  {
    return data_[KH_DATABLOCK];
  }

  inline
  SyncIndexStrategy::FieldType&
  SyncIndexStrategy::KeyHeader::mark() throw ()
  {
    return data_[KH_MARK];
  }

  inline
  SyncIndexStrategy::FieldType
  SyncIndexStrategy::KeyHeader::mark() const throw ()
  {
    return data_[KH_MARK];
  }

  inline
  void*
  SyncIndexStrategy::KeyHeader::key_value() throw ()
  {
    return &data_[KH_NUMBER_FIELDS];
  }

  inline
  const void*
  SyncIndexStrategy::KeyHeader::key_value() const throw ()
  {
    return &data_[KH_NUMBER_FIELDS];
  }

  //
  // Map<Key, KeyAccessor, MapTraits>::NodeValueType class
  //

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename SecondType>
  Map<Key, KeyAccessor, MapTraits>::NodeValueType<SecondType>::
    NodeValueType(const Key& key,
      SecondType plain_writer) throw ()
    : first(key),
      second(plain_writer)
  {
  }

  //
  // Map<Key, KeyAccessor, MapTraits>::MapBaseIterator struct
  //

  template <typename Key, typename KeyAccessor, typename MapTraits>
  Map<Key, KeyAccessor, MapTraits>::MapBaseIterator::MapBaseIterator()
    throw ()
    : container_ref_(0)
  {
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  Map<Key, KeyAccessor, MapTraits>::MapBaseIterator::MapBaseIterator(
    const typename IndexContainer::iterator& it,
    IndexContainer* container_ref) throw ()
    : it_(it),
      container_ref_(container_ref)
  {
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  void
  Map<Key, KeyAccessor, MapTraits>::MapBaseIterator::inc_()
    throw (OutOfRange)
  {
    if (container_ref_ == 0 || it_ == container_ref_->end())
    {
      Stream::Error ostr;
      ostr << FNS << "try to increase the end iterator";
      throw OutOfRange(ostr);
    }

    ++it_;
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  void
  Map<Key, KeyAccessor, MapTraits>::MapBaseIterator::dec_()
    throw (OutOfRange)
  {
    if (container_ref_ == 0 || it_ == container_ref_->begin())
    {
      Stream::Error ostr;
      ostr << FNS << "try to decrease the begin iterator";
      throw OutOfRange(ostr);
    }
    --it_;
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  void
  Map<Key, KeyAccessor, MapTraits>::MapBaseIterator::set_(
    const MapBaseIterator& right) throw ()
  {
    it_ = right.it_;
    container_ref_ = right.container_ref_;
  }

  //
  // Map<Key, KeyAccessor, MapTraits>::BiDiIterator class
  //

  //
  // Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
  //   ReturnedMediator struct
  //

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    ReturnedMediator::ReturnedMediator(const Key& key,
      PlainWriter_var& plain_writer)
      throw ()
    : Reference(key, plain_writer)
  {
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  Reference*
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    ReturnedMediator::operator ->()
      throw ()
  {
    return this;
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    BiDiIterator() throw ()
  {
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    BiDiIterator(const typename IndexContainer::iterator& it,
      IndexContainer& container) throw ()
    : MapBaseIterator(it, &container)
  {
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    BiDiIterator(const iterator& it) throw ()
    : MapBaseIterator(it.it_, it.container_ref_)
  {
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  Reference
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    operator *() const throw ()
  {
    return Reference(MapBaseIterator::it_->first,
      MapBaseIterator::it_->second.first);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  typename Map<Key, KeyAccessor, MapTraits>::
    template BiDiIterator<Reference>::ReturnedMediator
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    operator ->() const throw ()
  {
    return ReturnedMediator(MapBaseIterator::it_->first,
      MapBaseIterator::it_->second.first);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>&
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    operator ++() throw (OutOfRange)
  {
    MapBaseIterator::inc_();
    return *this;
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    operator ++(int) throw (OutOfRange)
  {
    BiDiIterator<Reference> old_it = *this;
    ++*this;
    return old_it;
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>&
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    operator --() throw (OutOfRange)
  {
    MapBaseIterator::dec_();
    return *this;
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    operator --(int) throw (OutOfRange)
  {
    BiDiIterator<Reference> old_it = *this;
    --*this;
    return old_it;
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  bool
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    operator ==(const BiDiIterator& right) const throw ()
  {
    return this->it_ == right.it_;
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  bool
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    operator !=(const BiDiIterator& right) const throw ()
  {
    return this->it_ != right.it_;
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  template <typename Reference>
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>&
  Map<Key, KeyAccessor, MapTraits>::BiDiIterator<Reference>::
    operator =(const iterator& it) throw ()
  {
    this->set_(it);
    return *this;
  }

  //
  // Map<Key, KeyAccessor, MapTraits> class
  //

  template <typename Key, typename KeyAccessor, typename MapTraits>
  Map<Key, KeyAccessor, MapTraits>::Map()
    throw ()
  {
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  Map<Key, KeyAccessor, MapTraits>::Map(
    const char* filename, unsigned long block_size)
    throw (eh::Exception)
  {
    load(filename, block_size);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  Map<Key, KeyAccessor, MapTraits>::~Map()
    throw ()
  {
    close();
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  typename Map<Key, KeyAccessor, MapTraits>::iterator
  Map<Key, KeyAccessor, MapTraits>::begin()
    throw ()
  {
    return iterator(index_container_.begin(), index_container_);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  typename Map<Key, KeyAccessor, MapTraits>::const_iterator
  Map<Key, KeyAccessor, MapTraits>::begin() const
    throw ()
  {
    return const_iterator(index_container_.begin(), index_container_);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  typename Map<Key, KeyAccessor, MapTraits>::iterator
  Map<Key, KeyAccessor, MapTraits>::end()
    throw ()
  {
    return iterator(index_container_.end(), index_container_);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  typename Map<Key, KeyAccessor, MapTraits>::const_iterator
  Map<Key, KeyAccessor, MapTraits>::end() const
    throw ()
  {
    return const_iterator(index_container_.end(), index_container_);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  typename Map<Key, KeyAccessor, MapTraits>::iterator
  Map<Key, KeyAccessor, MapTraits>::find(const Key& key)
    throw ()
  {
    return 
      typename Map<Key, KeyAccessor, MapTraits>::iterator(
        index_container_.find(key), index_container_);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  typename Map<Key, KeyAccessor, MapTraits>::const_iterator
  Map<Key, KeyAccessor, MapTraits>::find(const Key& key) const
    throw ()
  {
    return 
      typename Map<Key, KeyAccessor, MapTraits>::const_iterator(
        index_container_.find(key), index_container_);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  typename Map<Key, KeyAccessor, MapTraits>::size_type
  Map<Key, KeyAccessor, MapTraits>::erase(const Key& key)
    throw (eh::Exception)
  {
    typename IndexContainer::iterator it = index_container_.find(key);

    if (it != index_container_.end())
    {
      erase(iterator(it, index_container_));
      return 1;
    }

    return 0;
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  void
  Map<Key, KeyAccessor, MapTraits>::erase(
    Map<Key, KeyAccessor, MapTraits>::iterator it)
    throw (eh::Exception)
  {
    typename IndexContainer::iterator i_it = it.it_;
    sync_index_strategy_->erase(i_it->first, i_it->second.second);
    index_container_.erase(i_it);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  typename Map<Key, KeyAccessor, MapTraits>::iterator
  Map<Key, KeyAccessor, MapTraits>::insert(const Key& key)
    throw (eh::Exception)
  {
    typename IndexContainer::iterator ret_it = index_container_.find(key);

    if (ret_it == index_container_.end())
    {
      PlainWriter_var new_plain_writer;

      init_value_(new_plain_writer);

      KeyAddition key_addition;

      sync_index_strategy_->insert(
        key, new_plain_writer->index(), key_addition);

      std::pair<typename IndexContainer::iterator, bool> pair_ib_ =
        index_container_.insert(
          typename IndexContainer::value_type(
            key, ContainerValue(
              new_plain_writer,
              key_addition)));

      ret_it = pair_ib_.first;
    }

    return iterator(ret_it, index_container_);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  typename Map<Key, KeyAccessor, MapTraits>::Pairib_
  Map<Key, KeyAccessor, MapTraits>::insert(
    const Map<Key, KeyAccessor, MapTraits>::ValueType& val)
    throw (eh::Exception)
  {
    PlainWriter_var new_plain_writer;

    copy_value_(new_plain_writer, val.second);

    // erasing if key exist
    typename IndexContainer::iterator it = 
      index_container_.find(val.first);

    KeyAddition key_addition;

    if (it != index_container_.end())
    {
      it->second.size(0);
      sync_index_strategy_->update(
        it->first, new_plain_writer->index(), it->second.second);

      it->second.first = new_plain_writer;

      return 
        std::pair<iterator, bool>(
          iterator(it, index_container_), false);
    }
    else
    {
      KeyAddition key_addition;
      sync_index_strategy_->insert(
        val.first, new_plain_writer->index(), key_addition);

      std::pair<typename IndexContainer::iterator, bool> 
        pair_ib_ = index_container_.insert(
          val.first, std::pair<PlainWriter, KeyAddition>(
            new_plain_writer,
            key_addition));

      return 
        std::pair<iterator, bool>(
          iterator(pair_ib_.first, index_container_), true);
    }
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  PlainWriter_var
  Map<Key, KeyAccessor, MapTraits>::operator [](
    const Key& key) throw (eh::Exception)
  {
    typename IndexContainer::iterator it = index_container_.find(key);

    if (it != index_container_.end())
    {
      return it->second.first;
    }
    else
    {
      return insert(key)->second;
    }
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  void 
  Map<Key, KeyAccessor, MapTraits>::clear()
    throw (eh::Exception)
  {
    for (typename IndexContainer::const_iterator it = 
           index_container_.begin();
         it != index_container_.end(); ++it)
    {
      sync_index_strategy_->erase(it->first, it->second.second);
    }

    index_container_.clear();
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  std::size_t
  Map<Key, KeyAccessor, MapTraits>::size() const throw ()
  {
    return index_container_.size();
  }  

  template <typename Key, typename KeyAccessor, typename MapTraits>
  void
  Map<Key, KeyAccessor, MapTraits>::load(
    const char* filename, unsigned long block_size)
    throw (eh::Exception)
  {
    BlockIndex first_allocator_desc_block;
    BlockIndex first_index_desc_block;

    // open file with filename
    write_block_file_adapter_.reset(
      new WriteBlockFileAdapter(filename, block_size,
        WriteBlockFileAdapter::OT_OPEN_OR_CREATE));

    // empty file occupied 4 Data Blocks
    if (write_block_file_adapter_->max_block_index() == 0)
    {
      create_head_(
        write_block_file_adapter_.get(),
        first_allocator_desc_block,
        first_index_desc_block);
    }
    else
    {
      load_head_(
        write_block_file_adapter_.get(),
        first_allocator_desc_block,
        first_index_desc_block);
    }

    block_allocator_.reset(
      new typename Map<Key, KeyAccessor, MapTraits>::BlockAllocator(
        write_block_file_adapter_.get(), first_allocator_desc_block));

    sync_index_strategy_.reset(
      new typename Map<Key, KeyAccessor, MapTraits>::SyncIndexStrategy(
        write_block_file_adapter_.get(),
        block_allocator_.get(),
        first_index_desc_block));

    sync_index_strategy_->load(this);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  void
  Map<Key, KeyAccessor, MapTraits>::close()
    throw (eh::Exception)
  {
    if (write_block_file_adapter_.get())
    {
      if (sync_index_strategy_->begin_saving())
      {
        for (typename IndexContainer::const_iterator it =
               index_container_.begin();
             it != index_container_.end(); ++it)
        {
          sync_index_strategy_->save(
            it->first, it->second.first->index(), it->second.second);
        }
      }

      sync_index_strategy_->end_saving(); 

      sync_index_strategy_.reset(0);
      block_allocator_.reset(0);
      write_block_file_adapter_.reset(0);
    }
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  void
  Map<Key, KeyAccessor, MapTraits>::init_value_(
    PlainWriter_var& plain_writer)
    throw (eh::Exception)
  {
    BlockIndex first_index = 
      block_allocator_->allocate();

    plain_writer = 
      new PlainWriter(
        write_block_file_adapter_.get(),
        block_allocator_.get(),
        first_index,
        0);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  void
  Map<Key, KeyAccessor, MapTraits>::copy_value_(
    PlainWriter_var& plain_writer,
    PlainWriter* source_plain_writer)
    throw (eh::Exception)
  {
    plain_writer = 
      ReferenceCounting::add_ref(source_plain_writer);
  }

  template <typename Key, typename KeyAccessor, typename MapTraits>
  void
  Map<Key, KeyAccessor, MapTraits>::load_head_(
    WriteBlockFileAdapter* write_block_adapter,
    BlockIndex& first_allocator_desc_block,
    BlockIndex& first_index_desc_block)
    throw (eh::Exception, CorruptedFile)
  {
    try
    {
      WriteBlockFileAdapter::ReadBlockStruct_var
        header_block = write_block_adapter->get_read_block(0);

      if (header_block->size() < FileHeader::FILE_HEADER_SIZE)
      {
        throw CorruptedFile("Header block size is small");
      }

      const FileHeader& head = *static_cast<const FileHeader*>(
        header_block->read_content());
      first_allocator_desc_block = head.allocator_index();
      first_index_desc_block = head.first_index_block();
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't read header block: " << ex.what();
      throw CorruptedFile(ostr);
    }
  };

  template <typename Key, typename KeyAccessor, typename MapTraits>
  void
  Map<Key, KeyAccessor, MapTraits>::create_head_(
    WriteBlockFileAdapter* write_block_adapter,
    BlockIndex& first_allocator_desc_block,
    BlockIndex& first_index_desc_block)
    throw (eh::Exception, CreationFailed)
  {
    try
    {
      WriteBlockFileAdapter::WriteBlockStruct_var
        index_desc_block = write_block_adapter->get_block(2);

      typedef GenericField FirstKeysIndexBlock;
      index_desc_block->size(FirstKeysIndexBlock::SIZE);
      static_cast<FirstKeysIndexBlock*>(index_desc_block->content())->value()
        = 0;

      WriteBlockFileAdapter::WriteBlockStruct_var
        allocator_desc_block = write_block_adapter->get_block(1);
      allocator_desc_block->next_index(0);
 
      typedef GenericField AllocatedIndex;
      allocator_desc_block->size(AllocatedIndex::SIZE);
      static_cast<AllocatedIndex*>(allocator_desc_block->content())->value()
        = 0;

      WriteBlockFileAdapter::WriteBlockStruct_var
        header_block = write_block_adapter->get_block(0);
      header_block->next_index(0);
      
      header_block->size(FileHeader::FILE_HEADER_SIZE);

      FileHeader& head =
        *static_cast<FileHeader*>(header_block->content());

      first_allocator_desc_block = 1;
      first_index_desc_block = 2;

      head.allocator_index() = first_allocator_desc_block;
      head.first_index_block() = first_index_desc_block;
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't create header block: " << ex.what();
      throw CreationFailed(ostr);
    }
  };

  // IndexLoadCallback interface implementation
  template <typename Key, typename KeyAccessor, typename MapTraits>
  void
  Map<Key, KeyAccessor, MapTraits>::load_key(
    const Key& key,
    const BlockIndex& first_data_block,
    const typename Map<Key, KeyAccessor, MapTraits>::
      SyncIndexStrategy::KeyAddition& key_addition)
    throw (eh::Exception)
  {
    try
    {
      unsigned long data_size = 0;

      {
        // data size counting
        ReadBlockFileAdapter::ReadBlockStruct_var 
          cur_block = write_block_file_adapter_->get_block(first_data_block);

        while (cur_block.in())
        {
          data_size += cur_block->size();
          cur_block = cur_block->read_next();
        }
      }

      PlainWriter_var plain_writer = 
        new PlainWriter(
          write_block_file_adapter_.get(),
          block_allocator_.get(),
          first_data_block,
          data_size);

      index_container_.insert(
        typename IndexContainer::value_type(
          key,
          ContainerValue(plain_writer, key_addition)));
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't load key. Caught eh::Exception: " << ex.what();
      throw LoadFailed(ostr);
    }
  }
}

#endif /* PLAINSTORAGE_MAP_TPP */
