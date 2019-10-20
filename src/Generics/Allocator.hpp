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



// Allocator.hpp

#ifndef GENERICS_ALLOCATOR_HPP
#define GENERICS_ALLOCATOR_HPP

#include <iostream>
#include <list>

#include <signal.h>


#include <ReferenceCounting/ReferenceCounting.hpp>

#include <Generics/ArrayAutoPtr.hpp>
#include <Generics/Function.hpp>
#include <Generics/TAlloc.hpp>

#include <Stream/MemoryStream.hpp>


namespace Generics
{
  namespace Allocator
  {
    class Base;
    typedef ReferenceCounting::QualPtr<Base> Base_var;
    typedef ReferenceCounting::FixedPtr<Base> FixedBase_var;
    typedef ReferenceCounting::SmartPtr<Base> SmartBase_var;

    /**
     * Base class for all allocators objects, that realize some
     * allocation strategy.
     */
    struct Base : public virtual ReferenceCounting::Interface
    {
      /**
       * Exception means free memory exhausted.
       */
      DECLARE_EXCEPTION(OutOfMemory, eh::DescriptiveException);

      typedef void* Pointer;
      typedef const void* ConstPointer;

      /**
       * @param size mean request for size bytes for code needs,
       * but allocator can give >= size value bytes.
       * Really allocated value return to client by set size
       * in this case.
       * @return pointer to begin allocated memory, size available
       * for using returns in size.
       */
      virtual
      Pointer
      allocate(size_t& size) throw (eh::Exception, OutOfMemory) = 0;

      /**
       * All size T objects in the area pointed
       * by
       * @param ptr shall be destroyed prior to
       * this call.
       * @param size shall match the
       * value passed to allocate to
       * obtain this memory. Does not
       * throw exceptions. [Note: p shall not be null.]
       */
      virtual
      void
      deallocate(Pointer ptr, size_t size) throw () = 0;

      /**
       * Approximated cached memory size.
       * @return cached memory size.
       */
      virtual
      size_t
      cached() const throw (eh::Exception);

      /**
       * Print detailed approximate cached memory information.
       */
      virtual
      void
      print_cached(std::ostream& ostr) const throw (eh::Exception);

      /**
       * @return application level default allocator. Usually
       * simple new/delete behavior.
       */
      static
      Base*
      get_default_allocator() throw (eh::Exception);

    protected:
      /**
       * protected destructor because reference counting.
       */
      virtual
      ~Base() throw () = 0;
      
      /**
       * Align number to 2^mask number
       * @param number is number to align
       * @param mask power of 2 to be aligned.
       */ 
      static
      void
      align_(size_t& number, size_t mask) throw ();
    
    private:
      /// Application level default allocator object.
      static Sync::PosixMutex default_allocator_creation_mutex_;
      static volatile sig_atomic_t default_allocator_initialized_;
      static Base* volatile default_allocator_;
    };

    class Default :
      public Base,
      public ReferenceCounting::AtomicImpl
    {
     public:
      /// default power of 2 for alignment value.
      static const size_t DEF_ALIGN = 10;

      explicit
      Default(size_t align_code = DEF_ALIGN) throw ();

      /**
       * Align request size bytes according to MASK_
       * and allocate memory.
       * @param size at minimum memory to be allocated.
       * @return pointer to allocated memory block
       */
      virtual
      Pointer
      allocate(size_t& size) throw (eh::Exception, OutOfMemory);

      /**
       * Deallocate 
       * @param ptr pointer to releasing memory block.
       * @param size not used in this allocator.
       */
      virtual
      void
      deallocate(Pointer ptr, size_t size) throw ();

    protected:
      /**
       * Destructor
       */
      virtual
      ~Default() throw ();

    private:
      const size_t MASK_;
    };


    /**
     * Allocator realize strategy: give memory
     * from pool, if contain suitable memory block, or allocate.
     * When deallocating save memory block in pool.
     * Memory blocks stored in std::list
     */
    class VarSizeList :
      public Base,
      public ReferenceCounting::AtomicImpl
    {
    public:
      /**
       * @param align_code 2^align_code will be used for alignment of memory
       * blocks
       * @param blocks_count number of memory blocks
       */
      explicit
      VarSizeList(size_t align_code, size_t blocks_count)
        throw (eh::Exception);

      virtual
      Pointer
      allocate(size_t& size) throw (eh::Exception, OutOfMemory);

      virtual
      void
      deallocate(Pointer ptr, size_t size) throw ();

      virtual
      size_t
      cached() const throw (eh::Exception);

      virtual
      void
      print_cached(std::ostream& ostr) const throw (eh::Exception);

    protected:
      virtual
      ~VarSizeList() throw ();

    private:
      typedef std::pair<size_t, Pointer> MemoryBlock;
      typedef std::list<MemoryBlock,
        Generics::TAlloc::Aggregated<MemoryBlock, 256>> MemBlocks;

      /**
       * @param mb this memory block will be return to 
       * operation system
       */
      static
      void
      memory_block_delete_(MemoryBlock& mb) throw ();

      /// MASK_ + 1 is multiple power of 2
      const size_t MASK_;

      mutable Sync::PosixMutex mutex_;
      MemBlocks pool_memory_blocks_;
      size_t blocks_limit_;
      size_t cached_;
    };

