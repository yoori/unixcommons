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



#include <list>

#include <eh/Errno.hpp>

#include <XMLUtility/Utility.hpp>
#include <XMLUtility/DOMDocumentAdapter.hpp>
#include <XMLUtility/StringManip.hpp>

#include <CORBAConfigParser/CorbaConfigReader.hpp>


namespace XMLStrings
{
  const char CORBA_CONFIG[] = "CorbaConfig";
  const char THREADING_POOL_ATTR[] = "threading-pool";
  const char TIMEOUT_ATTR[] = "timeout";

  const char ENDPOINT_CONFIG[] = "Endpoint";
  const char HOST_ATTR[] = "host";
  const char IOR_NAMES_ATTR[] = "ior_names";
  const char PORT_ATTR[] = "port";

  const char SECURE_CONFIG[] = "Secure";
  const char KEY_ATTR[] = "key";
  const char CERTIFICATE_ATTR[] = "certificate";
  const char KEY_WORD_ATTR[] = "key-word";
  const char CERTIFICATE_AUTHORITY_ATTR[] = "certificate-authority";

  const char OBJECT_CONFIG[] = "Object";
  const char INTERNAL_NAME_ATTR[] = "servant";
  const char EXTERNAL_NAME_ATTR[] = "name";

  const char CORBA_OBJECT[] = "CorbaObject";
  const char NAME_ATTR[] = "name";
  const char REF_ATTR[] = "ref";
};

namespace
{
  typedef std::list<std::string> CertificateSeq;

  void
  parse_certificate_seq(const char* certificates,
    CertificateSeq& certificate_seq) throw (eh::Exception)
  {
    /* parse certificate sequence */
    const std::string CERTIFICATE_SEQ_S = certificates;
    std::string::size_type begin_word = 0;
    std::string::size_type end_word;

    while ((end_word = CERTIFICATE_SEQ_S.find(';', begin_word)) !=
      std::string::npos)
    {
      certificate_seq.emplace_back(
        CERTIFICATE_SEQ_S.begin() + begin_word,
          CERTIFICATE_SEQ_S.begin() + end_word);
        begin_word = end_word + 1;
    }

    certificate_seq.emplace_back(
      CERTIFICATE_SEQ_S.begin() + begin_word,
        CERTIFICATE_SEQ_S.end());
  }
}


namespace CORBAConfigParser
{
  void
  CorbaConfigReader::read_server_config(DOMNode* node,
    CORBACommons::CorbaConfig& corba_config, const char* xml_namespace)
    throw (eh::Exception, Exception)
  {
    DOMElement* corba_config_elem = dynamic_cast<DOMElement*>(node);
    if (!corba_config_elem)
    {
      Stream::Error ostr;
      ostr << FNS << "Node " << XMLStrings::CORBA_CONFIG <<
        " is not an element.";
      throw Exception(ostr);
    }

    unsigned int ival;

    if (XMLUtility::get_attribute(corba_config_elem,
      XMLStrings::THREADING_POOL_ATTR, ival))
    {
      corba_config.thread_pool = ival;
    }

    for (DOMNode* child = corba_config_elem->getFirstChild(); child;
      child = child->getNextSibling())
    {
      if (XMLUtility::has_name(child, XMLStrings::ENDPOINT_CONFIG,
        xml_namespace))
      {
        CORBACommons::EndpointConfig endpoint_config;
        read_endpoint(child, endpoint_config, xml_namespace);
        corba_config.endpoints.push_back(std::move(endpoint_config));
      }
    }
  }

  void
  CorbaConfigReader::read_client_config(DOMNode* node,
    CORBACommons::CorbaClientConfig& corba_config,
    const char* /*xml_namespace*/)
    throw (eh::Exception, Exception)
  {
    DOMElement* corba_config_elem = dynamic_cast<DOMElement*>(node);
    if (!corba_config_elem)
    {
      Stream::Error ostr;
      ostr << FNS << "Node " << XMLStrings::CORBA_CONFIG <<
        " is not an element.";
      throw Exception(ostr);
    }

    unsigned int ival;

    if (XMLUtility::get_attribute(corba_config_elem,
      XMLStrings::TIMEOUT_ATTR, ival))
    {
      corba_config.timeout = Generics::Time(ival);
    }
  }

