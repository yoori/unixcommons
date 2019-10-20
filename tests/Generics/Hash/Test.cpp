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



#include "Platform.h"
#include "Hashes.h"
#include "KeysetTest.h"
#include "SpeedTest.h"
#include "AvalancheTest.h"
#include "DifferentialTest.h"
#include "Test.hpp"

#include <stdio.h>
#include <time.h>

//-----------------------------------------------------------------------------
// Configuration. TODO - move these to command-line flags
bool g_testAll = false;

bool g_testSanity      = true;
bool g_testSpeed       = true;
bool g_testDiff        = true;
bool g_testDiffDist    = true;
bool g_testAvalanche   = true;
bool g_testBIC         = true;
bool g_testCyclic      = true;
bool g_testTwoBytes    = true;
bool g_testSparse      = true;
bool g_testPermutation = true;
bool g_testWindow      = true;
bool g_testText        = true;
bool g_testZeroes      = true;
bool g_testSeed        = false;

//-----------------------------------------------------------------------------
// This is the hard-coded list of hashes that SMHasher can test.

HashInfo g_hardcoded_hashes[] =
{
  { DoNothingHash,        32, 0x00000000, "donothing32", "Do-Nothing function (only valid for measuring call overhead)", 0 },
  { DoNothingHash,        64, 0x00000000, "donothing64", "Do-Nothing function (only valid for measuring call overhead)", 0 },
  { DoNothingHash,       128, 0x00000000, "donothing128", "Do-Nothing function (only valid for measuring call overhead)", 0 },

};

Hashes::Hashes()
{
  for (std::size_t i = 0;
    i < sizeof(g_hardcoded_hashes) / sizeof(g_hardcoded_hashes[0]);
    ++i)
  {
    push_back(g_hardcoded_hashes[i]);
  }
}

Hashes g_hashes;

HashInfo * findHash ( const char * name )
{
  for(size_t i = 0; i < g_hashes.size(); i++)
  {
    if(_stricmp(name,g_hashes[i].name.c_str()) == 0) return &g_hashes[i];
  }

  return NULL;
}

//-----------------------------------------------------------------------------
// Self-test on startup - verify that all installed hashes work correctly.

void SelfTest ( void )
{
  bool pass = true;

  for(size_t i = 0; i < g_hashes.size(); i++)
  {
    HashInfo * info = & g_hashes[i];

    pass &= VerificationTest(info->hash,info->hashbits,info->verification,false);
  }

  if(!pass)
  {
    printf("Self-test FAILED!\n");

    for(size_t i = 0; i < g_hashes.size(); i++)
    {
      HashInfo * info = & g_hashes[i];

      printf("%16s - ",info->name.c_str());
      pass &= VerificationTest(info->hash,info->hashbits,info->verification,true);
    }

    exit(1);
  }
}

//----------------------------------------------------------------------------

