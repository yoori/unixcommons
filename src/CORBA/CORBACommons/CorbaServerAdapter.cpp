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

#include <ace/Token.h>
#include <tao/default_resource.h>
#include <tao/IORTable/IORTable.h>
#include <tao/TransportCurrent/TC_IIOPC.h>
#include <tao/EndpointPolicy/EndpointPolicy.h>
#include <tao/EndpointPolicy/IIOPEndpointValue_i.h>

#include <Generics/GnuHashTable.hpp>
#include <Generics/HashTableAdapters.hpp>
#include <Generics/Network.hpp>

#include "CorbaAdaptersInternal.hpp"


namespace
{
  const char ORB_SERVER_NON_SECURE_NAME[] = "ServerNonSecureORB";
  const char ORB_SERVER_SECURE_NAME[] = "ServerSecureORB";
}

namespace
{
  //
  // Network interfaces
  //

  typedef std::vector<std::string> NetworkInterfaces;
  Sync::PosixMutex interfaces_mutex;
  NetworkInterfaces all_interfaces;

  class IPToString
  {
  public:
    const char*
    operator ()(const sockaddr_in* address) throw ();

  private:
    char ip_[32];
  };

  const char*
  IPToString::operator ()(const sockaddr_in* address) throw ()
  {
    return inet_ntop(AF_INET, &address->sin_addr, ip_, sizeof(ip_));
  }

  void
  get_interfaces(NetworkInterfaces& network_interfaces,
    const char* host) throw (eh::Exception)
  {
    if (*host == '*')
    {
      Sync::PosixGuard guard(interfaces_mutex);

      if (all_interfaces.empty())
      {
        Generics::Network::LocalInterfaces local_interfaces;
        local_interfaces.list_all(all_interfaces, IPToString());
      }
      network_interfaces = all_interfaces;
    }
    else
    {
      network_interfaces.emplace_back(host);
    }
  }
}

namespace CORBACommons
{
  ACE_Reactor_Impl*
  create_reactor_impl(ACE_Timer_Queue* tq) throw ();

  //
  // Data for control of number of unoccupied threads per orb
  //

  struct Threads
  {
    Threads(Sync::PosixMutex& mutex, unsigned& threads_running,
      CorbaConfig& corba_config)
      throw (eh::Exception);

    Sync::PosixMutex& mutex;
    Sync::Semaphore sem;
    unsigned& threads_running;

    CorbaConfig& corba_config_;
    Generics::ArrayAutoPtr<Generics::ThreadJob_var> jobs;
    Generics::ThreadJob_var* current_job;
    std::unique_ptr<Generics::ThreadRunner> thread_runner;
  };

  Threads::Threads(Sync::PosixMutex& mutex, unsigned& threads_running,
    CorbaConfig& corba_config) throw (eh::Exception)
    : mutex(mutex), sem(0), threads_running(threads_running),
      corba_config_(corba_config)
  {
  }


  class CorbaServerAdapter::Locator :
    public CORBACommons::ReferenceCounting::
      CorbaRefCountImpl<IORTable::Locator>
  {
  public:
    Locator(CORBA::ORB_ptr orb,
      TAO::Transport::IIOP::Current_ptr current_transport)
      throw (eh::Exception);

    void
    bind(const EndpointAddress& address, const char* name,
      CORBA::Object_ptr object)
      throw (eh::Exception, Exception);

    void
    unbind(const EndpointAddress& address)
      throw (eh::Exception);

    virtual
    char*
    locate(const char* name) throw (eh::Exception, IORTable::NotFound);

  protected:
    virtual
    ~Locator() throw ();

  private:
    typedef Generics::GnuHashTable<Generics::StringHashAdapter,
      CORBA::Object_var> EndpointServants;
    typedef std::map<EndpointAddress, EndpointServants> Mapping;

    Sync::PosixRWLock lock_;
    CORBA::ORB_var orb_;
    TAO::Transport::IIOP::Current_var current_transport_;
    Mapping mapping_;
  };

