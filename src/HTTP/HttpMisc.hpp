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





#ifndef HTTP_HTTPMISC_HPP
#define HTTP_HTTPMISC_HPP

#include <list>

#include <String/SubString.hpp>


namespace HTTP
{
  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

  /**
   * HTTP Header
   */
  struct Header
  {
    Header() throw (eh::Exception);
    Header(const char* nm, const char* vl) throw (eh::Exception);
    Header(const std::string& nm, const std::string& vl)
      throw (eh::Exception);

    std::string name;
    std::string value;
  };

  typedef std::list<Header> HeaderList;

  /**
   * HTTP Header based on SubString
   */
  struct SubHeader
  {
    SubHeader() throw ();
    SubHeader(const char* nm, const char* vl) throw ();
    SubHeader(const String::SubString& nm, const String::SubString& vl)
      throw ();
    SubHeader(const Header& header) throw (); // implicit

    String::SubString name;
    String::SubString value;
  };

  typedef std::list<SubHeader> SubHeaderList;

  /**
   * HTTP Parameter
   */
  struct Param
  {
    Param() throw (eh::Exception);
    Param(const char* nm, const char* vl) throw (eh::Exception);
    Param(const std::string& nm, const std::string& vl)
      throw (eh::Exception);

    std::string name;
    std::string value;
  };

  typedef std::list<Param> ParamList;

  /**
   * HTTP Parameter based on SubString
   */
  struct SubParam
  {
    SubParam() throw ();
    SubParam(const char* nm, const char* vl) throw ();
    SubParam(const String::SubString& nm, const String::SubString& vl)
      throw ();
    SubParam(const Param& param) throw (); // implicit

    String::SubString name;
    String::SubString value;
  };

  typedef std::list<SubParam> SubParamList;
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace HTTP
{
  //
  // Header class
  //

  inline
  Header::Header() throw (eh::Exception)
  {
  }

  inline
  Header::Header(const char* nm, const char* vl) throw (eh::Exception)
    : name(nm), value(vl)
  {
  }

  inline
  Header::Header(const std::string& nm, const std::string& vl)
    throw (eh::Exception)
    : name(nm), value(vl)
  {
  }


  //
  // SubHeader class
  //

  inline
  SubHeader::SubHeader() throw ()
  {
  }

  inline
  SubHeader::SubHeader(const char* nm, const char* vl) throw ()
    : name(nm), value(vl)
  {
  }

  inline
  SubHeader::SubHeader(const String::SubString& nm,
    const String::SubString& vl) throw ()
    : name(nm), value(vl)
  {
  }

  inline
  SubHeader::SubHeader(const Header& header) throw ()
    : name(header.name), value(header.value)
  {
  }


  //
  // Param class
  //

  inline
  Param::Param() throw (eh::Exception)
  {
  }

  inline
  Param::Param(const char* nm, const char* vl) throw (eh::Exception)
    : name(nm), value(vl)
  {
  }

  inline
  Param::Param(const std::string& nm, const std::string& vl)
    throw (eh::Exception)
    : name(nm), value(vl)
  {
  }


  //
  // SubParam class
  //

  inline
  SubParam::SubParam() throw ()
  {
  }

  inline
  SubParam::SubParam(const char* nm, const char* vl) throw ()
    : name(nm), value(vl)
  {
  }

  inline
  SubParam::SubParam(const String::SubString& nm,
    const String::SubString& vl) throw ()
    : name(nm), value(vl)
  {
  }

  inline
  SubParam::SubParam(const Param& param) throw ()
    : name(param.name), value(param.value)
  {
  }
}

#endif