template < typename hashtype >
void test ( hashfunc<hashtype> hash, HashInfo * info )
{
  const int hashbits = sizeof(hashtype) * 8;

  printf("-------------------------------------------------------------------------------\n");
  printf("--- Testing %s (%s)\n\n",info->name.c_str(),info->desc);

  //-----------------------------------------------------------------------------
  // Sanity tests

  if(g_testSanity || g_testAll)
  {
    printf("[[[ Sanity Tests ]]]\n\n");

    VerificationTest(hash,hashbits,info->verification,true);
    SanityTest(hash,hashbits);
    AppendedZeroesTest(hash,hashbits);
    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Speed tests

  if(g_testSpeed || g_testAll)
  {
    printf("[[[ Speed Tests ]]]\n\n");

    BulkSpeedTest(info->hash,info->verification);
    printf("\n");

    for(int i = 1; i < 32; i++)
    {
      double cycles;

      TinySpeedTest(hashfunc<hashtype>(info->hash),sizeof(hashtype),i,info->verification,true,cycles);
    }

    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Differential tests

  if(g_testDiff || g_testAll)
  {
    printf("[[[ Differential Tests ]]]\n\n");

    bool result = true;
    bool dumpCollisions = false;

    result &= DiffTest< Blob<64>,  hashtype >(hash,5,1000,dumpCollisions);
    result &= DiffTest< Blob<128>, hashtype >(hash,4,1000,dumpCollisions);
    result &= DiffTest< Blob<256>, hashtype >(hash,3,1000,dumpCollisions);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Differential-distribution tests

  if(g_testDiffDist /*|| g_testAll*/)
  {
    printf("[[[ Differential Distribution Tests ]]]\n\n");

    bool result = true;

    result &= DiffDistTest2<uint64_t,hashtype>(hash);

    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Avalanche tests

  if(g_testAvalanche || g_testAll)
  {
    printf("[[[ Avalanche Tests ]]]\n\n");

    bool result = true;

    result &= AvalancheTest< Blob< 32>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob< 40>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob< 48>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob< 56>, hashtype > (hash,300000);

    result &= AvalancheTest< Blob< 64>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob< 72>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob< 80>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob< 88>, hashtype > (hash,300000);

    result &= AvalancheTest< Blob< 96>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob<104>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob<112>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob<120>, hashtype > (hash,300000);

    result &= AvalancheTest< Blob<128>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob<136>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob<144>, hashtype > (hash,300000);
    result &= AvalancheTest< Blob<152>, hashtype > (hash,300000);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Bit Independence Criteria. Interesting, but doesn't tell us much about
  // collision or distribution.

  if(g_testBIC)
  {
    printf("[[[ Bit Independence Criteria ]]]\n\n");

    bool result = true;

    //result &= BicTest<uint64_t,hashtype>(hash,2000000);
    BicTest3<Blob<88>,hashtype>(hash,2000000);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Cyclic' - keys of the form "abcdabcdabcd..."

  if(g_testCyclic || g_testAll)
  {
    printf("[[[ Keyset 'Cyclic' Tests ]]]\n\n");

    bool result = true;
    bool drawDiagram = false;

    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+0,8,10000000,drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+1,8,10000000,drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+2,8,10000000,drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+3,8,10000000,drawDiagram);
    result &= CyclicKeyTest<hashtype>(hash,sizeof(hashtype)+4,8,10000000,drawDiagram);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Keyset 'TwoBytes' - all keys up to N bytes containing two non-zero bytes

  // This generates some huge keysets, 128-bit tests will take ~1.3 gigs of RAM.

  if(g_testTwoBytes || g_testAll)
  {
    printf("[[[ Keyset 'TwoBytes' Tests ]]]\n\n");

    bool result = true;
    bool drawDiagram = false;

    for(int i = 4; i <= 20; i += 4)
    {
      result &= TwoBytesTest2<hashtype>(hash,i,drawDiagram);
    }

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Sparse' - keys with all bits 0 except a few

  if(g_testSparse || g_testAll)
  {
    printf("[[[ Keyset 'Sparse' Tests ]]]\n\n");

    bool result = true;
    bool drawDiagram = false;

    result &= SparseKeyTest<  32,hashtype>(hash,6,true,true,true,drawDiagram);
    result &= SparseKeyTest<  40,hashtype>(hash,6,true,true,true,drawDiagram);
    result &= SparseKeyTest<  48,hashtype>(hash,5,true,true,true,drawDiagram);
    result &= SparseKeyTest<  56,hashtype>(hash,5,true,true,true,drawDiagram);
    result &= SparseKeyTest<  64,hashtype>(hash,5,true,true,true,drawDiagram);
    result &= SparseKeyTest<  96,hashtype>(hash,4,true,true,true,drawDiagram);
    result &= SparseKeyTest< 256,hashtype>(hash,3,true,true,true,drawDiagram);
    result &= SparseKeyTest<2048,hashtype>(hash,2,true,true,true,drawDiagram);

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Permutation' - all possible combinations of a set of blocks

  if(g_testPermutation || g_testAll)
  {
    {
      // This one breaks lookup3, surprisingly

      printf("[[[ Keyset 'Combination Lowbits' Tests ]]]\n\n");

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007,
      };

      result &= CombinationKeyTest<hashtype>(hash,8,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
    }

    {
      printf("[[[ Keyset 'Combination Highbits' Tests ]]]\n\n");

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x20000000, 0x40000000, 0x60000000, 0x80000000, 0xA0000000, 0xC0000000, 0xE0000000
      };

      result &= CombinationKeyTest<hashtype>(hash,8,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
    }

    {
      printf("[[[ Keyset 'Combination 0x8000000' Tests ]]]\n\n");

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x80000000,
      };

      result &= CombinationKeyTest<hashtype>(hash,20,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
    }

    {
      printf("[[[ Keyset 'Combination 0x0000001' Tests ]]]\n\n");

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x00000001,
      };

      result &= CombinationKeyTest<hashtype>(hash,20,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
    }

    {
      printf("[[[ Keyset 'Combination Hi-Lo' Tests ]]]\n\n");

      bool result = true;
      bool drawDiagram = false;

      uint32_t blocks[] =
      {
        0x00000000,

        0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007,

        0x80000000, 0x40000000, 0xC0000000, 0x20000000, 0xA0000000, 0x60000000, 0xE0000000
      };

      result &= CombinationKeyTest<hashtype>(hash,6,blocks,sizeof(blocks) / sizeof(uint32_t),true,true,drawDiagram);

      if(!result) printf("*********FAIL*********\n");
      printf("\n");
    }
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Window'

  // Skip distribution test for these - they're too easy to distribute well,
  // and it generates a _lot_ of testing

  if(g_testWindow || g_testAll)
  {
    printf("[[[ Keyset 'Window' Tests ]]]\n\n");

    bool result = true;
    bool testCollision = true;
    bool testDistribution = false;
    bool drawDiagram = false;

    result &= WindowedKeyTest< Blob<hashbits*2>, hashtype > ( hash, 20, testCollision, testDistribution, drawDiagram );

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Text'

  if(g_testText || g_testAll)
  {
    printf("[[[ Keyset 'Text' Tests ]]]\n\n");

    bool result = true;
    bool drawDiagram = false;

    const char * alnum = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    result &= TextKeyTest( hash, "Foo",    alnum,4, "Bar",    drawDiagram );
    result &= TextKeyTest( hash, "FooBar", alnum,4, "",       drawDiagram );
    result &= TextKeyTest( hash, "",       alnum,4, "FooBar", drawDiagram );

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Zeroes'

  if(g_testZeroes || g_testAll)
  {
    printf("[[[ Keyset 'Zeroes' Tests ]]]\n\n");

    bool result = true;
    bool drawDiagram = false;

    result &= ZeroKeyTest<hashtype>( hash, drawDiagram );

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
  }

  //-----------------------------------------------------------------------------
  // Keyset 'Seed'

  if(g_testSeed || g_testAll)
  {
    printf("[[[ Keyset 'Seed' Tests ]]]\n\n");

    bool result = true;
    bool drawDiagram = false;

    result &= SeedTest<hashtype>( hash, 1000000, drawDiagram );

    if(!result) printf("*********FAIL*********\n");
    printf("\n");
  }
}

//-----------------------------------------------------------------------------

uint32_t g_inputVCode = 1;
uint32_t g_outputVCode = 1;
uint32_t g_resultVCode = 1;

HashInfo * g_hashUnderTest = NULL;

void VerifyHash ( const void * key, int len, uint32_t seed, void * out )
{
  g_inputVCode = MurmurOAAT(key,len,g_inputVCode);
  g_inputVCode = MurmurOAAT(&seed,sizeof(uint32_t),g_inputVCode);

  g_hashUnderTest->hash(key,len,seed,out);

  g_outputVCode = MurmurOAAT(out,g_hashUnderTest->hashbits/8,g_outputVCode);
}

//-----------------------------------------------------------------------------

void testHash ( const char * name )
{
  HashInfo * pInfo = findHash(name);

  if(pInfo == NULL)
  {
    printf("Invalid hash '%s' specified\n",name);
    return;
  }
  else
  {
    g_hashUnderTest = pInfo;

    if(pInfo->hashbits == 32)
    {
      test<uint32_t>( VerifyHash, pInfo );
    }
    else if(pInfo->hashbits == 64)
    {
      test<uint64_t>( pInfo->hash, pInfo );
    }
    else if(pInfo->hashbits == 128)
    {
      test<uint128_t>( pInfo->hash, pInfo );
    }
    else if(pInfo->hashbits == 256)
    {
      test<uint256_t>( pInfo->hash, pInfo );
    }
    else
    {
      printf("Invalid hash bit width %d for hash '%s'",pInfo->hashbits,pInfo->name.c_str());
    }
  }
}