  class CorbaServerAdapter::POACreator
  {
  public:
    POACreator(CORBA::ORB_ptr orb,
      PortableServer::POA_ptr root_poa) throw (eh::Exception);

    void
    create_poa(const char* suffix, const char* host, int port,
      PortableServer::POAManager_out poa_manager,
      PortableServer::POA_out poa) throw (eh::Exception);

  private:
    CORBA::ORB_var orb_;
    PortableServer::POA_var root_poa_;
    CORBA::PolicyList policies_;
    CORBA::Policy_var policies_var[3];
    PortableServer::POAManagerFactory_var poa_manager_factory_;
  };

  class CorbaServerAdapter::ServerAdapterJob : public Generics::ThreadJob
  {
  public:
    void
    set(Orb* orb, Threads* threads) throw ();

    virtual
    void
    work() throw ();

  protected:
    virtual
    ~ServerAdapterJob() throw ();

  private:
    void
    check_waiters_(int waiters) throw ();

    static
    void
    waiters_cb_(int waiters) throw ();

    Orb* orb_;
    Threads* threads_;

    static Sync::Key<ServerAdapterJob> key_;
  };
}

namespace CORBACommons
{
  //
  // EndpointConfig class
  //

  const int EndpointConfig::BIND_PORT_OFFSET = 50;

  int
  EndpointConfig::bind_port() const throw ()
  {
    return secure_connection_config.is_secure() ?
      port + BIND_PORT_OFFSET : port;
  }


  //
  // CorbaServerAdapter::EndpointAddress class
  //

  CorbaServerAdapter::EndpointAddress::EndpointAddress(
    const char* host, unsigned long port) throw (eh::Exception)
    : host_(host), port_(port)
  {
    hostent addresses;
    char buf[2048];
    Generics::Network::Resolver::get_host_by_name(host, addresses,
      buf, sizeof(buf));
    inet_ntop(AF_INET, addresses.h_addr, buf, sizeof(buf));
    ip_ = buf;
  }


  //
  // CorbaServerAdapter::Locator class
  //

  CorbaServerAdapter::Locator::Locator(CORBA::ORB_ptr orb,
    TAO::Transport::IIOP::Current_ptr current_transport)
    throw (eh::Exception)
    : orb_(CORBA::ORB::_duplicate(orb)),
      current_transport_(TAO::Transport::IIOP::Current::_duplicate(
        current_transport))
  {
  }

  CorbaServerAdapter::Locator::~Locator() throw ()
  {
  }

  void
  CorbaServerAdapter::Locator::bind(const EndpointAddress& address,
    const char* name, CORBA::Object_ptr object)
    throw (eh::Exception, Exception)
  {
    if (CORBA::is_nil(object))
    {
      Stream::Error ostr;
      ostr << FNS << "nil object to bind";
      throw Exception(ostr);
    }

    Sync::PosixWGuard guard(lock_);

    CORBA::Object_var& stored_object = mapping_[address][name];
    if (!CORBA::is_nil(stored_object))
    {
      Stream::Error ostr;
      ostr << FNS << "duplicate object binding";
      throw Exception(ostr);
    }

    stored_object = CORBA::Object::_duplicate(object);
  }

  void
  CorbaServerAdapter::Locator::unbind(const EndpointAddress& address)
    throw (eh::Exception)
  {
    Sync::PosixWGuard guard(lock_);

    Mapping::iterator endpoint = mapping_.find(address);
    if (endpoint != mapping_.end())
    {
      mapping_.erase(endpoint);
    }
  }

