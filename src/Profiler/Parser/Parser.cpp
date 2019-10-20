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






#include <stdio.h>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <math.h>

#include "Parser.hpp"
#include <Profiler/Lib/Profiler.hpp>

std::string profiler_hpp = 
"#ifndef _PROFILER_H\n"
"#define _PROFILER_H\n"
"#include <time.h>\n"
"struct _funcprof\n"
"{\n"
"unsigned int function_index;\n"
"unsigned int number_of_calls;\n"
"timespec  tm;\n"
"unsigned int function_graph;\n"
"timespec child_tm;\n"
"unsigned int main_function;\n"
"};\n"
"\n"
"class Profiling\n"
"{\n"
"protected:\n"
"unsigned int _getspc;\n"
"unsigned int _func_index;\n"
"unsigned int prev_func_index;\n"
"timespec temp_tm;\n"
"timespec tm1;\n"
"timespec tm2;\n"
"clockid_t clock_id1;\n"
"clockid_t clock_id2;\n"
"public:\n"
" Profiling(unsigned int func_index);\n"
" ~Profiling();\n"
" static void SaveLog();\n"
" static void CreateMyKey(void);\n"
"};\n"
"#endif\n"
"";



static _funcprof FuncProf;
unsigned int mask_par = 0, func_calls;
unsigned int func_graph[PROF_FUNCTIONS];
unsigned int func_saved[PROF_FUNCTIONS];

std::string::size_type temp_pos[100];
std::string file_mask[10];
std::string file_masks;
std::string str1 = "namespace";
std::string str2 = "{";
std::string init_str  = "\nProfiling Prof_Object(";
std::string init_str1;
std::string init_str2 = ");\n";
std::string include_string = "#include <Profiler/Lib/Profiler.hpp>";
std::ofstream func_list;
std::ifstream ifunc_list;

std::string spaces = "                    ";
std::string temp_file_line, temp_file_line1;
std::stringstream temp_val; 

typedef std::vector<std::string> FileLinesArray;
FileLinesArray func_name;

int function_number = 0;
bool line_upd = false, cf = false, inc_upd = false, clean_flag = false, one_mark = false;
char temp_str[10];
int braces_num = 0, in_level = 0, class_braces_num[10];
unsigned int line_num = 0, i_num = 0, last_line_num = 0;
int func_number = 1;
int num_nspace = 0, num_class = 0, num_enum = 0, num_equal = 0, num_struct = 0, num_elements = 0, num_extern = 0;

