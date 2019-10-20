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



#include <fstream>
#include <iostream>
#include <signal.h>
#include <dirent.h>
#include "server_s.hpp"
#include "server.hpp"

class Echo_i : public POA_Test::Echo
{
public:
  virtual void
  echoString(const Test::AType & message) throw ();
};

void
Echo_i::echoString(const Test::AType & message) throw ()
{
  std::cout << "Server message sizeof=" << sizeof(::Test::AType)
            << std::endl;
  // some actions with AType sequence...
  std::cout << "Length=" << message.length() << std::endl;
  for(std::size_t i = 0; i< message.length(); ++i)
  {
    std::cout << message[i].aa << " "
      << std::endl;
  }
}

class OutVal_i : public POA_Test::OutVal
{
public:
  virtual void
  test(Test::B_out value) throw ();
};

void
OutVal_i::test(Test::B_out) throw ()
{
  // Do not assign anything into value
  std::cout << "Server do test(Test::B_out value) method. sizeof(B)="
    << sizeof(::Test::B) << std::endl;
}

int
main(int argc, char** argv)
{
  try
  {
    std::cout << "Server started" << std::endl;
    CORBA::ORB_ptr orb = CORBA::ORB_init(argc, argv);
    if(CORBA::is_nil(orb))
    {
      std::cerr << "CORBA::ORB_init failed" << std::endl;
      return 1;
    }        

    CORBA::Object_var obj =
      orb->resolve_initial_references("RootPOA");

    PortableServer::POA_var root_poa =
      PortableServer::POA::_narrow (obj.in ());
    if (CORBA::is_nil (root_poa.in ()))
    {
      std::cerr << "POA::_narrow failed. Error=" << LM_ERROR << std::endl;
      return 1;
    }

    Echo_i* myecho = new Echo_i();

    PortableServer::ServantBase_var owner_transfer(myecho);
    PortableServer::ObjectId_var myechoid =
      root_poa->activate_object (myecho);
    CORBA::Object_var object = root_poa->id_to_reference (myechoid.in ());
    Test::Echo_var echo = Test::Echo::_narrow (object.in ());

    obj = myecho->_this();
//    CORBA::String_var ior = orb->object_to_string (myecho.in ());
    CORBA::String_var sior(orb->object_to_string(obj));
    std::cout << "Server: first IOR ready" << std::endl;

    OutVal_i* myout = new OutVal_i();
    PortableServer::ServantBase_var transfer_owner(myout);
    PortableServer::ObjectId_var my_outval_id =
      root_poa->activate_object (myout);
    CORBA::Object_var out_object =
      root_poa->id_to_reference (my_outval_id.in ());
    Test::OutVal_var out = Test::OutVal::_narrow (out_object.in ());

    //obj = myout->_this();
    CORBA::String_var sout_ior(orb->object_to_string(out.in()));

    std::cout << "Second IOR ready" << std::endl;

    // Output the IOR to the ior_output_file Debug - purpose
    {
      std::ofstream ofs("server.ior", std::ios::binary);
      if (!ofs)
      {
          std::cerr << "Cannot open output file for writing IOR: "
            << " error=" << LM_ERROR << std::endl;
          return 1;
      }
      ofs << sior.in () << std::endl;
      ofs << sout_ior.in () << std::endl;
      std::cout << "Server IOR wrote:\n" << sior.in () << std::endl;
      std::cout << "Server IOR2 wrote:\n" << sout_ior.in () << std::endl;
    }
    // Another way transmit server ior through fork
    if (!fork())
    {
      execl("../Client/CORBAInfResolveClient", "../Client/CORBAInfResolveClient",
        (const char*)sior, (const char*)sout_ior, NULL);
    }
    myecho->_remove_ref();
    myout->_remove_ref();

    PortableServer::POAManager_var poa_manager =
      root_poa->the_POAManager ();

    poa_manager->activate();

    orb->run();

    return 0;
  }
  catch (const CORBA::Exception& ex)
  {
    std::cerr << "CORBA::Exception on server side:" << ex;
  }
  catch (const std::exception& e)
  {
    std::cerr << "eh::Exception on server side:" << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception on server side" << std::endl;
    throw;
  }
  return 1;
}
