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

#include "SampleModule.hpp"

TestModule::TestModule() throw (eh::Exception)
  : Apache::HandlerHook<TestModule>(APR_HOOK_MIDDLE),
    Apache::QuickHandlerAdapter<TestModule>(APR_HOOK_MIDDLE),
    Apache::ChildLifecycleAdapter<TestModule>(APR_HOOK_MIDDLE),
    test_(10)
{
  std::cerr << "In TestModule::TestModule().\n";

  add_directive("TestVoid", OR_OPTIONS, NO_ARGS, "TestVoid");
  add_directive("TestFlag", OR_OPTIONS, FLAG, "TestFlag");
  add_directive("TestTake12", OR_OPTIONS, TAKE12, "TestTake12");
}

TestModule::~TestModule() throw ()
{
}

int
TestModule::handler(request_rec* r) throw ()
{
  ap_rprintf(r, "You requested %s", r->uri);
  return OK;
}

const char*
TestModule::handle_command(const ConfigArgs& args) throw ()
{
  try
  {
    std::cerr << "In TestModule::handle_command().\n";
  }
  catch (...)
  {
  }

  test_ = 20;

  if (!strcmp(args.name(), "TestVoid"))
  {
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0,
                 args.command()->server, "Handling TestVoid.");
  }
  else if (!strcmp(args.name(), "TestFlag"))
  {
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, args.command()->server,
                 "Handling TestFlag.");

    try
    {
      ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, args.command()->server,
                   "  TestFlag parameter: %d.", args.flag());
    }
    catch (const Apache::ConfigParser::ConfigArgs::ArgNotExist& )
    {
      ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, args.command()->server,
                   "  TestFlag parameter failure.");
    }
  }
  else if (!strcmp(args.name(), "TestTake12"))
  {
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, args.command()->server,
                 "Handling TestTake12.");

    try
    {
      ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, args.command()->server,
                   "  TestTake12 parameter 1: %s.", args.str1());
    }
    catch (const Apache::ConfigParser::ConfigArgs::ArgNotExist& )
    {
      ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, args.command()->server,
                   "  TestTake12 parameter 1 failure.");
    }

    try
    {
      const char* param2 = args.str2();
      if (param2)
      {
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, args.command()->server,
                     "  TestTake12 parameter 2: %s.", param2);
      }
    }
    catch (const Apache::ConfigParser::ConfigArgs::ArgNotExist& )
    {
      ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, args.command()->server,
                   "  TestTake12 parameter 2 failure.");
    }
  }
  else
  {
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, args.command()->server,
                 "Unknown directive: %s.", args.name());
  }

  return 0;
}

bool
TestModule::will_handle(const char*) throw ()
{
  return true;
}

int
TestModule::handle_request(const Apache::HttpRequest& request,
  Apache::HttpResponse& response) throw ()
{
  try
  {
    std::cerr << "Value of test is " << test_ << std::endl;

    int method = request.method();
    if (method == M_GET || method == M_POST)
    {
      Stream::BinaryStreamWriter out(&(response.get_output_stream()));
      out << request.uri() << std::endl;

      out << "Headers: " << std::endl;
      for (HTTP::SubHeaderList::const_iterator it =
        request.headers().begin(); it != request.headers().end(); ++it)
      {
        out << "  " << it->name << ": " << it->value << std::endl;
      }

      out.flush();
    }
  }
  catch (...)
  {
    return DECLINED;
  }

  return OK;
}

void
TestModule::init() throw ()
{
  try
  {
    std::cerr << "TestModule::init()" << std::endl;
  }
  catch (...)
  {
  }
}

void
TestModule::shutdown() throw ()
{
  try
  {
    std::cerr << "TestModule::shutdown()" << std::endl;
  }
  catch (...)
  {
  }
}

TestModule::TestModule_var TestModule::instance(new TestModule);

Apache::ModuleDef<TestModule> test_module;
