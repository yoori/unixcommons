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





#ifndef _TEST_PROCESS_CONTROL_APPLICATION_HPP_
#define _TEST_PROCESS_CONTROL_APPLICATION_HPP_

#include <eh/Exception.hpp>

#include <tao/ORB.h>
#include <tao/IORTable/IORTable.h>

#include <CORBACommons/ProcessControlImpl.hpp>


class Application
{
public:

  /**X
   * Macros defining Application base exception class.
   */
  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

  /**X
   * Macros defining InvalidArgument exception class.
   */
  DECLARE_EXCEPTION(InvalidArgument, Exception);

  /**X
   * Macros defining InvalidOperationOrder exception class.
   */
  DECLARE_EXCEPTION(InvalidOperationOrder, Exception);

public:

  /**X
   * Construct Application object.
   */
  Application() throw(Exception, eh::Exception);

  /**X
   * Destructs Application object.
   */
  virtual ~Application() throw();

/**X
 * Initalizes ORB, POA, creates and
 * registers CORBACommons::ProcessControlImpl servant object.
 * @param argc Number of arguments passed to test process
 * @param argv Arguments passed to test process
 */
  void init(int& argc, char** argv)
    throw(InvalidArgument, Exception, eh::Exception);

/**X
 * Runs ORB loop.
 */
  void run()
    throw(InvalidOperationOrder, Exception, eh::Exception);

/**X
 * Destroys POA, ORB, release all resources.
 */
  void destroy() throw(eh::Exception);


private:

  typedef Sync::PosixRWLock Mutex_;
  typedef Sync::PosixRGuard Read_Guard_;
  typedef Sync::PosixWGuard Write_Guard_;

  mutable Mutex_ lock_;

  CORBA::ORB_var orb_;
  PortableServer::POA_var server_poa_;
  CORBA::String_var process_control_name_;
  PortableServer::ObjectId_var process_control_id_;
  IORTable::Table_var ior_table_;
  PortableServer::ServantBase_var servant_;
};

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

#endif // _TEST_PROCESS_CONTROL_APPLICATION_HPP_
