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





#ifndef APACHE_MODULE_HPP
#define APACHE_MODULE_HPP

#include <httpd/httpd.h>
#include <httpd/http_config.h>
#include <httpd/http_core.h>
#include <httpd/http_log.h>
#include <httpd/http_main.h>
#include <httpd/http_protocol.h>
#include <httpd/http_request.h>
#include <httpd/util_script.h>
#include <httpd/http_connection.h>
#include <apr_strings.h>
#undef strtoul

#include <list>

#include <Generics/ArrayAutoPtr.hpp>


/**
 * Classes to use with conjunction with apache
 */
namespace Apache
{
  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

  /**
   * Hooks base class
   */
  class Hook
  {
  protected:
    Hook(int where) throw ();
    virtual
    ~Hook() throw ();

  public:
    int
    where() throw ();

  private:
    int where_;
  };


  //
  // Various Hooks
  //

  template <typename T>
  class InsertFilterHook : public Hook
  {
  public:
    InsertFilterHook(int where) throw ();

    virtual
    void
    insert_filter(request_rec* r) throw () = 0;

    static
    void
    insert_filter_s(request_rec* r) throw ();
  };

  template <typename T>
  class QuickHandlerHook : public Hook
  {
  public:
    QuickHandlerHook(int where) throw ();

    virtual
    int
    quick_handler(request_rec* r, int lookup_uri) throw () = 0;

    static
    int
    quick_handler_s(request_rec* r, int lookup_uri) throw ();
  };

  template <typename T>
  class HandlerHook : public Hook
  {
  public:
    HandlerHook(int where) throw ();

    virtual
    int
    handler(request_rec* r) throw () = 0;

    static
    int
    handler_s(request_rec* r) throw ();
  };

  template <typename T>
  class ChildInitHook : public Hook
  {
  public:
    ChildInitHook(int where) throw ();

    virtual
    void
    child_init(apr_pool_t* p, server_rec* s) throw () = 0;

    static
    void
    child_init_s(apr_pool_t* p, server_rec* s) throw ();
  };

  template <typename T>
  class PostConfigHook : public Hook
  {
  public:
    PostConfigHook(int where) throw ();

    virtual
    int
    post_config(apr_pool_t* pconf, apr_pool_t* plog,
      apr_pool_t* ptemp, server_rec* s) throw () = 0;

    static
    int
    post_config_s(apr_pool_t* pconf, apr_pool_t* plog, apr_pool_t* ptemp,
      server_rec* s) throw ();
  };

  /**
   * Provides Apache configuration parsing capability.
   * To use derive you own module class from it.
   */
  class ConfigParser
  {
  public:
    /**
     * Encapsulates all configuration directive's info including name,
     * parameter, etc.
     */
    class ConfigArgs
    {
    public:
      DECLARE_EXCEPTION(ArgNotExist, eh::DescriptiveException);

      /**
       * Constructor for no-arg directive.
       */
      ConfigArgs(cmd_parms* cmd, void* mconfig) throw ();

      /**
       * Constructor for flag directive.
       */
      ConfigArgs(cmd_parms* cmd, void* mconfig, int flag) throw ();

      /**
       * Constructor for directive with 1 string parameter.
       */
      ConfigArgs(cmd_parms* cmd, void* mconfig, const char* str1) throw ();

      /**
       * Constructor for directive with at most 2 string parameters.
       */
      ConfigArgs(cmd_parms* cmd, void* mconfig, const char* str1,
        const char* str2) throw ();

      /**
       * Constructor for directive with at most 3 string parameters.
       */
      ConfigArgs(cmd_parms* cmd, void* mconfig, const char* str1,
        const char* str2, const char* str3) throw ();

      cmd_parms*
      command() const throw ();
      void*
      mconfig() const throw ();
      const char*
      name() const throw ();

      int
      flag() const throw (ArgNotExist);
      const char*
      str1() const throw (ArgNotExist);
      const char*
      str2() const throw (ArgNotExist);
      const char*
      str3() const throw (ArgNotExist);

