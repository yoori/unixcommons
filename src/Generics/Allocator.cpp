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



// Allocator.cpp

#include <algorithm>
#include <cassert>

#include <Generics/BitAlgs.hpp>
#include <Generics/Allocator.hpp>

//#define BUILD_WITH_DEBUG_MESSAGES
#include "Trace.hpp"

//////////////////////////////////////////////////////////////////////////
//  Implementations
//////////////////////////////////////////////////////////////////////////


namespace Generics
{
  namespace Allocator
  {
    Sync::PosixMutex Base::default_allocator_creation_mutex_;
    volatile sig_atomic_t Base::default_allocator_initialized_;
    Base* volatile Base::default_allocator_;

    Base*
    Base::get_default_allocator() throw (eh::Exception)
    {
      if (!default_allocator_initialized_)
      {
        {
          Sync::PosixGuard guard(default_allocator_creation_mutex_);
          if (!default_allocator_)
          {
            default_allocator_ = new Default;
          }
        }
        default_allocator_initialized_ = true;
      }

      return default_allocator_;
    }

    Base::~Base() throw ()
    {
    }

    inline
    void
    Base::align_(size_t& number, size_t mask) throw ()
    {
      number += (-number) & mask;
    }

    size_t
    Base::cached() const throw (eh::Exception)
    {
      return 0;
    }

    void
    Base::print_cached(std::ostream& ostr) const throw (eh::Exception)
    {
      ostr << '0';
    }


    //
    // class Default
    //

    const size_t Default::DEF_ALIGN;

    Default::Default(size_t align_code) throw ()
      : MASK_((1 << align_code) - 1)
    {
    }

    Default::~Default() throw ()
    {
    }

    Base::Pointer
    Default::allocate(size_t& size)
      throw (eh::Exception, OutOfMemory)
    {
      align_(size, MASK_);
      return new unsigned char[size];
    }

    void
    Default::deallocate(Pointer ptr, size_t size) throw ()
    {
      (void)size;
      assert(!(size & MASK_));
      delete [] static_cast<unsigned char*>(ptr);
    }


    //
    // class VarSizeList
    //

    VarSizeList::VarSizeList(size_t align_code, size_t blocks_count)
      throw (eh::Exception)
      : MASK_((1 << align_code) - 1), blocks_limit_(blocks_count),
        cached_(0)
    {
    }

    VarSizeList::~VarSizeList() throw ()
    {
      std::for_each(pool_memory_blocks_.begin(),
        pool_memory_blocks_.end(),
        VarSizeList::memory_block_delete_);
    }

    inline
    void
    VarSizeList::memory_block_delete_(MemoryBlock& mb) throw ()
    {
      delete [] (static_cast<unsigned char*>(mb.second));
    }

    Base::Pointer
    VarSizeList::allocate(size_t& size)
      throw (eh::Exception, OutOfMemory)
    {
      align_(size, MASK_);
      {
        Sync::PosixGuard lock(mutex_);

        for (MemBlocks::iterator it(pool_memory_blocks_.begin());
          it != pool_memory_blocks_.end(); ++it)
        {
          if (it->first >= size)
          {
            size = it->first;
            Pointer ptr = it->second;
            pool_memory_blocks_.erase(it);
            blocks_limit_--;
            cached_ -= size;
            return ptr;
          }
        }
      }
      return new unsigned char[size];
    }

    void
    VarSizeList::deallocate(Pointer ptr, size_t size) throw ()
    {
      try
      {
        MemBlocks::value_type save(size, ptr);
        ArrayByte holder_for_exception;
        holder_for_exception.unsafe_reset(static_cast<unsigned char*>(ptr));

        {
          Sync::PosixGuard lock(mutex_);
          if (blocks_limit_)
          {
            pool_memory_blocks_.push_front(save);
            blocks_limit_--;
          }
          else
          {
            // push front, erase last
            MemBlocks::iterator it(pool_memory_blocks_.end());
            if (it != pool_memory_blocks_.begin())
            {
              --it;
              cached_ -= it->first;
              memory_block_delete_(*it);
              *it = save;

              if (it != pool_memory_blocks_.begin())
              {
                pool_memory_blocks_.splice(
                  pool_memory_blocks_.begin(), pool_memory_blocks_, it);
              }
            }
          }
        }

        holder_for_exception.release();
        cached_ += size;
      }
      catch (const eh::Exception&)
      {
      }
    }