  char*
  CorbaServerAdapter::Locator::locate(const char* name)
    throw (eh::Exception, IORTable::NotFound)
  {
    Sync::PosixRGuard guard(lock_);

    CORBA::String_var local_host(current_transport_->local_host());
    EndpointAddress address(local_host,
      current_transport_->local_port());

#if 0
    std::cout << "Request " << address.get_host_addr() << ":" <<
      address.get_port_number() << "/" << name << std::endl;
#endif

    Mapping::const_iterator endpoint = mapping_.find(address);
    if (endpoint == mapping_.end())
    {
      throw IORTable::NotFound();
    }

    EndpointServants::const_iterator servant =
      endpoint->second.find(name);
    if (servant == endpoint->second.end())
    {
      throw IORTable::NotFound();
    }

    try
    {
      return orb_->object_to_string(servant->second);
    }
    catch (const CORBA::Exception& ex)
    {
      throw IORTable::NotFound();
    }
  }


  //
  // CorbaServerAdapter::Endpoint class
  //

  CorbaServerAdapter::Endpoint::Endpoint(POACreator& poa_creator,
    Locator* locator, const char* host, int port, int bind_port,
    const EndpointObjectTable& object_bind_names,
    TAO::SL2::AccessDecision_ptr access_decision, const char* orb_id)
    throw (eh::Exception)
    : locator_(::ReferenceCounting::add_ref(locator)),
      object_bind_names_(object_bind_names),
      access_decision_(TAO::SL2::AccessDecision::_duplicate(access_decision)),
      orb_id_(orb_id)
  {
    NetworkInterfaces network_interfaces;
    get_interfaces(network_interfaces, host);

    bind_points_.reserve(network_interfaces.size());
    for (NetworkInterfaces::const_iterator itor(network_interfaces.begin());
      itor != network_interfaces.end(); ++itor)
    {
      BindPoint bind_point;
      std::ostringstream suffix;
      suffix << *itor << "_" << port;
      poa_creator.create_poa(suffix.str().c_str(), itor->c_str(),
        bind_port, bind_point.poa_manager, bind_point.poa);
      if (!CORBA::is_nil(access_decision))
      {
        bind_point.poa_id = bind_point.poa->id();
      }
      bind_point.address = EndpointAddress(itor->c_str(), port);
      bind_points_.push_back(std::move(bind_point));
    }
  }

  CorbaServerAdapter::Endpoint::~Endpoint() throw ()
  {
    for (BindPoints::const_iterator itor(bind_points_.begin());
      itor != bind_points_.end(); ++itor)
    {
      locator_->unbind(itor->address);
    }
  }

  const ObjectsExternalNames&
  CorbaServerAdapter::Endpoint::find_name_(const char* name) const
    throw (eh::Exception, Exception)
  {
    EndpointObjectTable::const_iterator it = object_bind_names_.find(name);
    if (it == object_bind_names_.end())
    {
      Stream::Error ostr;
      ostr << FNS << "Object '" << name << "' not registered at endpoint.";
      throw Exception(ostr);
    }

    return it->second;
  }

  void
  CorbaServerAdapter::Endpoint::add_binding(const char* name,
    PortableServer::ServantBase* servant)
    throw (eh::Exception, Exception)
  {
    const ObjectsExternalNames& result_names = find_name_(name);

    for (BindPoints::const_iterator itor(bind_points_.begin());
     itor != bind_points_.end(); ++itor)
    {
      add_binding_(itor, servant, result_names);
    }
  }

  void
  CorbaServerAdapter::Endpoint::add_binding_(
    BindPoints::const_iterator bind_point,
    PortableServer::ServantBase* servant,
    const ObjectsExternalNames& result_names)
    throw (eh::Exception, Exception)
  {
    if (result_names.empty())
    {
      Stream::Error ostr;
      ostr << FNS << "External names set is empty";
      throw Exception(ostr);
    }
    try
    {
      for (ObjectsExternalNames::const_iterator result_name(
        result_names.begin()); result_name != result_names.end();
        ++result_name)
      {
        PortableServer::ObjectId_var obj_id =
          PortableServer::string_to_ObjectId(result_name->c_str());
        bind_point->poa->activate_object_with_id(obj_id, servant);

        CORBA::Object_var obj_ref =
          bind_point->poa->id_to_reference(obj_id);

        if (!CORBA::is_nil(access_decision_))
        {
          access_decision_->add_object(orb_id_.c_str(),
            bind_point->poa_id.in(), obj_id, true);
        }

        locator_->bind(bind_point->address, result_name->c_str(), obj_ref);

        bound_objects_.insert(std::make_pair(
          std::make_pair(bind_point->address, *result_name),
          std::make_pair(bind_point->poa, obj_id)));
      }
    }
    catch (const CORBA::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't bind object. CORBA::Exception:" << ex;
      throw Exception(ostr);
    }
  }