    private:
      enum ArgsType
      {
        AT_NONE,
        AT_FLAG,
        AT_ONE_STR,
        AT_TWO_STRS,
        AT_THREE_STRS
      };

    private:
      ArgsType type_;
      cmd_parms* command_;
      void* mconfig_;

      int flag_;
      const char* str1_;
      const char* str2_;
      const char* str3_;
    };

  public:
    virtual
    ~ConfigParser() throw ();

    /**
     * Add module's directives.
     * @param name Directive's name.
     * @param req_override What overrides need to be allowed to enable
     * this command.
     * @param args_how What the command expects as arguments.
     * @param errmsg What Apache should say if it can't parse this directive.
     */
    void
    add_directive(const char* name, int req_override, cmd_how args_how,
      const char* errmsg) throw (eh::Exception);

    typedef Generics::ArrayAutoPtr<command_rec> Commands;

    /**
     * Returns newly allocated command_rec array.
     */
    Commands
    commands() throw (eh::Exception);

    /**
     * Command handler.
     * This method will be invoked on every module's directive during
     * config file parsing.
     */
    virtual
    const char*
    handle_command(const ConfigArgs& args) throw () = 0;

  private:
    static
    const char*
    handle_NO_ARGS(cmd_parms* cmd, void* mconfig) throw ();
    static
    const char*
    handle_FLAG(cmd_parms* cmd, void* mconfig, int flag) throw ();
    static
    const char*
    handle_TAKE1(cmd_parms* cmd, void* mconfig, const char* word1)
      throw ();
    static
    const char*
    handle_TAKE2(cmd_parms* cmd, void* mconfig, const char* word1,
      const char* word2) throw ();
    static
    const char*
    handle_TAKE3(cmd_parms* cmd, void* mconfig, const char* word1,
      const char* word2, const char* word3) throw ();

  private:
    typedef std::list<command_rec> CommandList;

    CommandList commands_;
  };

  /**
   *
   */
  template <typename T>
  class CreateDirConfigHandler
  {
  public:
    virtual
    ~CreateDirConfigHandler() throw ();

    virtual
    void*
    create_dir_config(apr_pool_t* p, char* dirspec) throw () = 0;

    static
    void*
    create_dir_config_s(apr_pool_t* p, char* dirspec) throw ();
  };

  /**
   *
   */
  template <typename T>
  class MergeDirConfigHandler
  {
  public:
    virtual
    ~MergeDirConfigHandler() throw ();

    virtual
    void*
    merge_dir_config(apr_pool_t* p, void* parent_conf,
      void* newloc_conf) throw () = 0;

    static
    void*
    merge_dir_config_s(apr_pool_t* p, void* parent_conf, void* newloc_conf)
      throw ();
  };

  /**
   *
   */
  template <typename T>
  class CreateServerConfigHandler
  {
  public:
    virtual
    ~CreateServerConfigHandler() throw ();

    virtual
    void*
    create_server_config(apr_pool_t* p, server_rec* s) throw () = 0;

    static
    void*
    create_server_config_s(apr_pool_t* p, server_rec* s) throw ();
  };

  /**
   *
   */
  template <typename T>
  class MergeServerConfigHandler
  {
  public:
    virtual
    ~MergeServerConfigHandler() throw ();

    virtual
    void*
    merge_server_config(apr_pool_t* p, void* server1, void* server2)
      throw () = 0;

    static
    void*
    merge_server_config_s(apr_pool_t* p, void* server1, void* server2)
      throw ();
  };

  /**
   * InputFilter
   */
  class InputFilter
  {
  public:
    virtual
    ~InputFilter() throw ();

    static
    ap_filter_t*
    insert_filter(InputFilter* f, ap_filter_type ftype, request_rec* r,
      conn_rec* c) throw (Exception);

    static
    void
    remove_filter(ap_filter_t* f) throw ();

