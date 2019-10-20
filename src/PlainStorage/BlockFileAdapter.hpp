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



// @file PlainStorage/BlockFileAdapter.hpp
#ifndef UNIXCOMMONS_PLAINSTORAGE_BLOCKFILEADAPTER_HPP
#define UNIXCOMMONS_PLAINSTORAGE_BLOCKFILEADAPTER_HPP

#include <inttypes.h>
#include <sys/types.h>

#include <ReferenceCounting/ReferenceCounting.hpp>

namespace PlainStorage
{
  typedef u_int32_t BlockIndex;

  /**
   * ReadBlockFileAdapter
   * File divided on some parts, precisely, class calculate size of parts
   * by next equation:
   * PageSize = ::getpagesize() * ceil(RequestedBlockSize / ::getpagesize())
   * Adapter allow read file by some portions in several stages
   */
  class ReadBlockFileAdapter
  {
    friend class ReadBlockStruct;
  public:
    typedef u_int64_t FileOffset;

    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(PosixException, Exception);
    DECLARE_EXCEPTION(FileOpenFailure, Exception);
    DECLARE_EXCEPTION(ResizeFailure, Exception);
    DECLARE_EXCEPTION(BadParam, Exception);

    /**
     * Class organize access to mapped (read in other words) part of file
     * (block). Size of block calculated by ReadBlockFileAdapter and it
     * perform reading.
     * At the beginning of the block of shared memory store two auxiliary
     * u_int32_t variables. First variable store index of next block,
     * second store size of this mapped block. The index is a simple
     * number of block, numbered consequently while allocate.
     * ReadBlock's can be created by ReadBlockFileAdapter
     *   Block:
     *   [4 bytes for next index][size of used data Block][..Data variable length..]
     * i.e.
     *   [NextIndex][UserDataSize][Other memory]
     *
     */
    class ReadBlockStruct :
      public virtual ReferenceCounting::DefaultImpl<>
    {
      friend class ReadBlockFileAdapter;

    public:
      /**
       * Exclude service fields from shared memory and return
       * pointer to user data
       * @return pointer to content of block
       */ 
      const void*
      read_content() const throw ();

      /**
       * Size of data may be not equal to mapped block size, i.e. block
       * with last portion of file
       * @return size of data in block
       */
      unsigned long
      size() const throw ();

      /**
       * @return index of current block
       */
      BlockIndex
      index() const throw ();

      /**
       * Index of next block stored into allocated shared memory
       * @return index of next block: 0 if next block non exist
       */
      BlockIndex
      next_index() const throw ();

      /**
       * Continue reading, load next part of file
       * @return resolved next block, 0 if next block non exist
       */
      ReadBlockStruct*
      read_next() throw (eh::Exception);

    protected:

      /**
       * Auxiliary class need to more comfortable work with block service
       * fields
       */
      class BlockHeader
      {
      public:
        typedef uint32_t FieldType;
        typedef const uint32_t ConstFieldType;
      private:
        enum PlainBlockHeader
        {
          /// Place to store index of next block in chain of blocks
          BH_NEXT_INDEX,
          /// The place to store size of used space in block by user
          BH_USED_SIZE,
          /// Block header size - the number of FieldType fields into Block header
          BH_NUMBER_FIELDS
        };
        /// suppose that data stored continuously in arrays
        typedef FieldType BlockHeaderBody[BH_NUMBER_FIELDS];
        BlockHeaderBody data_;
      public:
        /// sizeof of header (header service fields)
        static const std::size_t BLOCK_HEADER_SIZE =
          sizeof(BlockHeaderBody);

        FieldType
        next_index() const throw ();

        FieldType&
        next_index() throw ();

        FieldType
        size() const throw ();

        FieldType&
        size() throw ();

        void*
        content() throw ();

        const void*
        content() const throw ();
      };

      /**
       * Create shared part of file struct and optionally (by default) load
       * data to it
       * @param block_file_adapter Adapter perform loading data and allocate
       * shared memory
       * @param block_index Index to be store in create block. This index of
       * creating block
       * @param resolve_content true allow allocate shared memory and load
       * part of file, false - nothing to be do
       */
      ReadBlockStruct(
        ReadBlockFileAdapter* block_file_adapter,
        BlockIndex block_index,
        bool resolve_content = true)
        throw (eh::Exception);

      /**
       * Deletes shared memory if it was allocated
       */
      virtual
      ~ReadBlockStruct() throw ();

      /// Variable to delete shared memory when object will be destroy
      ReadBlockFileAdapter* read_block_file_adapter_;
      /// Index of this block
      BlockIndex block_index_;
      /// pointer to shared memory with mapped file
//      void* content_;
      BlockHeader* content_;
    };
    typedef ReferenceCounting::SmartPtr<ReadBlockStruct> ReadBlockStruct_var;

