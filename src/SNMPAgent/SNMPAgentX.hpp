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



#ifndef SNMP_AGENTX_HPP
#define SNMP_AGENTX_HPP

#include <deque>
#include <vector>

#include <Generics/Singleton.hpp>
#include <Generics/Descriptors.hpp>
#include <Generics/Values.hpp>
#include <Generics/ThreadRunner.hpp>
#include <Generics/ArrayAutoPtr.hpp>

#include <Logger/Logger.hpp>


namespace SNMPAgentX
{
  namespace Helper
  {
    int
    proxy(void* reg, void* requests)
      throw ();
  }

  /**
   * Generics SNMP AgentX class
   * Loads MIB files, provides bindings for specified MIB subtree (using
   * passed unique identifier as an index to distinguish values of the
   * same kind of different processes.
   * As far as only one class may use net-snmp simultaneously
   * Generics::Unique<GenericSNMPAgent> is used to prevent creation
   * of several implementations to be created concurrently.
   */
  class GenericSNMPAgent : private Generics::Unique<GenericSNMPAgent>
  {
  public:
    typedef Generics::Unique<GenericSNMPAgent>::Exception Exception;

    /**
     * Constructor
     * Create list of oids. Binding occurs by request.
     * @param logger logger to use for net-snmp messages
     * @param profile net-snmp specific profile
     * @param root root of MIB subtree to bind
     * @param directory colon-separated list of directories with MIB
     * files to load
     * @param agentx_socket address of agentx master
     */
    GenericSNMPAgent(Logging::Logger* logger, const char* profile,
      const char* root, const char* directory,
      const char* agentx_socket = 0)
      throw (eh::Exception, Exception);

    /**
     * Destructor
     * Unbinds oids
     */
    virtual
    ~GenericSNMPAgent()
      throw ();

    /**
     * Breaks the main cycle
     */
    void
    stop()
      throw (eh::Exception);


  public:
    struct RootInfo;

    struct VariableInfo
    {
      enum VarType
      {
        VT_ULONG,
        VT_LONG,
        VT_ULONG64,
        VT_STRING
      };

      VariableInfo()
        throw (eh::Exception);
      VariableInfo(const VariableInfo& info)
        throw (eh::Exception);
      VariableInfo(const RootInfo& root, size_t length, void* data,
        const std::string& name, VarType type)
        throw (eh::Exception);

      static const size_t OID_SIZE;

      const RootInfo* root;
      size_t oid_length;
      Generics::ArrayByte oid;
      Generics::Values::Key name;
      VarType type;
    };

    typedef std::deque<VariableInfo> Vars;

    typedef std::map<int, std::string> EnumValue;
    typedef std::vector<EnumValue> EnumInfo;

    struct RootInfo
    {
      explicit
      RootInfo(GenericSNMPAgent* agent)
        throw ();
      RootInfo(RootInfo& root, const std::string& prefix,
        size_t index_length)
        throw (eh::Exception);

      GenericSNMPAgent* agent;
      Generics::Values::Key prefix;
      EnumInfo indices;
      Vars vars;

      /**
       * Notifies the main cycle to register loaded mib variables for
       * the specified id
       * @param size size of ids
       * @param ids unique identifier to use as an index
       */
      void
      register_index(size_t size, const unsigned* ids) const
        throw (eh::Exception);
    };


    static
    void
    set_variable(void* variable, unsigned long value)
      throw ();
    static
    void
    set_variable(void* variable, long value)
      throw ();
    static
    void
    set_variable64(void* variable, unsigned long value)
      throw ();
    static
    void
    set_variable(void* variable, const String::SubString& value)
      throw ();

    template <typename Values>
    static
    bool
    set_variable_from_values(void* variable, const VariableInfo& info,
      Values& values)
      throw (eh::Exception);

    /**
     * Returns root information for the specified prefix
     * @param prefix empty or name of the sequence
     * @return the root info for the prefix or NULL
     */
    const RootInfo*
    get_rootinfo(const Generics::Values::Key& prefix =
      Generics::Values::Key())
      throw (eh::Exception);

  protected:
    /**
     * Performs main loop on listening AgentX requests and processing them
     */
    void
    main_loop_()
      throw (eh::Exception);

