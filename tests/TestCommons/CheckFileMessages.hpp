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



#ifndef TESTCOMMONS_CHECKFILEMESSAGES
#define TESTCOMMONS_CHECKFILEMESSAGES

#include <vector>
#include <string>
#include <fstream>

#include <time.h>

#include <eh/Exception.hpp>

#include <Generics/Time.hpp>
#include <Generics/DirSelector.hpp>


namespace TestCommons
{
  class CheckFileMessages
  {
  public:

    DECLARE_EXCEPTION(CheckException, eh::DescriptiveException);

    void
    add_message() throw (eh::Exception);

    void
    check(std::string file, int size_span, int time_span)
      throw (eh::Exception, CheckException);

  private:
    class FileNameComparer
    {
    public:
      FileNameComparer(const std::string& common) throw (eh::Exception);

      bool
      operator ()(const std::string& left, const std::string& right)
        throw ();

    private:
      std::string common_;
    };

    typedef std::vector<time_t> Timestamps;

    static const time_t max_delay_;

    Timestamps timestamps_;
  };
}

#endif
