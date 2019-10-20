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





#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>

#include <Apache/Module.hpp>


namespace Apache
{
  namespace FilterNames
  {
    const char RESOURCE_FILTER[] = "__APACHE_MODULE_RESOURCE_FILTER__";
    const char CONTENT_SET_FILTER[] = "__APACHE_MODULE_CONTENT_SET_FILTER__";
    const char PROTOCOL_FILTER[] = "__APACHE_MODULE_PROTOCOL_FILTER__";
    const char TRANSCODE_FILTER[] = "__APACHE_MODULE_TRANSCODE_FILTER__";
    const char CONNECTION_FILTER[] = "__APACHE_MODULE_CONNECTION_FILTER__";
    const char NETWORK_FILTER[] = "__APACHE_MODULE_NETWORK_FILTE__R";
  };

  //
  // ConfigParser::ConfigArgs
  //

  ConfigParser::ConfigArgs::ConfigArgs(cmd_parms* cmd, void* mconfig)
    throw ()
    : type_(AT_NONE), command_(cmd), mconfig_(mconfig),
      flag_(0), str1_(0), str2_(0), str3_(0)
  {
  }

  ConfigParser::ConfigArgs::ConfigArgs(cmd_parms* cmd, void* mconfig,
    int flag) throw ()
    : type_(AT_FLAG), command_(cmd), mconfig_(mconfig),
      flag_(flag), str1_(0), str2_(0), str3_(0)
  {
  }

  ConfigParser::ConfigArgs::ConfigArgs(cmd_parms* cmd, void* mconfig,
    const char* str1) throw ()
    : type_(AT_ONE_STR), command_(cmd), mconfig_(mconfig),
      flag_(0), str1_(str1), str2_(0), str3_(0)
  {
  }

  ConfigParser::ConfigArgs::ConfigArgs(cmd_parms* cmd, void* mconfig,
    const char* str1, const char* str2) throw ()
    : type_(AT_TWO_STRS), command_(cmd), mconfig_(mconfig),
      flag_(0), str1_(str1), str2_(str2), str3_(0)
  {
  }

  ConfigParser::ConfigArgs::ConfigArgs(cmd_parms* cmd, void* mconfig,
    const char* str1, const char* str2, const char* str3) throw ()
    : type_(AT_THREE_STRS), command_(cmd), mconfig_(mconfig),
      flag_(0), str1_(str1), str2_(str2), str3_(str3)
  {
  }

  int
  ConfigParser::ConfigArgs::flag() const throw (ArgNotExist)
  {
    if (type_ != AT_FLAG)
    {
      Stream::Error ostr;
      ostr << FNS << "argument does not exist";
      throw ArgNotExist(ostr);
    }
    return flag_;
  }

  const char*
  ConfigParser::ConfigArgs::str1() const throw (ArgNotExist)
  {
    switch (type_)
    {
    case AT_ONE_STR:
    case AT_TWO_STRS:
    case AT_THREE_STRS:
      return str1_;

    default:
      Stream::Error ostr;
      ostr << FNS << "argument does not exist";
      throw ArgNotExist(ostr);
    }
  }

  const char*
  ConfigParser::ConfigArgs::str2() const throw (ArgNotExist)
  {
    switch (type_)
    {
    case AT_TWO_STRS:
    case AT_THREE_STRS:
      return str2_;

    default:
      Stream::Error ostr;
      ostr << FNS << "argument does not exist";
      throw ArgNotExist(ostr);
    }
  }

  const char*
  ConfigParser::ConfigArgs::str3() const throw (ArgNotExist)
  {
    switch (type_)
    {
    case AT_THREE_STRS:
      return str3_;

    default:
      Stream::Error ostr;
      ostr << FNS << "argument does not exist";
      throw ArgNotExist(ostr);
    }
  }

  //
  // ConfigParser
  //

