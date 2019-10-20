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



#pragma once

#include <vector>

#include "PhormHash.hpp"

//-----------------------------------------------------------------------------
void testHash ( const char * name );

//-----------------------------------------------------------------------------
// Self-test on startup - verify that all installed hashes work correctly.

void SelfTest ( void );

extern bool g_testAll;

extern bool g_testSanity;
extern bool g_testSpeed;
extern bool g_testDiff;
extern bool g_testDiffDist;
extern bool g_testAvalanche;
extern bool g_testBIC;
extern bool g_testCyclic;
extern bool g_testTwoBytes;
extern bool g_testSparse;
extern bool g_testPermutation;
extern bool g_testWindow;
extern bool g_testText;
extern bool g_testZeroes;
extern bool g_testSeed;

extern uint32_t g_inputVCode;
extern uint32_t g_outputVCode;
extern uint32_t g_resultVCode;

struct HashInfo
{
  pfHash hash;
  int hashbits;
  uint32_t verification;
  std::string name;
  const char * desc;
  std::size_t incremental_standard;
};

struct Hashes : public std::vector<HashInfo>
{
  Hashes();

  void
  append(const HashInfo& hash);

  template <typename Hash, const int N>
  void
  add_hash_n(const char* hash_name, HashInfo& hash);

  template <typename Hash, const int N>
  void
  add_hash_r(const char* hash_name, HashInfo& hash);

  template <typename Hash>
  void
  add_hash(std::size_t incremental_standard, uint32_t verification,
    const char* hash_name);
};

extern Hashes g_hashes;

//
// Hashes class implementation
//

inline
void
Hashes::append(const HashInfo& hash)
{
  test_custom_key_incremental_hash_indirect(hash.name.c_str(),
    hash.incremental_standard, hash.hash);
  push_back(hash);
}

template <typename Hash, const int N>
void
Hashes::add_hash_n(const char* hash_name, HashInfo& hash)
{
  hash.hash = hash_by_n<Hash, N>;
  char txt[1024];
  snprintf(txt, sizeof(txt), "%s%i", hash_name, N);
  hash.name = txt;
  append(hash);
}


template <typename Hash, const int N>
void
Hashes::add_hash_r(const char* hash_name, HashInfo& hash)
{
  hash.hash = hash_by_r<Hash, N>;
  char txt[1024];
  snprintf(txt, sizeof(txt), "%sr%i", hash_name, N);
  hash.name = txt;
  append(hash);
}

template <typename Hash>
void
Hashes::add_hash(std::size_t incremental_standard, uint32_t verification,
  const char* hash_name)
{
  // Check incremental calculation
  test_custom_key_incremental_hash<Hash>(hash_name, incremental_standard);
  HashInfo hash =
  {
    hash_simple<Hash>, 64, verification, hash_name, "", incremental_standard
  };
  append(hash);

  add_hash_n<Hash, 1>(hash_name, hash);
  add_hash_n<Hash, 2>(hash_name, hash);
  add_hash_n<Hash, 4>(hash_name, hash);
  add_hash_n<Hash, 8>(hash_name, hash);
  add_hash_n<Hash, 16>(hash_name, hash);
  add_hash_r<Hash, 32>(hash_name, hash);
}