    virtual
    apr_status_t
    filter(ap_filter_t* f, apr_bucket_brigade* b, ap_input_mode_t mode,
      apr_read_type_e block, apr_off_t readbytes) throw () = 0;

    static
    void
    register_filters_s() throw ();

  protected:
    static
    apr_status_t
    filter_func_s(ap_filter_t* f, apr_bucket_brigade* b,
      ap_input_mode_t mode, apr_read_type_e block,
      apr_off_t readbytes) throw ();
  };

  /**
   * RequestInputFilter should be used as per-request filter because
   * it destroys automatically.
   */
  class RequestInputFilter : public InputFilter
  {
  public:
    RequestInputFilter(ap_filter_type ftype, request_rec* r, conn_rec* c)
      throw (Exception);

    void
    remove() throw ();

    apr_status_t
    get_brigade(apr_bucket_brigade* bucket, ap_input_mode_t mode,
      apr_read_type_e block, apr_off_t readbytes) throw ();

  protected:
    static
    apr_status_t
    cleanup_s(void* data) throw ();

    static
    apr_status_t
    empty_s(void* data) throw ();

  protected:
    ap_filter_t* ap_filter_;
  };

  /**
   * OutputFilter
   */
  class OutputFilter
  {
  public:
    virtual
    ~OutputFilter() throw ();

    static
    ap_filter_t*
    insert_filter(OutputFilter* f, ap_filter_type ftype, request_rec* r,
      conn_rec* c) throw (Exception);

    static
    void
    remove_filter(ap_filter_t* f) throw ();

    virtual
    apr_status_t
    filter(ap_filter_t* f, apr_bucket_brigade* b) throw () = 0;

    static
    void
    register_filters_s() throw ();

  protected:
    static
    apr_status_t
    filter_func_s(ap_filter_t* f, apr_bucket_brigade* b) throw ();
  };

  /**
   * RequestOutputFilter should be used as per-request filter because
   * it destroys automatically.
   */
  class RequestOutputFilter : public OutputFilter
  {
  public:
    RequestOutputFilter(ap_filter_type ftype, request_rec* r,
      conn_rec* c) throw (Exception);

    void
    remove() throw ();

    apr_status_t
    pass_brigade(apr_bucket_brigade* bucket) throw ();

  protected:
    static
    apr_status_t
    cleanup_s(void* data) throw ();

    static
    apr_status_t
    empty_s(void* data) throw ();

  protected:
    ap_filter_t* ap_filter_;
  };

  /**
   *
   */
  template <typename M>
  class ModuleDef : public module
  {
  public:
    ModuleDef() throw ();
    ~ModuleDef() throw ();

    static
    void
    register_hooks_s(apr_pool_t*) throw ();

  private:
    // module initializers
    void
    initialize_config_parser(ConfigParser* parser) throw ();
    void
    initialize_config_parser(...) throw ();

    void
    initialize_create_dir_config_handler(
      CreateDirConfigHandler<M>* handler) throw ();
    void
    initialize_create_dir_config_handler(...) throw ();

    void
    initialize_merge_dir_config_handler(
      MergeDirConfigHandler<M>* handler) throw ();
    void
    initialize_merge_dir_config_handler(...) throw ();

    void
    initialize_create_server_config_handler(
      CreateServerConfigHandler<M>* handler) throw ();
    void
    initialize_create_server_config_handler(...) throw ();

    void
    initialize_merge_server_config_handler(
      MergeServerConfigHandler<M>* handler) throw ();
    void
    initialize_merge_server_config_handler(...) throw ();

    // hooks installers
    static
    void
    register_post_config_hook(PostConfigHook<M>* hook) throw ();
    static
    void
    register_post_config_hook(...) throw ();
    static
    void
    register_insert_filter_hook(InsertFilterHook<M>* hook) throw ();
    static
    void
    register_insert_filter_hook(...) throw ();
    static
    void
    register_quick_handler_hook(QuickHandlerHook<M>* hook) throw ();
    static
    void
    register_quick_handler_hook(...) throw ();
    static
    void
    register_handler_hook(HandlerHook<M>* hook) throw ();
    static
    void
    register_handler_hook(...) throw ();
    static
    void
    register_child_init_hook(ChildInitHook<M>* hook) throw ();
    static
    void
    register_child_init_hook(...) throw ();
  };
}

