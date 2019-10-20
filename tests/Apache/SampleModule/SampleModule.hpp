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




#ifndef APACHE_SAMPLE_MODULE_HPP
#define APACHE_SAMPLE_MODULE_HPP

#include <ReferenceCounting/ReferenceCounting.hpp>

#include <Apache/Adapters.hpp>


class TestModule :
  public Apache::ConfigParser,
  public Apache::HandlerHook<TestModule>,
  public Apache::QuickHandlerAdapter<TestModule>,
  public Apache::ChildLifecycleAdapter<TestModule>,
  public ReferenceCounting::AtomicImpl
{
public:
  TestModule() throw (eh::Exception);

  typedef ReferenceCounting::QualPtr<TestModule> TestModule_var;
  static TestModule_var instance;

  virtual int
  handler(request_rec *r) throw ();
  virtual const char*
  handle_command(const ConfigArgs& args) throw ();

  virtual bool
  will_handle(const char* uri) throw ();
  virtual int
  handle_request(const Apache::HttpRequest& request,
    Apache::HttpResponse& response) throw ();

  virtual void
  init() throw ();
  virtual void
  shutdown() throw ();

protected:
  virtual
  ~TestModule() throw ();

private:
  int test_;
};

#endif // _SAMPLE_MODULE_HPP_