  void
  CorbaServerAdapter::Endpoint::activate() throw ()
  {
    for (BindPoints::const_iterator itor(bind_points_.begin());
     itor != bind_points_.end(); ++itor)
    {
      itor->poa_manager->activate();
    }
  }

  const CorbaServerAdapter::Endpoint::ObjectIdTable&
  CorbaServerAdapter::Endpoint::bound_objects() const throw ()
  {
    return bound_objects_;
  }


  //
  // CorbaServerAdapter::Orb class
  //

  CorbaServerAdapter::Orb::Orb(CORBA::ORB_var orb) throw ()
    : orb(orb)
  {
  }


  //
  // CorbaServerAdapter::ServerAdapterJob class
  //

  Sync::Key<CorbaServerAdapter::ServerAdapterJob>
    CorbaServerAdapter::ServerAdapterJob::key_(
      ((ACE_Token::waiters_callback_ = waiters_cb_),
      static_cast<void (*)(void*)>(0)));

  CorbaServerAdapter::ServerAdapterJob::~ServerAdapterJob() throw ()
  {
  }

  void
  CorbaServerAdapter::ServerAdapterJob::set(Orb* orb, Threads* threads)
    throw ()
  {
    orb_ = orb;
    threads_ = threads;
  }

  void
  CorbaServerAdapter::ServerAdapterJob::work() throw ()
  {
    key_.set_data(this);

    try
    {
      orb_->orb->run();
    }
    catch (...)
    {
    }

    Sync::PosixGuard guard(threads_->mutex);
    if (!--threads_->threads_running)
    {
      threads_->sem.release();
    }
  }

  void
  CorbaServerAdapter::ServerAdapterJob::check_waiters_(int waiters) throw ()
  {
    Sync::PosixGuard guard(threads_->mutex);
    orb_->waiters = waiters;
    if (!orb_->threads_left)
    {
      return;
    }

    if (static_cast<unsigned>(waiters) <
      threads_->corba_config_.min_threads)
    {
      orb_->expanding = true;
    }
    else if (static_cast<unsigned>(waiters) >=
      threads_->corba_config_.normal_threads)
    {
      orb_->expanding = false;
    }
    if (orb_->expanding)
    {
      static_cast<ServerAdapterJob*>(threads_->current_job->in())->
        set(orb_, threads_);
      try
      {
        threads_->thread_runner->start_one();
      }
      catch (const eh::Exception&)
      {
        return;
      }
      threads_->current_job++;
      threads_->threads_running++;
      orb_->threads_left--;
    }
  }

  void
  CorbaServerAdapter::ServerAdapterJob::waiters_cb_(int waiters) throw ()
  {
    if (ServerAdapterJob* job = key_.get_data())
    {
      job->check_waiters_(waiters);
    }
  }


  //
  // CorbaServerAdapter class
  //

