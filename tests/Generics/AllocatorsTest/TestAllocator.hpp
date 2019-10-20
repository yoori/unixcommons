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

#ifndef _TEST_ALLOCATOR_HPP_INCLUDED_
#define _TEST_ALLOCATOR_HPP_INCLUDED_

#include <stdint.h>
#include <map>
#include <list>
#include <Generics/Allocator.hpp>

#include <eh/Exception.hpp>
#include <Sync/PosixLock.hpp>
#include <ReferenceCounting/ReferenceCounting.hpp>

//#define BUILD_WITH_DEBUG_MESSAGES
#include <Generics/Trace.hpp>
#include <cassert>

namespace Generics
{
  /**
   * Possible strategies of allocation:
   * Simple
   * SimpleReserve
   * SimpleAligned
   * SimpleAlignedReserve
   * PoolListSimple
   * PoolListSimpleReserve
   * PoolListAligned
   * PoolListAlignedReserve
   * PoolMultiMapSimple
   * PoolMultiMapSimpleReserve
   * PoolMultiMapAligned
   * PoolMultiMapAlignedReserve
   * in all: 12 item
   *
   */

  namespace Allocator
  {


    /**
     * Base class for all allocators objects, that realize some
     * allocation strategy.
     */
    struct BaseAllocator : 
      public Base
    {
      /**
       * Exception means free memory exhausted.
       */
      DECLARE_EXCEPTION(MemoryOut, eh::DescriptiveException);

      typedef void* Pointer;
      typedef const void* ConstPointer;

      BaseAllocator() throw ();

      /**
       * @param n mean request for n bytes for code needs,
       * but allocator can give >= n value bytes.
       * Really allocated value return to client by set n
       * in this case.
       * @return pointer to begin allocated memory, size available
       * for using returns into n.
       */
      virtual Pointer
      allocate(std::size_t& n) throw (MemoryOut) = 0;

      /**
       * All n T objects in the area pointed
       * by
       * @param ptr shall be destroyed prior to
       * this call. n shall match the
       * value passed to allocate to
       * obtain this memory. Does not
       * throw exceptions. [Note: p shall not be null.]
       */
      virtual void
      deallocate(Pointer ptr, std::size_t) throw () = 0;

      /**
       * Get and collect client allocation queries
       * @param get if true method simply return statistics value
       * without increment, if false increment internal statistics
       * value and return it.
       */
      std::size_t 
      stat_allocate(bool get = false) throw ();

      /**
       * Get and collect client deallocation queries
       * @param get if true method simply return statistics value
       * without increment, if false increment internal statistics
       * value and return it.
       */
      std::size_t 
      stat_deallocate(bool get = false) throw ();

      /**
       * Get and collect amount of system resources queries by
       * allocator.
       * @param get if true method simply return statistics value
       * without increment, if false increment internal statistics
       * value and return it.
       */
      std::size_t 
      stat_sys_allocate(bool get = false) throw ();

      /**
       * Get and collect amount of system resources releases by
       * allocator.
       * @param get if true method simply return statistics value
       * without increment, if false increment internal statistics
       * value and return it.
       */
      std::size_t 
      stat_sys_deallocate(bool get = false) throw ();

      /**
       * Need this method, because system resources are releasing
       * by pools while destroying pools.
       */
      virtual
      void
      calc_sys_deallocate() throw () = 0;

      /**
       * Reset statistics values to zeros
       */
      void
      stat_reset() throw ();

    protected:
      typedef std::pair<std::size_t, Pointer> MemoryBlock;

      virtual
      ~BaseAllocator() throw () = 0;

      Pointer
      alloc_size_block_(std::size_t n) throw (MemoryOut);

      struct SizeBlockEraser
      {
        void
        operator() (const MemoryBlock& ptr) const throw ();
      };

      const SizeBlockEraser&
      get_eraser_() throw ();

    private:

      SizeBlockEraser eraser_;

      volatile _Atomic_word stat_allocate_;
      volatile _Atomic_word stat_deallocate_;
      volatile _Atomic_word stat_sys_allocate_;
      volatile _Atomic_word stat_sys_deallocate_;
    };

    typedef ReferenceCounting::QualPtr<BaseAllocator> BaseAllocator_var;