    /**
     * Called by main_loop_ to process GET request on previously bound
     * oid
     * @param variable net-snmp variable
     * @param info bind information
     * @param size size of ids
     * @param ids unique identifier used as index
     */
    virtual
    bool
    process_variable_(void* variable, const VariableInfo& info,
      unsigned size, const unsigned* ids)
      throw (eh::Exception, Exception) = 0;

    /**
     * Callback for logging process_variable_ failure
     * @param info variable requested
     * @param size size of ids
     * @param ids ids requested
     * @param reason failure reason
     */
    virtual
    void
    no_such_value_(const VariableInfo& info, unsigned size,
      const unsigned* ids, const char* reason)
      throw ();

    /**
     * Logging severity for no_such_value_ default behaviour.
     * @return severity for logging
     */
    virtual
    unsigned
    no_such_value_severity_()
      throw ();

    mutable Logging::FLogger_var logger_;

  private:
    /**
     * Notifies the main cycle to register loaded mib variables for
     * the specified id
     * @param root subtree to register
     * @param size size of ids
     * @param ids unique identifier to use as an index
     */
    void
    register_index_(const RootInfo* root, size_t size, const unsigned* ids)
      throw (eh::Exception, Exception);

    /**
     * Registers loaded mib variables for specified id
     * @param root subtree to register
     * @param size size of ids
     * @param ids unique identifier to use as an index
     */
    void
    do_register_index_(const RootInfo* root, size_t size,
      const unsigned* ids)
      throw (eh::Exception, Exception);

    friend
    int
    SNMPAgentX::Helper::proxy(void* reginfo, void* requests)
      throw ();

    friend
    struct RootInfo;

    typedef std::deque<RootInfo> Roots;


    /**
     * Information for variables distinguishing and unbinding
     */
    struct RegInfo
    {
      /**
       * Constructor
       * @param agent agent to use for call
       * @param info bind information
       * @param registration net-snmp registration information for unbind
       */
      RegInfo(GenericSNMPAgent* agent, const VariableInfo* info,
        void* registration)
        throw ();

      GenericSNMPAgent* agent;
      const VariableInfo* info;
      void* registration;
    };
    typedef std::deque<RegInfo*> Registrations;


    /**
     * Performs calls to process_variable_ virtual function
     * @param info bind information
     * @param requests net-snmp requests
     */
    int
    process_requests_(const VariableInfo& info, void* requests)
      throw (eh::Exception);

    /**
     * Callback for net-snmp logger
     * @param major
     * @param minor
     * @param serverarg
     * @param clientarg
     */
    static
    int
    log_callback_(int major, int minor, void* serverarg, void* clientarg)
      throw ();

    /**
     * Puts the net-snmp log message to the logger
     * @param arg net-snmp log message
     */
    void
    log_handler_(void* arg)
      throw (eh::Exception);

    /**
     * Create list of all variables for later binding
     * @param prefix prefix to use for key
     * @param node MIB subtree node
     * @param curoid corresponding oid for the node
     * @param length length of the oid
     */
    void
    list_values_(RootInfo& root, std::string prefix,
      void* node, void* curoid, size_t length)
      throw (eh::Exception);


    Sync::PosixMutex mutex_;
    Generics::Pipe pipe_;
    std::string profile_;
    Roots roots_;
    Registrations registrations_;
  };

  /**
   * Starts SNMP processing in the separate thread on creation.
   * Thread is terminated by the object destruction.
   */
  class SNMPAgentAsync
  {
  public:
    typedef GenericSNMPAgent::Exception Exception;

    /**
     * Destructor
     * Terminates working thread and waits for its termination.
     */
    virtual
    ~SNMPAgentAsync()
      throw ();

  protected:
    /**
     * ThreadJob for ThreadRunner. Performs SNMP processing.
     */
    class SNMPJob :
      public Generics::ThreadJob,
      public GenericSNMPAgent
    {
    public:
      /**
       * Constructor
       * @param logger logger to use for net-snmp messages
       * @param profile net-snmp specific profile
       * @param root root of MIB subtree to bind
       * @param directory colon-separated list of directories with MIB
       * files to load
       * @param agentx_socket address of agentx master
       */
      SNMPJob(Logging::Logger* logger, const char* profile,
        const char* root, const char* directory,
        const char* agentx_socket)
        throw (eh::Exception, Exception);

      /**
       * Runs main_loop_().
       */
      virtual
      void
      work()
        throw ();

    protected:
      /**
       * Destructor
       */
      virtual
      ~SNMPJob()
        throw ();
    };
    typedef ReferenceCounting::FixedPtr<SNMPJob> SNMPJob_var;