  CorbaServerAdapter::CorbaServerAdapter(const CorbaConfig& corba_config,
    Logging::Logger* logger) throw (eh::Exception)
    : corba_config_(corba_config), shutdown_complete_(false),
      logger_(::ReferenceCounting::add_ref(logger))
  {
    if (corba_config_.thread_pool <= 2 * PARTS)
    {
      corba_config_.thread_pool += PARTS;
    }
    if (!corba_config_.normal_threads ||
      corba_config_.normal_threads > corba_config_.thread_pool)
    {
      corba_config_.normal_threads = corba_config_.thread_pool;
    }
    if (corba_config_.min_threads > corba_config_.normal_threads)
    {
      corba_config_.min_threads = corba_config_.normal_threads;
    }

    if (logger_)
    {
      AceLogger::add_logger(logger_);
    }

    TAO_Default_Resource_Factory::custom_reactor_impl_factory =
      &create_reactor_impl;

    init_env_();
  }

  CorbaServerAdapter::~CorbaServerAdapter() throw ()
  {
    try
    {
      endpoints_.clear();
      object_to_endpoints_.clear();

      for (Orbs::iterator itor(orbs_.begin()); itor != orbs_.end(); ++itor)
      {
        itor->orb->destroy();
      }
    }
    catch (const CORBA::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Error on destroying ORB: " << ex;

      if (logger_)
      {
        logger_->log(ostr.str(), Logging::Logger::ERROR);
      }
      else
      {
        std::cerr << ostr << std::endl;
      }
    }
    AceLogger::remove_logger(logger_);
  }

  void
  CorbaServerAdapter::add_binding(const char* name,
    PortableServer::ServantBase* servant)
    throw (eh::Exception, Exception)
  {
    ObjectEndpointsMap::iterator it = object_to_endpoints_.find(name);
    if (it == object_to_endpoints_.end())
    {
      return;
    }

    Endpoints& endpoint_list = it->second;

    try
    {
      for (Endpoints::iterator ep_it = endpoint_list.begin();
        ep_it != endpoint_list.end(); ++ep_it)
      {
        (*ep_it)->add_binding(name, servant);
      }
    }
    catch (const CORBA::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't bind object '" << name << "': " << ex;
      throw Exception(ostr);
    }
  }

  void
  CorbaServerAdapter::register_value_factory(const char* type_name,
    CORBA::ValueFactoryBase* factory)
    throw (eh::Exception, Exception)
  {
    try
    {
      for (Orbs::iterator itor(orbs_.begin()); itor != orbs_.end(); ++itor)
      {
        CORBA::ValueFactoryBase_var old_factory =
          itor->orb->register_value_factory(type_name, factory);
      }
    }
    catch (const CORBA::SystemException& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't register factory for type '" << type_name <<
        "'. Caught CORBA::SystemException: " << ex;
      throw Exception(ostr);
    }
  }

  void
  CorbaServerAdapter::run()
    throw (eh::Exception, Exception)
  {
    Threads threads(threads_mutex_, threads_running_, corba_config_);

    try
    {
      Sync::PosixGuard guard(mutex_);

      if (shutdown_complete_)
      {
        return;
      }

      for (Endpoints::iterator it = endpoints_.begin();
        it != endpoints_.end(); ++it)
      {
        (*it)->activate();
      }

      Sync::PosixGuard guard2(threads.mutex);
      unsigned njobs = orbs_.size() * corba_config_.thread_pool;
      threads.jobs.reset(njobs);
      for (unsigned i = 0; i < njobs; i++)
      {
        threads.jobs[i] = new ServerAdapterJob();
      }
      threads.current_job = threads.jobs.get();
      unsigned threads_to_run = corba_config_.normal_threads;
      if (threads_to_run <= PARTS)
      {
        threads_to_run += PARTS;
      }
      for (Orbs::iterator orb(orbs_.begin()); orb != orbs_.end(); ++orb)
      {
        orb->threads_left =
          corba_config_.thread_pool - threads_to_run;
        orb->expanding = false;
        for (unsigned i = 0; i < threads_to_run; i++)
        {
          static_cast<ServerAdapterJob*>(threads.current_job++->in())->set(
            &*orb, &threads);
        }
      }
      threads.threads_running = threads.current_job - threads.jobs.get();

      threads.thread_runner.reset(
        new Generics::ThreadRunner(threads.jobs.get(),
          threads.jobs.get() + njobs,
          Generics::ThreadRunner::Options(corba_config_.stack_size)));

      threads.thread_runner->start(threads.threads_running);
    }
    catch (const CORBA::SystemException& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Caught CORBA::SystemException: " << ex;
      throw Exception(ostr);
    }

#if 0
    threads.sem.acquire();
#else
    for (;;)
    {
      if (threads.sem.timed_acquire(&Generics::Time::ONE_SECOND, true))
      {
        break;
      }
#if 0
      Sync::PosixGuard guard(threads.mutex);
      std::cerr << "Threads: " <<
        threads.thread_runner->running() << std::endl;
#endif
    }
#endif
    Sync::PosixGuard guard(threads.mutex);
    threads.thread_runner->wait_for_completion();
  }

