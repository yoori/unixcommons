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



// @file PhormHash.hpp
#ifndef PHORM_HASH_HPP
#define PHORM_HASH_HPP

#include <iostream>

#include <Generics/Hash.hpp>
#include <String/SubString.hpp>
#include <Generics/Uuid.hpp>
#include <Generics/Time.hpp>

/**
 * Adapt Phorm hash class to test suitable function
 */
template <typename Hash>
void
hash_simple(const void* key, int len, uint32_t seed, void* out)
{
#if 0
  Hash hash(*static_cast<std::size_t*>(out), seed);
  hash.add(key, len);
#else
  typename Hash::BaseHasher hasher(seed);
  hasher.add(key, len);
  *static_cast<std::size_t*>(out) = hasher.finalize();
#endif
}

template <typename Hash, const int N>
void
hash_by_n(const void* key, int len, uint32_t seed, void* out)
{
  Hash hash(*static_cast<std::size_t*>(out), seed);
  for (; len > N; len -= N)
  {
    hash.add(key, N);
    key = static_cast<const unsigned char*>(key) + N;
  }
  hash.add(key, len);
}

template <typename Hash, const int N>
void
hash_by_r(const void* key, int len, uint32_t seed, void* out)
{
  Hash hash(*static_cast<std::size_t*>(out), seed);
  while (len)
  {
    int size = rand() % N;
    if (size > len)
    {
      size = len;
    }
    hash.add(key, size);
    key = static_cast<const unsigned char*>(key) + size;
    len -= size;
  }
}

struct CustomKey
{
  std::string f1;
  String::SubString f2;
  Generics::Uuid f3;
  Generics::Time f4;
  double f5;
  int f6;
  bool f7;
  char f8;
  char f9[11];
  const char* f10;
};

template <typename Hash>
void
test_custom_key_incremental_hash_int(Hash& hash)
{
  const CustomKey key =
  {
    "f1fjksdfmnqwef", String::SubString("f2fom4fq3409fm34f8n34f"),
    Generics::Uuid(), Generics::Time(),
    0., 6, true, '8', {'\xFB', '\xFB', '\xFB', '\xFB', '\xFB', '\xFB',
    '\xFB', '\xFB', '\xFB', '\xFB', '\xFB'}, "f10abcdefghijklmni"
  };

  hash_add(hash, key.f1);
  hash_add(hash, key.f2);
  hash_add(hash, key.f3);
  hash_add(hash, key.f4);
  hash_add(hash, key.f5);
  hash_add(hash, key.f6);
  hash_add(hash, key.f7);
  hash_add(hash, key.f8);
  hash.add(key.f9, sizeof(key.f9));
  hash.add(key.f10, strlen(key.f10));
}

template <typename Hash>
void
test_custom_key_incremental_hash(const char* hash_name,
  std::size_t standard_hash)
{
  uint64_t hash_value;
  {
    Hash hash(hash_value);
    test_custom_key_incremental_hash_int(hash);
  }
  if (hash_value != standard_hash)
  {
    std::cerr << hash_name << ": not standard hash result: " << std::hex <<
      hash_value << " (" << std::hex << standard_hash <<
      " expected)" << std::endl;
  }
}

namespace Generics
{
  class HashAdapter
  {
  public:
    HashAdapter(std::size_t& result, pfHash hash_func);
    ~HashAdapter();
    void
    add(const void* key, std::size_t len);

  private:
    std::size_t& result_;
    pfHash hash_func_;
    std::vector<uint8_t> data_;
  };

  inline
  HashAdapter::HashAdapter(std::size_t& result, pfHash hash_func)
    : result_(result), hash_func_(hash_func)
  {
  }

  inline
  HashAdapter::~HashAdapter()
  {
    hash_func_(data_.size() ? &data_[0] : 0, data_.size(), 0, &result_);
  }

  inline
  void
  HashAdapter::add(const void* key, std::size_t len)
  {
    const uint8_t* data = static_cast<const uint8_t*>(key);
    data_.insert(data_.end(), data, data + len);
  }
}

inline
void
test_custom_key_incremental_hash_indirect(const char* hash_name,
  std::size_t standard_hash, pfHash hash_func)
{
  uint64_t hash_value;
  {
    Generics::HashAdapter hash(hash_value, hash_func);
    test_custom_key_incremental_hash_int(hash);
  }
  if (hash_value != standard_hash)
  {
    std::cerr << hash_name << ": not standard hash result: " << std::hex <<
      hash_value << " (" << std::hex << standard_hash <<
      " expected) by hash adapter" << std::endl;
  }
}

#endif // PHORM_HASH_HPP