int main(int argc, char* argv[])
{
 for (unsigned int k = 0; k < 10; k++)
  {
   class_braces_num[k] = 0;
  }
int i = 0;
std::string::iterator is;
 
 if (argc != 1) 
  {
   if (!strncmp(argv[1], "help", 4))
    {
     std::cout << "Possible parameters:" << std::endl;
     std::cout << "1. mask=<extension 1>,<extension 2>,...,<extension n> <Directory 1> <Directory 2> ... <Directory N>" << std::endl;
     std::cout << "Example:" << std::endl;
     std::cout << "./Parser mask=cpp,hpp projects/Ad/Server2/ChannelSvcs/ChannelManager projects/UnixCommons/src/Generics" << std::endl;
     std::cout << "It implements calls to profiler services into *.cpp and *.hpp files in " 
    		  "projects/Ad/Server2/ChannelSvcs/ChannelManager and projects/UnixCommons/src/Generics directories." << std::endl;
     std::cout << "In addition, it creates a file 'funclist' with names of profiled functions in the current directory" << std::endl;
		  
     std::cout << "2. func=<number of profiled function> <logfile>" << std::endl;
     std::cout << "Example:" << std::endl;
     std::cout << "./Parser func=5 ChannelManager.log" << std::endl;
     std::cout << "It creates a file 'Func_5.log' which contains some information about calls of the function with number 5 in the current directory." << std::endl;
     std::cout << "IMPORTANT: File 'funclist' must be in the current directory." << std::endl;

     std::cout << "3. clean=<extension 1>,<extension 2>,...,<extension n> <Directory 1> <Directory 2> ... <Directory N>" << std::endl;
     std::cout << "Example:" << std::endl;
     std::cout << "./Parser clean=cpp,hpp projects/Ad/Server2/ChannelSvcs/ChannelManager projects/UnixCommons/src/Generics" << std::endl;
     std::cout << "It deletes calls to profiler services from *.cpp and *.hpp files in " 
    		  "projects/Ad/Server2/ChannelSvcs/ChannelManager and projects/UnixCommons/src/Generics directories." << std::endl;
     
     std::cout << "4. help" << std::endl;
     
    } else
  
   if (!strncmp(argv[1], "mask=", 5) || (!strncmp(argv[1], "clean=", 6)))
    {
     if (!strncmp(argv[1], "mask=", 5))
      {
       file_masks = argv[1] + 5;
       clean_flag = false;
      } else
      {
       file_masks = argv[1] + 6;
       clean_flag = true;
      }
      
     is = file_masks.begin();

      while (file_masks.find(",") != std::string::npos)
       {
        file_mask[i] = file_masks.substr(0, file_masks.find(","));
	file_masks = file_masks.substr(file_masks.find(",") + 1);
	i++;
       }
     file_mask[i] = file_masks;
     mask_par = i + 1;
      
     func_list.open("funclist", std::ios::out);

      for (i = 2; i < argc; i++)
       {
        ParseFiles(argv[i]);
       }

     func_list.close();
    } else

   if (!strncmp(argv[1],"func=",5) || !strncmp(argv[1],"main",4))  
    {
     char* function_number_string;
     FILE* log_list;
     typedef std::vector<unsigned int> int_array;
     int_array indexes_array, main_indexes_array;

     if (!strncmp(argv[1],"func=",5))
     {
       function_number_string = &argv[1][5];
       function_number = atoi(function_number_string);
     }

     cf = false;
     log_list = fopen(argv[2], "r");
     ifunc_list.open("funclist", std::ios::in);

     if (!strncmp(argv[1],"main",4))
     {
       fseek(log_list, 0, SEEK_END);
       long log_filesize = ftell(log_list);
       fseek(log_list, 0, SEEK_SET);
       void* plog_list = malloc(log_filesize);
       fread(plog_list, PROF_FUNCTIONS, sizeof(_funcprof), log_list);

       for (unsigned int i = 1; i <= PROF_FUNCTIONS; i++)
       {
         _funcprof* _pfuncprof = (_funcprof*)((unsigned long)plog_list + i * sizeof(_funcprof));
         if (_pfuncprof->main_function == 1)
         {
           main_indexes_array.push_back(i);
         }
       }
     } else
     {
       main_indexes_array.push_back(function_number);
     }

     if (log_list != 0)
     {
       func_name.push_back("");
       while (ifunc_list && !ifunc_list.eof())
       {
         line_num++;
         temp_file_line.erase(); 
         std::getline(ifunc_list, temp_file_line);
         is = temp_file_line.begin();
         while ((*is != ' ') && (is != temp_file_line.end()))
         {
           is++;
         }
         if ((*is == ' ') && (is != temp_file_line.end()))
	 {
           is++;
         }
         func_name.push_back(std::string(temp_file_line, is - temp_file_line.begin(), std::string::npos));
       }

       for (unsigned int n = 0; n < main_indexes_array.size(); n++)
       {
         function_number = main_indexes_array[n];
         indexes_array.clear();

         snprintf(temp_str, sizeof(temp_str), "%i", function_number);
         std::string log_name = temp_str;
         std::string dot_name = temp_str;

         log_name = "Func_" + log_name + ".log";
         dot_name = "Func_" + dot_name + ".dot";
      
         std::ofstream log_out(log_name.c_str(), std::ios::out);
         std::ofstream dot_out(dot_name.c_str(), std::ios::out);      
     
         dot_out << "digraph FuncLog {" << std::endl 
                 << "rankdir = LR;" << std::endl 
                 << "node [color = red, fontsize = 14];" << std::endl
                 << "edge [color = black, fontcolor = darkgrey, fontsize = 12];" << std::endl;
      
         indexes_array.push_back(function_number);
         bool bf = true;
         unsigned int func_index = 0;

         for (unsigned int k = 0; k < PROF_FUNCTIONS; k++)
         {
           func_saved[k] = 0;
         }

         while (bf)
         {
           function_number = indexes_array[func_index];       

           fseek(log_list, function_number * sizeof(_funcprof), 0);
           fread(&FuncProf, 1, sizeof(_funcprof), log_list);

           if (func_saved[function_number] == 0)
           {
             func_calls = FuncProf.number_of_calls;
             SaveFunctionLog(function_number, &log_out, &dot_out);
             log_out << ">----------------------------------------------------------------------------------------------------------------------------------------" << std::endl;
        
             if (FuncProf.function_graph)
             {
               fseek(log_list, FuncProf.function_graph, SEEK_SET);
               fread(&func_graph, 1, PROF_FUNCTIONS * sizeof(unsigned int), log_list);
        
               log_out << "Called Functions:" << std::endl;
        
               cf = true;
               for (unsigned int i = 1; i < PROF_FUNCTIONS; i++)
               {
                 if (func_graph[i] != 0)
                 {
                   fseek(log_list, i * sizeof(_funcprof), 0);
                   fread(&FuncProf, 1, sizeof(_funcprof), log_list);
		   func_calls = func_graph[i];
                   indexes_array.push_back(i);
                   SaveFunctionLog(i, &log_out, &dot_out);
                 }
               }
	     }
             func_saved[function_number] = 1;	     
             cf = false;
             log_out << "<----------------------------------------------------------------------------------------------------------------------------------------" << std::endl;        
           }
	 
           ++func_index;

           if (func_index == indexes_array.size())
           {
             bf = false;
           }
         }
         ifunc_list.close();
         log_out.close();
         dot_out << "}" << std::endl;
         dot_out.close();
       }
       fclose(log_list);       
     }	                //loglist
    } 			//strncmp
  } 			//argc
}

