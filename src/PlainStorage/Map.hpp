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



// @file PlainStorage/Map.hpp
#ifndef PLAINSTORAGE_MAP_HPP
#define PLAINSTORAGE_MAP_HPP

#include <memory>
#include <map>

#include <eh/Exception.hpp>
#include <Sync/PosixLock.hpp>
#include <ReferenceCounting/AtomicImpl.hpp>
#include <PlainStorage/BlockFileAdapter.hpp>

/**
 * Library map files in memory, providing fast access
 * and removal of the parts (Data blocks) of file
 */
namespace PlainStorage
{
  class BaseBlockAllocator;
  class PlainWriter;

  /**
   * Lock policies for PlainTransaction object
   */
  class Write
  {
  protected:
    typedef PlainWriter PlainActor;
    Write(PlainActor* plain_actor) throw ();

    ~Write() throw ();

    PlainActor* plain_actor_;

  public:
    /**
     * Perform write Data of specified size. The existing chain of
     * Data blocks increases, as necessary or deallocate excess if less
     * data is written
     * 1. Calculate size of data to be write for each Data block
     * 2. Fill current writing block part of the Data (do write)
     * 3. Get next Block for writing go 1 until all data will not be wrote
     * 4. Deallocate excess of chain
     * @param buf The pointer to data to be saved
     * @param buf_size The size of data to be written pointed by buf
     */
    void
    write(const void* buf, unsigned long buf_size)
      throw (eh::Exception);
  };

  class PlainReader;
  class PlainWriter;

  class Read
  {
  protected:
    typedef PlainReader PlainActor;
    Read(PlainActor* plain_actor) throw ();

    ~Read() throw ();

    PlainActor* plain_actor_;
  };

  /**
   * PlainTransaction class is guard that allow read-write operation
   * automatically protected by mutex
   */
  template <typename LockPolicy = Read>
  class PlainTransaction : public LockPolicy,
    public ReferenceCounting::AtomicImpl
  {
  public:
    friend class PlainReader;
    friend class PlainWriter;

    /**
     * @return The size of the data that will be write or read in this
     * transaction
     */
    unsigned long
    size() const throw ();

    /**
     * Reads the data provided by PlainWriter at the creation of the
     * PlainTransaction
     * @param buf The pointer to allocated memory to store loaded data
     * @param buf_size The size of allocated memory pointed by buf
     */
    bool
    read(void* buf, unsigned long buf_size) const throw (eh::Exception);

  protected:
    /**
     * Do lock at PlainWriter
     * @param plain_actor Pointer to PlainWriter or PlainReader, must be
     * non zero
     */
    PlainTransaction(typename LockPolicy::PlainActor* plain_actor)
      throw (eh::Exception);

    /**
     * Do unlock at PlainWriter
     */
    virtual
    ~PlainTransaction() throw ();
  };

  typedef PlainTransaction<Read> PlainReadOnlyTransaction;
  typedef PlainTransaction<Write> PlainReadWriteTransaction;

  typedef ReferenceCounting::SmartPtr<PlainReadWriteTransaction>
    PlainReadWriteTransaction_var;
  typedef ReferenceCounting::SmartPtr<PlainReadOnlyTransaction>
    PlainTransaction_var;

  //
  // wrappers to plain data access
  //