    /**
     * Constructor
     * Creates a working thread.
     * @param job descendant of SNMPJob ready to answer SNMP requests
     */
    SNMPAgentAsync(SNMPJob* job)
      throw (eh::Exception);

    GenericSNMPAgent& agent_;
    Generics::ThreadRunner thread_runner_;
  };

  template <typename Values>
  class ValuesProcessor
  {
  public:
    explicit
    ValuesProcessor(unsigned id)
      throw ();

    void
    register_ids(GenericSNMPAgent* agent) const
      throw (eh::Exception);

    bool
    process_variable(void* variable,
      const GenericSNMPAgent::VariableInfo& info,
      unsigned size, const unsigned* ids, const Values* values) const
      throw (eh::Exception);

  protected:
    unsigned id_;
  };

  /**
   * Implementation of SNMP AgentX subagent using generalized Values as
   * a container of values for bound variables
   */
  template <typename Values, typename Processor = ValuesProcessor<Values> >
  class SNMPStatsGen :
    protected SNMPAgentAsync,
    public ReferenceCounting::AtomicImpl
  {
  public:
    using SNMPAgentAsync::Exception;

    /**
     * Constructor
     * @param stats container with values
     * @param logger logger to use for net-snmp messages
     * @param profile net-snmp specific profile
     * @param root root of MIB subtree to bind
     * @param processor_initializer initializer for values processor
     * @param directory colon-separated list of directories with MIB
     * files to load
     * @param agentx_socket address of agentx master
     */
    template <typename ProcessorInitializer>
    SNMPStatsGen(Values* stats, ProcessorInitializer processor_initializer,
      Logging::Logger* logger, const char* profile, const char* root,
      const char* directory, const char* agentx_socket = 0)
      throw (eh::Exception, Exception);

  protected:
    /**
     * Destructor
     */
    virtual
    ~SNMPStatsGen()
      throw ();

    class SNMPStatsImplJob : public SNMPJob
    {
    public:
      template <typename ProcessorInitializer>
      SNMPStatsImplJob(Values* stats,
        ProcessorInitializer processor_initializer,
        Logging::Logger* logger, const char* profile, const char* root,
        const char* directory, const char* agentx_socket)
        throw (eh::Exception, Exception);

    protected:
      virtual
      ~SNMPStatsImplJob()
        throw ();

      /**
       * Processing GET request on previously bound oid by looking for the
       * value in the container
       * @param variable net-snmp variable
       * @param info bind information
       * @param size unique identifier size
       * @param ids unique identifier used as an index
       */
      virtual
      bool
      process_variable_(void* variable, const VariableInfo& info,
        unsigned size, const unsigned* ids)
        throw (eh::Exception, Exception);

    private:
      ReferenceCounting::FixedPtr<Values> stats_;
      Processor processor_;
    };
  };

  /**
   * Implementation of SNMP AgentX subagent using Generics::Values as
   * a container of values for bound variables
   */
  typedef SNMPStatsGen<Generics::Values> SNMPStatsImpl;
  typedef ReferenceCounting::QualPtr<SNMPStatsImpl> SNMPStatsImpl_var;
}

//
// INLINES
//

namespace SNMPAgentX
{
  //
  // GenericSNMPAgent::VariableInfo class
  //

  inline
  GenericSNMPAgent::VariableInfo::VariableInfo()
    throw (eh::Exception)
  {
  }

  inline
  GenericSNMPAgent::VariableInfo::VariableInfo(const VariableInfo& info)
    throw (eh::Exception)
    : root(info.root), oid_length(info.oid_length),
      oid(info.oid_length * OID_SIZE), name(info.name), type(info.type)
  {
    memcpy(oid.get(), info.oid.get(), oid_length * OID_SIZE);
  }

  inline
  GenericSNMPAgent::VariableInfo::VariableInfo(const RootInfo& root,
    size_t length, void* data, const std::string& name, VarType type)
    throw (eh::Exception)
    : root(&root), oid_length(length), oid(length * OID_SIZE),
      name(name), type(type)
  {
    memcpy(oid.get(), data, length * OID_SIZE);
  }