    struct DefaultModifier
    {
      static void
      modify(std::size_t& n, std::size_t align,
        bool pool = false)
        throw ();
    };

    struct Reserve
    {
      static void
      modify(std::size_t& n, std::size_t align,
        bool pool = false)
        throw ();
    };

    struct Aligned
    {
      static void
      modify(std::size_t& n, std::size_t align,
        bool pool = false)
        throw ();
    };

    struct AlignedReserve
    {
      static void
      modify(std::size_t& n, std::size_t align,
        bool pool = false)
        throw ();
    };

    class DefaultAllocator : public BaseAllocator,
      public ReferenceCounting::AtomicImpl
    {
    public:

      virtual
      Pointer
      allocate(std::size_t& n) throw (MemoryOut)
      {
        return new unsigned char[n];
      }

      virtual
      void
      deallocate(Pointer ptr, std::size_t) throw ()
      {
        delete[] static_cast<unsigned char*>(ptr);
      }

      virtual
      void
      calc_sys_deallocate() throw ();
    };


    /**
     *
     */

    template<typename SizeModificator, size_t align = 0>
    class FakeAllocator : public BaseAllocator,
      public ReferenceCounting::AtomicImpl
    {
      static const std::size_t BUFFER_SIZE = 1024 * 2048 + 65535;
    public:
      FakeAllocator() throw (eh::Exception);

      virtual
      Pointer
      allocate(std::size_t& n) throw (MemoryOut);

      virtual
      void
      deallocate(Pointer ptr, std::size_t) throw ();

      virtual
      void
      calc_sys_deallocate() throw ();

    private:
      ArrayByte ptr_;
    };

    /**
     * Simple Allocator for raw memory allocation. (new-delete)
     */
    template<typename SizeModificator, size_t align = 0>
    class Simple : public BaseAllocator,
      public ReferenceCounting::AtomicImpl
    {
    public:

      virtual
      Pointer
      allocate(std::size_t& n) throw (MemoryOut);

      virtual
      void
      deallocate(Pointer ptr, std::size_t) throw ();

      virtual
      void
      calc_sys_deallocate() throw ();
    };

    /**
     * Allocator realize vector strategy: give memory
     * from pool, if contain suitable memory block, or allocate.
     * When deallocating save memory block into pool.
     * Memory blocks stored into std::list
     */
    template<typename SizeModificator, size_t align = 0>
    class PoolListSimple : public BaseAllocator,
      public ReferenceCounting::AtomicImpl
    {
    public:
      PoolListSimple() throw ();

      Pointer
      allocate(std::size_t& n) throw (MemoryOut);

      void
      deallocate(Pointer ptr, std::size_t) throw ();

      virtual
      void
      calc_sys_deallocate() throw ();

      virtual
      ~PoolListSimple() throw ();

    private:
      typedef std::list<MemoryBlock> MemBlocks;
      MemBlocks pool_memory_blocks_;
      MemBlocks used_memory_blocks_;
      // Store last block iterator, for first circled deletion from list..
      std::size_t blocks_in_pool_;
      Sync::PosixMutex mutex_;
    };

    /**
     * Pool like PoolListSimple but, optimized to store
     * blocks with equal size..
     */

    class BlocksPool : public BaseAllocator,
      public ReferenceCounting::AtomicImpl
    {
    public:
      BlocksPool() throw ();

      Pointer
      allocate(std::size_t& n) throw (MemoryOut);

      void
      deallocate(Pointer ptr, std::size_t) throw ();

      virtual
      void
      calc_sys_deallocate() throw ();

      virtual
      ~BlocksPool() throw ();

    private:
      typedef std::list<MemoryBlock> MemBlocks;
      MemBlocks pool_memory_blocks_;
      MemBlocks used_memory_blocks_;
      // Store last block iterator, for first circled deletion from list..
      std::size_t blocks_in_pool_;
      Sync::PosixMutex mutex_;
      std::size_t blocks_size_;
    };
    