  /**
   * Allows one to read some fixed part of file of fixed length,
   * this part accessible through chain of Data blocks
   */
  class PlainReader :
    public virtual ReferenceCounting::AtomicImpl
  {
    friend class PlainTransaction<Read>;
    friend class Read;
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(BufferExhausted, Exception);
    DECLARE_EXCEPTION(ReadFailed, Exception);

    /**
     * Constructor
     * @param read_block_file_adapter Interface to load Data blocks in
     * shared memory
     * @param first_block_index The index of the block from which will
     * start reading
     * @param data_size The size of data from file to be loaded in some buffer
     */
    PlainReader(
      ReadBlockFileAdapter* read_block_file_adapter,
      unsigned long first_block_index,
      unsigned long data_size)
      throw (eh::Exception);

    /**
     * Thread-Safe version of size_i_()
     * @return The size of the data that PlainReader able to read
     */
    unsigned long
    size() const throw ();

    /**
     * Thread-Safe version of read_i_() Perform reading data from file to
     * buffer
     * @param buf The pointer to allocated buffer to read data from file
     * @param buf_size The size of buf. Must be greater than size()
     * @return The number of bytes actually read, i.e. size() or 0.
     */
    unsigned long
    read(void* buf, unsigned long buf_size) const
      throw (eh::Exception);

    /**
     * make-function to create reference countable object PlainTransaction.
     * Transaction can be created on that way only.
     * @return The pointer to created transaction,
     * should be putted in smart pointer
     */
    PlainReadOnlyTransaction*
    create_readonly_transaction() throw (eh::Exception);

    /**
     * @return The index from which start reading, in other words
     * return index of first Data block that store data.
     */
    BlockIndex
    index() const throw ();

  protected:
    /**
     * Empty virtual destructor
     */
    virtual
    ~PlainReader() throw ();

    /**
     * Without thread sync
     * @return The size of the data that PlainReader able to read
     */
    unsigned long
    size_i_() const throw ();

    unsigned long
    read_i_(void* buf, unsigned long buf_size) const
      throw (eh::Exception, ReadFailed);

    void
    read_lock_() throw ();

    void
    unlock_() throw ();

    typedef Sync::PosixRWLock Mutex_;
    typedef Sync::PosixRGuard ReadGuard_;
    typedef Sync::PosixWGuard WriteGuard_;

    mutable Mutex_ lock_;

    ReadBlockFileAdapter* read_block_file_adapter_;
    /// Index of first block with Data
    const BlockIndex FIRST_BLOCK_INDEX_;
    unsigned long data_size_;
  };
  typedef ReferenceCounting::SmartPtr<PlainReader> PlainReader_var;

  /**
   * Allows one to read/write some part of file of some length,
   * this part accessible through chain of Data blocks, when we write Data
   * the chain can be resized to be actually our saved Data
   */
  class PlainWriter :
    public virtual PlainReader,
    public virtual ReferenceCounting::AtomicImpl
  {
    friend class PlainTransaction<Write>;
    friend class Write;

  public:
    DECLARE_EXCEPTION(Exception, PlainReader::Exception);
    DECLARE_EXCEPTION(WriteFailed, Exception);

    /**
     * Constructor
     * @param write_block_file_adapter
     * @param block_allocator The strategy of new Data blocks allocation
     * @param first_block_index The index of first Data block that contain
     * necessary data
     * @param data_size The size of Data stored in chain of Data blocks
     * at this time
     */
    PlainWriter(
      WriteBlockFileAdapter* write_block_file_adapter,
      BaseBlockAllocator* block_allocator,
      unsigned long first_block_index,
      unsigned long data_size)
      throw (eh::Exception);

    /**
     * Do thread-safe writing buffer with data to file.
     * See write_i_ for details
     * @param buf The pointer to data to be saved
     * @param buf_size The size of data to be written pointed by buf
     */
    void
    write(const void* buf, unsigned long buf_size)
      throw (eh::Exception);

    PlainReadWriteTransaction*
    create_readwrite_transaction() throw (eh::Exception);

  protected:
    /**
     * Empty virtual destructor
     */
    virtual
    ~PlainWriter() throw ();

    /**
     * Thread-unsafe. Perform write Data of specified size.
     * The existing chain of Data blocks increases, as necessary or
     * deallocate excess if less data is written
     * 1. Calculate size of data to be write for each Data block
     * 2. Fill current writing block part of the Data (do write)
     * 3. Get next Block for writing go 1 until all data will not be wrote
     * 4. Deallocate excess of chain
     * @param buf The pointer to data to be saved
     * @param buf_size The size of data to be written pointed by buf
     */
    void
    write_i_(const void* buf, unsigned long buf_size)
      throw (eh::Exception, WriteFailed);

    void
    write_lock_() throw ();

    WriteBlockFileAdapter* write_block_file_adapter_;
    BaseBlockAllocator* block_allocator_;
  };
  typedef ReferenceCounting::SmartPtr<PlainWriter> PlainWriter_var;

  //
  // Default strategies
  //

  /**
   * DefaultReadIndexAccessor & DefaultWriteIndexAccessor
   * delegate save & load calls to key object
   */
  template <typename Key>
  class DefaultReadIndexAccessor
  {
  public:
    void
    load(const void* buf, unsigned long size, Key& out)
      throw (eh::Exception);
  };

  template <typename Key>
  class DefaultWriteIndexAccessor :
    public DefaultReadIndexAccessor<Key>
  {
  public:
    DefaultWriteIndexAccessor() throw ();

    /**
     * @param in Key to calculate his size
     * @return size of input key
     */
    unsigned long
    size(const Key& in) throw (eh::Exception);

    /**
     * Save key into Index of file
     * @param in The key to be saved into Index
     * @param buf The pointer to shared memory with mapped file,
     * method should write data of key to pointed by buf memory region
     * @param size The size of shared memory pointed by buf
     */
    void
    save(const Key& in, void* buf, unsigned long size)
      throw (eh::Exception);
  };

