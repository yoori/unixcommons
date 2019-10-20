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

#include <stdio.h>

void testRDTSC ( void )
{
  int64_t temp = rdtsc();

  printf("%d",(int)temp);
}

#if defined(_MSC_VER)

#include <windows.h>

void SetAffinity ( int cpu )
{
  SetProcessAffinityMask(GetCurrentProcess(),cpu);
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
}

#else

#include <sched.h>

void SetAffinity ( int /*cpu*/ )
{
#ifndef __CYGWIN__
  cpu_set_t mask;

  CPU_ZERO(&mask);

  CPU_SET(2,&mask);

  if( sched_setaffinity(0,sizeof(mask),&mask) == -1)
  {
    printf("WARNING: Could not set CPU affinity\n");
  }
#endif
}

#endif
