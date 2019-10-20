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





#include <string>

#include <xercesc/util/XMLString.hpp>

#include <eh/Exception.hpp>

#include <XMLUtility/StringManip.hpp>


namespace XMLUtility
{
  namespace StringManip
  {
    void
    xmlch_to_mbc(const XMLCh* src, std::string& dest)
      throw (eh::Exception, InvalidFormatException)
    {
      char* str = 0;

      try
      {
        if (src)
        {
          str = XMLString::transcode(src);
        }
        else
        {
          XMLCh tmp[2];
          XMLString::transcode("", tmp, 1);

          str = XMLString::transcode(tmp);
        }

        if (str)
        {
          try
          {
            dest = str;
          }
          catch (...)
          {
            XMLString::release(&str);
            throw;
          }

          XMLString::release(&str);
        }
        else
        {
          throw InvalidFormatException(
            "XMLUtility::StringManip::xmlch_to_mbc(): "
            "XMLString::transcode failed");
        }
      }
      catch (const XMLException& ex)
      {
        throw InvalidFormatException(
          "XMLUtility::StringManip::xmlch_to_mbc(): "
          "Can't transcode text.");
      }
    }

    void
    mbc_to_xmlch(const char* src, XMLChAdapter& dest)
      throw (eh::Exception, InvalidFormatException)
    {
      dest = src;
    }

    //
    // XMLChAdapter class
    //

    XMLChAdapter::XMLChAdapter(const char* text)
      throw (eh::Exception, InvalidFormatException)
    {
      try
      {
        string_ = XMLString::transcode(text ? text : "");
      }
      catch (const XMLException& ex)
      {
        throw InvalidFormatException(
          "XMLUtility::StringManip::XMLChAdapter::XMLChAdapter(): "
          "Can't transcode text.");
      }
    }

    XMLChAdapter&
    XMLChAdapter::operator =(const char* text)
      throw (eh::Exception, InvalidFormatException)
    {
      if (string_)
      {
        try
        {
          XMLString::release(&string_);
        }
        catch(...)
        {
        }

        string_ = 0;
      }

      try
      {
        string_ = XMLString::transcode(text ? text : "");
      }
      catch (const XMLException& ex)
      {
        throw InvalidFormatException(
          "XMLUtility::StringManip::XMLChAdapter::operator =(): "
          "Can't transcode text.");
      }

      return *this;
    }
  }
}