    /**
     * Constructor
     * @param filename The name of file to be open
     * @param block_size The size of Data block
     */
    ReadBlockFileAdapter(
      const char* filename,
      unsigned long block_size)
      throw (eh::Exception);

    /**
     * Close file if it has been opened
     */
    virtual
    ~ReadBlockFileAdapter() throw ();

    /**
     * make function for ReadBlockStructs
     * @param block_index The index of the Data block to be put in memory
     * @return The pointer to accessor to mapped Data block
     */
    ReadBlockStruct*
    get_block(BlockIndex block_index) throw (eh::Exception);

    /**
     * @return Shared memory block size - size of reserved fields
     */
    unsigned long
    block_data_size() const throw (eh::Exception);

    /**
     * @return file size divided on size of elemental portion of shared memory
     * used to store file. I.e. maximum number of blocks need to hold file in
     * memory
     */
    BlockIndex
    max_block_index() const throw (eh::Exception);

  protected:

    /**
     * This constructor do not open file
     * @param block_size The value for Data block size
     */
    ReadBlockFileAdapter(unsigned long block_size) throw (eh::Exception);

    void*
    read_resolve_block_(BlockIndex index)
      throw (PosixException, eh::Exception);

    /**
     * deletes the mappings for the specified address pointer
     */
    void
    read_unresolve_block_(void* content)
      throw (PosixException, eh::Exception);

    /**
     * Open file in read only mode
     */
    void
    open_file_(const char* filename) throw (PosixException, eh::Exception);

    /// Description of opened file
    int file_desc_;
    /// The size of shared memory portion. Data block hold into some portions
    static const std::size_t SYSTEM_PAGE_SIZE_;
    /// The size of shared memory used to store one Data block
    std::size_t map_page_size_;
    /// Requested Data block size
    std::size_t block_size_;
    /// Total size, in bytes of opened file
    FileOffset file_size_;
  };

  /**
   * WriteBlockFileAdapter like ReadBlockFileAdapter, but allow read/write
   * portions of files in several stages
   */
  class WriteBlockFileAdapter : public ReadBlockFileAdapter
  {
  public:
    class WriteBlockStruct :
      public ReadBlockFileAdapter::ReadBlockStruct
    {
      friend class WriteBlockFileAdapter;

    public:
      /**
       * Exclude service fields from shared memory and return
       * pointer to user data
       * @return pointer to content of block
       */ 
      void*
      content() const throw ();

      /**
       * @return size of user data in block
       */
      using ReadBlockStruct::size;

      /**
       * set size of data in block, size must be < block size
       * @param sz Size of used data that will store in the block
       */
      void
      size(unsigned long sz) throw ();

      /**
       * @return The size of memory in Data block available for user.
       * The size of the block minus the size of the service fields
       */
      unsigned long
      available_size() const throw (eh::Exception);

      /**
       * Index of next block stored into allocated shared memory
       * @return index of next block: 0 if next block non exist
       */
      using ReadBlockStruct::next_index;

      /**
       * Save index of next block in the beginning of shared memory
       * @param next_block Number of index to be saved
       */
      void
      next_index(BlockIndex next_block) throw ();

      /**
       * @return resolved next block, 0 if next block non exist
       */
      WriteBlockStruct*
      next() throw (eh::Exception);

    protected:
      WriteBlockStruct(
        WriteBlockFileAdapter* block_file_adapter,
        BlockIndex block_index)
        throw (eh::Exception);

      /**
       * Deletes shared memory if it was allocated
       */
      virtual
      ~WriteBlockStruct() throw ();

      WriteBlockFileAdapter* write_block_file_adapter_;
    };
    typedef ReferenceCounting::SmartPtr<WriteBlockStruct>
      WriteBlockStruct_var;

