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



#include <String/StringManip.hpp>

#include <Generics/FDSetSize.hpp>

#include <SNMPAgent/SNMPAgentX.hpp>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>


//#define SNMP_DEBUG

#ifdef SNMP_DEBUG
#include <iostream>
#endif

namespace
{
  const char MEDIATOR('.');
}

namespace SNMPAgentX
{
  namespace Helper
  {
    inline
    int
    proxy(void* reginfo, void* requests)
      throw ()
    {
      try
      {
        GenericSNMPAgent::RegInfo* reg =
          static_cast<GenericSNMPAgent::RegInfo*>(reginfo);
        return reg->agent->process_requests_(*reg->info, requests);
      }
      catch (...)
      {
      }

      return SNMP_ERR_GENERR;
    }
  }
}

namespace
{
  int
  request_handler(netsnmp_mib_handler* /*handler*/,
    netsnmp_handler_registration* reginfo,
    netsnmp_agent_request_info* reqinfo,
    netsnmp_request_info* requests)
    throw ()
  {
    return reqinfo->mode != MODE_GET ? SNMP_ERR_NOERROR :
      SNMPAgentX::Helper::proxy(reginfo->my_reg_void, requests);
  }

#ifdef SNMP_DEBUG
  void
  print_name(const oid* name, size_t name_length)
  {
    while (name_length--)
    {
      std::cout << "." << static_cast<unsigned>(*name++);
    }
  }
#endif
}

namespace SNMPAgentX
{
  //
  // GenericSNMPAgent::VariableInfo class
  //

  const size_t GenericSNMPAgent::VariableInfo::OID_SIZE = sizeof(::oid);


  //
  // GenericSNMPAgent::RootInfo class
  //

  GenericSNMPAgent::RootInfo::RootInfo(GenericSNMPAgent* agent)
    throw ()
    : agent(agent)
  {
  }

  GenericSNMPAgent::RootInfo::RootInfo(RootInfo& root,
    const std::string& prefix, size_t index_length)
    throw (eh::Exception)
    : agent(root.agent), prefix(prefix), indices(index_length)
  {
  }

  void
  GenericSNMPAgent::RootInfo::register_index(size_t size,
    const unsigned* ids) const
    throw (eh::Exception)
  {
    agent->register_index_(this, size, ids);
  }


  //
  // GenericSNMPAgent::RegInfo class
  //

  GenericSNMPAgent::RegInfo::RegInfo(GenericSNMPAgent* agent,
    const VariableInfo* info, void* registration)
    throw ()
    : agent(agent), info(info), registration(registration)
  {
  }


  //
  // GenericSNMPAgent class
  //

  GenericSNMPAgent::GenericSNMPAgent(Logging::Logger* logger,
    const char* profile, const char* root,
    const char* directory, const char* agentx_socket)
    throw (eh::Exception, Exception)
    : logger_(ReferenceCounting::add_ref(logger)), profile_(profile)
  {
    // Init logging
    GenericSNMPAgent** arg = static_cast<GenericSNMPAgent**>(
      malloc(sizeof(GenericSNMPAgent*)));
    *arg = this;
    if (snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_LOGGING,
      log_callback_, arg) != SNMPERR_SUCCESS)
    {
      Stream::Error ostr;
      ostr << FNS << "failed to register logger callback";
      throw Exception(ostr);
    }
    snmp_enable_calllog();

    // Init agent
    if (agentx_socket)
    {
      netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID,
        NETSNMP_DS_AGENT_X_SOCKET, agentx_socket);
    }
    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID,
      NETSNMP_DS_AGENT_ROLE, 1);
    init_agent(profile_.c_str());

#ifdef SNMP_DEBUG
    snmp_set_do_debugging(1000);