    size_t
    VarSizeList::cached() const throw (eh::Exception)
    {
      Sync::PosixGuard lock(mutex_);
      return cached_;
    }

    void
    VarSizeList::print_cached(std::ostream& ostr) const
      throw (eh::Exception)
    {
      ostr << cached();
    }


    //
    // class ConstSizeArray
    //

    ConstSizeArray::ConstSizeArray(size_t max_blocks_count,
      size_t block_size) throw (eh::Exception)
      : MAX_BLOCKS_COUNT_(max_blocks_count), BLOCK_SIZE_(block_size),
        pool_memory_blocks_(max_blocks_count),
        blocks_in_pool_(0), hits_(0), misses_(0)
    {
    }

    ConstSizeArray::~ConstSizeArray() throw ()
    {
    }

    Base::Pointer
    ConstSizeArray::allocate(size_t& size)
      throw (eh::Exception, OutOfMemory)
    {
      if (size > BLOCK_SIZE_)
      {
        Stream::Error ostr;
        ostr << FNS << "request block is bigger than allowed";
        throw OutOfMemory(ostr);
      }
      size = BLOCK_SIZE_;

      {
        Sync::PosixSpinGuard lock(mutex_);
        if (blocks_in_pool_)
        {
          hits_++;
          return pool_memory_blocks_[--blocks_in_pool_].release();
        }
        misses_++;
      }
      return new unsigned char[BLOCK_SIZE_];
    }

    void
    ConstSizeArray::deallocate(Pointer ptr, size_t size) throw ()
    {
      (void)size;
      assert(size == BLOCK_SIZE_);

      {
        Sync::PosixSpinGuard lock(mutex_);
        if (blocks_in_pool_ < MAX_BLOCKS_COUNT_)
        {
          pool_memory_blocks_[blocks_in_pool_++].unsafe_reset(
            static_cast<unsigned char*>(ptr));
          return;
        }
      }
      delete [] static_cast<unsigned char*>(ptr);
    }

    size_t
    ConstSizeArray::cached() const throw (eh::Exception)
    {
      size_t blocks_in_pool;
      {
        Sync::PosixSpinGuard lock(mutex_);
        blocks_in_pool = blocks_in_pool_;
      }
      return blocks_in_pool * BLOCK_SIZE_;
    }

    void
    ConstSizeArray::print_cached(std::ostream& ostr) const
      throw (eh::Exception)
    {
      size_t blocks_in_pool, hits, misses;
      {
        Sync::PosixSpinGuard lock(mutex_);
        blocks_in_pool = blocks_in_pool_;
        hits = hits_;
        misses = misses_;
      }
      ostr << BLOCK_SIZE_ << ':' << blocks_in_pool <<
        '(' << hits << '+' << misses << ')';
    }


    //
    // class Universal
    //

    const size_t Universal::DEF_DEFAULT_THRESHOLD_LOW;
    const size_t Universal::DEF_DEFAULT_THRESHOLD_HIGH;
    const size_t Universal::DEF_FIRST_BUCKET;
    const size_t Universal::DEF_STEP_BETWEEN_BUCKET;
    const size_t Universal::DEF_BUCKET_NUMBER;
    const size_t Universal::DEF_BUCKET_BLOCKS_COUNT;
    const size_t Universal::DEF_UNLIMITED_MASK;
    const size_t Universal::DEF_UNLIMITED_VOLUME;

    const size_t Universal::DEF_STATISTICS_PRECISION;
    const size_t Universal::DEF_STATISTICS_LIMIT;

