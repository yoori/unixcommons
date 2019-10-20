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



#include "AvalancheTest.h"

//-----------------------------------------------------------------------------

void PrintAvalancheDiagram ( int x, int y, int reps, double scale, int * bins )
{
  const char * symbols = ".123456789X";

  for(int i = 0; i < y; i++)
  {
    printf("[");
    for(int j = 0; j < x; j++)
    {
      int k = (y - i) -1;

      int bin = bins[k + (j*y)];

      double b = double(bin) / double(reps);
      b = fabs(b*2 - 1);

      b *= scale;

      int s = (int)floor(b*10);

      if(s > 10) s = 10;
      if(s < 0) s = 0;

      printf("%c",symbols[s]);
    }

    printf("]\n");
  }
}

//----------------------------------------------------------------------------

double maxBias ( std::vector<int> & counts, int reps )
{
  double worst = 0;

  for(int i = 0; i < (int)counts.size(); i++)
  {
    double c = double(counts[i]) / double(reps);

    double d = fabs(c * 2 - 1);

    if(d > worst)
    {
      worst = d;
    }
  }

  return worst;
}

//-----------------------------------------------------------------------------
