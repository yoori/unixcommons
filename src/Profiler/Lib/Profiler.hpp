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





#ifndef _PROFILER_H
#define _PROFILER_H
#define PROF_FUNCTIONS 3500

#include <time.h>

struct _funcprof
{
unsigned int function_index;
unsigned int number_of_calls;
timespec tm;
unsigned int function_graph;
timespec child_tm;
unsigned int main_function;
};

class Profiling
{
protected:
unsigned int _getspc;
unsigned int _func_index;
unsigned int prev_func_index;
timespec temp_tm;
timespec tm1;
timespec tm2;
clockid_t clock_id1;
clockid_t clock_id2;

public:
 Profiling(unsigned int func_index);
 ~Profiling();
 static void SaveLog();
 static void CreateMyKey(void);
protected:
 void
 add_time(timespec& tm) throw ();
};
#endif