//////////////////////////////////////////////////////////////////////////
// Inlines
//////////////////////////////////////////////////////////////////////////

namespace Apache
{
  //
  // Hook class
  //

  inline
  Hook::Hook(int where) throw ()
    : where_(where)
  {
  }

  inline
  Hook::~Hook() throw ()
  {
  }

  inline
  int
  Hook::where() throw ()
  {
    return where_;
  }


  //
  // ModuleDef class
  //

  template <typename M>
  ModuleDef<M>::ModuleDef() throw ()
  {
    version = MODULE_MAGIC_NUMBER_MAJOR;
    minor_version = MODULE_MAGIC_NUMBER_MINOR;
    module_index = -1;
    name = __FILE__;
    dynamic_load_handle = 0;
    next = 0;
    magic = MODULE_MAGIC_COOKIE;
    rewrite_args = 0;
    create_dir_config = 0;
    merge_dir_config = 0;
    create_server_config = 0;
    merge_server_config = 0;
    cmds = 0;
    register_hooks = register_hooks_s;

    initialize_config_parser(&*M::instance);
    initialize_create_dir_config_handler(&*M::instance);
    initialize_merge_dir_config_handler(&*M::instance);
    initialize_create_server_config_handler(&*M::instance);
    initialize_merge_server_config_handler(&*M::instance);

    InputFilter::register_filters_s();
    OutputFilter::register_filters_s();
  }

  template <typename M>
  void
  ModuleDef<M>::initialize_config_parser(ConfigParser* parser) throw ()
  {
    cmds = parser->commands().release();
  }

  template <typename M>
  void
  ModuleDef<M>::initialize_config_parser(...) throw ()
  {
  }

  template <typename M>
  void
  ModuleDef<M>::initialize_create_dir_config_handler(
    CreateDirConfigHandler<M>*) throw ()
  {
    create_dir_config = CreateDirConfigHandler<M>::create_dir_config_s;
  }

  template <typename M>
  void
  ModuleDef<M>::initialize_create_dir_config_handler(...) throw ()
  {
  }

  template <typename M>
  void
  ModuleDef<M>::initialize_merge_dir_config_handler(
    MergeDirConfigHandler<M>*) throw ()
  {
    merge_dir_config = MergeDirConfigHandler<M>::merge_dir_config_s;
  }

  template <typename M>
  void
  ModuleDef<M>::initialize_merge_dir_config_handler(...) throw ()
  {
  }

  template <typename M>
  void
  ModuleDef<M>::initialize_create_server_config_handler(
    CreateServerConfigHandler<M>*) throw ()
  {
    create_server_config =
      CreateServerConfigHandler<M>::create_server_config_s;
  }

  template <typename M>
  void
  ModuleDef<M>::initialize_create_server_config_handler(...) throw ()
  {
  }

  template <typename M>
  void
  ModuleDef<M>::initialize_merge_server_config_handler(
    MergeServerConfigHandler<M>*) throw ()
  {
    merge_server_config =
      MergeServerConfigHandler<M>::merge_server_config_s;
  }

  template <typename M>
  void
  ModuleDef<M>::initialize_merge_server_config_handler(...) throw ()
  {
  }

  template <typename M>
  inline
  ModuleDef<M>::~ModuleDef() throw ()
  {
    if (cmds)
    {
      delete [] cmds;
    }
  }

  template <typename M>
  void
  ModuleDef<M>::register_hooks_s(apr_pool_t*) throw ()
  {
    register_post_config_hook(&*M::instance);
    register_insert_filter_hook(&*M::instance);
    register_quick_handler_hook(&*M::instance);
    register_handler_hook(&*M::instance);
    register_child_init_hook(&*M::instance);
  }

