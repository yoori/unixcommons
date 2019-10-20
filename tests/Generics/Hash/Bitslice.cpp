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



#include "Bitvec.h"
#include <vector>
#include <assert.h>

// handle xnor

typedef std::vector<uint32_t> slice;
typedef std::vector<slice> slice_vec;

int countbits ( slice & v )
{
  int c = 0;

  for(size_t i = 0; i < v.size(); i++)
  {
    int d = countbits(v[i]);

    c += d;
  }

  return c;
}

int countxor ( slice & a, slice & b )
{
  assert(a.size() == b.size());

  int c = 0;

  for(size_t i = 0; i < a.size(); i++)
  {
    int d = countbits(a[i] ^ b[i]);

    c += d;
  }

  return c;
}

void xoreq ( slice & a, slice & b )
{
  assert(a.size() == b.size());

  for(size_t i = 0; i < a.size(); i++)
  {
    a[i] ^= b[i];
  }
}

//-----------------------------------------------------------------------------
// Bitslice a hash set

template< typename hashtype >
void Bitslice ( std::vector<hashtype> & hashes, slice_vec & slices )
{
  const int hashbytes = sizeof(hashtype);
  const int hashbits = hashbytes * 8;
  const int slicelen = ((int)hashes.size() + 31) / 32;

  slices.clear();
  slices.resize(hashbits);

  for(int i = 0; i < (int)slices.size(); i++)
  {
    slices[i].resize(slicelen,0);
  }

  for(int j = 0; j < hashbits; j++)
  {
    void * sliceblob = &(slices[j][0]);

    for(int i = 0; i < (int)hashes.size(); i++)
    {
      int b = getbit(hashes[i],j);

      setbit(sliceblob,slicelen*4,i,b);
    }
  }
}

void FactorSlices ( slice_vec & slices )
{
  std::vector<int> counts(slices.size(),0);

  for(size_t i = 0; i < slices.size(); i++)
  {
    counts[i] = countbits(slices[i]);
  }

  bool changed = true;

  while(changed)
  {
    //int bestA = -1;
    //int bestB = -1;

    for(int j = 0; j < (int)slices.size()-1; j++)
    {
      for(int i = j+1; i < (int)slices.size(); i++)
      {
        int d = countxor(slices[i],slices[j]);

        if((d < counts[i]) && (d < counts[j]))
        {
          if(counts[i] < counts[j])
          {
            //bestA = j;
            //bestB = i;
          }
        }
        else if(d < counts[i])
        {
          //bestA =
        }
      }
    }
  }
}


void foo ( void )
{
  slice a;
  slice_vec b;

  Bitslice(a,b);
}