    /**
     * Allocator realize vector strategy: give memory
     * from pool, if contain suitable memory block, or allocate.
     * When deallocating save memory block into pool.
     * Memory blocks stored into std::multimap
     */
    template<typename SizeModificator, size_t align = 0>
    class PoolMultiMapSimple : public BaseAllocator,
      public ReferenceCounting::AtomicImpl
    {
    public:
      Pointer
      allocate(std::size_t& n) throw (MemoryOut);

      void
      deallocate(Pointer ptr, std::size_t) throw ();

      virtual
      void
      calc_sys_deallocate() throw ();

      virtual
      ~PoolMultiMapSimple() throw ();

    private:
      typedef std::multimap<uint32_t, void*> MemBlocks;
      MemBlocks pool_memory_blocks_;
      Sync::PosixMutex mutex_;
    };


    /**
     * Allocator store some baskets with fixed size memory blocks.
     * More exactly PoolListSimple.
     * Each basket protect by personal mutex.
     */
    template<typename SizeModificator>
    class PoolMultiThread : public BaseAllocator,
      public ReferenceCounting::AtomicImpl
    {
      static const std::size_t MIN_BACKET = 8 * 1024;
      static const std::size_t AMOUNT_BACKET = 8;
    public:
      Pointer
      allocate(std::size_t& n) throw (MemoryOut);

      void
      deallocate(Pointer ptr, std::size_t) throw ();

      virtual
      void
      calc_sys_deallocate() throw ();

    protected:
      ~PoolMultiThread() throw ()
      {
      }

    private:

      BlocksPool  allocators_[AMOUNT_BACKET];

      typedef PoolListSimple<DefaultModifier, 0> Allocator;
      Allocator unlimited_pool_;  // last is for unlimited blocks
    };

  } //namespace Allocator

}

//////////////////////////////////////////////////////////////////////////
//  Implementations
//////////////////////////////////////////////////////////////////////////

namespace Generics
{
  namespace Allocator
  {
  
    void
    DefaultModifier::modify(std::size_t& n, std::size_t /*align*/,
      bool pool)
      throw ()
    {
      if (pool)
      {
        n += sizeof(std::size_t);
      }
    }

    void
    Reserve::modify(std::size_t& n, std::size_t /*align*/,
      bool pool)
      throw ()
    {
      n <<= 1;
      if (pool)
      {
        n += sizeof(std::size_t);
      }
    }

    void
    Aligned::modify(std::size_t& n, std::size_t align,
      bool pool)
      throw ()
    {
      if (pool)
      {
        n += sizeof(std::size_t);
      }
      n = (( n + align - 1) / align) * align;
    }

    void
    AlignedReserve::modify(std::size_t& n, std::size_t align,
      bool pool)
      throw ()
    {
      // maximum n = 0x7FFFFFFF
      Reserve::modify(n, align, false);
      Aligned::modify(n, align, pool);
    }

    inline
    BaseAllocator::BaseAllocator() throw ()
      : stat_allocate_(0),
        stat_deallocate_(0),
        stat_sys_allocate_(0),
        stat_sys_deallocate_(0)
    {
    }

    inline
    BaseAllocator::~BaseAllocator() throw ()
    {
    }

    inline
    const BaseAllocator::SizeBlockEraser&
    BaseAllocator::get_eraser_() throw ()
    {
      return eraser_;
    }

    inline
    std::size_t 
    BaseAllocator::stat_allocate(bool get) throw ()
    {
      return get ? stat_allocate_ :
        __gnu_cxx::__exchange_and_add(&stat_allocate_, 1);
    }

    inline
    std::size_t 
    BaseAllocator::stat_deallocate(bool get) throw ()
    {
      return get ? stat_deallocate_ :
        __gnu_cxx::__exchange_and_add(&stat_deallocate_, 1);
    }

    inline
    std::size_t 
    BaseAllocator::stat_sys_allocate(bool get) throw ()
    {
      return get ? stat_sys_allocate_ :
        __gnu_cxx::__exchange_and_add(&stat_sys_allocate_, 1);
    }

    inline
    std::size_t 
    BaseAllocator::stat_sys_deallocate(bool get) throw ()
    {
      return get ? stat_sys_deallocate_ :
        __gnu_cxx::__exchange_and_add(&stat_sys_deallocate_, 1);
    }

    inline
    BaseAllocator::Pointer
    BaseAllocator::alloc_size_block_(std::size_t n) throw (MemoryOut)
    {
      stat_sys_allocate();
      trace_message("Generics::BaseAllocator::alloc_size_block(): ", n);
      Pointer ptr = new unsigned char[n];
      trace_message("Generics::BaseAllocator::allocated(): ", (std::size_t)ptr);
      *static_cast<std::size_t*>(ptr) = n;
      return static_cast<unsigned char*>(ptr) +
             sizeof(std::size_t);
    }