    /**
     * Possible modes of opening files, all files open for read-write
     */
    enum OpenType
    {
      OT_OPEN,
      OT_OPEN_OR_CREATE
    };

    /**
     * Constructor open file in read/write mode
     * @param filename The name of file to be open
     * @param block_size The size for Data block
     * @param open_type Traits for opening file, by default if the file does not
     * exist it will be created
     */
    WriteBlockFileAdapter(
      const char* filename,
      unsigned long block_size,
      OpenType open_type = OT_OPEN_OR_CREATE)
      throw (eh::Exception);

    WriteBlockStruct*
    get_block(BlockIndex block_index) throw (eh::Exception);

    ReadBlockStruct*
    get_read_block(BlockIndex block_index) throw (eh::Exception);

    /**
     * Empty virtual destructor
     */
    virtual
    ~WriteBlockFileAdapter() throw ();

  protected:
    /**
     * Load the part of opened file into shared memory references by index.
     * Do resize of the file if requested to the block outside the file
     * @param index The number of Data block to be located in memory
     * @param need_to_init true mean file was enlarge and we should
     *   initialize data
     * @return Pointer to shared memory with Data block by index
     */
    void*
    write_resolve_block_(
      BlockIndex index, bool& need_to_init)
      throw (PosixException, eh::Exception);

    /**
     * Deallocate shared memory by pointer. All allocated shared memory
     * blocks have equal size = map_page_size_, and we able to free memory
     * by pointer
     * @param content Pointer to shared memory to do unmap
     */
    void
    write_unresolve_block_(void* content)
      throw (PosixException, eh::Exception);

    /**
     * Open file in read-write mode and with given type
     */
    void
    open_file_(const char* filename, OpenType open_type)
      throw (BadParam, PosixException, eh::Exception);

    /**
     * @return number of shared memory blocks need to hold file
     */
    BlockIndex
    size_file_() const throw ();

    /**
     * Use to extend file while allocate shared memory for writing data.
     * @param new_size_in_blocks Usually, currently allocated block index + 1, to
     * resize file to current size + block size.
     */
    void
    resize_file_(BlockIndex new_size_in_blocks)
      throw (FileOpenFailure, PosixException, eh::Exception);
  };

} // namespace PlainStorage

//////////////////////////////////////////////////////////////////////////
// Inlines implementations

namespace PlainStorage
{
  //
  // ReadBlockFileAdapter::ReadBlockStruct
  //

  inline
  const void*
  ReadBlockFileAdapter::ReadBlockStruct::read_content() const
    throw ()
  {
    return content_->content();
  }

  inline
  unsigned long
  ReadBlockFileAdapter::ReadBlockStruct::size() const
    throw ()
  {
    return content_->size();
  }

  inline
  BlockIndex
  ReadBlockFileAdapter::ReadBlockStruct::index() const
    throw ()
  {
    return block_index_;
  }

  inline
  BlockIndex
  ReadBlockFileAdapter::ReadBlockStruct::next_index() const
    throw ()
  {
    return content_->next_index();
  }

  //
  // WriteBlockFileAdapter class
  //

  inline
  void*
  WriteBlockFileAdapter::WriteBlockStruct::content() const
    throw ()
  {
    return content_->content();
  }

  inline
  void
  WriteBlockFileAdapter::WriteBlockStruct::size(
    unsigned long new_size)
    throw ()
  {
    content_->size() = new_size;
  }

  inline
  unsigned long
  WriteBlockFileAdapter::WriteBlockStruct::available_size() const
    throw (eh::Exception)
  {
    return write_block_file_adapter_->block_data_size();
  }