  template <typename M>
  void
  ModuleDef<M>::register_post_config_hook(
    PostConfigHook<M>* hook) throw ()
  {
    ap_hook_post_config(PostConfigHook<M>::post_config_s, 0, 0,
      hook->where());
  }

  template <typename M>
  void
  ModuleDef<M>::register_post_config_hook(...) throw ()
  {
  }

  template <typename M>
  void
  ModuleDef<M>::register_insert_filter_hook(
    InsertFilterHook<M>* hook) throw ()
  {
    ap_hook_insert_filter(InsertFilterHook<M>::insert_filter_s, 0, 0,
      hook->where());
  }

  template <typename M>
  void
  ModuleDef<M>::register_insert_filter_hook(...) throw ()
  {
  }

  template <typename M>
  void
  ModuleDef<M>::register_quick_handler_hook(
    QuickHandlerHook<M>* hook) throw ()
  {
    ap_hook_quick_handler(QuickHandlerHook<M>::quick_handler_s, 0, 0,
      hook->where());
  }

  template <typename M>
  void
  ModuleDef<M>::register_quick_handler_hook(...) throw ()
  {
  }

  template <typename M>
  void
  ModuleDef<M>::register_handler_hook(
    HandlerHook<M>* hook) throw ()
  {
    ap_hook_handler(HandlerHook<M>::handler_s, 0, 0, hook->where());
  }

  template <typename M>
  void
  ModuleDef<M>::register_handler_hook(...) throw ()
  {
  }

  template <typename M>
  void
  ModuleDef<M>::register_child_init_hook(
    ChildInitHook<M>* hook) throw ()
  {
    ap_hook_child_init(ChildInitHook<M>::child_init_s, 0, 0, hook->where());
  }

  template <typename M>
  void
  ModuleDef<M>::register_child_init_hook(...) throw ()
  {
  }


  //
  // Hooks
  //

  template <typename T>
  InsertFilterHook<T>::InsertFilterHook(int where) throw ()
    : Hook(where)
  {
  }

  template <typename T>
  void
  InsertFilterHook<T>::insert_filter_s(request_rec* r) throw ()
  {
    InsertFilterHook<T>& hook =
      static_cast<InsertFilterHook<T>&>(*T::instance);
    return hook.insert_filter(r);
  }

  template <typename T>
  QuickHandlerHook<T>::QuickHandlerHook(int where) throw ()
    : Hook(where)
  {
  }

  template <typename T>
  int
  QuickHandlerHook<T>::quick_handler_s(request_rec* r, int lookup_uri)
    throw ()
  {
    QuickHandlerHook<T>& hook =
      static_cast<QuickHandlerHook<T>&>(*T::instance);
    return hook.quick_handler(r, lookup_uri);
  }

  template <typename T>
  HandlerHook<T>::HandlerHook(int where) throw ()
    : Hook(where)
  {
  }

  template <typename T>
  int
  HandlerHook<T>::handler_s(request_rec* r) throw ()
  {
    HandlerHook<T>& hook =
      static_cast<HandlerHook<T>&>(*T::instance);
    return hook.handler(r);
  }

  template <typename T>
  ChildInitHook<T>::ChildInitHook(int where) throw ()
    : Hook(where)
  {
  }

  template <typename T>
  void
  ChildInitHook<T>::child_init_s(apr_pool_t* p, server_rec* s) throw ()
  {
    ChildInitHook<T>& hook =
      static_cast<ChildInitHook<T>&>(*T::instance);
    hook.child_init(p, s);
  }

  template <typename T>
  PostConfigHook<T>::PostConfigHook(int where) throw ()
    : Hook(where)
  {
  }

  template <typename T>
  int
  PostConfigHook<T>::post_config_s(apr_pool_t* pconf, apr_pool_t* plog,
    apr_pool_t* ptemp, server_rec* s) throw ()
  {
    PostConfigHook<T>& hook =
      static_cast<PostConfigHook<T>&>(*T::instance);
    return hook.post_config(pconf, plog, ptemp, s);
  }