    inline
    void
    BaseAllocator::SizeBlockEraser::operator() (const MemoryBlock& mb) const
      throw ()
    {
      trace_message("Generics::SizeBlockEraser::operator(): ", mb.first);
      trace_message("Generics::BaseAllocator::deleting: ",
        reinterpret_cast<std::size_t>(static_cast<unsigned char*>(mb.second) -
          sizeof(std::size_t)));

      delete[] (static_cast<unsigned char*>(mb.second) -
                sizeof(std::size_t));
    }

    //
    // class FakeAllocator
    //

    template<typename SizeModificator, size_t align>
    FakeAllocator<SizeModificator, align>::FakeAllocator() throw (eh::Exception)
      : ptr_(BUFFER_SIZE)
    {
      ::memset(ptr_.get(), 0xDD, BUFFER_SIZE);
    }

    template<typename SizeModificator, size_t align>
    BaseAllocator::Pointer
    FakeAllocator<SizeModificator, align>::allocate(
      std::size_t& n) throw (MemoryOut)
    {
      stat_allocate();
      SizeModificator::modify(n, align);
      return ptr_.get() + BUFFER_SIZE - n;
    }

    template<typename SizeModificator, size_t align>
    void
    FakeAllocator<SizeModificator, align>::deallocate(FakeAllocator::Pointer, std::size_t) throw ()
    {
      stat_deallocate();
    }

    template<typename SizeModificator, size_t align>
    void
    FakeAllocator<SizeModificator, align>::calc_sys_deallocate() throw ()
    {
    }

    //
    // class Simple
    //

    template<typename SizeModificator, size_t align>
    inline
    BaseAllocator::Pointer
    Simple<SizeModificator, align>::allocate(
      std::size_t& n) throw (MemoryOut)
    {
      stat_allocate();
      stat_sys_allocate();
      SizeModificator::modify(n, align);

      trace_message("Generics::Simple::allocate(): ", n);
      Pointer ptr = new unsigned char[n];
      trace_message("Generics::Simple::allocated(): ",
        reinterpret_cast<size_t>(ptr));
      return ptr;
    };

    template<typename SizeModificator, size_t align>
    inline
    void
    Simple<SizeModificator, align>::deallocate(
      BaseAllocator::Pointer ptr, std::size_t) throw ()
    {
      stat_deallocate();
      stat_sys_deallocate();
      trace_message("Generics::Simple::deallocate(): ",
        reinterpret_cast<size_t>(ptr));
      delete[] static_cast<unsigned char*>(ptr);
    }

    template<typename SizeModificator, size_t align>
    inline
    void
    Simple<SizeModificator, align>::calc_sys_deallocate() throw ()
    {
    }

    //
    // class PoolListSimple
    //

    template<typename SizeModificator, size_t align>
    inline
    PoolListSimple<SizeModificator, align>::PoolListSimple()
      throw ()
      : blocks_in_pool_(0)
    {
    }

    template<typename SizeModificator, size_t align>
    inline
    BaseAllocator::Pointer
    PoolListSimple<SizeModificator, align>::allocate(
      std::size_t& n) throw (MemoryOut)
    {
      SizeModificator::modify(n, align, true);
      std::size_t to_alloc = n;
      n -= sizeof(std::size_t);

      {
        Sync::PosixGuard lock(mutex_);
        stat_allocate();
        MemBlocks::iterator it(pool_memory_blocks_.begin());

        while (it != pool_memory_blocks_.end())
        {
          if (it->first >= to_alloc)
          {
            to_alloc = it->first;
            BaseAllocator::Pointer ptr = it->second;
            used_memory_blocks_.splice(
              used_memory_blocks_.begin(), pool_memory_blocks_, it);
            return ptr;
          }
          ++it;
        }
      }
      return alloc_size_block_(to_alloc);
    }