  class SyncIndexStrategy
  {
  public:
    /// Base exception for synchronization strategies
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    /// Raise when keys loading failed
    DECLARE_EXCEPTION(LoadIndexFail, Exception);
    /// Raise if violated constraints of Plain file format
    DECLARE_EXCEPTION(FileFormatError, Exception);

    /**
     * Additional key location information
     */
    struct KeyAddition
    {
      /// The index of the block which holds the Key
      BlockIndex block_index;
      /// Offset from Block beginning to locate Key data
      unsigned long block_offset;
    };

    /**
     * IndexLoadCallback interface
     */
    template <typename Key>
    struct IndexLoadCallback
    {
      /**
       * Implemented in derived Map. It will call from index synchronization
       * strategy for each Key that wasn't delete
       */
      virtual
      void
      load_key(
        const Key& key,
        const BlockIndex& first_data_block,
        const KeyAddition& key_addition)
        throw (eh::Exception) = 0;

      /**
       * Virtual empty destructor
       */
      virtual
      ~IndexLoadCallback() throw ();
    };

    /// Type of field in some headers of file data structures
    typedef uint32_t FieldType;
    typedef const uint32_t ConstFieldType;

    /**
     * Generic structure for some user data typification.
     * Using with local typedefs for concrete user data read/write.
     */
    class GenericField
    {
    public:
      /// sizeof of field
      static const std::size_t SIZE = sizeof(FieldType);

      FieldType&
      value() throw ();

      FieldType
      value() const throw ();

    private:
      FieldType data_;
    };

    class FileHeader
    {
      enum PlainFileHeader
      {
        /// By historical reason
        FH_RESERVED,
        /// Place to store index of first allocator block
        FH_FIRST_ALLOCATOR_DESC_BLOCK,
        /// The index of first block, with Index body
        FH_FIRST_INDEX_DESC_BLOCK,
        FH_NUMBER_FIELDS
      };
      typedef FieldType FileHeaderBody[FH_NUMBER_FIELDS];
      FileHeaderBody data_;
    public:
      /// sizeof of header (header service fields)
      static const std::size_t FILE_HEADER_SIZE = sizeof(FileHeaderBody);

      FieldType&
      allocator_index() throw ();

      FieldType
      allocator_index() const throw ();

      FieldType&
      first_index_block() throw ();

      FieldType
      first_index_block() const throw ();
    };
    /**
     * Auxiliary class need to more comfortable work with key service
     * fields (header of key)
     */
    class KeyHeader
    {
      enum PlainKeyHeader
      {
        /// Place to store sizeof(All key) (Key body + key header)
        KH_KEYSIZE,
        /// The index of first Data block, that should be available for this key
        KH_DATABLOCK,
        /// Place to mark Data block as Valid or Deleted
        KH_MARK,
        /// Key header size - the number of FieldType fields into Key header
        KH_NUMBER_FIELDS
      };
      typedef FieldType KeyHeaderBody[KH_NUMBER_FIELDS];
      KeyHeaderBody data_;
    public:
      /// sizeof of header (header service fields)
      static const std::size_t KEY_HEADER_SIZE = sizeof(KeyHeaderBody);
      enum
      {
        MARK_VALID = 0,
        MARK_DELETED = 1
      };

      FieldType&
      key_size() throw ();

      FieldType
      key_size() const throw ();

      /**
       * @return sizeof of data of the key (sizeof(body))
       */
      unsigned long
      get_key_body_size() const throw ();

      FieldType&
      data_block_index() throw ();

      FieldType
      data_block_index() const throw ();

      FieldType&
      mark() throw ();

      FieldType
      mark() const throw ();

      void*
      key_value() throw ();

      const void*
      key_value() const throw ();
    };

  };