  //
  // ConfigParser::ConfigArgs class
  //

  inline
  cmd_parms*
  ConfigParser::ConfigArgs::command() const throw ()
  {
    return command_;
  }

  inline
  void*
  ConfigParser::ConfigArgs::mconfig() const throw ()
  {
    return mconfig_;
  }

  inline
  const char*
  ConfigParser::ConfigArgs::name() const throw ()
  {
    return command_->cmd->name;
  }

  //
  // ConfigParser class
  //

  inline
  ConfigParser::~ConfigParser() throw ()
  {
  }

  //
  // Config Handlers
  //

  template <typename T>
  CreateDirConfigHandler<T>::~CreateDirConfigHandler() throw ()
  {
  }

  template <typename T>
  void*
  CreateDirConfigHandler<T>::create_dir_config_s(apr_pool_t* p,
    char* dirspec) throw ()
  {
    CreateDirConfigHandler<T>& handler =
      static_cast<CreateDirConfigHandler<T>&>(*T::instance);
    return handler.create_dir_config(p, dirspec);
  }


  template <typename T>
  MergeDirConfigHandler<T>::~MergeDirConfigHandler() throw ()
  {
  }

  template <typename T>
  void*
  MergeDirConfigHandler<T>::merge_dir_config_s(apr_pool_t* p,
    void* parent_conf, void* newloc_conf) throw ()
  {
    MergeDirConfigHandler<T>& handler =
      static_cast<MergeDirConfigHandler<T>&>(*T::instance);
    return handler.merge_dir_config(p, parent_conf, newloc_conf);
  }

  template <typename T>
  void*
  MergeServerConfigHandler<T>::merge_server_config_s(
    apr_pool_t* p, void* server1, void* server2) throw ()
  {
    MergeServerConfigHandler<T>& handler =
      static_cast<MergeServerConfigHandler<T>&>(*T::instance);
    return handler.merge_server_config(p, server1, server2);
  }


  template <typename T>
  CreateServerConfigHandler<T>::~CreateServerConfigHandler() throw ()
  {
  }

  template <typename T>
  void*
  CreateServerConfigHandler<T>::create_server_config_s(
    apr_pool_t* p, server_rec* s) throw ()
  {
    CreateServerConfigHandler<T>& handler =
      static_cast<CreateServerConfigHandler<T>&>(*T::instance);
    return handler.create_server_config(p, s);
  }



  template <typename T>
  MergeServerConfigHandler<T>::~MergeServerConfigHandler() throw ()
  {
  }


  //
  // class InputFilter
  //

  inline
  InputFilter::~InputFilter() throw ()
  {
  }

  inline
  void
  InputFilter::remove_filter(ap_filter_t* f) throw ()
  {
    ::ap_remove_input_filter(f);
  }

  //
  // class RequestInputFilter
  //

  inline
  void
  RequestInputFilter::remove() throw ()
  {
    remove_filter(ap_filter_);
  }

  inline
  apr_status_t
  RequestInputFilter::get_brigade(apr_bucket_brigade* bucket,
    ap_input_mode_t mode, apr_read_type_e block, apr_off_t readbytes)
    throw ()
  {
    return ::ap_get_brigade(ap_filter_->next, bucket, mode, block,
      readbytes);
  }

  //
  // class OutputFilter
  //

  inline
  OutputFilter::~OutputFilter() throw ()
  {
  }

  inline
  void
  OutputFilter::remove_filter(ap_filter_t* f) throw ()
  {
    ::ap_remove_output_filter(f);
  }

  //
  // class RequestOutputFilter
  //

  inline
  void
  RequestOutputFilter::remove() throw ()
  {
    remove_filter(ap_filter_);
  }

  inline
  apr_status_t
  RequestOutputFilter::pass_brigade(apr_bucket_brigade* bucket) throw ()
  {
    return ::ap_pass_brigade(ap_filter_->next, bucket);
  }
} // namespace Apache

#endif