#endif

    if (directory && *directory)
    {
      String::SubString s(directory);
      String::StringManip::SplitColon tokenizer(s);
      String::SubString token;
      while (tokenizer.get_token(token))
      {
        add_mibdir(token.str().c_str());
      }
    }

    read_all_mibs();

    // Determine root of mapped values
    oid curoid[1024];
    size_t size = sizeof(curoid) / sizeof(*curoid) / 2;
    if (!snmp_parse_oid(root, curoid, &size))
    {
      Stream::Error ostr;
      ostr << FNS << "failed to determine root oid";
      throw Exception(ostr);
    }

    struct tree* nodes = get_tree(curoid, size, get_tree_head());
    if (!nodes)
    {
      Stream::Error ostr;
      ostr << FNS << "requested subtree is empty";
      throw Exception(ostr);
    }

    roots_.emplace_back(this);
    list_values_(roots_.front(), std::string(), nodes, curoid, size);

    init_snmp(profile_.c_str());
  }

  GenericSNMPAgent::~GenericSNMPAgent()
    throw ()
  {
#ifdef SNMP_DEBUG
    snmp_set_do_debugging(0);
#endif

    snmp_shutdown(profile_.c_str());

    while (!registrations_.empty())
    {
      RegInfo* reg = registrations_.front();
      registrations_.pop_front();
      if (reg)
      {
        netsnmp_unregister_handler(
          static_cast<netsnmp_handler_registration*>(reg->registration));
        delete reg;
      }
    }

    snmp_unregister_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_LOGGING,
      log_callback_, this, 1);

    shutdown_agent();
  }

  void
  GenericSNMPAgent::stop()
    throw (eh::Exception)
  {
    Sync::PosixGuard guard(mutex_);

    const RootInfo* ZERO = 0;
    pipe_.write_n(&ZERO, sizeof(ZERO));
  }

  void
  GenericSNMPAgent::do_register_index_(const RootInfo* root,
    size_t size, const unsigned* ids)
    throw (eh::Exception, Exception)
  {
    oid curoid[1024];

    for (Vars::const_iterator itor(root->vars.begin());
      itor != root->vars.end(); ++itor)
    {
      const char* cname = itor->name.text().c_str();

      registrations_.push_back(0);

      netsnmp_mib_handler* handler =
        netsnmp_create_handler(cname, &request_handler);
      if (!handler)
      {
        Stream::Error ostr;
        ostr << FNS << "failed to create handler";
        throw Exception(ostr);
      }

      memcpy(curoid, &itor->oid[0], itor->oid_length * sizeof(*curoid));
      std::copy(ids, ids + size, curoid + itor->oid_length);

      netsnmp_handler_registration* registration =
        netsnmp_handler_registration_create(cname, handler,
          curoid, itor->oid_length + size, HANDLER_CAN_DEFAULT);
      if (!registration)
      {
        netsnmp_handler_free(handler);
        Stream::Error ostr;
        ostr << FNS << "failed to create registration";
        throw Exception(ostr);
      }

      std::unique_ptr<RegInfo> reg;
      {
        try
        {
          reg.reset(new RegInfo(this, &*itor, registration));
        }
        catch (...)
        {
          netsnmp_handler_registration_free(registration);
          throw;
        }
      }

      registration->my_reg_void = reg.get();

      if (netsnmp_register_handler(registration) != SNMP_ERR_NOERROR)
      {
        Stream::Error ostr;
        ostr << FNS << "failed to register handler";
        throw Exception(ostr);
      }

      registrations_.back() = reg.get();
      reg.release();
    }
  }

  const GenericSNMPAgent::RootInfo*
  GenericSNMPAgent::get_rootinfo(const Generics::Values::Key& prefix)
    throw (eh::Exception)
  {
    for (Roots::const_iterator itor(roots_.begin()); itor != roots_.end();
      ++itor)
    {
      if (itor->prefix == prefix)
      {
        return &*itor;
      }
    }
    return 0;
  }

  void
  GenericSNMPAgent::register_index_(const RootInfo* root,
    size_t size, const unsigned* ids)
    throw (eh::Exception, Exception)
  {
    Sync::PosixGuard guard(mutex_);

    pipe_.write_n(&root, sizeof(root));
    pipe_.write_n(&size, sizeof(size));
    pipe_.write_n(ids, size * sizeof(*ids));
  }

  void
  GenericSNMPAgent::main_loop_()
    throw (eh::Exception)
  {
    for (;;)
    {
      Generics::FDSet readset;
      FD_SET(pipe_.read_descriptor(), &readset);
      int descriptors = pipe_.read_descriptor() + 1;
      int block = 0;
      Generics::Time timeout(1);
      Generics::Time block_timeout(1);

      {
        snmp_select_info(&descriptors, &readset, &timeout, &block);

        int result = select(descriptors, &readset, 0, 0,
          block ? &block_timeout : &timeout);
        if (result < 0)
        {
          if (errno == EINTR)
          {
            continue;
          }
          eh::throw_errno_exception<Exception>(FNE, "select failure");
        }

        if (FD_ISSET(pipe_.read_descriptor(), &readset))
        {
          const RootInfo* root;
          size_t size;
          unsigned ids[1024];
          pipe_.read_n(&root, sizeof(root));
          if (!root)
          {
            break;
          }
          pipe_.read_n(&size, sizeof(size));
          pipe_.read_n(ids, size * sizeof(*ids));
          do_register_index_(root, size, ids);

          result--;
        }

        if (result)
        {
          snmp_read(&readset);
        }
        else
        {
          snmp_timeout();
        }

        run_alarms();
        netsnmp_check_outstanding_agent_requests();
      }
    }
  }

  int
  GenericSNMPAgent::process_requests_(
    const VariableInfo& info, void* requests)
    throw (eh::Exception)
  {
    for (netsnmp_request_info* request =
      static_cast<netsnmp_request_info*>(requests); request;
      request = request->next)
    {
      for (netsnmp_variable_list* variable = request->requestvb; variable;
        variable = variable->next_variable)
      {
#ifdef SNMP_DEBUG
        std::cout << variable << "\t" << info.root << " \t";
        print_name(variable->name, variable->name_length);
        std::cout << " \t" << static_cast<unsigned>(variable->type) <<
          std::endl;
#endif

        bool success = true;
        unsigned size = variable->name_length - info.oid_length;
        unsigned ids[1024];
        std::copy(variable->name + info.oid_length,
          variable->name + variable->name_length, ids);
        try
        {
          if (!process_variable_(variable, info, size, ids))
          {
            no_such_value_(info, size, ids, "no such value");
            success = false;
          }
        }
        catch (const eh::Exception& ex)
        {
          no_such_value_(info, size, ids, ex.what());
          success = false;
        }

        if (!success)
        {
          netsnmp_request_set_error(request, SNMP_ERR_NOSUCHNAME);
        }
      }
    }

    return SNMP_ERR_NOERROR;
  }

  void
  GenericSNMPAgent::no_such_value_(const VariableInfo& info, unsigned size,
    const unsigned* ids, const char* reason)
    throw ()
  {
    Stream::Error ostr;
    ostr << FNS << "failed to process variable " << info.name << "[";
    for (unsigned i = 0; i < size; i++)
    {
      (i ? ostr << "." : ostr) << ids[i];
    }
    ostr << "]: " << reason;
    logger_->log(ostr.str(), no_such_value_severity_());
  }

  unsigned
  GenericSNMPAgent::no_such_value_severity_()
    throw ()
  {
    return Logging::Logger::DEBUG;
  }

  void
  GenericSNMPAgent::log_handler_(void* arg)
    throw (eh::Exception)
  {
    struct snmp_log_message* slm =
      static_cast<struct snmp_log_message*>(arg);
    unsigned long severity = Logging::Logger::TRACE;

    if (slm->priority == LOG_ERR)
    {
      static const char EXPECTED[] = "read_config_store open failure on ";
      if (!memcmp(slm->msg, EXPECTED, sizeof(EXPECTED) - 1))
      {
        return;
      }
    }

    switch (slm->priority)
    {
    case LOG_EMERG:
      severity = Logging::Logger::EMERGENCY;
      break;
    case LOG_ALERT:
      severity = Logging::Logger::ALERT;
      break;
    case LOG_CRIT:
      severity = Logging::Logger::CRITICAL;
      break;
    case LOG_ERR:
      severity = Logging::Logger::ERROR;
      break;
    case LOG_WARNING:
      severity = Logging::Logger::WARNING;
      break;
    case LOG_NOTICE:
      severity = Logging::Logger::NOTICE;
      break;
    case LOG_INFO:
      severity = Logging::Logger::INFO;
      break;
    case LOG_DEBUG:
      severity = Logging::Logger::DEBUG;
      break;
    }

    logger_->log(String::SubString(slm->msg), severity);
  }

  int
  GenericSNMPAgent::log_callback_(int /*major*/, int /*minor*/,
    void* serverarg, void* clientarg)
    throw ()
  {
    try
    {
      (*static_cast<GenericSNMPAgent**>(clientarg))->log_handler_(serverarg);
    }
    catch (...)
    {
    }

    return 0;
  }

  void
  GenericSNMPAgent::list_values_(RootInfo& parent, std::string prefix,
    void* node, void* cur_oid, size_t length)
    throw (eh::Exception)
  {
    struct tree* pn = static_cast<struct tree*>(node);

    RootInfo* root = &parent;
    if (pn->indexes)
    {
      unsigned index_length = 0;
      for (struct index_list* index = pn->indexes; index;
        index = index->next)
      {
        index_length++;
      }
      roots_.emplace_back(parent, prefix, index_length);
      root = &roots_.back();
      prefix.clear();
    };

    if (!prefix.empty())
    {
      prefix.push_back(MEDIATOR);
    }

    oid* curoid = static_cast<oid*>(cur_oid);
    length++;

    // Translate textual-conventions of indices
    bool has_children = false;
    for (struct tree* child = pn->child_list; child;
      child = child->next_peer)
    {
      if (child->child_list)
      {
        has_children = true;
      }
      if (struct index_list* index = pn->indexes)
      {
        size_t iindex = 0;
        do
        {
          if (!strcmp(child->label, index->ilabel))
          {
            break;
          }
          iindex++;
        }
        while ((index = index->next));
        if (index)
        {
          if (child->tc_index != -1)
          {
            if (struct enum_list* en = child->enums)
            {
              assert(iindex < root->indices.size());
              EnumValue& ev = root->indices[iindex];
              do
              {
                ev[en->value] = en->label;
              }
              while ((en = en->next));
            }
          }
        }
      }
    }

    RootInfo* sequence_root = nullptr;
    // Check if 
    // 1. it's a sequence and
    // 2. at least one index and
    // 3. the first index is TEXTUAL-CONVENSION and
    // 4. no children
    if (pn->indexes && !root->indices[0].empty() && !has_children)
    {
      // if it has only one index
      if (!pn->indexes->next)
      {
        sequence_root = &roots_.front();
      }
      else
      {
        roots_.emplace_back(parent,
          root->prefix.text() + MEDIATOR + pn->indexes->ilabel, 0);
        sequence_root = &roots_.back();
      }
    }

    // Map all of the values
    for (struct tree* child = pn->child_list; child;
      child = child->next_peer)
    {
      const std::string& var_name = prefix + child->label;

#ifdef SNMP_DEBUG
      std::cout << root << "\t" << var_name << "\n" <<
        static_cast<unsigned>(child->subid) << " \t" << child->type <<
        " \t" << child->access << " \t" << child->status << " \t" <<
        child->tc_index << " \t" << child->enums << " \t" <<
        child->ranges << " \t" << child->indexes << " \t" <<
        child->varbinds << std::endl;
#endif

      // Skip indices
      if (struct index_list* index = pn->indexes)
      {
        size_t iindex = 0;
        do
        {
          if (!strcmp(child->label, index->ilabel))
          {
            break;
          }
          iindex++;
        }
        while ((index = index->next));
        if (index)
        {
          continue;
        }
      }

      curoid[length - 1] = child->subid;

      VariableInfo::VarType type;
      switch (child->type)
      {
      case TYPE_OTHER:
        list_values_(*root, var_name, child, curoid, length);
        continue;
      case TYPE_UNSIGNED32:
        type = VariableInfo::VT_ULONG;
        break;
      case TYPE_INTEGER32:
        type = VariableInfo::VT_LONG;
        break;
      case TYPE_COUNTER64:
        type = VariableInfo::VT_ULONG64;
        break;
      case TYPE_OCTETSTR:
        type = VariableInfo::VT_STRING;
        break;
      default:
        logger_->stream(Logging::Logger::ERROR) << FNS <<
          "invalid type of variable " << child->label;
        continue;
      }

      if (child->access != MIB_ACCESS_READONLY)
      {
        logger_->stream(Logging::Logger::ERROR) << FNS <<
          "invalid access rights for variable " << child->label;
        continue;
      }

      root->vars.emplace_back(*root, length, curoid, var_name, type);
      if (sequence_root)
      {
        const EnumValue& ev = root->indices[0];
        for (EnumValue::const_iterator itor(ev.begin());
          itor != ev.end(); ++itor)
        {
          curoid[length] = itor->first;
          const std::string& field = var_name + MEDIATOR + itor->second;
          sequence_root->vars.emplace_back(*sequence_root, length + 1,
            curoid, field, type);
        }
      }
    }
  }

  void
  GenericSNMPAgent::set_variable(void* variable, unsigned long value)
    throw ()
  {
    snmp_set_var_typed_value(static_cast<netsnmp_variable_list*>(variable),
      ASN_UNSIGNED, reinterpret_cast<const u_char*>(&value), sizeof(value));
  }

  void
  GenericSNMPAgent::set_variable(void* variable, long value)
    throw ()
  {
    snmp_set_var_typed_value(static_cast<netsnmp_variable_list*>(variable),
      ASN_INTEGER, reinterpret_cast<const u_char*>(&value), sizeof(value));
  }

  void
  GenericSNMPAgent::set_variable64(void* variable, unsigned long value)
    throw ()
  {
    counter64 v;
    v.high = value >> 32;
    v.low = value & ((1lu << 32) - 1);
    snmp_set_var_typed_value(static_cast<netsnmp_variable_list*>(variable),
      ASN_COUNTER64, reinterpret_cast<const u_char*>(&v), sizeof(v));
  }

  void
  GenericSNMPAgent::set_variable(void* variable,
    const String::SubString& value)
    throw ()
  {
    snmp_set_var_typed_value(static_cast<netsnmp_variable_list*>(variable),
      ASN_OCTET_STR, reinterpret_cast<const u_char*>(value.data()),
      value.size());
  }


  //
  // SNMPAgentAsync::SNMPJob class
  //

  SNMPAgentAsync::SNMPJob::SNMPJob(Logging::Logger* logger,
    const char* profile, const char* root,
    const char* directory, const char* agentx_socket)
    throw (eh::Exception, Exception)
    : GenericSNMPAgent(logger, profile, root, directory, agentx_socket)
  {
  }

  SNMPAgentAsync::SNMPJob::~SNMPJob()
    throw ()
  {
  }

  void
  SNMPAgentAsync::SNMPJob::work()
    throw ()
  {
    try
    {
      main_loop_();
    }
    catch (const eh::Exception& ex)
    {
      logger_->sstream(Logging::Logger::CRITICAL) << FNS <<
        "exception caught: " << ex.what();
    }
  }


  //
  // SNMPAgentAsync class
  //

  SNMPAgentAsync::SNMPAgentAsync(SNMPJob* job)
    throw (eh::Exception)
    : agent_(*job),
      thread_runner_(static_cast<Generics::ThreadJob*>(job), 1)
  {
    thread_runner_.start();
  }

  SNMPAgentAsync::~SNMPAgentAsync()
    throw ()
  {
    agent_.stop();
    //thread_runner_.wait_for_completion(); // will be called in destructor
  }
}

// Workaround for some problems
extern "C"
{
  NETSNMP_INLINE void*
  netsnmp_request_get_list_data(netsnmp_request_info* /*request*/,
    const char* /*name*/)
  {
    return 0;
  }
}