  /**
   * The strategy for saving and reading data to a file and from the file.
   * The strategy allows you to change the details of I/O, for example,
   * apply caching without changing the overall architecture of the library
   */
  template <typename Key, typename INDEX_ACCESSOR>
  class DefaultSyncIndexStrategy : public SyncIndexStrategy
  {
  public:
    typedef typename SyncIndexStrategy::KeyAddition KeyAddition;
    typedef typename SyncIndexStrategy::IndexLoadCallback<Key>
      IndexLoadCallback;
    typedef SyncIndexStrategy BaseType;
    /**
     * Constructor calculate and save reference to index description and
     * reference to first Keys Block. If cannot calculate index of Keys begin,
     * create Block to store Keys, this may occur file resize.
     * @param write_block_file_adapter allow read/write
     * portions of file in several stages
     * @param block_allocator New blocks allocation strategy
     * @param descr_block_index Index of first block that describe index,
     * usually 2. [Head 0][Allocator 1][Index Description 2]
     */
    DefaultSyncIndexStrategy(
      WriteBlockFileAdapter* write_block_file_adapter,
      BaseBlockAllocator* block_allocator,
      BlockIndex descr_block_index)
      throw (typename BaseType::Exception);

    /**
     * Load Keys - body of index
     * Key have header - 4 uint32_t numbers, structure of KeyHead described
     * by PlainKeyHeader enumeration:
     * [KeySize][DataBlock][Mark][Size]
     * [Mark] can be in two states Valid or Deleted
     *
     * One Block can contain some Keys, iterate all Blocks and Keys into Blocks
     * while loading. For each Key call index_load_callback, i.e. Map::load
     *   Map::load calculate data size, create PlainWriter and insert
     * pair of (Key, ContainerValue) in std::map
     *
     * @param index_load_callback At this time pointer to base of Map
     */
    void
    load(IndexLoadCallback* index_load_callback)
      throw (eh::Exception, typename BaseType::LoadIndexFail);

    /**
     * Inserts an key in file. Do not check existence of key, simply
     * save key to file
     * @param key The key to be saved
     * @param first_data_block The reference to data, index of first Data
     * block
     * @param key_addition Additional key information need to locate key in
     * file
     */
    void
    insert(const Key& key,
      BlockIndex first_data_block,
      KeyAddition& key_addition)
      throw (eh::Exception);

    /**
     * Update reference to the data stored into key header
     * @param key The key to be updated
     * @param first_data_block The index of Data block with new data that
     * should be associated with key
     * @param key_addition Additional key information, need to locate key
     */
    void
    update(const Key& key,
      BlockIndex first_data_block,
      const KeyAddition& key_addition)
      throw (eh::Exception);

    /**
     * Find Data block use index of block from additional key information
     * Find Key in the Data block use offset from additional key information
     * Free space in the block if the key was the last in the block, mark key
     * as DELETED in his header for other cases
     * @param key The key to be deleted
     * @param key_addition Additional key information for fist parameter of method
     */
    void
    erase(const Key& key,
      const KeyAddition& key_addition)
      throw (eh::Exception);

    /**
     * Loop for each keys before container destroying
     * @return true will lead to call a method save with each Key element
     * false will lead to immediately call end_saving
     */
    bool
    begin_saving() throw (eh::Exception);

    /**
     * Do nothing
     */
    void
    save(const Key& key,
      BlockIndex first_data_block,
      const KeyAddition& key_addition)
      throw (eh::Exception);

    /**
     * Do nothing
     */
    void
    end_saving() throw (eh::Exception);

  protected:
    /**
     * Thread-safe!
     * 1. Calculate constant size of key header
     * 2. Get used data size and available space for writing
     * 3. If we able to save key in current Data block do it, else
     * allocate new Data block and insert in front of keys chain of Data blocks
     * 4. Save metadata, i.e. Key Header
     * 5. And finally, call KeyAccessor save to do write data of key
     * in the place prepared
     * @return true means new Data block with key was inserted into chain and
     * we should call sync_ to save the beginning block of Keys into Description
     * block. Also returns results in key_addition reference: index of saved Data
     * block with key and offset to data of key in this block
     */
    bool
    save_(
      WriteBlockFileAdapter::WriteBlockStruct_var& write_block,
      const Key& key,
      BlockIndex first_data_block,
      KeyAddition& key_addition)
      throw (eh::Exception, typename BaseType::FileFormatError);

    /**
     * Write index of the first Keys Block to Index Description Block.
     * Perform synchronizations between Keys and Description
     */
    void
    sync_() throw (eh::Exception);

    typedef Sync::PosixRWLock Mutex_;
    typedef Sync::PosixRGuard ReadGuard_;
    typedef Sync::PosixWGuard WriteGuard_;

    /// Used in multiple readers writers synchronization
    mutable Mutex_ lock_;

    WriteBlockFileAdapter* write_block_file_adapter_;
    BaseBlockAllocator* block_allocator_;
    /// Store Index description block, usually index=2
    WriteBlockFileAdapter::WriteBlockStruct_var descr_block_;
    /// Store first Data block with keys of Index
    WriteBlockFileAdapter::WriteBlockStruct_var first_keys_block_;
  };