void SaveFunctionLog(unsigned int func_number, std::ofstream* _log_out, std::ofstream* _dot_out)
{
  int dec = 0, sign = 0;
  double fraction = 0;
  double int_part;
  std::string time_buff, own_time_buff;
  long double aver_sec;
  long double aver_nsec;
  double graph_time_sec, graph_time_msec, log_time_sec, temp_log_time_sec;
  double func_time_sec;
  unsigned int temp_func_calls = 0;
 
  double own_time_sec =
    double(FuncProf.tm.tv_sec - FuncProf.child_tm.tv_sec) +
    double(FuncProf.tm.tv_nsec - FuncProf.child_tm.tv_nsec) / 1E9;

  own_time_buff = fcvt(own_time_sec, 6, &dec, &sign);
  while (dec < 1)
  {
    own_time_buff = "0" + own_time_buff;     
    ++dec;
  }
  own_time_buff.insert(dec, ".");

  FuncProf.tm.tv_nsec = FuncProf.tm.tv_nsec / 1000;
  temp_func_calls = FuncProf.number_of_calls;
  temp_log_time_sec = double(FuncProf.tm.tv_sec) + double(FuncProf.tm.tv_nsec) / 1000000;
    
  if (cf)
  {
    graph_time_sec  = double(FuncProf.tm.tv_sec) / FuncProf.number_of_calls;
    graph_time_sec = graph_time_sec * func_calls;
    graph_time_msec = double(FuncProf.tm.tv_nsec) / FuncProf.number_of_calls;
    graph_time_msec = (graph_time_msec / 1000000) * func_calls;
    graph_time_msec = modf(graph_time_sec + graph_time_msec, &int_part);
    func_time_sec = 1000000 * graph_time_msec;
    FuncProf.tm.tv_nsec = int(func_time_sec);
    FuncProf.tm.tv_sec  = int(int_part);
      
    log_time_sec = double(FuncProf.tm.tv_sec) + graph_time_msec;

    time_buff = fcvt(log_time_sec, 6, &dec, &sign);
    while (dec < 1)
    {
      time_buff = "0" + time_buff;     
      ++dec;
    }
    time_buff.insert(dec, ".");
     
    *_dot_out << function_number << "->" << func_number << "[label = \x22" << func_calls << " calls\x22];" << std::endl;
      
    *_log_out << "	";
    FuncProf.number_of_calls = func_calls;      
  }
  else
  {
    log_time_sec = double(FuncProf.tm.tv_sec) + double(double(FuncProf.tm.tv_nsec) / 1000000);
    func_time_sec = FuncProf.tm.tv_nsec;
  }
    
  *_log_out << func_name[func_number] << std::endl;
  temp_file_line.erase();

  temp_val << FuncProf.function_index;
  temp_file_line = temp_file_line + temp_val.str() + spaces;
  temp_file_line = std::string(temp_file_line, 0, 13);    
  temp_val.str("");
    
  temp_val << FuncProf.number_of_calls;    
  temp_file_line = temp_file_line + temp_val.str() + spaces;    
  temp_file_line = std::string(temp_file_line, 0, 26);
  temp_val.str("");    

  temp_val << FuncProf.tm.tv_sec;    
  temp_file_line = temp_file_line + temp_val.str() + spaces;    
  temp_file_line = std::string(temp_file_line, 0, 39);
  temp_val.str("");

  temp_val << func_time_sec;

  temp_file_line = temp_file_line + temp_val.str() + spaces;    
  temp_file_line = std::string(temp_file_line, 0, 55);
  temp_val.str("");
    
  aver_sec  = double(FuncProf.tm.tv_sec) / FuncProf.number_of_calls;

  aver_nsec = func_time_sec / FuncProf.number_of_calls;     
    
  fraction = modf(aver_sec, &int_part);

  if ((int_part == 0) && (fraction < 0.0005))
  {
    time_buff = fcvt(1000000*fraction + aver_nsec, 10, &dec, &sign);
    time_buff += " microseconds";		 
  }
  else
  {
    time_buff = fcvt(int_part + fraction + aver_nsec/1000000, 10, &dec, &sign);
    time_buff += " seconds";		 
  }

  while (dec < 1)
  {
    time_buff = "0" + time_buff;     
    ++dec;
  }
  time_buff.insert(dec, ".");

  temp_file_line = temp_file_line + time_buff + spaces;    
  temp_val.str("");
    
  if (cf)
  {
    *_log_out << "\t";
  }

  *_log_out << "Index    |   " << "Calls    |   " << "Seconds  |   " << "Microseconds |  " << "Average time in seconds/microseconds" << std::endl;

  if (cf)
  {
    *_log_out << "\t";
  }

  *_log_out << temp_file_line << std::endl << std::endl;
    
  time_buff = fcvt(temp_log_time_sec, 6, &dec, &sign);
  while (dec < 1)
  {
    time_buff = "0" + time_buff;     
    ++dec;
  }
  time_buff.insert(dec, ".");
    
  *_dot_out << func_number << "[shape = rectangle, style = filled, fillcolor = lightgrey, label = \x22" 
	    << func_name[func_number].substr(0, func_name[func_number].find("(")) 
            << "\x5c\x6e" << temp_func_calls << " calls, " << time_buff << " seconds"
            << "\x5c\x6e" << "own time " << own_time_buff << "seconds"
            << "\x22];" << std::endl;
}