  ConfigParser::Commands
  ConfigParser::commands() throw (eh::Exception)
  {
    Generics::ArrayAutoPtr<command_rec> cmds(commands_.size() + 1);

    std::copy(commands_.begin(), commands_.end(), cmds.get());
    memset(&cmds[commands_.size()], 0, sizeof(command_rec));

    return cmds;
  }

  void
  ConfigParser::add_directive(const char* name, int req_override,
    cmd_how args_how, const char* errmsg) throw (eh::Exception)
  {
    command_rec cmd;
    cmd.name = name;
    cmd.cmd_data = this;
    cmd.req_override = req_override;
    cmd.args_how = args_how;
    cmd.errmsg = errmsg;

    switch (args_how)
    {
    case RAW_ARGS:
      cmd.AP_RAW_ARGS = ConfigParser::handle_TAKE1;
      break;

    case TAKE1:
    case ITERATE:
      cmd.AP_TAKE1 = ConfigParser::handle_TAKE1;
      break;

    case TAKE2:
    case ITERATE2:
    case TAKE12:
      cmd.AP_TAKE2 = ConfigParser::handle_TAKE2;
      break;

    case FLAG:
      cmd.AP_FLAG = ConfigParser::handle_FLAG;
      break;

    case NO_ARGS:
      cmd.AP_NO_ARGS = ConfigParser::handle_NO_ARGS;
      break;

    case TAKE3:
    case TAKE23:
    case TAKE123:
    case TAKE13:
      cmd.AP_TAKE3 = ConfigParser::handle_TAKE3;
      break;

    default:
      break;
    }

    commands_.push_back(std::move(cmd));
  }

  const char*
  ConfigParser::handle_NO_ARGS(cmd_parms* cmd, void* mconfig) throw ()
  {
    ConfigParser* obj = reinterpret_cast<ConfigParser*>(cmd->cmd->cmd_data);
    ConfigArgs args(cmd, mconfig);
    return obj->handle_command(args);
  }

  const char*
  ConfigParser::handle_FLAG(cmd_parms* cmd, void* mconfig, int flag)
    throw ()
  {
    ConfigParser* obj = reinterpret_cast<ConfigParser*>(cmd->cmd->cmd_data);
    ConfigArgs args(cmd, mconfig, flag);
    return obj->handle_command(args);
  }

  const char*
  ConfigParser::handle_TAKE1(cmd_parms* cmd, void* mconfig,
    const char* word1) throw ()
  {
    ConfigParser* obj = reinterpret_cast<ConfigParser*>(cmd->cmd->cmd_data);
    ConfigArgs args(cmd, mconfig, word1);
    return obj->handle_command(args);
  }

  const char*
  ConfigParser::handle_TAKE2(cmd_parms* cmd, void* mconfig,
    const char* word1, const char* word2) throw ()
  {
    ConfigParser* obj = reinterpret_cast<ConfigParser*>(cmd->cmd->cmd_data);
    ConfigArgs args(cmd, mconfig, word1, word2);
    return obj->handle_command(args);
  }

  const char*
  ConfigParser::handle_TAKE3(cmd_parms* cmd, void* mconfig,
    const char* word1, const char* word2, const char* word3) throw ()
  {
    ConfigParser* obj = reinterpret_cast<ConfigParser*>(cmd->cmd->cmd_data);
    ConfigArgs args(cmd, mconfig, word1, word2, word3);
    return obj->handle_command(args);
  }

  //
  // class InputFilter
  //