    Universal::Universal(
      size_t default_threshold_low,
      size_t default_threshold_high,
      size_t step_between_bucket,
      size_t first_bucket,
      size_t buckets_number,
      size_t bucket_blocks_count,
      size_t unlimited_align,
      size_t unlimited_volume,
      size_t statistics_precision,
      size_t statistics_limit) throw (eh::Exception)
      : DEFAULT_THRESHOLD_LOW_(default_threshold_low),
        DEFAULT_THRESHOLD_HIGH_(default_threshold_high),
        FIRST_BUCKET_(first_bucket),
        STEP_BETWEEN_BUCKETS_(step_between_bucket),
        BUCKETS_NUMBER_(buckets_number),
        default_allocator_(
          ReferenceCounting::add_ref(get_default_allocator())),
        blocks_pools_(BUCKETS_NUMBER_),
        unlimited_(new VarSizeList(unlimited_align, unlimited_volume)),
        STATISTICS_PRECISION_(statistics_precision),
        STATISTICS_LIMIT_(statistics_limit),
        statistics_(statistics_limit + 1)
    {
      for (size_t i = 0, size = FIRST_BUCKET_;
        i < BUCKETS_NUMBER_; ++i, size += STEP_BETWEEN_BUCKETS_)
      {
        blocks_pools_[i] = new ConstSizeArray(bucket_blocks_count, size);
      }
      std::fill(statistics_.get(),
        statistics_.get() + statistics_limit + 1, 0);
    }

    Universal::~Universal() throw ()
    {
    }

    inline
    Base_var
    Universal::get_allocator_(size_t size) throw ()
    {
      if (size <= DEFAULT_THRESHOLD_LOW_ || size >= DEFAULT_THRESHOLD_HIGH_)
      {
        return default_allocator_;
      }
      if (size > FIRST_BUCKET_ + STEP_BETWEEN_BUCKETS_ *
        (BUCKETS_NUMBER_ - 1) || BUCKETS_NUMBER_ == 0)
      {
        return unlimited_;
      }

      size_t index = size <= FIRST_BUCKET_ ? 0 :
        (size - FIRST_BUCKET_ - 1) / STEP_BETWEEN_BUCKETS_ + 1;
      return blocks_pools_[index];
    }

    Base::Pointer
    Universal::allocate(size_t& size) throw (eh::Exception, OutOfMemory)
    {
      __gnu_cxx::__atomic_add(&statistics_[
        std::min(size >> STATISTICS_PRECISION_, STATISTICS_LIMIT_)], 1);
      return get_allocator_(size)->allocate(size);
    }

    void
    Universal::deallocate(Pointer ptr, size_t size) throw ()
    {
      get_allocator_(size)->deallocate(ptr, size);
    }

    size_t
    Universal::cached() const throw (eh::Exception)
    {
      size_t cached = default_allocator_->cached();
      for (size_t i = 0; i < BUCKETS_NUMBER_; i++)
      {
        cached += blocks_pools_[i]->cached();
      }
      cached += unlimited_->cached();
      return cached;
    }

    void
    Universal::print_cached(std::ostream& ostr) const throw (eh::Exception)
    {
      ostr << cached() << " D:";
      default_allocator_->print_cached(ostr);
      for (size_t i = 0; i < BUCKETS_NUMBER_; i++)
      {
        ostr << ' ';
        blocks_pools_[i]->print_cached(ostr);
      }
      ostr << " U:";
      unlimited_->print_cached(ostr);
      ostr << " S:";
      for (size_t i = 0; i <= STATISTICS_LIMIT_; i++)
      {
        static const char SCALE[] =
          "123456789"
          "abcdefghijklmnopqrstuvwxyz"
          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
          "#$%0";
        const volatile _Atomic_word& ref = statistics_[i];
        ostr << SCALE[BitAlgs::highest_bit_64(ref)];
      }
    }


    //
    // class Align
    //

    const size_t Align::DEF_PTR_ALIGN;
    const size_t Align::DEF_ALIGN;

    Align::Align(size_t ptr_align_code, size_t align_code) throw ()
      : ALIGN_(1 << ptr_align_code), MASK_((1 << align_code) - 1)
    {
    }

    Base::Pointer
    Align::allocate(size_t& size)
      throw (eh::Exception, OutOfMemory)
    {
      align_(size, MASK_);
      void* ptr;
      if (int res = posix_memalign(&ptr, ALIGN_, size))
      {
        eh::throw_errno_exception<OutOfMemory>(res, FNE,
          "Failed to allocate aligned memory of size ", size);
      }
      return ptr;
    }

    void
    Align::deallocate(Pointer ptr, size_t size) throw ()
    {
      (void)size;
      assert(!(size & MASK_));
      free(ptr);
    }
  } // namespace Allocator
} //namespace Generics