void ParseFiles(const char* current_dir_name)
{
 std::string full_file_name;
 std::string directory_name;
 std::string next_dir_name;
 struct dirent* direntry; 
 struct stat fileentry;
 DIR *cur_dir;
 unsigned int ii = 0;

  cur_dir = opendir(current_dir_name);
   if (cur_dir)
    {
     directory_name = current_dir_name; 

      while ((direntry = readdir(cur_dir)) != 0)
       { 
        full_file_name = directory_name + "/" + direntry->d_name;
        next_dir_name = direntry->d_name;
        stat(full_file_name.c_str(), &fileentry);
         if ((next_dir_name != ".") && (next_dir_name != "..") 
	 && (next_dir_name != "Profiler.cpp") && (next_dir_name != "Parser.cpp")
	 && (next_dir_name != "Profiler.hpp") && (next_dir_name != "Parser.hpp"))     
          {
           if (S_ISDIR(fileentry.st_mode))
            {
    	     ParseFiles(full_file_name.c_str());
            } else
            {
	     for (ii = 0; ii < mask_par; ii++)
	      {
               if (full_file_name.substr(full_file_name.length() - file_mask[ii].length()) == file_mask[ii])
                {
                 std::cout << full_file_name << std::endl;
  	         UpdateFile(full_file_name.c_str());
                 break;
                }
              }
           }
         }
       }
     } else
     {
      std::cout << "Cannot find directory " << current_dir_name << std::endl;
     }
}
 