  inline
  void
  WriteBlockFileAdapter::WriteBlockStruct::next_index(
    BlockIndex new_next_index)
    throw ()
  {
    content_->next_index() = new_next_index;
  }

  inline
  ReadBlockFileAdapter::ReadBlockFileAdapter(
    const char* file_name,
    unsigned long block_size)
    throw (eh::Exception)
    : file_desc_(-1),
      map_page_size_(0),
      block_size_(block_size),
      file_size_(0)
  {
    open_file_(file_name);
  }

  inline
  WriteBlockFileAdapter::WriteBlockStruct*
  WriteBlockFileAdapter::get_block(BlockIndex block_index)
    throw (eh::Exception)
  {
    return new WriteBlockStruct(this, block_index);
  }

  inline
  WriteBlockFileAdapter::ReadBlockStruct*
  WriteBlockFileAdapter::get_read_block(BlockIndex block_index)
    throw (eh::Exception)
  {
    return ReadBlockFileAdapter::get_block(block_index);
  }

  inline
  BlockIndex
  WriteBlockFileAdapter::size_file_() const
    throw ()
  {
    return file_size_ / map_page_size_;
  }

  inline
  ReadBlockFileAdapter::ReadBlockStruct::BlockHeader::FieldType
  ReadBlockFileAdapter::ReadBlockStruct::BlockHeader::next_index() const
    throw ()
  {
    return data_[BH_NEXT_INDEX];
  }

  inline
  ReadBlockFileAdapter::ReadBlockStruct::BlockHeader::FieldType&
  ReadBlockFileAdapter::ReadBlockStruct::BlockHeader::next_index() throw ()
  {
    return data_[BH_NEXT_INDEX];
  }

  inline
  ReadBlockFileAdapter::ReadBlockStruct::BlockHeader::FieldType
  ReadBlockFileAdapter::ReadBlockStruct::BlockHeader::size() const throw ()
  {
    return data_[BH_USED_SIZE];
  }

  inline
  ReadBlockFileAdapter::ReadBlockStruct::BlockHeader::FieldType&
  ReadBlockFileAdapter::ReadBlockStruct::BlockHeader::size() throw ()
  {
    return data_[BH_USED_SIZE];
  }

  inline
  void*
  ReadBlockFileAdapter::ReadBlockStruct::BlockHeader::content() throw ()
  {
    return &data_[BH_NUMBER_FIELDS];
  }

  inline
  const void*
  ReadBlockFileAdapter::ReadBlockStruct::BlockHeader::content() const throw ()
  {
    return &data_[BH_NUMBER_FIELDS];
  }

  inline
  ReadBlockFileAdapter::ReadBlockFileAdapter(
    unsigned long block_size)
    throw (eh::Exception)
    : block_size_(block_size)
  {
  }

  inline
  ReadBlockFileAdapter::ReadBlockStruct*
  ReadBlockFileAdapter::get_block(BlockIndex block_index)
    throw (eh::Exception)
  {
    return new ReadBlockStruct(this, block_index);
  }

  inline
  unsigned long
  ReadBlockFileAdapter::block_data_size() const
    throw (eh::Exception)
  {
    return block_size_ - ReadBlockStruct::BlockHeader::BLOCK_HEADER_SIZE;
  }

  inline
  BlockIndex
  ReadBlockFileAdapter::max_block_index() const
    throw (eh::Exception)
  {
    return file_size_ / map_page_size_;
  }

  //
  // WriteBlockFileAdapter
  //

  inline
  WriteBlockFileAdapter::WriteBlockFileAdapter(
    const char* filename,
    unsigned long block_size,
    OpenType open_type)
    throw (eh::Exception)
    : ReadBlockFileAdapter(block_size)
  {
    open_file_(filename, open_type);
  }

  inline
  WriteBlockFileAdapter::~WriteBlockFileAdapter()
    throw ()
  {
  }

}

#endif // UNIXCOMMONS_PLAINSTORAGE_BLOCKFILEADAPTER_HPP