    /**
     * Pool optimized to store
     * blocks with equal size.. Set of memory blocks with equal size.
     */
    class ConstSizeArray :
      public Base,
      public ReferenceCounting::AtomicImpl
    {
    public:
      /**
       * @param max_blocks_count get count of blocks that object
       * store lifetime.
       * @param block_size size of each memory block in bytes.
       */
      ConstSizeArray(size_t max_blocks_count,
        size_t block_size) throw (eh::Exception);
      
      virtual
      Pointer
      allocate(size_t& size) throw (eh::Exception, OutOfMemory);

      virtual
      void
      deallocate(Pointer ptr, size_t size) throw ();

      virtual
      size_t
      cached() const throw (eh::Exception);

      virtual
      void
      print_cached(std::ostream& ostr) const throw (eh::Exception);

    protected:
      virtual
      ~ConstSizeArray() throw ();

    private:
      const size_t MAX_BLOCKS_COUNT_;
      const size_t BLOCK_SIZE_;
 
      mutable Sync::PosixSpinLock mutex_;
      ArrayAutoPtr<ArrayByte> pool_memory_blocks_;
      size_t blocks_in_pool_;
      uint64_t hits_, misses_;
    };


    class Universal :
      public Base,
      public ReferenceCounting::AtomicImpl
    {
    public:
      static const size_t DEF_DEFAULT_THRESHOLD_LOW = 32 * 1024;
      static const size_t DEF_DEFAULT_THRESHOLD_HIGH = 2 * 1024 * 1024;
      static const size_t DEF_FIRST_BUCKET = 64 * 1024;
      static const size_t DEF_STEP_BETWEEN_BUCKET = 64 * 1024;
      static const size_t DEF_BUCKET_NUMBER = 15;
      static const size_t DEF_BUCKET_BLOCKS_COUNT = 100;
      static const size_t DEF_UNLIMITED_MASK = 16;
      static const size_t DEF_UNLIMITED_VOLUME = 10;

      static const size_t DEF_STATISTICS_PRECISION = 14;
      static const size_t DEF_STATISTICS_LIMIT = 128;

      /**
       * [0, default_threshold_low] and [default_threshold_high, inf)
       * are allocated by default allocator
       * (new/delete + alignment) 
       * (default_threshold_low .. first_bucket] - ConstSizeArray bucket 0
       * (first_bucket + (i - 1) * step_between_bucket ..
       * first_bucket + i * step_between_bucket] where i is in
       * [1 .. buckets_number - 1] - ConstSizeArray bucket i
       * (first_bucket + (buckets_numbet - 1) * step_between_bucket ..
       * default_threshold_high) (if buckets_number if not zero) or
       * (default_threshold_low .. default_threshold_high) (if
       * buckets_number is zero) - VarList
       * @param default_threshold_low define default memory ranges
       * @param default_threshold_high define default memory ranges
       * @param step_between_bucket step in bytes between BlockAllocators
       * @param first_bucket value in bytes for first block allocator in
       * sequence of BlockAllocators. 
       * @param buckets_number number of BlockAllocator that will be used.
       * @param bucket_blocks_count parameter define how many memory blocks
       * will hold each BlockAllocator.
       * @param unlimited_align power of two alignment for VarList allocator
       * @param unlimited_volume number of memory blocks that will be stored
       * in pool of last allocator
       * @param statistics_precision power of two statistics block size
       * @param statistics_limit number of statistics blocks
       */
      explicit
      Universal(
        size_t default_threshold_low = DEF_DEFAULT_THRESHOLD_LOW,
        size_t default_threshold_high = DEF_DEFAULT_THRESHOLD_HIGH,
        size_t step_between_bucket = DEF_STEP_BETWEEN_BUCKET,
        size_t first_bucket = DEF_FIRST_BUCKET,
        size_t buckets_number = DEF_BUCKET_NUMBER,
        size_t bucket_blocks_count = DEF_BUCKET_BLOCKS_COUNT,
        size_t unlimited_align = DEF_UNLIMITED_MASK,
        size_t unlimited_volume = DEF_UNLIMITED_VOLUME,
        size_t statistics_precision = DEF_STATISTICS_PRECISION,
        size_t statistics_limit = DEF_STATISTICS_LIMIT)
        throw (eh::Exception);

      virtual
      Pointer
      allocate(size_t& size) throw (eh::Exception, OutOfMemory);

      virtual
      void
      deallocate(Pointer ptr, size_t size) throw ();

      virtual
      size_t
      cached() const throw (eh::Exception);

      virtual
      void
      print_cached(std::ostream& ostr) const throw (eh::Exception);

    protected:
      virtual
      ~Universal() throw ();

    private:
      const size_t DEFAULT_THRESHOLD_LOW_;
      const size_t DEFAULT_THRESHOLD_HIGH_;
      const size_t FIRST_BUCKET_;
      const size_t STEP_BETWEEN_BUCKETS_;
      const size_t BUCKETS_NUMBER_;

      /// place for storing default allocator - usually
      /// used for small blocks management.
      Base_var default_allocator_;
      ArrayAutoPtr<Base_var> blocks_pools_;  /*ConstSizeArray*/
      Base_var unlimited_; /*VarSizeList*/

      const size_t STATISTICS_PRECISION_;
      const size_t STATISTICS_LIMIT_;

      ArrayAutoPtr<volatile _Atomic_word> statistics_;

      /**
       * @param size in bytes
       * @return allocator suitable for manage size memory block.
       */
      Base_var
      get_allocator_(size_t size) throw ();
    };