  /**
   * Interface to allocate blocks
   * Separates the allocation of blocks of shared memory and manage it from
   * the client code
   */
  class BaseBlockAllocator
  {
  public:
    typedef SyncIndexStrategy::GenericField AllocatorIndex;
    typedef const SyncIndexStrategy::GenericField ConstAllocatorIndex;

    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    /// Raise when allocation of block is impossible
    DECLARE_EXCEPTION(AllocationFailed, Exception);
    DECLARE_EXCEPTION(DeallocationFailed, Exception);

    /**
     * Request for Data block. Memory will allocated on disk
     * @return index of Data block available for user
     */
    virtual
    BlockIndex
    allocate() throw (eh::Exception) = 0;

    /**
     * Free allocated early Data block
     * @param index The index of Data block to be free
     */
    virtual
    void
    deallocate(BlockIndex index) throw (eh::Exception) = 0;

    /**
     * Virtual empty destructor
     */
    virtual
    ~BaseBlockAllocator() throw ();
  };

  /**
   * DefaultBlockAllocator allocate several blocks if needed to minimize
   * the number of allocation of system resources. I.e. allocate more than
   * need now, but in the end allocation number is less
   */
  class DefaultBlockAllocator : public BaseBlockAllocator
  {
  public:
    /**
     * Constructor, do index of first free block equal 0
     * @param write_block_file_adapter The pointer to existing reader/writer
     * of blocks of file
     * @param first_description_block The index of first block with description
     * of index saved in file
     */
    DefaultBlockAllocator(
      WriteBlockFileAdapter* write_block_file_adapter,
      BlockIndex first_description_block)
      throw (eh::Exception);

    /**
     * Do sync (correct header)
     */
    virtual
    ~DefaultBlockAllocator() throw ();

    /**
     * Allocate with caching, just a few instead of one. Note: remember that
     * the memory allocated on the disk
     * @return The index of block available for user
     */
    virtual
    BlockIndex
    allocate() throw (eh::Exception, AllocationFailed);

    /**
     * deallocate one block
     * @param block_to_free Index of block to be free
     */
    virtual
    void
    deallocate(BlockIndex block_to_free)
      throw (eh::Exception, DeallocationFailed);

  protected:
    /**
     * Write index of the first free Block to Allocator Description Block.
     * Perform synchronizations between Available Blocks and Allocator
     * Description
     */
    void
    sync_() throw ();

    typedef Sync::PosixRWLock Mutex_;
    typedef Sync::PosixRGuard ReadGuard_;
    typedef Sync::PosixWGuard WriteGuard_;

    mutable Mutex_ lock_;

    WriteBlockFileAdapter* write_block_file_adapter_;
    BlockIndex first_free_block_;
    WriteBlockFileAdapter::WriteBlockStruct_var
      block_allocator_description_;
  };

  /**
   * DefaultMapTraits
   */
  template <typename Key, typename KeyAccessor>
  struct DefaultMapTraits
  {
    /// block allocation strategy
    typedef DefaultBlockAllocator BlockAllocator;

    /// key sync strategy
    typedef KeyAccessor IndexAccessor;
    typedef DefaultSyncIndexStrategy<Key, IndexAccessor>
      SyncIndexStrategy;
  };

  /**
   * Map class
   *
   * See:
   * https://confluence.ocslab.com/display/ADS/PlainStorage+library+file+format
   * to get information about file format and structure
   */
  template <typename Key,
            typename KeyAccessor = DefaultWriteIndexAccessor<Key>,
            typename MapTraits = DefaultMapTraits<Key, KeyAccessor> >
  class Map :
    protected MapTraits::SyncIndexStrategy::IndexLoadCallback
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(OutOfRange, Exception);
    /// Raise when fail to interpret data from file
    DECLARE_EXCEPTION(CorruptedFile, Exception);
    /// Raise when it is not able to create a file header
    DECLARE_EXCEPTION(CreationFailed, Exception);
    /// Raise when key loading failed
    DECLARE_EXCEPTION(LoadFailed, Exception);

    typedef typename MapTraits::SyncIndexStrategy SyncIndexStrategy;
    typedef typename MapTraits::BlockAllocator BlockAllocator;
    typedef typename SyncIndexStrategy::KeyAddition KeyAddition;
    /// associative container types
    typedef std::pair<const Key, PlainWriter_var> ValueType;
    typedef std::pair<const Key, const PlainWriter_var> ConstValueType;

