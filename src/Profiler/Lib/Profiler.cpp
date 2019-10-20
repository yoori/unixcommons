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





#include <iostream>
#include <sstream>
#include <fstream>

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "Profiler.hpp"


static _funcprof FuncProf[PROF_FUNCTIONS];
static unsigned long FuncGraph[PROF_FUNCTIONS][PROF_FUNCTIONS];
static pthread_mutex_t my_mutex[PROF_FUNCTIONS];

static pthread_mutex_t setspec_mutex  = PTHREAD_MUTEX_INITIALIZER;
static unsigned int thread_number = 1;

static unsigned int CurrentFunc[65535];

static unsigned int res_atexit = 1;
static pthread_once_t key_created = PTHREAD_ONCE_INIT;
FILE* fp;
FILE* fp1;
static pthread_key_t pkey;
int pvalue = 0;

Profiling::Profiling (unsigned int func_index)
{
unsigned int* getspc;
unsigned int* spc_value;

 pthread_once(&key_created, CreateMyKey);
 getspc = (unsigned int*)pthread_getspecific(pkey);

 if (getspc == 0)
  {
   spc_value = (unsigned int *)malloc(sizeof(unsigned int));
   pthread_mutex_lock(&setspec_mutex);
    *spc_value = thread_number;
    pthread_setspecific(pkey, spc_value);
    CurrentFunc[thread_number] = 0;
    ++thread_number;
    FuncProf[func_index].main_function = 1;
   pthread_mutex_unlock(&setspec_mutex);
  }

 _func_index = func_index;

  if (getspc == 0)
   {
    _getspc = *spc_value;
   } else
   {
    _getspc = *getspc;
   }


 pthread_getcpuclockid(pthread_self(), &clock_id1);
 clock_gettime(clock_id1,&tm1);

 pthread_mutex_lock(&my_mutex[func_index]);
  prev_func_index = CurrentFunc[_getspc];
   if (prev_func_index != 0)
    {
     FuncProf[prev_func_index].function_graph = 1;
     ++FuncGraph[prev_func_index][func_index];
    }
  CurrentFunc[_getspc] = func_index;
  FuncProf[func_index].function_index = func_index;
  temp_tm = tm1;
  ++FuncProf[func_index].number_of_calls;
 pthread_mutex_unlock(&my_mutex[func_index]);
}


inline
void
Profiling::add_time(timespec& tm) throw ()
{
  timespec t = { tm.tv_sec + tm2.tv_sec - tm1.tv_sec,
    tm.tv_nsec + tm2.tv_nsec - tm1.tv_nsec };
  if (t.tv_nsec < 0)
  {
    t.tv_sec--;
    t.tv_nsec += 1000000000;
  }
  tm = t;
}

Profiling::~Profiling()
{
 pthread_getcpuclockid(pthread_self(), &clock_id2);
 clock_gettime(clock_id2,&tm2);

 pthread_mutex_lock(&my_mutex[_func_index]);
  {
   add_time(FuncProf[_func_index].tm);
   CurrentFunc[_getspc] = prev_func_index;
  }
 pthread_mutex_unlock(&my_mutex[_func_index]);

 pthread_mutex_lock(&my_mutex[prev_func_index]);
  add_time(FuncProf[prev_func_index].child_tm);
 pthread_mutex_unlock(&my_mutex[prev_func_index]);
}

void Profiling::SaveLog()
{

unsigned int graph_num = 0;
unsigned int basic_log_size = PROF_FUNCTIONS*sizeof(_funcprof);
unsigned int graph_log_size = basic_log_size;
std::string log_file, stat_file, temp_string;
char grp_id[10];
std::ifstream stat_handle;

 snprintf(grp_id, sizeof(grp_id), "%u", static_cast<unsigned>(getpid()));
 temp_string = grp_id;
 stat_file = "/proc/." + temp_string + "/stat";
 stat_handle.open(stat_file.c_str(), std::ios::in);
  if (stat_handle)
   {
    std::getline(stat_handle, log_file);
    log_file = log_file.substr(log_file.find(" ") + 2, log_file.find(" ", log_file.find(" ") + 1) - 3 - log_file.find(" ")) + ".log";
   }

 fp = fopen (log_file.c_str(), "w");

 for (unsigned int i = 0; i < PROF_FUNCTIONS; i++)
  {
   if (FuncProf[i].function_graph)
    {
     FuncProf[i].function_graph = graph_log_size;
     graph_log_size += PROF_FUNCTIONS * sizeof(unsigned long);
     memcpy (&FuncGraph[graph_num][0], &FuncGraph[i][0], PROF_FUNCTIONS * sizeof(unsigned long));
     graph_num++;
    }
  }

 if (fp)
  {
   fwrite (&FuncProf[0], 1, sizeof(_funcprof)*PROF_FUNCTIONS, fp);
   fwrite (&FuncGraph[0][0], 1, graph_log_size - basic_log_size, fp);
   fclose(fp);
  }
}

void Profiling::CreateMyKey(void)
{
unsigned int i;
 for (i = 0; i < PROF_FUNCTIONS; i++)
  {
   pthread_mutex_init(&my_mutex[i], 0);
  }

 res_atexit = atexit(SaveLog);
 pthread_key_create(&pkey, 0);
}

//}
