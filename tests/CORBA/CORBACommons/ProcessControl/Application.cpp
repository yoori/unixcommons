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
#include <sstream>

#include <CORBACommons/ProcessControlImpl.hpp>

#include "Application.hpp"


//#define TRACE

Application::Application() throw(Application::Exception, eh::Exception)
{
}

Application::~Application() throw()
{
  try
  {
    destroy();
  }
  catch(...)
  {
  }
}

void
Application::init(int& argc, char** argv)
  throw(InvalidArgument, Exception, eh::Exception)
{
  std::cout << "Initializing ...\n";

  Write_Guard_ guard(lock_);

  try
  {
    std::cout << "  obtaining ORB\n";

    orb_ = CORBA::ORB_init(argc, argv);

    if(CORBA::is_nil(orb_))
    {
      throw InvalidArgument("Application::init: CORBA::ORB_init failed");
    }

    PortableServer::POA_var poa;
    PortableServer::POAManager_var poa_manager;

    std::cout << "  resolving RootPOA\n";

    CORBA::Object_var obj = orb_->resolve_initial_references("RootPOA");

    if(CORBA::is_nil(obj.in()))
    {
      throw Exception("Application::init: CORBA::ORB::"
                      "resolve_initial_references(RootPOA) failed");
    }

    poa = PortableServer::POA::_narrow(obj.in());

    if(CORBA::is_nil(poa.in()))
    {
      throw Exception("Application::init: "
                      "PortableServer::POA::_narrow failed");
    }

    std::cout << "  obtaining POAManager\n";
    poa_manager = poa->the_POAManager();

    if(CORBA::is_nil(poa_manager.in()))
    {
      throw Exception("Application::init: "
                      "PortableServer::POA::the_POAManager failed");
    }

    CORBA::PolicyList policies;
    policies.length(2);

    policies[0] =
      poa->create_lifespan_policy(PortableServer::PERSISTENT);

    policies[1] =
      poa->create_id_assignment_policy(PortableServer::USER_ID);

    std::cout << "  creating ProcessControlPOA\n";
    server_poa_ = poa->create_POA("ProcessControlPOA",
                                 poa_manager.in(),
                                 policies);

    policies[0]->destroy();
    policies[1]->destroy();

    if(CORBA::is_nil(server_poa_.in()))
    {
      throw Exception("Application::init: "
                      "PortableServer::POA::create_POA failed");
    }

    process_control_name_ = "ProcessControl";
    process_control_id_ = PortableServer::string_to_ObjectId(
      process_control_name_);

    std::cout << "  creating ProcessControl servant\n";

    servant_ = new CORBACommons::ProcessControlImpl(
      CORBACommons::OrbShutdowner_var(
        new CORBACommons::SimpleOrbShutdowner(orb_.in())));

    std::cout << "  activating ProcessControl object\n";
    server_poa_->activate_object_with_id(process_control_id_, servant_.in());

    std::cout << "  activating POPManager\n";
    poa_manager->activate();

    CORBA::Object_var object =
      server_poa_->id_to_reference(process_control_id_);

    std::cout << "  resolving IORTable\n";

    CORBA::Object_var table_obj =
      orb_->resolve_initial_references("IORTable");

    if(CORBA::is_nil(table_obj))
    {
      throw Exception("Application::init: CORBA::ORB::"
                      "resolve_initial_references(IORTable) failed");
    }

    ior_table_ = IORTable::Table::_narrow(table_obj.in());

    if (CORBA::is_nil(ior_table_.in()))
    {
      throw Exception("Application::init: "
                      "IORTable::Table::_narrow failed");
    }

    std::cout << "  binding ProcessControl with IORTable\n";

    CORBA::String_var ior = orb_->object_to_string(object);
    ior_table_->bind(process_control_name_, ior);
  }
  catch(const CORBA::Exception& e)
  {
    destroy();

    std::ostringstream ostr;
    ostr << "Application::init: CORBA::Exception caught. Description:\n"
         << e;

    throw Exception(ostr.str());
  }
  catch(...)
  {
    destroy();
    throw;
  }

}

void
Application::run()
  throw(InvalidOperationOrder, Exception, eh::Exception)
{
  try
  {
    if(CORBA::is_nil(orb_))
    {
      throw InvalidOperationOrder("Application::run: orb not constructed");
    }

    std::cout << "Running ORB loop ...\n";

    orb_->run();

    std::cout << "Escaped ORB loop\nWaiting for ProcessControl ...\n";

    CORBACommons::ProcessControlImpl* process_control =
      dynamic_cast<CORBACommons::ProcessControlImpl*>(servant_.in());

    if(process_control == 0)
    {
      throw Exception("Application::run: dynamic_cast<CORBACommons::"
                      "ProcessControlImpl*> failed");
    }

    process_control->wait();
  }
  catch(const CORBA::Exception& e)
  {
    std::ostringstream ostr;
    ostr << "Application::run: CORBA::Exception caught. Description:\n"
         << e;

    throw Exception(ostr.str());
  }

}

void
Application::destroy() throw(eh::Exception)
{
  try
  {
    if(!CORBA::is_nil(orb_))
    {
      std::cout << "Cleaning up ...\n";

      if (!CORBA::is_nil(ior_table_.in()))
      {
        std::cout << "  removing binding ...\n";

        ior_table_->unbind(process_control_name_);
        ior_table_ = IORTable::Table::_nil();
      }

      std::cout << "  releasing POA ...\n";
      server_poa_ = PortableServer::POA::_nil();

      std::cout << "  releasing servant ...\n";
      servant_ = 0;

      std::cout << "  destroying ORB ...\n";

      orb_->destroy();
      orb_ = CORBA::ORB::_nil();
    }
  }
  catch(const CORBA::Exception& e)
  {
    std::ostringstream ostr;
    ostr << "Application::run: CORBA::Exception caught. Description:\n"
         << e;

    throw Exception(ostr.str());
  }
}

int
main(int argc, char** argv)
{
  int result = 1;

  try
  {
    Application app;

    app.init(argc, argv);
    app.run();
    app.destroy();

    result = 0;
  }
  catch(const eh::Exception& e)
  {
    std::cerr << "main: eh::Exception exception caught. "
      "Description:" << std::endl << e.what() << std::endl;
  }
  catch(...)
  {
    std::cerr << "main: unknown exception caught";
  }

  return result;
}

