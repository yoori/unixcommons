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





#ifndef XML_UTILITY_UTILITY_HPP
#define XML_UTILITY_UTILITY_HPP

#include <string>
#include <sstream>
#include <memory>

#include <eh/Exception.hpp>

#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMLSSerializer.hpp>

#include <xercesc/framework/MemBufFormatTarget.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

XERCES_CPP_NAMESPACE_USE

#include <String/UTF8Case.hpp>
#include <Stream/MemoryStream.hpp>
#include <XMLUtility/StringManip.hpp>

namespace XMLUtility
{
  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
  DECLARE_EXCEPTION(InvalidFormat, Exception);

  /**
   * Performs per process Xerces initialization.
   * You must call <code>initialize</code> before parsing.
   */
  void
  initialize() throw (Exception, eh::Exception);

  /**
   * Performs per process Xerces termination.
   */
  void
  terminate() throw ();

  /**
   * Retrieves an attribute value for the specified XML node.
   *
   * @param node The node to retrieve the attribute value from.
   * @param attr Attribute name.
   * @param value Variable to store the attribute value in. The type
   *  of the variable should be the one readable from std::istream.
   *
   * @return <code>true</code> if the attribute value was retrieved;
   * <code>false</code> otherwise.
   */
  template <typename T>
  bool
  get_attribute(DOMElement* node, const char* attr, T& value)
    throw (InvalidFormat, Exception, eh::Exception);

  /**
   * Retrieves a string attribute value for the specified XML node.
   *
   * @param node The node to retrieve the attribute value from.
   * @param attr Attribute name.
   * @param value Variable to store the attribute value in.
   *
   * @return <code>true</code> if the attribute value was retrieved;
   * <code>false</code> otherwise.
   */
  bool
  get_attribute(DOMElement* node, const char* attr, std::string& value)
    throw (InvalidFormat, Exception, eh::Exception);

  /**
   * Retrieves a boolean attribute value for the specified XML node.
   *
   * @param node The node to retrieve the attribute value from.
   * @param attr Attribute name.
   * @param value Variable to store the attribute value in.
   *
   * @return <code>true</code> if the attribute value was retrieved;
   * <code>false</code> otherwise.
   */
  bool
  get_attribute(DOMElement* node, const char* attr, bool& value)
    throw (InvalidFormat, Exception, eh::Exception);

  /**
   * Retrieves an XML element data for the specified node.
   *
   * @param node The node of the XML element that serves as the starting
   * point for retrieval.
   * @param value A variable of type T. <code>get_element</code> converts
   * the retrieval results into type <code>T</code>.
   * @param content_only If <code>true</code>, retrieve only the element
   * contents (exclude the element's tag); otherwise, include the element's
   * tag also. Note that nested tags always appear in the result.
   *
   * @return <code>true</code> on success; <code>false</code> otherwise.
   */
  template <typename T>
  bool
  get_element(DOMNode* node, T& value, bool content_only = true)
    throw (InvalidFormat, Exception, eh::Exception);

  /**
   * Retrieves an XML element data for the specified node, as a string.
   *
   * @param node The node of the XML element that serves as the starting
   * point for retrieval.
   * @param value A string variable.
   * @param content_only If <code>true</code>, retrieve only the element
   * contents (exclude the element's tag); otherwise, include the element's
   * tag also. Note that nested tags always appear in the result.
   *
   * @return <code>true</code> on success; <code>false</code> otherwise.
   */
  bool
  get_element(DOMNode* node, std::string& value, bool content_only = true)
    throw (InvalidFormat, Exception, eh::Exception);

