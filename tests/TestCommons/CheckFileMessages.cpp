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
#include <fstream>
#include <assert.h>

#include <TestCommons/CheckFileMessages.hpp>


namespace TestCommons
{
  const time_t CheckFileMessages::max_delay_ = 10;

  void CheckFileMessages::add_message() throw (eh::Exception)
  {
    timestamps_.push_back(time(0));
  }

  void CheckFileMessages::check(std::string file, int size_span, int time_span)
    throw (eh::Exception, CheckException)
  {
    std::cout << "Checking..." << std::endl;

    typedef std::vector<std::string> Files;
    Files files;

    if (!file.length() || file[0] != '/')
    {
      file = "./" + file;
    }
    const char* ptr = strrchr(file.c_str(), '/');
    assert(ptr);
    ptr++;
    std::string path(file.c_str(), ptr);
    std::string mask(ptr);
    mask.append("*");

    Generics::DirSelect::directory_selector(path.c_str(),
      Generics::DirSelect::list_creator(std::back_inserter(files)),
      mask.c_str());

    std::sort(files.begin(), files.end(), FileNameComparer(file));

    std::size_t msg = 0;
    time_t time_upper = 0, time_lower = 0, time_middle = 0;
    time_t last = 0;
    for (Files::iterator itor(files.begin()); itor != files.end(); ++itor)
    {
      std::cout << "Processing " << *itor << std::endl;
      struct stat st;
      if (stat(itor->c_str(), &st) < 0)
      {
        throw CheckException("Failed to stat");
      }
      if (size_span && st.st_size > size_span + 1024)
      {
        throw CheckException("Too great size");
      }
      time_lower = time_upper;
      if (*itor != file)
      {
        int year, month, day, hour, min, sec, usec;
        if (sscanf(itor->c_str() + file.length(), ".%4d%2d%2d.%2d%2d%2d%6d",
          &year, &month, &day, &hour, &min, &sec, &usec) != 7)
        {
          throw CheckException("Invalid file name format");
        }
        Generics::ExtendedTime time(year, month, day, hour, min, sec, usec);
        time_upper = time.operator Generics::Time().tv_sec;
      }
      else
      {
        time_upper = time(0);
      }
      time_middle = time_lower && time_span ? time_lower + time_span : time_upper;
      std::ifstream in(itor->c_str());
      if (!in)
      {
        throw CheckException("Failed to open");
      }
      std::string line;
      while (std::getline(in, line))
      {
        Generics::Time time;
        time.set(line, "%a %d %b %Y %H:%M:%S");
        time_t sec = time.tv_sec;
        if (sec < timestamps_[msg] ||
          sec > timestamps_[msg] + max_delay_)
        {
          throw CheckException("Invalid time of message");
        }
        if (sec < last)
        {
          throw CheckException("Invalid time sequence of messages");
        }
        last = sec;
        if (sec + max_delay_ < time_lower || sec > time_middle)
        {
          throw CheckException("Incorrect file for the message");
        }
        const char* ptr = strstr(line.c_str(), "message [");
        if (!ptr)
        {
          throw CheckException("Invalid format of message");
        }
        std::size_t num = atoi(ptr + 9);
        if (num != msg)
        {
          throw CheckException("Invalid number of message (unexpected)");
        }
        if (msg > timestamps_.size())
        {
          throw CheckException("Invalid number of message (too great)");
        }
        msg++;
        if (!(msg % 1000))
        {
          std::cout << msg << " messages" << std::endl;
        }
      }
    }
    if (msg != timestamps_.size())
    {
      throw CheckException("Invalid number of messages (different)");
    }
  }

  CheckFileMessages::FileNameComparer::FileNameComparer(
    const std::string& common) throw (eh::Exception)
    : common_(common)
  {
  }

  bool
  CheckFileMessages::FileNameComparer::operator ()(
    const std::string& left, const std::string& right) throw ()
  {
    return left == common_ ? false : right == common_ ? true : left < right;
  }
}