void UpdateFile(const char* file_name)
{
bool old_profiler_hpp = false, next_prof_str = false;
typedef std::vector<std::string> FileLinesArray;
FileLinesArray file_line;
std::string temp_file_line;
std::string func_name_line;
std::string::iterator is;
unsigned int i = 0;

 i_num = 0;
 line_num = 0;
 inc_upd = false;
 last_line_num = 0;
 file_line.clear();
 braces_num = 0;
 std::ifstream fp(file_name, std::ios::in);
 std::ofstream fp_out("hren.cpp", std::ios::out);

  if (fp && fp.is_open())
   {
     while (fp && !fp.eof())
      {
       line_num++;
       temp_file_line.erase(); 
       std::getline(fp,temp_file_line);
       is = temp_file_line.begin();
        
        while ((is != temp_file_line.end()) && (*is == ' ' || *is == '\t')) 
	 {
          is++;
	 }

	if (temp_file_line.substr(0, init_str.length() - 1) == init_str.substr(1, init_str.length() - 1)) //Profiling Prof...
	 {
	  temp_file_line.erase();
	  next_prof_str = false;
	 } else
        if (next_prof_str)
         {
          fp_out << std::endl;
         } else
	 {
	  next_prof_str = true;
	 }
	
	if ( (is == temp_file_line.end()) || ((*is == '/') && (*(is+1) == '/')) )
	 {
	  fp_out << temp_file_line;   	  
	  continue;
	 }

        if ( (temp_file_line.find("#define ") != std::string::npos) && (!clean_flag) )
        {
          std::string::iterator def_is;
          bool def_ended = false;

          while (!def_ended)
          {
            def_is = temp_file_line.end() - 1;
            while ( ((*def_is == ' ') || (*def_is == '\t')) && (def_is != temp_file_line.begin()) )
            {
              def_is--;
            }
            fp_out << temp_file_line;
            if (*def_is != '\x5c')
            {
              def_ended = true;
            } else
            {
              fp_out << std::endl;
              std::getline(fp,temp_file_line);
            }
          }
          continue;
        }
 
        
	
	if (temp_file_line.find("/*") != std::string::npos)
	 {
	   if (temp_file_line.find("*/") == std::string::npos)
	    {
	     next_prof_str = false;							//trick for searching "*/"
	     fp_out << temp_file_line << std::endl;;	     
	    } else
	    {
	     next_prof_str = true;
  	     fp_out << temp_file_line;	     
	    }
	  
 	   while (!next_prof_str)
	    {
             std::getline(fp,temp_file_line);
	      if (temp_file_line.find("*/") == std::string::npos)
	       {
  	        fp_out << temp_file_line << std::endl;
	       } else
	       {
	        next_prof_str = true;
 	        fp_out << temp_file_line;		
	       }
	    }
	  continue;
	 }
	
        if (temp_file_line == "#ifndef _PROFILER_H")
	 {
	  old_profiler_hpp = true;
	   if (clean_flag)
	    {
	     for (unsigned int k = 0; k < 30; k++)
	      {
	       std::getline(fp,temp_file_line);	       
	       next_prof_str = false;
	      }
	     continue;
	    }
	 }
	
       file_line.push_back(std::string(temp_file_line, is - temp_file_line.begin(), std::string::npos));
       line_upd = false;

	for (i = 0; i < 10; i++)
	 {
  	  temp_pos[i] = std::string::npos;
	 }

       i = 0; 
       temp_pos[i] = temp_file_line.find("\x22");

	while (temp_pos[i] != std::string::npos)
	 {
	  one_mark = true - one_mark;
	  ++i;
	  temp_pos[i] = temp_file_line.find("\x22", temp_pos[i - 1] + 1);
	 }

        if (!clean_flag)
	 {
          ParseLine(&temp_file_line, &fp_out);
	 }
	
        if ((!clean_flag) && (temp_file_line.substr(0, 9) == "#include ") && (!inc_upd) && (!old_profiler_hpp))
	 {
	  inc_upd = true;
	  fp_out << profiler_hpp;
	 }

        if (line_upd)
	 {
	  line_upd = false;
	  ++last_line_num;
	  func_name_line.erase();

 	   for (unsigned int i = last_line_num; i < i_num; i++)
	    {
	     if (file_line[i].find("#") == std::string::npos)
	      {
               func_name_line += file_line[i] + " ";
	      }
	    }
	   func_name_line += file_line[i_num].substr(0, file_line[i_num].find("{"));

	  last_line_num = i_num;
	  func_list << func_number - 1 << " " << func_name_line << std::endl;
	  fp_out << temp_file_line;      	  
	 } else
	 {
	  if (!temp_file_line.empty())
	   {
 	    fp_out << temp_file_line;      
	   }
	 }
       i_num++;
      }
      
    fp.close();
    fp_out.close();
    rename("hren.cpp", file_name);
   } 
}