  protected:
    typedef Map<Key, KeyAccessor, MapTraits> ThisType;
    typedef typename SyncIndexStrategy::FileHeader FileHeader;
    typedef typename SyncIndexStrategy::GenericField GenericField;
    typedef std::pair<PlainWriter_var, KeyAddition> ContainerValue;
    typedef std::map<Key, ContainerValue> IndexContainer;
  public:

    /**
     * Template to define references to ValueType - pair of Key and writer.
     * Using to get usable pair from node really stored in std::map
     */
    template <typename SecondType>
    struct NodeValueType
    {
      /**
       * Construct reference with specified Key and writer
       * @param key Key to be the first member of a pair
       * @param plain_writer will reference to Writer to be the second
       * member of a pair
       */
      NodeValueType(const Key& key, SecondType plain_writer) throw ();

      const Key& first;
      SecondType second;
    };
    typedef NodeValueType<PlainWriter_var&> ValueTypeRef;
    typedef NodeValueType<const PlainWriter_var&> ConstValueTypeRef;

    typedef ValueTypeRef reference;
    typedef ConstValueTypeRef const_reference;
    typedef unsigned long size_type;

  private:

    // iterators
    struct MapBaseIterator
    {
      /**
       * Default constructor
       */
      MapBaseIterator() throw ();

      /**
       * Constructor resolve reference on value if possible
       * @param it The iterator to IndexContainer to be store into
       * MapBaseIterator
       * @param container_ref Give access to container that give it iterator
       */
      MapBaseIterator(
        const typename IndexContainer::iterator& it,
        IndexContainer* container_ref) throw ();

    protected:
      void
      inc_() throw (OutOfRange);

      void
      dec_() throw (OutOfRange);

      /**
       * Assign iterator from the right side to this, set reference to
       * pointed value if it exists
       * @param right The iterator to be assigned to this
       */
      void
      set_(const MapBaseIterator& right) throw ();

      typename IndexContainer::iterator it_;
      /// Container pointer is need to check bounds and throw OutOfRange()
      IndexContainer* container_ref_;
    };

  public:
    /**
     * A type that provides a bidirectional iterator that can read
     * or modify any element in a Map
     */
    template <typename Reference>
    class BiDiIterator : public MapBaseIterator
    {
      /// Friendship need to get it_ from base for Map members, see erase..
      friend class Map<Key, KeyAccessor, MapTraits>;
      /**
       * Impede obtaining a pointer to a temporary object of Reference
       */
      struct ReturnedMediator : private Reference
      {
        ReturnedMediator(const Key& key, PlainWriter_var& plain_writer)
          throw ();

        Reference*
        operator ->() throw ();
      };

    public:
      typedef BiDiIterator<ValueTypeRef> iterator;
      typedef BiDiIterator<ConstValueTypeRef> const_iterator;

      /**
       * Default constructor calls base default constructor
       */
      BiDiIterator() throw ();

      /**
       * Constructor calls base constructor with parameters
       */
      BiDiIterator(const typename IndexContainer::iterator& it,
        IndexContainer& container) throw ();

      /**
       * copy constructor for iterator and constructor from iterator for
       * const_iterator
       */
      BiDiIterator(const iterator& it) throw ();

      /**
       * @return Returns the element that a BiDiIterator addresses
       */
      Reference
      operator *() const throw ();

      /**
       * @return Returns a special mediator object that return pointer to Reference.
       * This pointer used to get value of BiDiIterator
       */
      ReturnedMediator
      operator ->() const throw ();

      /**
       * Increments the BiDiIterator to the next element
       * @return Reference on the incremented BiDiIterator
       */
      BiDiIterator&
      operator ++() throw (OutOfRange);

      /**
       * Increments the BiDiIterator to the next element
       * @return Previous value (copy) of the non-incremented BiDiIterator
       */
      BiDiIterator
      operator ++(int) throw (OutOfRange);

      /**
       * Decrements the BiDiIterator to the previous element
       * @return Reference on the decremented BiDiIterator
       */
      BiDiIterator&
      operator --() throw (OutOfRange);

      /**
       * Decrements the BiDiIterator to the previous element
       * @return Previous value (copy) of the non-decremented BiDiIterator
       */
      BiDiIterator
      operator --(int) throw (OutOfRange);

