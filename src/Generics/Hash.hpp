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



/// @file Generics/Hash.hpp
#ifndef GENERICS_HASH_HPP
#define GENERICS_HASH_HPP

#include <string>
#include <limits>
#include <utility>

#include <ReferenceCounting/NullPtr.hpp>

#include <Generics/Uncopyable.hpp>
#include <Generics/CRC.hpp>


namespace Generics
{
  /*
   * Below provided some hash classes able to incremental hash calculation,
   *   Usage:
   * std::size_t result_hash;
   * {
   *   SomeHash hash(result_hash);
   *   hash.add(data1, size1);
   *   hash_add(hash, String::SubString("text"));
   *   ...
   *   hash_add(hash, static_cast<short>(N));
   * }
   * And here result_hash is calculated.
   *
   *
   * Another interface is Hasher
   *  Usage:
   * SomeHasher hasher;
   * hasher.add(data1, size1);
   * hash_add(hasher, String::SubString("text"));
   * ...
   * hash_add(hasher, static_cast<short>(N));
   * std::size_t result_hash = hasher.finalize ();
   * hasher is not usable here.
   * Hashers are copyable.
   */

  class CRC32Hasher
  {
  public:
    typedef uint32_t Calc;

    explicit
    CRC32Hasher(Calc seed = 0) throw ();

    void
    add(const void* key, std::size_t len) throw ();

    std::size_t
    finalize () throw ();

  private:
    Calc hash_;
  };

  namespace HashHelper
  {
    template <typename Mix>
    class Aggregator
    {
    public:
      typedef typename Mix::Calc Calc;

      explicit
      Aggregator(Calc seed = 0) throw ();

      void
      add(const void* key, std::size_t len) throw ();

      std::size_t
      finalize () throw ();

    private:
      std::size_t count_;
      Calc tail_;
      Calc size_;
      Mix mix_;
    };

    class Murmur64
    {
    public:
      typedef std::size_t Calc;

      explicit
      Murmur64(Calc seed) throw ();
      void
      operator ()(Calc key) throw ();
      std::size_t
      operator ()(std::size_t count, Calc tail, Calc size) throw ();

    private:
      static const std::size_t MULTIPLIER_ = 0xC6A4A7935BD1E995ull;
      static const std::size_t R_ = 47;

      Calc hash_;
    };
 
    class Murmur32v3
    {
    public:
      typedef uint32_t Calc;

      explicit
      Murmur32v3(Calc seed) throw ();
      void
      operator ()(Calc key, bool rh = true) throw ();
      std::size_t
      operator ()(std::size_t count, Calc tail, Calc size) throw ();

    private:
      Calc hash_;
    };
  }

  /**
   * The algorithm is based on MurmurHash64A.
   * Hash function with added Merkle–Damgård construction.
   */
  typedef HashHelper::Aggregator<HashHelper::Murmur64> Murmur64Hasher;

  /**
   * Murmur3 x86 32
   */
  typedef HashHelper::Aggregator<HashHelper::Murmur32v3> Murmur32v3Hasher;

  namespace HashHelper
  {
    template <typename Hasher>
    class Adapter : private Generics::Uncopyable
    {
    public:
      typedef Hasher BaseHasher;
      typedef typename Hasher::Calc Calc;

      explicit
      Adapter(std::size_t& result, Calc seed = 0) throw ();
      ~Adapter() throw ();
      void
      add(const void* key, std::size_t len) throw ();

    private:
      Hasher hasher_;
      std::size_t& result_;
    };
  }

  typedef HashHelper::Adapter<CRC32Hasher> CRC32Hash;
  typedef HashHelper::Adapter<Murmur64Hasher> Murmur64Hash;
  typedef HashHelper::Adapter<Murmur32v3Hasher> Murmur32v3Hash;

  template <typename Hash, typename Value, typename Check = typename
    std::enable_if<std::numeric_limits<Value>::is_specialized>::type>
  void
  hash_add(Hash& hash, const Value& value) throw ();

  template <typename Hash, typename Value,
    typename Check1 = decltype(std::declval<Value>().data()),
    typename Check2 = decltype(std::declval<Value>().size()),
    typename Check3 = typename std::enable_if<
      std::numeric_limits<typename std::remove_const<
        typename std::remove_reference<
          decltype(*std::declval<Value>().data())>::type>::
      type>::is_specialized>::type>
  void
  hash_add(Hash& hash, const Value& value) throw ();
}

namespace Generics
{
  //
  // CRC32Hasher class
  //

  inline
  CRC32Hasher::CRC32Hasher(uint32_t seed) throw ()
    : hash_(seed)
  {
  }

  inline
  void
  CRC32Hasher::add(const void* key, std::size_t len) throw ()
  {
    hash_ = CRC::quick(hash_, key, len);
  }

  inline
  std::size_t
  CRC32Hasher::finalize() throw ()
  {
    return hash_;
  }