    template<typename SizeModificator, size_t align>
    inline
    void
    PoolListSimple<SizeModificator, align>::deallocate(
      BaseAllocator::Pointer ptr, std::size_t) throw ()
    {
      MemBlocks::value_type save(
        *(static_cast<std::size_t*>(ptr) - 1),
        ptr);

      Sync::PosixGuard lock(mutex_);
      stat_deallocate();
      MemBlocks::iterator it(used_memory_blocks_.begin());

      if (it != used_memory_blocks_.end())
      {
        *it = save;
        pool_memory_blocks_.splice(
          pool_memory_blocks_.begin(), used_memory_blocks_, it);
        return;
      }

      trace_message("Save MemoryBlock to pool(): ", save.first);
      trace_message("Save MemoryBlock to pool(): ", save.second);
      if (blocks_in_pool_ < 100)
      {
        pool_memory_blocks_.push_front(save);
        ++blocks_in_pool_;
      }
      else
      {
        // push into front, erase last
        MemBlocks::iterator it = pool_memory_blocks_.end();
        --it;
        get_eraser_()(*it);
        stat_sys_deallocate();
        pool_memory_blocks_.erase(it);
        pool_memory_blocks_.push_front(save);
      }
    }

    template<typename SizeModificator, size_t align>
    void
    PoolListSimple<SizeModificator, align>::calc_sys_deallocate() throw ()
    {
    }

    template<typename SizeModificator, size_t align>
    inline
    PoolListSimple<SizeModificator, align>::~PoolListSimple()
      throw ()
    {
      std::for_each(pool_memory_blocks_.begin(),
                    pool_memory_blocks_.end(),
                    get_eraser_());
      // used_memory_blocks must be empty!
//      assert(used_memory_blocks_.size() == 0);
    }

    //
    // class PoolMultiMapSimple
    //

    template<typename SizeModificator, size_t align>
    inline
    BaseAllocator::Pointer
    PoolMultiMapSimple<SizeModificator, align>::allocate(
      std::size_t& n) throw (MemoryOut)
    {
      SizeModificator::modify(n, align, true);
      std::size_t to_alloc = n;
      n -= sizeof(std::size_t);

      Sync::PosixGuard lock(mutex_);
      stat_allocate();

      MemBlocks::iterator it = pool_memory_blocks_.lower_bound(to_alloc);
      if (it == pool_memory_blocks_.end())
      {
        return alloc_size_block_(to_alloc);
      }
      // Get memory block from pool.
      Pointer ptr = (*it).second;
      pool_memory_blocks_.erase(it);
      return ptr;
    }

    template<typename SizeModificator, size_t align>
    inline
    void
    PoolMultiMapSimple<SizeModificator, align>::deallocate(
      BaseAllocator::Pointer ptr, std::size_t) throw ()
    {
      MemBlocks::value_type save(
        *(static_cast<std::size_t*>(ptr) - 1),
        ptr);

      Sync::PosixGuard lock(mutex_);
      stat_deallocate();

      trace_message("Save MemoryBlock to pool(): ", save.first);
      trace_message("Save MemoryBlock to pool(): ", save.second);
      if (pool_memory_blocks_.size() >= 100)
      {
        MemBlocks::iterator it(pool_memory_blocks_.begin());
        get_eraser_()(*it);
        pool_memory_blocks_.erase(it);
        stat_sys_deallocate();
      }
      pool_memory_blocks_.insert(save);
    }

    template<typename SizeModificator, size_t align>
    void
    PoolMultiMapSimple<SizeModificator, align>::calc_sys_deallocate() throw ()
    {
      for (std::size_t i = 0; i < pool_memory_blocks_.size(); ++i)
      {
        stat_sys_deallocate();
      }
    }

    template<typename SizeModificator, size_t align>
    inline
    PoolMultiMapSimple<SizeModificator, align>::~PoolMultiMapSimple()
      throw ()
    {
      std::for_each(pool_memory_blocks_.begin(),
        pool_memory_blocks_.end(),
        SizeBlockEraser());
    }

    //
    // class BlocksPool
    //

    inline
    BlocksPool::BlocksPool()
      throw ()
      : blocks_in_pool_(0)
    {
    }

    inline
    BaseAllocator::Pointer
    BlocksPool::allocate(
      std::size_t& n) throw (MemoryOut)
    {
      {
        Sync::PosixGuard lock(mutex_);
        stat_allocate();
        MemBlocks::iterator it(pool_memory_blocks_.begin());

        if (it != pool_memory_blocks_.end())
        {
          BaseAllocator::Pointer ptr = it->second;
          used_memory_blocks_.splice(
            used_memory_blocks_.begin(), pool_memory_blocks_, it);
          return ptr;
        }
      }
      return alloc_size_block_(n);
    }