  void
  CorbaConfigReader::read_endpoint(DOMNode* node,
    CORBACommons::EndpointConfig& endpoint_config, const char* xml_namespace)
    throw (eh::Exception, Exception)
  {
    {
      // read attributes
      DOMElement* endpoint_config_elem = dynamic_cast<DOMElement*>(node);
      if (!endpoint_config_elem)
      {
        Stream::Error ostr;
        ostr << FNS << "Node " << XMLStrings::ENDPOINT_CONFIG <<
          " is not an element.";
        throw Exception(ostr);
      }

      std::string val;
      unsigned long ival;

      if (XMLUtility::get_attribute(endpoint_config_elem,
        XMLStrings::HOST_ATTR, val))
      {
        endpoint_config.host = val;
      }
      else
      {
        // default: use canonical host name
        char canonical_host_name[MAXHOSTNAMELEN + 1];
        if (gethostname(canonical_host_name, MAXHOSTNAMELEN) < 0)
        {
          eh::throw_errno_exception<Exception>(FNE,
            "Failed to determine canonical host name");
        }

        endpoint_config.host = canonical_host_name;
      }

      if (XMLUtility::get_attribute(endpoint_config_elem,
        XMLStrings::IOR_NAMES_ATTR, val))
      {
        endpoint_config.ior_names = val;
      }
      else
      {
        endpoint_config.ior_names = endpoint_config.host;
      }

      if (XMLUtility::get_attribute(endpoint_config_elem,
        XMLStrings::PORT_ATTR, ival))
      {
        endpoint_config.port = ival;
      }
      else
      {
        Stream::Error ostr;
        ostr << FNS << "Not defined port attribute.";
        throw Exception(ostr);
      }
    }

    for (DOMNode* child = node->getFirstChild(); child;
      child = child->getNextSibling())
    {
      if (XMLUtility::has_name(child, XMLStrings::OBJECT_CONFIG,
        xml_namespace))
      {
        std::string name;
        std::string ex_name;

        DOMElement* object_config_elem = dynamic_cast<DOMElement*>(child);
        if (!object_config_elem)
        {
          Stream::Error ostr;
          ostr << FNS << "Node " << XMLStrings::OBJECT_CONFIG <<
            " is not an element.";
          throw Exception(ostr);
        }

        std::string val;

        if (XMLUtility::get_attribute(object_config_elem,
          XMLStrings::INTERNAL_NAME_ATTR, val))
        {
          name = val;
        }
        else
        {
          Stream::Error ostr;
          ostr << FNS << "In " << XMLStrings::OBJECT_CONFIG <<
            " not defined " << XMLStrings::INTERNAL_NAME_ATTR <<
            " attribute.";
          throw Exception(ostr);
        }

        if (XMLUtility::get_attribute(object_config_elem,
          XMLStrings::EXTERNAL_NAME_ATTR, val))
        {
          ex_name = val;
        }
        else
        {
          Stream::Error ostr;
          ostr << FNS << "In " << XMLStrings::OBJECT_CONFIG <<
            " not defined " << XMLStrings::EXTERNAL_NAME_ATTR <<
            " attribute.";
          throw Exception(ostr);
        }

        endpoint_config.objects[name].insert(ex_name);
      }
      else if (XMLUtility::has_name(child, XMLStrings::SECURE_CONFIG,
        xml_namespace))
      {
        read_secure_params(child, endpoint_config.secure_connection_config);
      }
    }
  }