  namespace HashHelper
  {
    template <typename Calc>
    std::size_t
    get_unsafe(const void* key) throw ()
    {
      return *static_cast<const Calc*>(key);
    }

    inline
    std::size_t
    get_safe(const uint8_t* key, std::size_t size) throw ()
    {
      std::size_t key_part = 0;
      for (size_t i = 0; i < size; i++)
      {
        key_part |= static_cast<uint64_t>(key[i]) << (i * 8);
      }
      return key_part;
    }


    //
    // Aggregator class
    //

    template <typename Mix>
    Aggregator<Mix>::Aggregator(Calc seed) throw ()
      : count_(0), tail_(0), size_(0), mix_(seed)
    {
    }

    template <typename Mix>
    void
    Aggregator<Mix>::add(const void* key, std::size_t len) throw ()
    {
      if (!len)
      {
        return;
      }

      size_ += len;

      const uint8_t* data = static_cast<const uint8_t*>(key);
      const bool LEN = len < sizeof(Calc);

      if (LEN || count_)
      {
        std::size_t required = sizeof(Calc) - count_;
        if (required <= len)
        {
          mix_(tail_ |
            ((LEN ? get_safe(data, required) :
              get_unsafe<Calc>(data)) << (count_ * 8)));
          tail_ = 0;
          count_ = 0;
          len -= required;
          data += required;
        }
        else
        {
          tail_ |= get_safe(data, len) << (count_ * 8);
          count_ += len;
          return;
        }
      }

      while (len >= sizeof(Calc))
      {
        mix_(get_unsafe<Calc>(data));
        data += sizeof(Calc);
        len -= sizeof(Calc);
      }

      if (len)
      {
        tail_ = get_safe(data, len);
        count_ = len;
      }
    }

    template <typename Mix>
    std::size_t
    Aggregator<Mix>::finalize() throw ()
    {
      return mix_(count_, tail_, size_);
    }


    //
    // MurMur64 class
    //

    inline
    Murmur64::Murmur64(Calc seed) throw ()
      : hash_(seed)
    {
    }

    inline
    void
    Murmur64::operator ()(Calc key) throw ()
    {
      key *= MULTIPLIER_;
      key ^= key >> R_;
      key *= MULTIPLIER_;
      hash_ *= MULTIPLIER_;
      hash_ ^= key;
    }

    inline
    std::size_t
    Murmur64::operator ()(std::size_t /*count*/, Calc tail, Calc size)
      throw ()
    {
      operator()(tail);
      // Merkle–Damgård strengthening
      operator()(size);
      // finalization
      hash_ ^= hash_ >> R_;
      hash_ *= MULTIPLIER_;
      hash_ ^= hash_ >> R_;
      return hash_;
    }


    //
    // MurMur32v3 class
    //

    inline
    Murmur32v3::Murmur32v3(Calc seed) throw ()
      : hash_(seed)
    {
    }

    inline
    void
    Murmur32v3::operator ()(Calc key, bool rh) throw ()
    {
      key *= static_cast<Calc>(0xCC9E2D51u);
      key = (key << 15) | (key >> 17);
      key *= static_cast<Calc>(0x1B873593u);
      hash_ ^= key;
      if (rh)
      {
        hash_ = ((hash_ << 13) | (hash_ >> 19)) * 5 +
          static_cast<Calc>(0xE6546B64u);
      }
    }

    inline
    std::size_t
    Murmur32v3::operator ()(std::size_t count, Calc tail, Calc size)
      throw ()
    {
      if (count)
      {
        operator()(tail, false);
      }
      hash_ ^= size;
      hash_ ^= hash_ >> 16;
      hash_ *= static_cast<Calc>(0x85EBCA6Bu);
      hash_ ^= hash_ >> 13;
      hash_ *= static_cast<Calc>(0xC2B2AE35u);
      hash_ ^= hash_ >> 16;
      return hash_;
    }


    template <typename Hasher>
    Adapter<Hasher>::Adapter(std::size_t& result, Calc seed) throw ()
      : hasher_(seed), result_(result)
    {
    }

    template <typename Hasher>
    Adapter<Hasher>::~Adapter() throw ()
    {
      result_ = hasher_.finalize();
    }

    template <typename Hasher>
    void
    Adapter<Hasher>::add(const void* key, std::size_t len) throw ()
    {
      hasher_.add(key, len);
    }
  }


  //
  // Hash adders' implementations
  //

  template <typename Hash, typename Value, typename Check>
  void
  hash_add(Hash& hash, const Value& value) throw ()
  {
    hash.add(&value, sizeof(value));
  }

  template <typename Hash, typename Value, typename Check1, typename Check2,
    typename Check3>
  void
  hash_add(Hash& hash, const Value& value) throw ()
  {
    hash.add(value.data(), value.size() * sizeof(*value.data()));
  }
}

#endif