      /**
       * Tests if the iterator on the left side of the operator is equal
       * to the iterator on the right side. Check equality of
       * IndexContaiter::iterator's
       * @param right The right part of equality
       * @return true if the iterator on the left side of the operator is
       * equal to the iterator on right side of the operator, otherwise false
       */
      bool
      operator ==(const BiDiIterator& right) const throw ();

      /**
       * Tests if the iterator on the left side of the operator is not equal
       * to the iterator on the right side. Check inequality of
       * IndexContaiter::iterator's
       * @param right The right part of equality
       * @return true if the iterators are not equal, false if iterators
       * are equal
       */
      bool
      operator !=(const BiDiIterator& right) const throw ();

      /**
       * Assign some iterator to this
       * @param it The iterator to be assigned to this
       * @return The reference on self
       */
      BiDiIterator&
      operator =(const iterator& it) throw ();
    };

    typedef BiDiIterator<reference> iterator;
    typedef BiDiIterator<const_reference> const_iterator;

    typedef std::pair<iterator, bool> Pairib_;

    /**
     * Map default constructor, nothing to do
     */
    Map() throw ();

    /**
     * Construct Map and load data from file
     * @param filename The name of file to load in Map
     * @param block_size The size of Data block that will
     * operate BlockFile adapter, cannot be equal zero!
     */
    Map(const char* filename, unsigned long block_size = 64*1024)
      throw (eh::Exception);

    /**
     * Destructor call close()
     */
    virtual
    ~Map() throw ();

    // associative container interface
    /**
     * Returns an iterator that addresses the first element in the Map
     * @return A bidirectional iterator addressing the first element
     * in the Map or the location succeeding an empty Map
     */
    iterator
    begin() throw ();

    /**
     * Returns an const iterator that addresses the first element in the Map
     * @return A const bidirectional iterator addressing the first element
     * in the Map or the location succeeding an empty Map
     */
    const_iterator
    begin() const throw ();

    /**
     * Returns an iterator that addresses the location succeeding the last
     * element in a Map
     * @return A bidirectional iterator that addresses the location succeeding
     * the last element in a Map. If the Map is empty, then
     * Map::end() == Map::begin()
     */
    iterator
    end() throw ();

    /**
     * Returns an const iterator that addresses the location succeeding the
     * last element in a Map
     * @return A const bidirectional iterator that addresses the location
     * succeeding the last element in a Map. If the Map is empty, then
     * Map::end() == Map::begin()
     */
    const_iterator
    end() const throw ();

    /**
     * Returns an iterator addressing the location of an element in a Map
     * that has a key equivalent to a specified key
     * @param key The key value to be matched
     * @return An iterator that addresses the location of an element with
     * the key, or the location succeeding the last element in the map
     * if no match is found for the key
     */
    iterator
    find(const Key& key) throw ();

    /**
     * Returns an const iterator addressing the location of an element
     * in a Map that has a key equivalent to a specified key
     * @param key The key value to be matched
     * @return An const iterator that addresses the location of an element
     * with the key, or the location succeeding the last element in the map
     * if no match is found for the key
     */
    const_iterator
    find(const Key& key) const throw ();

    /**
     * Removes an element in a Map that match a specified key.
     * Performs a find an element in a Map
     * @param key The key value of the element to be removed from the Map
     * @return The number of elements that have been removed from the Map
     */
    size_type
    erase(const Key& key) throw (eh::Exception);

    /**
     * Removes an element in a Map that referenced by a specified iterator.
     * Warning: call map.erase(map.end()) insecure, this is a serious error
     * and can destroy container
     * @param it The iterator of the element to be removed from the Map
     */
    void
    erase(iterator it) throw (eh::Exception);

    /**
     * 1. Find element with specified key. If found return its iterator
     * 2. If not found, do insert in file and in IndexContainer
     * @param key The key to be inserted in Index in file and in IndexContainer
     * @return The iterator to existing element or iterator to inserted element
     */
    iterator
    insert(const Key& key) throw (eh::Exception);

    /**
     * Perform find an element of a Map that matches value.first key value.
     * Update existing element or insert new
     * @param value The value of an element to be inserted into the map
     * unless the map already contains that element
     * @return a pair whose bool component returns true if an insertion was made
     * and false if the Map already contained an element matched value.first
     * key value, and whose iterator component locate a new element was
     * inserted or locate the element was already
     */
    Pairib_
    insert(const ValueType& value) throw (eh::Exception);

    /**
     * Not implemented
     */
    iterator
    insert(iterator position, const ValueType& value)
      throw (eh::Exception);