  void
  CorbaConfigReader::read_secure_params(DOMNode* node,
    CORBACommons::SecureConnectionConfig& secure_connection_config)
    throw (eh::Exception, Exception)
  {
    // read attributes
    DOMElement* secure_connection_config_elem =
      dynamic_cast<DOMElement*>(node);

    std::string key;
    std::string certificate;
    std::string pass_word;
    std::string ca;

    if (!XMLUtility::get_attribute(secure_connection_config_elem,
      XMLStrings::KEY_ATTR, key))
    {
      Stream::Error ostr;
      ostr << FNS << "In " << XMLStrings::SECURE_CONFIG << " not defined " <<
        XMLStrings::KEY_ATTR << " attribute.";
      throw Exception(ostr);
    }

    if (!XMLUtility::get_attribute(secure_connection_config_elem,
      XMLStrings::KEY_WORD_ATTR, pass_word))
    {
      Stream::Error ostr;
      ostr << FNS << "In " << XMLStrings::SECURE_CONFIG << " not defined " <<
        XMLStrings::KEY_WORD_ATTR << " attribute.";
      throw Exception(ostr);
    }

    if (!XMLUtility::get_attribute(secure_connection_config_elem,
      XMLStrings::CERTIFICATE_ATTR, certificate))
    {
      Stream::Error ostr;
      ostr << FNS << "In " << XMLStrings::SECURE_CONFIG << " not defined " <<
        XMLStrings::CERTIFICATE_ATTR << " attribute.";
      throw Exception(ostr);
    }

    if (!XMLUtility::get_attribute(secure_connection_config_elem,
      XMLStrings::CERTIFICATE_AUTHORITY_ATTR, certificate))
    {
      Stream::Error ostr;
      ostr << FNS << "In " << XMLStrings::SECURE_CONFIG << " not defined " <<
        XMLStrings::CERTIFICATE_AUTHORITY_ATTR << " attribute.";
      throw Exception(ostr);
    }

    try
    {
      secure_connection_config.parse(key.c_str(), pass_word.c_str(),
        certificate.c_str(), ca.c_str());
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Failed to read security files: " << ex.what();
      throw Exception(ostr);
    }
  }

  void
  CorbaConfigReader::read_corba_connection(DOMNode* node,
    CORBACommons::CorbaObjectRef& corba_object_ref, std::string& /*name*/,
    const char* xml_namespace)
    throw (eh::Exception, Exception)
  {
    DOMElement* elem = dynamic_cast<DOMElement*>(node);
    if (!elem)
    {
      Stream::Error ostr;
      ostr << FNS << "Node " << XMLStrings::CORBA_OBJECT <<
        " is not an element.";
      throw Exception(ostr);
    }

    corba_object_ref.type = CORBACommons::CorbaObjectRef::CT_NON_SECURE;

    for (DOMNode* child = node->getFirstChild(); child;
      child = child->getNextSibling())
    {
      if (XMLUtility::has_name(child, XMLStrings::SECURE_CONFIG,
        xml_namespace))
      {
        corba_object_ref.type = CORBACommons::CorbaObjectRef::CT_SECURE;
        read_secure_params(child, corba_object_ref.secure_connection_config);
      }
    }
  }

  void
  CorbaConfigReader::read_corba_ref(DOMNode* node,
    CORBACommons::CorbaObjectRef& corba_object_ref, std::string& name,
    const char* xml_namespace)
    throw (eh::Exception, Exception)
  {
    DOMElement* elem = dynamic_cast<DOMElement*>(node);
    if (!elem)
    {
      Stream::Error ostr;
      ostr << FNS << "Node " << XMLStrings::CORBA_OBJECT <<
        " is not an element.";
      throw Exception(ostr);
    }

    if (XMLUtility::get_attribute(elem, XMLStrings::NAME_ATTR, name))
    {
      std::string ref;
      if (XMLUtility::get_attribute(elem, XMLStrings::REF_ATTR, ref))
      {
        corba_object_ref.object_ref = ref;
      }
      else
      {
        Stream::Error ostr;
        ostr << FNS << "In '" << XMLStrings::CORBA_OBJECT <<
          " not defined attribute '" << XMLStrings::REF_ATTR << "'.";
        throw Exception(ostr);
      }
    }
    else
    {
      Stream::Error ostr;
      ostr << FNS << "In '" << XMLStrings::CORBA_OBJECT <<
        " not defined attribute '" << XMLStrings::NAME_ATTR << "'.";
      throw Exception(ostr);
    }

    read_corba_connection(node, corba_object_ref, name, xml_namespace);
  }
}