  void
  CorbaServerAdapter::create_corba_endpoints_(
    const EndpointConfig& endpoint_config, ORBProperties& properties)
    throw (eh::Exception)
  {
    NetworkInterfaces network_interfaces;
    get_interfaces(network_interfaces, endpoint_config.host.c_str());

    const char* const PREFIX =
      endpoint_config.secure_connection_config.is_secure() ?
      "ssliop://" : "iiop://";

    for (NetworkInterfaces::const_iterator itor(network_interfaces.begin());
      itor != network_interfaces.end(); ++itor)
    {
      std::stringstream ostr;
      ostr << PREFIX << *itor << ":" << endpoint_config.bind_port();
      if (endpoint_config.secure_connection_config.is_secure())
      {
        ostr << "/ssl_port=" << endpoint_config.port;
      }
      properties.emplace_back("-ORBEndpoint");
      properties.emplace_back(ostr.str());
    }
  }

  void
  CorbaServerAdapter::init_orb_properties_(ORBProperties& properties,
    const SecureConnectionConfig& secure_connection_config,
    const EndpointConfigs& endpoints)
    throw (eh::Exception)
  {
    PropertiesHandling::create_common_properties(properties,
      corba_config_.custom_reactor);

    if (secure_connection_config.is_secure())
    {
      PropertiesHandling::create_secure_properties(properties,
        secure_connection_config);
    }

    for (EndpointConfigs::const_iterator it = endpoints.begin();
      it != endpoints.end(); ++it)
    {
      create_corba_endpoints_(*it, properties);
    }

    properties.emplace_front();
  }

  void
  CorbaServerAdapter::init_env_() throw (eh::Exception, Exception)
  {
    {
      // check unique ex-name of objects
      for (EndpointConfigs::const_iterator
        it = corba_config_.endpoints.begin();
        it != corba_config_.endpoints.end(); ++it)
      {
        for (EndpointObjectTable::const_iterator object_it =
          it->objects.begin();
          object_it != it->objects.end(); ++object_it)
        {
          EndpointObjectTable::const_iterator sub_object_it = object_it;
          ++sub_object_it;

          for (; sub_object_it != it->objects.end(); ++sub_object_it)
          {
            if (object_it->second == sub_object_it->second)
            {
              Stream::Error ostr;
              ostr << FNS << "Not unique external name of object. "
                "Endpoint host=" << it->host << ", port=" << it->port <<
                ". " << "Object external names=";
              for (ObjectsExternalNames::const_iterator itor(
                object_it->second.begin());
                itor != object_it->second.end(); ++itor)
              {
                ostr << " '" << *itor << '\'';
              }
              ostr << "'. ";
              throw Exception(ostr);
            }
          }
        }
      }
    }

    if (corba_config_.endpoints.empty())
    {
      Stream::Error ostr;
      ostr << FNS << "endpoints not defined.";
      throw Exception(ostr);
    }

    if (corba_config_.orb_per_endpoint)
    {
      for (EndpointConfigs::iterator it = corba_config_.endpoints.begin();
        it != corba_config_.endpoints.end(); ++it)
      {
        EndpointConfigs endpoints;
        endpoints.push_back(*it);
        create_orb_(it->secure_connection_config, endpoints);
      }
    }
    else
    {
      // Group endpoints by secure_connection_config
      typedef Generics::GnuHashTable<SecureConnectionConfigAdaptor,
        EndpointConfigs> ConfigEndpoints;
      ConfigEndpoints config_endpoints;

      for (EndpointConfigs::iterator it = corba_config_.endpoints.begin();
        it != corba_config_.endpoints.end(); ++it)
      {
        config_endpoints[it->secure_connection_config].push_back(*it);
      }

      for (ConfigEndpoints::iterator secure_config(
        config_endpoints.begin()); secure_config != config_endpoints.end();
        ++secure_config)
      {
        create_orb_(*secure_config->first, secure_config->second);
      }
    }
  }