void ParseLine(std::string* _file_line, std::ofstream* /*_fp_out*/)
{
 SearchText(_file_line, "namespace", &num_nspace);
 SearchText(_file_line, "class", &num_class); 
 SearchText(_file_line, "enum", &num_enum);
 SearchText(_file_line, "struct", &num_struct); 
 SearchText(_file_line, "extern", &num_extern); 
 
 SearchBrace(_file_line);

 SearchEqualSign(_file_line, &num_equal); 
}


void SearchEqualSign(std::string* __pfile_line, int* __numtext)
{
std::string::iterator i = __pfile_line->end();

 while ( (*i != '=') && (i != __pfile_line->begin()) )
  {
   if ( (*i == ' ') || (*i == '\t') || (*i == '\n') || (*i == '\r') || (*i == '\0') )
    {
     i--;
    } else break;
  }
  
 if ( (i != __pfile_line->begin()) && (*i == '='))
  {
   *__numtext = 1;
  } else *__numtext = 0;
}
  
void SearchText(std::string* __pfile_line, std::string __mytext, int* __numtext)
{
std::string::size_type pos;
bool __one_mark = false;
unsigned int _i = 0;

 pos = __pfile_line->rfind(__mytext);

  if (pos != std::string::npos)							// __mytext SomeName
   {
    while ( (_i < 50) && (temp_pos[2 * _i] != std::string::npos) )
     {
      if ( ((pos > temp_pos[2 * _i]) && (pos < temp_pos[((2 * _i) + 1)])) || (temp_pos[((2 * _i) + 1)] == std::string::npos) )
       {
        __one_mark = true;
       }
      ++_i;
     }
    
    if (!__one_mark)
     {
      if ( (pos > __pfile_line->find("//")) && (__pfile_line->find("//") != std::string::npos) )
       {
        *__numtext = -1;
       } else
      if (pos != 0)
       {
        char text_sym = *(__pfile_line->begin() + pos - 1);
         if ((text_sym != ' ') && (text_sym != '\t') && (text_sym != ';'))
          {
  	   *__numtext = -1;
          }
       } else
       {
	char end_text_sym = *(__pfile_line->begin() + pos + __mytext.length()); 
         if ((end_text_sym != ' ') && (end_text_sym != '\t') && (end_text_sym != '\x0'))
          {
  	   *__numtext = -1;
          }
       }
      
      std::string::size_type brack_pos  = __pfile_line->find("<");
      std::string::size_type brack1_pos = __pfile_line->find(">");      
      if ( ((brack_pos != std::string::npos) && (pos > brack_pos)) && 
           ((brack1_pos == std::string::npos) || ((brack1_pos != std::string::npos) && (pos < brack1_pos))) )
       {
        *__numtext = -1;
       }
      ++*__numtext;
     }
   }

  if ( (__pfile_line->find(";") != std::string::npos) && 
     !((__pfile_line->find("{") != std::string::npos) && (__pfile_line->find("}") != std::string::npos)))
   {
    *__numtext = 0;
   }

  if ((__mytext == "extern") && (*__numtext != 0))
  {
    pos = __pfile_line->find('\x22');
    std::string::size_type extern_length = __pfile_line->length();
    if ((pos != std::string::npos) && (extern_length > pos + 3))
    {
      std::string::iterator extern_is = __pfile_line->begin() + pos + 3;
      while ( ((*extern_is == ' ') || (*extern_is == '\t')) && (extern_is != __pfile_line->end()) )
      {
        extern_is++;
      }
      if ((extern_is != __pfile_line->end()) && (*extern_is != '{'))
      {
        *__numtext = 0;
      }
    }
  }
}