  /**
   * Determines whether the specified node belongs to the
   * given namespace and has the given name.
   *
   * @param node The node for assessment.
   * @param name The expected name for the element.
   * @param name_space The expected namespace for the element.
   *
   * @return <code>true</code>, if element's name and namespace match the
   * provided parameters; <code>false</code> otherwise.
   */
  bool has_name(const DOMNode* node, const char* name, const char* name_space)
    throw(eh::Exception);
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace XMLUtility
{
  inline
  bool
  get_attribute(DOMElement* node, const char* attr, std::string& value)
    throw (InvalidFormat, Exception, eh::Exception)
  {
    if (node == 0 || attr == 0)
    {
      return false;
    }

    StringManip::XMLChAdapter attr_name;
    StringManip::mbc_to_xmlch(attr, attr_name);

    DOMAttr* attr_node = node->getAttributeNode(attr_name);
    if (attr_node == 0)
    {
      return false;
    }

    StringManip::xmlch_to_mbc(attr_node->getNodeValue(), value);
    return true;
  }

  inline
  bool
  get_attribute(DOMElement* node, const char* attr, bool& value)
    throw (InvalidFormat, Exception, eh::Exception)
  {
    std::string str_val;

    if (!get_attribute(node, attr, str_val))
    {
      return false;
    }

    try
    {
      std::string val;
      String::case_change<String::Uniform>(str_val, val);
      val.swap(str_val);
    }
    catch (const String::StringManip::InvalidFormatException& e)
    {
      Stream::Error ostr;
      ostr << "XMLUtility::get_attribute(): "
        "while parsing value InvalidFormatException was thrown by "
        "String::StringManip::to_lower(): " << e.what();
      throw Exception(ostr);
    }

    if (str_val == "true" || str_val == "1")
    {
      value = true;
    }
    else if (str_val == "false" || str_val == "0")
    {
      value = false;
    }
    else
    {
      Stream::Error ostr;
      ostr << "XMLUtility::get_attribute(): "
        "failed to convert attribute '" << attr << "' value '" <<
        str_val << "' to bool type ('true','1','false','0')";
      throw InvalidFormat(ostr);
    }

    return true;
  }

  template <typename T>
  bool
  get_attribute(DOMElement* node, const char* attr, T& value)
    throw (InvalidFormat, Exception, eh::Exception)
  {
    std::string str_val;
    if (!get_attribute(node, attr, str_val))
    {
      return false;
    }

    Stream::Parser istr(str_val);
    istr >> value;

    if (istr.fail())
    {
      Stream::Error ostr;
      ostr << "XMLUtility::get_attribute(): "
        "failed to convert attribute '" << attr << "' value '" <<
        str_val << "' to target type";
      throw InvalidFormat(ostr);
    }

    return true;
  }

  inline
  bool
  get_element(DOMNode* node, std::string& value, bool content_only)
    throw (InvalidFormat, Exception, eh::Exception)
  {
    if (node == 0)
    {
      return false;
    }

    class GetStringFilter : public DOMLSSerializerFilter
    {
    public:
      GetStringFilter(DOMNode* node, bool content_only) throw ()
        : node_(node), content_only_(content_only),
          to_show_(DOMNodeFilter::SHOW_ALL)
      {
      }

      virtual
      FilterAction
      acceptNode (const DOMNode* node) const
      {
//        std::cerr << "acceptNode : 0x" << std::hex << node->getNodeType() <<
//          std::endl;
        return node == node_ && content_only_ ? FILTER_SKIP : FILTER_ACCEPT;
      }

      virtual
      unsigned long
      getWhatToShow() const
      {
        return to_show_;
      }

      virtual
      void
      setWhatToShow(unsigned long to_show)
      {
        to_show_ = to_show;
      }

    private:
      DOMNode* node_;
      bool content_only_;
      unsigned long to_show_;
    };

    try
    {
      GetStringFilter filter(node, content_only);

      DOMImplementation* impl =
        DOMImplementationRegistry::getDOMImplementation(
          StringManip::XMLChAdapter("LS"));

      std::unique_ptr<DOMLSSerializer> serializer(impl->createLSSerializer());

      serializer.get()->setFilter(&filter);

      value = reinterpret_cast<const char*>(
        serializer.get()->writeToString(node));
    }
    catch (const XMLException& e)
    {
      StringManip::XMLMbcAdapter msg(e.getMessage());

      Stream::Error ostr;
      ostr << "XMLUtility::get_element(): "
        "XMLException caught:" << msg;
      throw Exception(ostr);
    }

    return true;
  }

  template <typename T>
  bool
  get_element(DOMNode* node, T& value, bool content_only)
    throw (InvalidFormat, Exception, eh::Exception)
  {
    std::string str_val;
    if (!get_element(node, str_val, content_only))
    {
      return false;
    }

    Stream::Parser istr(str_val);
    istr >> value;

    if (istr.fail())
    {
      Stream::Error ostr;
      ostr << "XMLUtility::get_element(): "
        "failed to convert element value '" << str_val << "' to target type";
      throw InvalidFormat(ostr);
    }

    return true;
  }

  inline
  bool
  has_name(const DOMNode* node, const char* name, const char* name_space)
    throw (eh::Exception)
  {
    return node != 0 && name != 0 &&
      !strcasecmp(StringManip::XMLMbcAdapter(node->getLocalName()), name) &&
      (name_space == 0 ||
        !strcasecmp(StringManip::XMLMbcAdapter(node->getNamespaceURI()),
          name_space));
  }
}

#endif
