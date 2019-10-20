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





#ifndef _PARSER_H_
#define _PARSER_H_

void ParseFiles(const char *dir_name);
void UpdateFile(const char *file_name);
void ParseLine(std::string* _file_line, std::ofstream* fp_out);
void SearchText(std::string* __file_line, std::string __mytext, int* __numtext);
void SearchBrace(std::string* __pfile_line);
void SearchEqualSign(std::string* __file_line, int* __numtext);
void SaveFunctionLog(unsigned int func_number, std::ofstream* _log_out, std::ofstream* _dot_out);

#endif
