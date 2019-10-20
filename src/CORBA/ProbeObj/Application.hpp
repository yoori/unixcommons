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





#ifndef CORBA_PROBE_OBJ_APPLICATION_HPP
#define CORBA_PROBE_OBJ_APPLICATION_HPP

#include <eh/Exception.hpp>

#include <CORBACommons/CorbaAdapters.hpp>


/**
 * Class which tests and shutdowns CORBA server processes by means of
 * CORBACommons::IProcessControl interface usage.
 */
class Application
{
public:
  /**
   * Application base exception class.
   */
  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
  /**
   * Macros defining InvalidArgument exception class.
   */
  DECLARE_EXCEPTION(InvalidArgument, Exception);
  /**
   * Macros defining InvalidReference exception class.
   */
  DECLARE_EXCEPTION(InvalidReference, Exception);

public:
  /**
   * Tests or shutdowns CORBA server processes.
   * @param argc Number of arguments passed to utility process
   * @param argv Arguments passed to utility process
   */
  int
  run(int& argc, char** argv)
    throw (InvalidArgument, InvalidReference, Exception, eh::Exception,
      CORBA::Exception);

private:
  int
  shutdown_(int argc, char** argv)
    throw (InvalidArgument, InvalidReference, Exception, eh::Exception,
      CORBA::Exception);

  int
  probe_(int argc, char** argv)
    throw (InvalidArgument, InvalidReference, Exception, eh::Exception,
      CORBA::Exception);

  int
  status_(int argc, char** argv)
    throw (InvalidArgument, InvalidReference, Exception, eh::Exception,
      CORBA::Exception);

  int
  control_(int argc, char** argv)
    throw (InvalidArgument, InvalidReference, Exception, eh::Exception,
      CORBA::Exception);


  Logging::Logger_var logger_;
  CORBACommons::CorbaClientAdapter_var adapter_;
};

#endif