  ap_filter_t*
  InputFilter::insert_filter(InputFilter* f, ap_filter_type ftype,
    request_rec* r, conn_rec* c) throw (Exception)
  {
    switch (ftype)
    {
    case AP_FTYPE_RESOURCE:
      return ::ap_add_input_filter(FilterNames::RESOURCE_FILTER, f, r, c);
      break;

    case AP_FTYPE_CONTENT_SET:
      return ::ap_add_input_filter(FilterNames::CONTENT_SET_FILTER, f, r, c);
      break;

    case AP_FTYPE_PROTOCOL:
      return ::ap_add_input_filter(FilterNames::PROTOCOL_FILTER, f, r, c);
      break;

    case AP_FTYPE_TRANSCODE:
      return ::ap_add_input_filter(FilterNames::TRANSCODE_FILTER, f, r, c);
      break;

    case AP_FTYPE_CONNECTION:
      return ::ap_add_input_filter(FilterNames::CONNECTION_FILTER, f, r, c);
      break;

    case AP_FTYPE_NETWORK:
      return ::ap_add_input_filter(FilterNames::NETWORK_FILTER, f, r, c);
      break;

    default:
      Stream::Error ostr;
      ostr << FNS << "filter type not supported";
      throw Exception(ostr);
    }
  }

  void
  InputFilter::register_filters_s() throw ()
  {
    if (!::ap_get_input_filter_handle(FilterNames::RESOURCE_FILTER))
    {
      ::ap_register_input_filter(FilterNames::RESOURCE_FILTER,
        filter_func_s, 0, AP_FTYPE_RESOURCE);
    }

    if (!::ap_get_input_filter_handle(FilterNames::CONTENT_SET_FILTER))
    {
      ::ap_register_input_filter(FilterNames::CONTENT_SET_FILTER,
        filter_func_s, 0, AP_FTYPE_CONTENT_SET);
    }

    if (!::ap_get_input_filter_handle(FilterNames::PROTOCOL_FILTER))
    {
      ::ap_register_input_filter(FilterNames::PROTOCOL_FILTER,
        filter_func_s, 0, AP_FTYPE_PROTOCOL);
    }

    if (!::ap_get_input_filter_handle(FilterNames::TRANSCODE_FILTER))
    {
      ::ap_register_input_filter(FilterNames::TRANSCODE_FILTER,
        filter_func_s, 0, AP_FTYPE_TRANSCODE);
    }

    if (!::ap_get_input_filter_handle(FilterNames::CONNECTION_FILTER))
    {
      ::ap_register_input_filter(FilterNames::CONNECTION_FILTER,
        filter_func_s, 0, AP_FTYPE_CONNECTION);
    }

    if (!::ap_get_input_filter_handle(FilterNames::NETWORK_FILTER))
    {
      ::ap_register_input_filter(FilterNames::NETWORK_FILTER,
        filter_func_s, 0, AP_FTYPE_NETWORK);
    }
  }

  apr_status_t
  InputFilter::filter_func_s(ap_filter_t* f, apr_bucket_brigade* b,
    ap_input_mode_t mode, apr_read_type_e block, apr_off_t readbytes)
    throw ()
  {
    InputFilter* filt = static_cast<InputFilter*>(f->ctx);
    return filt->filter(f, b, mode, block, readbytes);
  }

  //
  // class RequestInputFilter
  //

  RequestInputFilter::RequestInputFilter(ap_filter_type ftype,
    request_rec* r, conn_rec* c) throw (Exception)
    : ap_filter_(0)
  {
    if (!c && !r)
    {
      Stream::Error ostr;
      ostr << FNS << "both c and r parameters are NULL";
      throw Exception(ostr);
    }

    ap_filter_ = insert_filter(this, ftype, r, c);

    ::apr_pool_cleanup_register(c ? c->pool : r->pool, this, cleanup_s,
      empty_s);
  }

  apr_status_t
  RequestInputFilter::cleanup_s(void* data) throw ()
  {
    if (data)
    {
      RequestInputFilter* f = static_cast<RequestInputFilter*>(data);
      delete f;
    }

    return APR_SUCCESS;
  }

  apr_status_t
  RequestInputFilter::empty_s(void* /*data*/) throw ()
  {
    return APR_SUCCESS;
  }

  //
  // class OutputFilter
  //

