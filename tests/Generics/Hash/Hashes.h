#pragma once

#include "Types.h"

inline void DoNothingHash  ( const void * /*key*/, int /*len*/, uint32_t /*seed*/, void * /*out*/ )
{
}

inline uint32_t MurmurOAAT ( const void * key, int len, uint32_t seed )
{
  const uint8_t * data = (const uint8_t*)key;

  uint32_t h = seed;

  for(int i = 0; i < len; i++)
  {
    h ^= data[i];
    h *= 0x5bd1e995;
    h ^= h >> 15;
  }

  return h;
}