  //
  // GenericSNMPAgent class
  //

  template <typename Values>
  bool
  GenericSNMPAgent::set_variable_from_values(void* variable,
    const VariableInfo& info, Values& values)
    throw (eh::Exception)
  {
    switch (info.type)
    {
    case VariableInfo::VT_ULONG:
      {
        Generics::Values::UnsignedInt value = 0;
        if (!values.get(info.name, value))
        {
          return false;
        }
        set_variable(variable, value);
        break;
      }

    case VariableInfo::VT_LONG:
      {
        Generics::Values::SignedInt value = 0;
        if (!values.get(info.name, value))
        {
          return false;
        }
        set_variable(variable, value);
        break;
      }

    case VariableInfo::VT_ULONG64:
      {
        Generics::Values::UnsignedInt value = 0;
        if (!values.get(info.name, value))
        {
          return false;
        }
        set_variable64(variable, value);
        break;
      }

    case VariableInfo::VT_STRING:
      try
      {
        Generics::Values::Floating value = 0;
        if (!values.get(info.name, value))
        {
          return false;
        }
        char str[256];
        set_variable(variable,
          String::SubString(str, snprintf(str, sizeof(str), "%f", value)));
      }
      catch (const Generics::Values::InvalidType&)
      {
        Generics::Values::String value;
        if (!values.get(info.name, value))
        {
          return false;
        }
        set_variable(variable, value);
      }
      break;
    }

    return true;
  }


  //
  // ValuesProcessor class
  //

  template <typename Values>
  ValuesProcessor<Values>::ValuesProcessor(unsigned id)
    throw ()
    : id_(id)
  {
  }

  template <typename Values>
  void
  ValuesProcessor<Values>::register_ids(GenericSNMPAgent* agent) const
    throw (eh::Exception)
  {
    if (const GenericSNMPAgent::RootInfo* root = agent->get_rootinfo())
    {
      root->register_index(1, &id_);
    }
  }

  template <typename Values>
  bool
  ValuesProcessor<Values>::process_variable(void* variable,
    const GenericSNMPAgent::VariableInfo& info,
    unsigned /*size*/, const unsigned* /*ids*/, const Values* values) const
    throw (eh::Exception)
  {
    return GenericSNMPAgent::set_variable_from_values(
      variable, info, *values);
  }


  //
  // SNMPStatsGen::SNMPStartImplJob class
  //

  template <typename Values, typename Processor>
  template <typename ProcessorInitializer>
  SNMPStatsGen<Values, Processor>::SNMPStatsImplJob::SNMPStatsImplJob(
    Values* stats, ProcessorInitializer processor_initializer,
    Logging::Logger* logger, const char* profile, const char* root,
    const char* directory, const char* agentx_socket)
    throw (eh::Exception, Exception)
    : SNMPJob(logger, profile, root, directory, agentx_socket),
      stats_(ReferenceCounting::add_ref(stats)),
      processor_(processor_initializer)
  {
    processor_.register_ids(this);
  }

  template <typename Values, typename Processor>
  bool
  SNMPStatsGen<Values, Processor>::SNMPStatsImplJob::
    process_variable_(void* variable, const VariableInfo& info,
    unsigned size, const unsigned* ids)
    throw (eh::Exception, Exception)
  {
    return processor_.process_variable(variable, info, size, ids, stats_);
  }

  template <typename Values, typename Processor>
  SNMPStatsGen<Values, Processor>::SNMPStatsImplJob::~SNMPStatsImplJob()
    throw ()
  {
  }


  //
  // SNMPStatsGen class
  //

  template <typename Values, typename Processor>
  template <typename ProcessorInitializer>
  SNMPStatsGen<Values, Processor>::SNMPStatsGen(Values* stats,
    ProcessorInitializer processor_initializer, Logging::Logger* logger,
    const char* profile, const char* root,
    const char* directory, const char* agentx_socket)
    throw (eh::Exception, Exception)
    : SNMPAgentAsync(SNMPJob_var(new SNMPStatsImplJob(
        stats, processor_initializer, logger,
        profile, root, directory, agentx_socket)))
  {
  }

  template <typename Values, typename Processor>
  SNMPStatsGen<Values, Processor>::~SNMPStatsGen()
    throw ()
  {
  }
}

#endif