  void
  CorbaServerAdapter::create_orb_(
    const SecureConnectionConfig& secure_config,
    const EndpointConfigs& endpoints) throw (eh::Exception)
  {
    ORBProperties properties;

    init_orb_properties_(properties, secure_config, endpoints);

    try
    {
      CORBA::ORB_var orb;
      if (secure_config.is_secure())
      {
        orb = OrbCreator::create_orb(properties,
          ORB_SERVER_SECURE_NAME, &secure_config);
      }
      else
      {
        orb = OrbCreator::create_orb(properties,
          ORB_SERVER_NON_SECURE_NAME);
      }

      CORBA::String_var orb_id = orb->id();

      /* initialize needed services */
      TAO::Transport::IIOP::Current_var current_transport =
        resolve_initial_reference_<TAO::Transport::IIOP::Current>(
          orb, "TAO::Transport::IIOP::Current");

      Locator_var locator(new Locator(orb, current_transport));

      IORTable::Table_var ior_table =
        resolve_initial_reference_<IORTable::Table>(orb, "IORTable");

      try
      {
        ior_table->set_locator(locator);
      }
      catch (const CORBA::SystemException& ex)
      {
        Stream::Error ostr;
        ostr << FNS << "Can't set IORTable locator." << ex;
        throw Exception(ostr);
      }

      TAO::SL2::AccessDecision_var access_decision;

      if (secure_config.is_secure())
      {
        SecurityLevel2::SecurityManager_var security_manager =
          resolve_initial_reference_<SecurityLevel2::SecurityManager>(
            orb, "SecurityLevel2:SecurityManager");

        try
        {
          SecurityLevel2::AccessDecision_var ad =
            security_manager->access_decision();
          access_decision = TAO::SL2::AccessDecision::_narrow(ad);
        }
        catch (const CORBA::SystemException& ex)
        {
          Stream::Error ostr;
          ostr << FNS << "Can't get AccessDecision interface: " << ex;
          throw Exception(ostr);
        }
      }

      PortableServer::POA_var root_poa =
        resolve_initial_reference_<PortableServer::POA>(orb, "RootPOA");

      /* init endpoints */

      {
        POACreator poa_creator(orb, root_poa);

        for (EndpointConfigs::const_iterator it = endpoints.begin();
          it != endpoints.end(); ++it)
        {
          Endpoint_var new_endpoint(
            new Endpoint(poa_creator, locator, it->host.c_str(),
              it->port, it->bind_port(), it->objects,
              it->secure_connection_config.is_secure() ? 0 :
                access_decision, orb_id));

          endpoints_.push_back(new_endpoint);

          for (EndpointObjectTable::const_iterator object_it =
            it->objects.begin();
            object_it != it->objects.end(); ++object_it)
          {
            object_to_endpoints_[object_it->first].push_back(new_endpoint);
          }
        }
      }

      // Orb has been created and initialized by here
      orbs_.emplace_back(orb);
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << ex.what() << " (Probably failed to bind with ";
      bool is_secure = secure_config.is_secure();
      for (EndpointConfigs::const_iterator itor(endpoints.begin());
        itor != endpoints.end(); ++itor)
      {
        if (itor != endpoints.begin())
        {
          ostr << " ,";
        }
        ostr << itor->host << ":" << itor->bind_port();
        if (is_secure)
        {
          ostr << ", " << itor->host << ":" << itor->port;
        }
        ostr << ")";
      }
      throw Exception(ostr);
    }
  }

