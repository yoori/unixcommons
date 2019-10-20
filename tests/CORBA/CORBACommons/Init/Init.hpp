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



#ifndef CORBACOMMONS_COMBINED_HPP
#define CORBACOMMONS_COMBINED_HPP

#include <Generics/AppUtils.hpp>
#include <String/StringManip.hpp>
#include <CORBACommons/CorbaAdapters.hpp>
#include <CORBAConfigParser/ParameterConfig.hpp>


class Initializer
{
public:
  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

  bool
  require_value() throw ()
  {
    return true;
  }

  void
  set(const char*, const char* value) throw (eh::Exception)
  {
    try
    {
      init();

      String::SubString v(value);
      String::StringManip::SplitHash splitter(v);
      String::SubString token;
      while (splitter.get_token(token))
      {
        std::string token_str = token.str();
        const char* param = token_str.c_str();
        args_.parse(1, &param);
      }

      work();
    }
    catch (const CORBA::Exception& ex)
    {
      throw Exception(ex._info().c_str());
    }
  }

protected:
  Initializer() throw ()
  {
  }

  virtual
  ~Initializer() throw ()
  {
  }

  virtual
  void
  init() throw (eh::Exception) = 0;

  virtual
  void
  work() throw (eh::Exception) = 0;

  Generics::AppUtils::Args args_;
};

class Client : public Initializer
{
public:
  Client() throw (eh::Exception)
    : corba_client_adapter(new CORBACommons::CorbaClientAdapter()),
      opt_url(corba_client_adapter.in()),
      opt_secure_url(corba_client_adapter.in())
  {
    args_.add(
      Generics::AppUtils::equal_name("url") ||
      Generics::AppUtils::short_name("u"),
      opt_url);
    args_.add(
      Generics::AppUtils::equal_name("secure-url") ||
      Generics::AppUtils::short_name("su"),
      opt_secure_url);
  }

  virtual
  ~Client() throw ()
  {
  }

protected:
  virtual
  void
  init() throw (eh::Exception)
  {
  }

  virtual
  void
  work() throw (eh::Exception);

protected:
  CORBACommons::CorbaClientAdapter_var corba_client_adapter;
  CORBAConfigParser::CorbaRefOption<CORBATest::TestInt> opt_url;
  CORBAConfigParser::CorbaRefOption<CORBATest::TestInt> opt_secure_url;
};

class Server : public Initializer
{
public:
  Server() throw (eh::Exception)
    : opt_host_("*")
  {
  }

  virtual
  ~Server() throw ()
  {
  }

protected:
  virtual
  void
  init() throw (eh::Exception)
  {
    args_.add(
      Generics::AppUtils::equal_name("port") ||
      Generics::AppUtils::short_name("p"),
      opt_port_);
    args_.add(
      Generics::AppUtils::equal_name("host") ||
      Generics::AppUtils::short_name("h"),
      opt_host_);
    args_.add(
      Generics::AppUtils::equal_name("secure-port"),
      opt_secure_port_);
    args_.add(
      Generics::AppUtils::equal_name("secure-params") ||
      Generics::AppUtils::short_name("sp"),
      opt_secure_params_);
  }

  virtual
  void
  init_endpoint(CORBACommons::EndpointConfig&) throw (eh::Exception)
  {
  }

  virtual
  void
  work() throw (eh::Exception)
  {
    CORBACommons::CorbaConfig corba_config;

    corba_config.thread_pool = 15;

    if (opt_port_.installed())
    {
      CORBACommons::EndpointConfig endpoint_config;
      endpoint_config.host = *opt_host_;
      endpoint_config.port = *opt_port_;
      init_endpoint(endpoint_config);
      corba_config.endpoints.push_back(endpoint_config);
    }

    if (opt_secure_port_.installed() && opt_secure_params_.installed())
    {
      CORBACommons::EndpointConfig endpoint_config;
      endpoint_config.host = *opt_host_;
      endpoint_config.port = *opt_secure_port_;
      endpoint_config.secure_connection_config = *opt_secure_params_;
      init_endpoint(endpoint_config);
      corba_config.endpoints.push_back(endpoint_config);
    }

    corba_server_adapter =
      new CORBACommons::CorbaServerAdapter(corba_config);
  }

protected:
  CORBACommons::CorbaServerAdapter_var corba_server_adapter;
  Generics::AppUtils::Option<unsigned long> opt_port_;
  Generics::AppUtils::Option<unsigned long> opt_secure_port_;
  Generics::AppUtils::Option<std::string> opt_host_;
  CORBAConfigParser::SecureParamsOption opt_secure_params_;
};

template <typename Client, typename Server>
struct Usage
{
  virtual
  ~Usage() throw ()
  {
  }

  virtual
  void
  action(Client&, Server&) throw (eh::Exception)
  {
  }

  int
  use(int argc, char* argv[]) throw ()
  {
    try
    {
      Client client;
      Server server;
      Generics::AppUtils::Args args;

      args.add(
        Generics::AppUtils::equal_name("client") ||
        Generics::AppUtils::short_name("c"),
        client);
      args.add(
        Generics::AppUtils::equal_name("server") ||
        Generics::AppUtils::short_name("s"),
        server);

      args.parse(argc - 1, argv + 1);

      action(client, server);

      return 0;
    }
    catch (const CORBA::Exception& e)
    {
      std::cerr << "main(): " << e << std::endl;    
    }
    catch (const eh::Exception& e)
    {
      std::cerr << "main(): " << e.what() << std::endl;    
    }
    catch (...)
    {
      std::cerr << "main(): unknown exception caught" << std::endl;
    }

    return 1;
  }
};

#endif
