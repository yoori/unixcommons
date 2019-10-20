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

#include <stdio.h>
#include <time.h>

#include "Test.hpp"

int
main(int argc, char** argv)
{
  // Add Phorm hash classes to test environment
  // x - The phorm's increment hash class name
  // incremental_code - Control code for test_custom_key_incremental_hash,
  // validation_code  - Control code for SMHasher environment
#define ADD(type, incremental_code, validation_code) \
  g_hashes.add_hash<type>(incremental_code, validation_code, #type)

  ADD(Generics::CRC32Hash, 0x177752D8, 0x5705184A);
  ADD(Generics::Murmur64Hash, 0xF9ED10E038AA02F9ull, 0x375F2D47);
//  ADD(Generics::Murmur128Hash, 0x84E3A693E37B76D9ull, 0x6CF9C2DE);
  ADD(Generics::Murmur32v3Hash, 0xB1D66F58u, 0xAB9F3AEA);

#undef ADD

  const char* hashToTest = 0;
  // Select algorithm to test by his name
  //hashToTest = "Generics::Murmur64Hash";

  if (argc < 2)
  {
    printf(
      "(No test hash given on command line, testing all known functions.)\n");
  }
  else
  {
    hashToTest = argv[1];
  }

  // Code runs on the 3rd CPU by default

  SetAffinity((1 << 2));

  SelfTest();

  int timeBegin = clock();

  g_testAll = false;

  g_testSanity = true;
  g_testSpeed = true;
  g_testAvalanche = false;
  g_testBIC = false;
  g_testCyclic = false;
  g_testTwoBytes = false;
  g_testDiff = false;
  g_testDiffDist = false;
  g_testSparse = false;
  g_testPermutation = false;
  g_testWindow = false;
  g_testZeroes = false;
  g_testText = false;

  if (!hashToTest)
  {
    for (Hashes::const_iterator cit(g_hashes.begin());
      cit != g_hashes.end(); ++cit)
    {
      testHash(cit->name.c_str());
    }
  }
  else
  {
    testHash(hashToTest);
  }

  //----------

  int timeEnd = clock();

  printf("\n");
  printf("Input vcode 0x%08x, Output vcode 0x%08x, Result vcode 0x%08x\n",g_inputVCode,g_outputVCode,g_resultVCode);
  printf("Verification value is 0x%08x - Testing took %f seconds\n",g_verify,double(timeEnd-timeBegin)/double(CLOCKS_PER_SEC));
  printf("-------------------------------------------------------------------------------\n");
  return 0;
}
