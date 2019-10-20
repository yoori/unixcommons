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



#ifndef CORBACOMMONS_CORBASERVER_ADAPTER_HPP
#define CORBACOMMONS_CORBASERVER_ADAPTER_HPP

#include <set>

#include <ReferenceCounting/Vector.hpp>
#include <ReferenceCounting/Map.hpp>

#include <CORBACommons/ProcessControlImpl.hpp>
#include <CORBACommons/CorbaAdapters.hpp>


namespace CORBACommons
{
  typedef std::set<std::string> ObjectsExternalNames;
  typedef std::map<std::string, ObjectsExternalNames> EndpointObjectTable;

  /**X
   * EndpointConfig
   */
  struct EndpointConfig
  {
    std::string host;
    std::string ior_names;
    int port;

    SecureConnectionConfig secure_connection_config;

    EndpointObjectTable objects;

    int
    bind_port() const throw ();

    static const int BIND_PORT_OFFSET;
  };

  typedef std::vector<EndpointConfig> EndpointConfigs;


  /**X CorbaConfig */
  struct CorbaConfig
  {
    CorbaConfig() throw (eh::Exception);

    unsigned thread_pool;
    unsigned min_threads;
    unsigned normal_threads;
    size_t stack_size;
    bool orb_per_endpoint;
    bool custom_reactor;
    EndpointConfigs endpoints;
  };


  /**X
   * CorbaServerAdapter
   */
  class CorbaServerAdapter :
    public ::ReferenceCounting::AtomicImpl,
    private OrbShutdowner
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    explicit
    CorbaServerAdapter(const CorbaConfig& corba_config,
      Logging::Logger* logger = 0) throw (eh::Exception);

    void
    add_binding(const char* name, PortableServer::ServantBase* servant)
      throw (eh::Exception, Exception);

    void
    register_value_factory(const char* type_name,
      CORBA::ValueFactoryBase* factory) throw (eh::Exception, Exception);

    void
    run() throw (eh::Exception, Exception);

    OrbShutdowner_var
    shutdowner() throw ();


    struct ThreadsUsage
    {
      unsigned total;
      unsigned waiting;
      unsigned working;
    };

    void
    get_threads_usage(ThreadsUsage& usage) throw (eh::Exception);


  protected:
    virtual
    ~CorbaServerAdapter() throw ();

    class EndpointAddress
    {
    public:
      EndpointAddress() throw (eh::Exception);

      EndpointAddress(const char* host, unsigned long port)
        throw (eh::Exception);

      const char*
      host() throw ();

      const char*
      ip() throw ();

      unsigned long
      port() throw ();

      bool
      operator <(const EndpointAddress& address) const throw ();

    private:
      std::string host_;
      std::string ip_;
      unsigned long port_;
    };

    class Locator;
    typedef ::ReferenceCounting::FixedPtr<Locator> Locator_var;

    class POACreator;

    class Endpoint : public ::ReferenceCounting::AtomicImpl
    {
    public:
      typedef std::map<std::pair<EndpointAddress, std::string>,
        std::pair<PortableServer::POA_var, PortableServer::ObjectId_var> >
        ObjectIdTable;

      Endpoint(POACreator& poa_creator, Locator* locator,
        const char* host, int port, int bind_port,
        const EndpointObjectTable& endpoint_bind_names,
        TAO::SL2::AccessDecision_ptr access_decision,
        const char* orb_id)
        throw (eh::Exception);

      void
      add_binding(const char* name, PortableServer::ServantBase* servant)
        throw (eh::Exception, Exception);

      void
      activate() throw ();

      const ObjectIdTable&
      bound_objects() const throw ();

    protected:
      struct BindPoint
      {
        PortableServer::POAManager_var poa_manager;
        PortableServer::POA_var poa;
        CORBA::OctetSeq_var poa_id;
        EndpointAddress address;
      };
      typedef std::vector<BindPoint> BindPoints;


      virtual
      ~Endpoint() throw ();

      const ObjectsExternalNames&
      find_name_(const char* name) const throw (eh::Exception, Exception);

      void
      add_binding_(BindPoints::const_iterator bind_point,
        PortableServer::ServantBase* servant,
        const ObjectsExternalNames& result_names)
        throw (eh::Exception, Exception);


      BindPoints bind_points_;

