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



// @file Server/ObjectPoolTestServer.cpp
#include <iostream>
#include <sstream>

#include <CORBACommons/ProcessControlImpl.hpp>
#include <CORBACommons/CorbaAdapters.hpp>
#include <Logger/StreamLogger.hpp>
#include <Generics/AppUtils.hpp>

#include "ObjectPoolTestServer.hpp"

namespace
{
  const char PROCESS_CONTROL_SERVANT[] = "ProcessControl";
  const char POOL_OBJ_INT_SERVANT[] = "PoolObj";
  const char EXT_POOL_OBJ_INT_SERVANT[] = "PoolObj";
}

Application::Application() throw (eh::Exception)
{
}

void
Application::create_names(std::size_t port, std::size_t count)
  throw (eh::Exception)
{
  std::size_t current_char = -1;
  std::string result;
  servants.clear();
  std::ofstream ost("./urls.txt");
  for (std::size_t i = 0; i < count; ++i)
  {
    int rest = i % ('z' - 'a' + 1);
    if (!rest)
    {
      result += 'a';
      ++current_char;
    }
    else
    {
      result[current_char] = 'a' + rest;
    }

    ost << "corbaloc::localhost:" << port << '/'
      << result << "\n";
    servants.push_back(result);
  }
  result = "UpOnline";
  servants.push_back(result);
  ost << "corbaloc::localhost:" << port << '/'
    << result << "\n";
}

void
Application::run(int argc, char* argv[], std::size_t before_up)
  throw (Exception, eh::Exception)
{
  try
  {
    Generics::AppUtils::Option<unsigned long> objects_count;
    Generics::AppUtils::Option<unsigned long> opt_port;
    Generics::AppUtils::Option<std::string> opt_host("localhost");
    Generics::AppUtils::Args args;

    args.add(
      Generics::AppUtils::equal_name("port") ||
      Generics::AppUtils::short_name("p"),
      opt_port);
    args.add(
      Generics::AppUtils::equal_name("host") ||
      Generics::AppUtils::short_name("h"),
      opt_host);
    args.add(
      Generics::AppUtils::equal_name("objects_amount") ||
      Generics::AppUtils::short_name("o"),
      objects_count);

    args.parse(argc - 1, argv + 1);

    CORBACommons::CorbaConfig corba_config;
    corba_config.thread_pool = 10;

    if (opt_port.installed())
    {
      unsigned long port = *opt_port;
      if (objects_count.installed())
      {
        create_names(port, *objects_count);
      }
      else
      {
        create_names(port);
      }
      ObjectNames ext_servants = servants;

      CORBACommons::EndpointConfig endpoint_config;
      endpoint_config.host = *opt_host;
      endpoint_config.port = *opt_port;
      for (std::size_t i = 0; i < servants.size() - before_up; ++i)
      {
        endpoint_config.objects[servants[i]].insert(ext_servants[i]);
      }
      endpoint_config.objects[POOL_OBJ_INT_SERVANT].insert(
        EXT_POOL_OBJ_INT_SERVANT);

      endpoint_config.objects[PROCESS_CONTROL_SERVANT].insert(
        PROCESS_CONTROL_SERVANT);
      corba_config.endpoints.push_back(endpoint_config);
    }

    Logging::FLogger_var logger(
      new Logging::OStream::Logger(Logging::OStream::Config(std::cout)));

    CORBACommons::CorbaServerAdapter_var corba_server_adapter(
      new CORBACommons::CorbaServerAdapter(corba_config, logger));

    CORBATest::TestObjectPoolImpl_var tester[servants.size()];

    for (std::size_t i = 0; i < servants.size() - before_up; ++i)
    {
      tester[i] = new CORBATest::TestObjectPoolImpl;
      corba_server_adapter->add_binding(servants[i].c_str(), tester[i]);
    }
    CORBATest::PoolObjectImpl_var pool_obj(new CORBATest::PoolObjectImpl);

    corba_server_adapter->add_binding(POOL_OBJ_INT_SERVANT, pool_obj);

    corba_server_adapter->add_binding(PROCESS_CONTROL_SERVANT, this);
    ProcessControlImpl::shutdowner_ =
      corba_server_adapter->shutdowner();
    shuter = ProcessControlImpl::shutdowner_;

    std::cout << (before_up ? "First server up" : "Second server started")
      << std::endl;

    corba_server_adapter->run();
    shutdowner_.reset();
    shuter.reset();
  }
  catch (const CORBA::Exception& e)
  {
    std::ostringstream ostr;
    ostr << "Application::run: CORBA::Exception caught. Description:\n"
         << e;

    throw Exception(ostr.str());
  }
}

CORBACommons::OrbShutdowner_var Application::shuter;

int
main(int argc, char** argv)
{
  try
  {
    ReferenceCounting::QualPtr<Application> app(new Application);

    app->run(argc, argv, 1);
    app->run(argc, argv, 0);

    return 0;
  }
  catch (const eh::Exception& e)
  {
    std::cerr
      << "main: eh::Exception exception caught. Description:" << e.what()
      << std::endl;
  }
  catch (...)
  {
    std::cerr << "main: unknown exception caught" << std::endl;
  }

  return 1;
}

volatile _Atomic_word CORBATest::TestObjectPoolImpl::stat_counter_ = 0;
