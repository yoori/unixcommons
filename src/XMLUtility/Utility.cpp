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





#include <sstream>
#include <string>

#include <xercesc/util/PlatformUtils.hpp>

#include <eh/Exception.hpp>
#include <Sync/PosixLock.hpp>
#include <String/StringManip.hpp>
#include <Stream/MemoryStream.hpp>

#include <XMLUtility/Utility.hpp>


namespace XMLUtility
{
  typedef Sync::PosixMutex Mutex_;
  typedef Sync::PosixGuard Guard_;

  static Mutex_ lock_;
  static unsigned long init_counter_ = 0;

  void
  initialize() throw (Exception, eh::Exception)
  {
    Guard_ guard(lock_);

    if (!init_counter_)
    {
      try
      {
        XMLPlatformUtils::Initialize();
      }
      catch (const XMLException& e)
      {
        std::string msg;
        StringManip::xmlch_to_mbc(e.getMessage(), msg);

        Stream::Error ostr;
        ostr << "XMLUtility::initialize(): "
          "XMLException thrown by XMLPlatformUtils::Initialize():" << msg;
        throw Exception(ostr);
      }
    }

    init_counter_++;
  }

  void
  terminate() throw ()
  {
    Guard_ guard(lock_);

    if (init_counter_)
    {
      if (!--init_counter_)
      {
        try
        {
          XMLPlatformUtils::Terminate();
        }
        catch (const XMLException& e)
        {
        }
      }
    }
  }
}