void SearchBrace(std::string* __pfile_line)
{
std::string::size_type pos = 0;
std::string::iterator _it;
unsigned int _i = 0;
bool _one_mark = false;


 
 pos = __pfile_line->find("{");
 
  if (pos != std::string::npos)
   {
    _it = __pfile_line->begin() + pos;
    while ( (_i < 50) && (temp_pos[2 * _i] != std::string::npos) )
     {
      if ( ((pos > temp_pos[2 * _i]) && (pos < temp_pos[((2 * _i) + 1)])) || (temp_pos[((2 * _i) + 1)] == std::string::npos) )
       {
        _one_mark = true;
       }
      ++_i;
     }
 
    while ((_it != __pfile_line->begin()) && ((*_it == ' ') || (*_it == '\t') || (*_it == '{')))
     {
      --_it;
     }

    if (*_it == '=')
     {
      num_struct = 1; 
     }     
 
    if (!_one_mark)
     {
      if ((num_extern != 0) || (num_nspace != 0) || (num_class != 0) || (num_enum != 0) || (num_struct != 0))	// __mytext SomeName %s\n {
       {
         if (num_class != 0)
	  {
	   class_braces_num[in_level] = braces_num;
	   braces_num = 0;
	   ++in_level;
	  }
        last_line_num = i_num;
        num_nspace = 0;
        num_class = 0;
        num_enum = 0;
	num_struct = 0;
	num_extern = 0;
	++num_elements;
       } else 
      if ((braces_num == 0) && (num_equal == 0))
       {
        num_elements = 0;
        ++braces_num;
	line_upd = true;
        snprintf(temp_str, sizeof(temp_str), "%u", func_number);
        init_str1 = temp_str;      
        __pfile_line->insert(pos + 1, init_str + init_str1 + init_str2);	
        ++func_number;    
       } else 
       {
        num_elements = 0;       
        last_line_num = i_num;
        ++braces_num;
        num_equal = 0;
       }
     }
   }
 
 _i = 0;
 _one_mark = false;
 pos = __pfile_line->find("}");

  if (pos != std::string::npos)
   {
    while ( (_i < 50) && (temp_pos[2 * _i] != std::string::npos) )
     {
      if ( ((pos > temp_pos[2 * _i]) && (pos < temp_pos[((2 * _i) + 1)])) || (temp_pos[((2 * _i) + 1)] == std::string::npos) )
       {
        _one_mark = true;
       }
      ++_i;
     }
   
     if (!line_upd)
      {
       last_line_num = i_num;
      }

     while ((braces_num == 0) && (in_level != 0))
      {
       --in_level;
       braces_num = class_braces_num[in_level];
       class_braces_num[in_level] = 0;
       _one_mark = true;					// trick for decreasing of braces_num
     }

     if ((braces_num != 0) && (!_one_mark) && (!num_elements))
      {
       --braces_num;
      } else 

     if (num_elements != 0)
      {
       --num_elements = 0;
      }
   } else
  
   {
    pos = __pfile_line->find(";");
    if ((pos != std::string::npos) && (!line_upd))
     {
      last_line_num = i_num;
     }
   }
   
  
}  
  