  ap_filter_t*
  OutputFilter::insert_filter(OutputFilter* f, ap_filter_type ftype,
    request_rec* r, conn_rec* c) throw (Exception)
  {
    switch (ftype)
    {
    case AP_FTYPE_RESOURCE:
      return ::ap_add_output_filter(FilterNames::RESOURCE_FILTER, f, r, c);

    case AP_FTYPE_CONTENT_SET:
      return ::ap_add_output_filter(FilterNames::CONTENT_SET_FILTER,
        f, r, c);

    case AP_FTYPE_PROTOCOL:
      return ::ap_add_output_filter(FilterNames::PROTOCOL_FILTER, f, r, c);

    case AP_FTYPE_TRANSCODE:
      return ::ap_add_output_filter(FilterNames::TRANSCODE_FILTER, f, r, c);

    case AP_FTYPE_CONNECTION:
      return ::ap_add_output_filter(FilterNames::CONNECTION_FILTER,
        f, r, c);

    case AP_FTYPE_NETWORK:
      return ::ap_add_output_filter(FilterNames::NETWORK_FILTER, f, r, c);

    default:
      Stream::Error ostr;
      ostr << FNS << "filter type not supported";
      throw Exception(ostr);
    }
  }

  void
  OutputFilter::register_filters_s() throw ()
  {
    if (!::ap_get_output_filter_handle(FilterNames::RESOURCE_FILTER))
    {
      ::ap_register_output_filter(FilterNames::RESOURCE_FILTER,
        filter_func_s, 0, AP_FTYPE_RESOURCE);
    }

    if (!::ap_get_output_filter_handle(FilterNames::CONTENT_SET_FILTER))
    {
      ::ap_register_output_filter(FilterNames::CONTENT_SET_FILTER,
        filter_func_s, 0, AP_FTYPE_CONTENT_SET);
    }

    if (!::ap_get_output_filter_handle(FilterNames::PROTOCOL_FILTER))
    {
      ::ap_register_output_filter(FilterNames::PROTOCOL_FILTER,
        filter_func_s, 0, AP_FTYPE_PROTOCOL);
    }

    if (!::ap_get_output_filter_handle(FilterNames::TRANSCODE_FILTER))
    {
      ::ap_register_output_filter(FilterNames::TRANSCODE_FILTER,
        filter_func_s, 0, AP_FTYPE_TRANSCODE);
    }

    if (!::ap_get_output_filter_handle(FilterNames::CONNECTION_FILTER))
    {
      ::ap_register_output_filter(FilterNames::CONNECTION_FILTER,
        filter_func_s, 0, AP_FTYPE_CONNECTION);
    }

    if (!::ap_get_output_filter_handle(FilterNames::NETWORK_FILTER))
    {
      ::ap_register_output_filter(FilterNames::NETWORK_FILTER,
        filter_func_s, 0, AP_FTYPE_NETWORK);
    }
  }

  apr_status_t
  OutputFilter::filter_func_s(ap_filter_t* f, apr_bucket_brigade* b)
    throw ()
  {
    OutputFilter* filt = static_cast<OutputFilter*>(f->ctx);
    return filt->filter(f, b);
  }

  //
  // class RequestOutputFilter
  //

  RequestOutputFilter::RequestOutputFilter(ap_filter_type ftype,
    request_rec* r, conn_rec* c) throw (Exception)
  {
    if (!c && !r)
    {
      Stream::Error ostr;
      ostr << FNS << "both c and r parameters are NULL";
      throw Exception(ostr);
    }

    ap_filter_ = insert_filter(this, ftype, r, c);

    ::apr_pool_cleanup_register(c ? c->pool : r->pool, this, cleanup_s,
      empty_s);
  }

  apr_status_t
  RequestOutputFilter::cleanup_s(void* data) throw ()
  {
    if (data)
    {
      RequestOutputFilter* f = static_cast<RequestOutputFilter*>(data);
      delete f;
    }

    return APR_SUCCESS;
  }

  apr_status_t
  RequestOutputFilter::empty_s(void* /*data*/) throw ()
  {
    return APR_SUCCESS;
  }
} // namespace Apache