    inline
    void
    BlocksPool::deallocate(
      BaseAllocator::Pointer ptr, std::size_t) throw ()
    {
      MemBlocks::value_type save(
        *(static_cast<std::size_t*>(ptr) - 1),
        ptr);

      Sync::PosixGuard lock(mutex_);
      stat_deallocate();
      MemBlocks::iterator it(used_memory_blocks_.begin());

      if (it != used_memory_blocks_.end())
      {
        *it = save;
        pool_memory_blocks_.splice(
          pool_memory_blocks_.begin(), used_memory_blocks_, it);
        return;
      }

      trace_message("Save MemoryBlock to pool(): ", save.first);
      trace_message("Save MemoryBlock to pool(): ", save.second);
      if (blocks_in_pool_ < 100)
      {
        pool_memory_blocks_.push_front(save);
        ++blocks_in_pool_;
      }
      else
      {
        // push into front, erase last
        MemBlocks::iterator it = pool_memory_blocks_.end();
        --it;
        get_eraser_()(*it);
        stat_sys_deallocate();
        pool_memory_blocks_.erase(it);
        pool_memory_blocks_.push_front(save);
      }
    }

    void
    BlocksPool::calc_sys_deallocate() throw ()
    {
    }

    inline
    BlocksPool::~BlocksPool()
      throw ()
    {
      std::for_each(pool_memory_blocks_.begin(),
                    pool_memory_blocks_.end(),
                    get_eraser_());
      // used_memory_blocks must be empty!
      assert(used_memory_blocks_.size() == 0);
    }


    //
    // class PoolMultiThread
    //

    template<typename SizeModificator>
    inline
    BaseAllocator::Pointer
    PoolMultiThread<SizeModificator>::allocate(
      std::size_t& n) throw (MemoryOut)
    {
      stat_allocate();
      SizeModificator::modify(n, 0, true);
      std::size_t to_alloc = n;
      std::size_t original_size = n;
      

      std::size_t position = 0;
      n = MIN_BACKET;
      while (to_alloc > MIN_BACKET)
      {
        to_alloc >>= 1;   // division by 2.
        n <<= 1;
        ++position;
      }
      if (position >= AMOUNT_BACKET)
      {
        n = original_size;
        return unlimited_pool_.allocate(n);
      }
      
      return allocators_[position].allocate(n);
    }

    template<typename SizeModificator>
    inline
    void
    PoolMultiThread<SizeModificator>::deallocate(
      BaseAllocator::Pointer ptr, std::size_t) throw ()
    {
      stat_deallocate();
      // And now select allocators by size
      std::size_t deallocate_size =
        *(static_cast<std::size_t*>(ptr) - 1);

      std::size_t position = 0;
      while (deallocate_size > MIN_BACKET)
      {
        deallocate_size >>= 1;
        ++position;
      }
      if (position >= AMOUNT_BACKET)
      {
        unlimited_pool_.deallocate(ptr, 0);
      }
      else
      {
        allocators_[position].deallocate(ptr, 0);
      }

    }

    template<typename SizeModificator>
    void
    PoolMultiThread<SizeModificator>::calc_sys_deallocate() throw ()
    {
      std::size_t amount_allocate = 0;
      std::size_t amount = 0;
      for (std::size_t i = 0;
           i < sizeof(allocators_) / sizeof(allocators_[0]);
           ++i)
      {
        amount += allocators_[i].stat_sys_deallocate(true);
        amount_allocate += allocators_[i].stat_sys_allocate(true);
      }
      amount += unlimited_pool_.stat_sys_deallocate(true);
      amount_allocate += unlimited_pool_.stat_sys_allocate(true);
      for (std::size_t i = 0; i < amount; ++i)
      {
        stat_sys_deallocate();
      }
      for (std::size_t i = 0; i < amount_allocate; ++i)
      {
        stat_sys_allocate();
      }
    }

  } // namespace Allocator

} //namespace Generics

#endif  // _TEST_ALLOCATOR_HPP_INCLUDED_