      PortableServer::POAManager_var poa_manager_;
      PortableServer::POA_var poa_;
      Locator_var locator_;
      EndpointObjectTable object_bind_names_;
      TAO::SL2::AccessDecision_var access_decision_;
      std::string orb_id_;

      ObjectIdTable bound_objects_;
    };
    typedef ::ReferenceCounting::QualPtr<Endpoint> Endpoint_var;

    typedef ::ReferenceCounting::Vector<Endpoint_var> Endpoints;
    typedef ::ReferenceCounting::Map<std::string, Endpoints>
      ObjectEndpointsMap;

    struct Orb
    {
      explicit
      Orb(CORBA::ORB_var orb = CORBA::ORB_var()) throw ();

      CORBA::ORB_var orb;
      unsigned waiters;
      unsigned threads_left;
      bool expanding;
    };
    typedef std::vector<Orb> Orbs;

  protected:
    void
    init_env_() throw (eh::Exception, Exception);

    void
    create_orb_(const SecureConnectionConfig& secure_config,
      const EndpointConfigs& endpoints) throw (eh::Exception);

    void
    create_corba_endpoints_(const EndpointConfig& endpoint_config,
      ORBProperties& properties) throw (eh::Exception);

    void
    init_orb_properties_(ORBProperties& properties,
      const SecureConnectionConfig& secure_connection_config,
      const EndpointConfigs& endpoints) throw (eh::Exception);

    template <typename T>
    T*
    resolve_initial_reference_(CORBA::ORB_ptr orb, const char* obj_name)
      throw (eh::Exception);

    virtual
    void
    shutdown(bool type) throw ();


  protected:
    class ServerAdapterJob;
    friend class ServerAdapterJob;

    CorbaConfig corba_config_;

    Sync::PosixMutex mutex_;
    Orbs orbs_;
    bool shutdown_complete_;

    Endpoints endpoints_;
    ObjectEndpointsMap object_to_endpoints_;
    Logging::FLogger_var logger_;

    Sync::PosixMutex threads_mutex_;
    unsigned threads_running_;
  };
  typedef ::ReferenceCounting::QualPtr<CorbaServerAdapter>
    CorbaServerAdapter_var;
}

namespace CORBACommons
{
  //
  // CorbaConfig class
  //

  inline
  CorbaConfig::CorbaConfig() throw (eh::Exception)
    : thread_pool(1), min_threads(0), normal_threads(0), stack_size(0),
      orb_per_endpoint(true), custom_reactor(true)
  {
  }


  //
  // CorbaServerAdapter::EndpointAddress class
  //

  inline
  CorbaServerAdapter::EndpointAddress::EndpointAddress()
    throw (eh::Exception)
    : port_(0)
  {
  }

  inline
  const char*
  CorbaServerAdapter::EndpointAddress::host() throw ()
  {
    return host_.c_str();
  }

  inline
  const char*
  CorbaServerAdapter::EndpointAddress::ip() throw ()
  {
    return ip_.c_str();
  }

  inline
  unsigned long
  CorbaServerAdapter::EndpointAddress::port() throw ()
  {
    return port_;
  }

  inline
  bool
  CorbaServerAdapter::EndpointAddress::operator <(
    const EndpointAddress& address) const throw ()
  {
    return ip_ == address.ip_ ? port_ < address.port_ :
      ip_ < address.ip_;
  }


  //
  // CorbaServerAdapter class
  //

  template <typename T>
  T*
  CorbaServerAdapter::resolve_initial_reference_(CORBA::ORB_ptr orb,
    const char* obj_name) throw (eh::Exception)
  {
    try
    {
      CORBA::Object_var obj = orb->resolve_initial_references(obj_name);

      if (CORBA::is_nil(obj))
      {
        Stream::Error ostr;
        ostr << FNS << "Can't resolve object '" << obj_name << "'";
        throw Exception(ostr);
      }

      typename T::_var_type obj_var = T::_narrow(obj);

      if (CORBA::is_nil(obj_var))
      {
        Stream::Error ostr;
        ostr << FNS << "Can't narrow object '" << obj_name << "'";
        throw Exception(ostr);
      }

      return obj_var._retn();
    }
    catch (const CORBA::SystemException& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't resolve object '" << obj_name << "': " << ex;
      throw Exception(ostr);
    }
  }
}

#endif