  void
  CorbaServerAdapter::shutdown(bool type) throw ()
  {
    Sync::PosixGuard guard_(mutex_);

    if (shutdown_complete_)
    {
      return;
    }

    for (Orbs::iterator itor(orbs_.begin()); itor != orbs_.end(); ++itor)
    {
      try
      {
        itor->orb->shutdown(type);
      }
      catch (...)
      {
      }
    }

    shutdown_complete_ = true;
  }

  OrbShutdowner_var
  CorbaServerAdapter::shutdowner() throw ()
  {
    add_ref();
    return OrbShutdowner_var(static_cast<OrbShutdowner*>(this));
  }

  void
  CorbaServerAdapter::get_threads_usage(ThreadsUsage& usage)
    throw (eh::Exception)
  {
    unsigned waiting = 0, running;
    {
      Sync::PosixGuard guard(threads_mutex_);
      running = threads_running_;
      for (Orbs::const_iterator itor(orbs_.begin());
        itor != orbs_.end(); ++itor)
      {
        waiting += itor->waiters;
      }
    }
    usage.total = running;
    usage.waiting = waiting;
    usage.working = usage.total - waiting;
  }

  //
  // CorbaServerAdapter::POACreator class
  //

  CorbaServerAdapter::POACreator::POACreator(CORBA::ORB_ptr orb,
    PortableServer::POA_ptr root_poa) throw (eh::Exception)
    : orb_(CORBA::ORB::_duplicate(orb)),
      root_poa_(PortableServer::POA::_duplicate(root_poa))
  {
    poa_manager_factory_ = root_poa_->the_POAManagerFactory();
    policies_.length(3);
    policies_[0] = policies_var[0] =
      root_poa_->create_lifespan_policy(PortableServer::PERSISTENT);
    policies_[1] = policies_var[1] =
      root_poa_->create_id_uniqueness_policy(PortableServer::MULTIPLE_ID);
    policies_[2] = policies_var[2] =
      root_poa->create_id_assignment_policy(PortableServer::USER_ID);
  }

  void
  CorbaServerAdapter::POACreator::create_poa(const char* suffix,
    const char* host, int port, PortableServer::POAManager_out poa_manager,
    PortableServer::POA_out poa) throw (eh::Exception)
  {
    const char POA_MANAGER_NAME_PREFIX[] = "POAManager_";
    const char POA_NAME_PREFIX[] = "POA_";

    std::ostringstream poa_name;
    poa_name << POA_NAME_PREFIX << suffix;

    {
      std::ostringstream poa_manager_name;

      poa_manager_name << POA_MANAGER_NAME_PREFIX << suffix;

      EndpointPolicy::EndpointValueBase_var endpoint =
        new IIOPEndpointValue_i(host, port);
      EndpointPolicy::EndpointList list;
      list.length(1);
      list[0] = endpoint;

      CORBA::Any policy_value;
      policy_value <<= list;

      CORBA::Policy_var policy = orb_->create_policy(
        EndpointPolicy::ENDPOINT_POLICY_TYPE,
        policy_value);
      CORBA::PolicyList manager_policies;
      manager_policies.length(1);
      manager_policies[0] = policy;

      poa_manager = poa_manager_factory_->create_POAManager(
        poa_manager_name.str().c_str(), manager_policies);
    }

    poa = root_poa_->create_POA(poa_name.str().c_str(), poa_manager,
      policies_);
  }
}