    /**
     * Get a PlainWriter from a map with a specified key value.
     * Inserts an element with specified key value, if the element
     * does not exist
     * @param key The key value of the element that is to be inserted or
     * accessed
     * @return The PlainWriter variable that able read/write data from/to a Map
     * element
     */
    PlainWriter_var
    operator [](const Key& key) throw (eh::Exception);

    /**
     * Erases all the elements of a Map
     */
    void
    clear() throw (eh::Exception);

    /**
     * Returns the number of elements in the Map
     * @return The current length of the Map
     */
    std::size_t
    size() const throw ();

    /**
     * Open file filename. This method is not thread-safe.
     * You should do not use parallel loading of files in a same object
     * from different threads
     * Load or create header of file with filename
     * Create Block Allocator
     * Create sync index strategy
     * Delegate further loading to sync index strategy
     */
    void
    load(
      const char* filename,
      unsigned long block_size = 64*1024) throw (eh::Exception);

    /**
     * If file have been opened and loaded in map, do following:
     * Try initialize save all unsaved data through
     * SyncStrategy::begin_saving,
     * if saving started successfully (we need saving), call for
     * each elements in Map SyncStrategy::save and finally call
     * SyncStrategy::end_saving. Furthermore, close opened file.
     * DefaultSyncIndexStrategy do not save anything on close call,
     * just close the file and free shared memory used to hold file
     */
    void
    close() throw (eh::Exception);

  protected:
    /**
     * Allocate Data block and create PlainWriter with it, size of data is zero
     * @param plain_writer The reference to return result (PlainWriter)
     */
    void
    init_value_(PlainWriter_var& plain_writer) throw (eh::Exception);

    /**
     * Do copy of source PlainWriter to plain_writer. Actually, call
     * the ReferenceCounting::add_ref for reference counting variables.
     * @param plain_writer The reference to store copied value
     * @param source_plain_writer The source PlainWriter to be copied
     */
    void
    copy_value_(
      PlainWriter_var& plain_writer,
      PlainWriter* source_plain_writer)
      throw (eh::Exception);

    /**
     * Load header from file
     */
    void
    load_head_(
      WriteBlockFileAdapter* write_block_adapter,
      BlockIndex& first_allocator_desc_block,
      BlockIndex& first_index_desc_block)
      throw (eh::Exception, CorruptedFile);

    /**
     * Header is located at the beginning of the file
     *  Header:
     *  [Header Block #0][Allocator Block #1][Index description #2]
     * 0: At the beginning of [Header Block 0] saved 2 number: 1 and 2 - index
     * of Allocator Block and index of Description Block
     * 2: [Index description] := [Index of first Block with Keys]
     * At the beginning of [Index description 2] store index (reference to)
     * of first block with Keys
     * @param write_block_adapter Access to blocked mapped file
     * @param first_allocator_desc_block Return allocator block index i.e 1
     * @param first_index_desc_block Return index of index description
     * Block i.e. 2
     */
    void
    create_head_(
      WriteBlockFileAdapter* write_block_adapter,
      BlockIndex& first_allocator_desc_block,
      BlockIndex& first_index_desc_block)
      throw (eh::Exception, CreationFailed);

  protected:
    /**
     * SyncIndexStrategy::IndexLoadCallback interface
     * 1. Calculate data size - do review thread of Data blocks
     * (the list of Data Blocks)
     * 2. Create PlainWriter able to load/write whole portion of data
     * with specified size
     * 3. Insert the created pair (key; writer) in the map
     * @param key Key of Data need to fast access Data
     * @param first_data_block The index of first Data block with beginning
     * of Data to be loaded
     * @param key_addition Additional part of key need to Data location
     */
    virtual
    void
    load_key(
      const Key& key,
      const BlockIndex& first_data_block,
      const typename SyncIndexStrategy::KeyAddition& key_addition)
      throw (eh::Exception);

    typedef std::unique_ptr<WriteBlockFileAdapter> WriteBlockFileAdapterPtr;
    typedef std::unique_ptr<BlockAllocator> BlockAllocatorPtr;
    typedef std::unique_ptr<SyncIndexStrategy> SyncIndexStrategyPtr;

    WriteBlockFileAdapterPtr write_block_file_adapter_;
    BlockAllocatorPtr block_allocator_;
    SyncIndexStrategyPtr sync_index_strategy_;

    IndexContainer index_container_;
  };
}

#include <PlainStorage/Map.tpp>
#include <PlainStorage/DefaultSyncIndexStrategy.tpp>

#endif // PLAINSTORAGE_MAP_HPP