    /**
     * Returns aligned pointer
     */
    class Align :
      public Base,
      public ReferenceCounting::AtomicImpl
    {
     public:
      /// default power of 2 for alignment value.
      static const size_t DEF_PTR_ALIGN = 9;
      static const size_t DEF_ALIGN = 10;

      explicit
      Align(size_t ptr_align_code = DEF_PTR_ALIGN,
        size_t align_code = DEF_ALIGN) throw ();

      /**
       * Align request size bytes according to MASK_
       * and allocate memory aligned according to ALIGN_.
       * @param size at minimum memory to be allocated.
       * @return pointer to allocated memory block
       */
      virtual
      Pointer
      allocate(size_t& size) throw (eh::Exception, OutOfMemory);

      /**
       * Deallocate 
       * @param ptr pointer to releasing memory block.
       * @param size not used in this allocator.
       */
      virtual
      void
      deallocate(Pointer ptr, size_t size) throw ();

    protected:
      /**
       * Destructor
       */
      virtual
      ~Align() throw () = default;

    private:
      const size_t ALIGN_;
      const size_t MASK_;
    };

    template <typename Allocator>
    class Template :
      public Base,
      public ReferenceCounting::AtomicImpl
    {
    public:
      virtual
      Pointer
      allocate(size_t& size) throw (eh::Exception, OutOfMemory);

      virtual
      void
      deallocate(Pointer ptr, size_t size) throw ();

      static
      Base*
      allocator(const Allocator& allocator = Allocator())
        throw (eh::Exception);

    protected:
      virtual
      void
      delete_this_() const throw ();

    private:
      explicit
      Template(Allocator& allocator) throw (eh::Exception);

      virtual
      ~Template() throw ();

    private:
      Allocator allocator_;
    };
  } //namespace Allocator
} //namespace Generics

namespace Generics
{
  namespace Allocator
  {
    template <typename Allocator>
    Template<Allocator>::Template(Allocator& allocator)
      throw (eh::Exception)
      : allocator_(allocator)
    {
    }

    template <typename Allocator>
    Template<Allocator>::~Template() throw ()
    {
    }

    template <typename Allocator>
    typename Template<Allocator>::Pointer
    Template<Allocator>::allocate(size_t& size)
      throw (eh::Exception, OutOfMemory)
    {
      try
      {
        void* ptr = allocator_.allocate(size);
        if (ptr)
        {
          return ptr;
        }
      }
      catch (const eh::Exception& ex)
      {
        Stream::Error ostr;
        ostr << FNS << "Failed to allocate " << size << " bytes: " <<
          ex.what();
        throw OutOfMemory(ostr);
      }

      Stream::Error ostr;
      ostr << FNS << "Failed to allocate " << size << " bytes";
      throw OutOfMemory(ostr);
    }

    template <typename Allocator>
    void
    Template<Allocator>::deallocate(
      Pointer ptr, size_t size) throw ()
    {
      allocator_.deallocate(
        static_cast<typename Allocator::pointer>(ptr), size);
    }

    template <typename Allocator>
    void
    Template<Allocator>::delete_this_() const throw ()
    {
      Allocator allocator(allocator_);
      this->~Template();
      allocator.deallocate(
        reinterpret_cast<typename Allocator::pointer>(
          const_cast<Template*>(this)), sizeof(Template));
    }

    template <typename Allocator>
    Base*
    Template<Allocator>::allocator(const Allocator& allocator)
      throw (eh::Exception)
    {
      static_assert(sizeof(typename Allocator::value_type) == 1,
        "Allocator::value_type should have size 1");
      Allocator alloc(allocator);
      return new (alloc.allocate(sizeof(Template))) Template(alloc);
    }
  }
}

#endif
