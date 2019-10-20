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





#ifndef XML_UTILITY_DOM_DOCUMENT_ADAPTER_HPP
#define XML_UTILITY_DOM_DOCUMENT_ADAPTER_HPP

#include <memory>
#include <string>

#include <eh/Exception.hpp>

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

XERCES_CPP_NAMESPACE_USE

namespace XMLUtility
{
  /**
   * This Xerces parser wrapper does a lot of dirty work and
   * also converts Xerces errors to exceptions.
   */
  class DOMDocumentAdapter
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(InvalidArgument, Exception);
    DECLARE_EXCEPTION(NotParsed, Exception);

  public:
    /** Default constructor. */
    DOMDocumentAdapter() throw (Exception, eh::Exception);

    /**X Parametric constructor. Parses the provided text into a DOM tree.
     *
     * @param text The source XML text.
     * @param validate Can take one of 3 values:
     *        AbstractDOMParser::Val_Always - report validation errors,
     *        AbstractDOMParser::Val_Never - do not report validation errors,
     *        AbstractDOMParser::Val_Auto - report validation errors
     *        if grammar specified.
     */
    explicit
    DOMDocumentAdapter(const char* text,
      AbstractDOMParser::ValSchemes validate = AbstractDOMParser::Val_Auto)
      throw (InvalidArgument, Exception, eh::Exception);

    /** Destructor. */
    ~DOMDocumentAdapter() throw();

    /** Parses the provided text into a DOM tree.
     *
     * @param text The source XML text.
     * @param validate validation error reporting
     */
    void
    parse(const char* text,
      AbstractDOMParser::ValSchemes validate = AbstractDOMParser::Val_Auto)
      throw (InvalidArgument, Exception, eh::Exception);

    /** Parses an XML file into a DOM tree.
     *
     * @param file The source file name.
     * @param validate validation error reporting
     */
    void
    parse_file(const char* file,
      AbstractDOMParser::ValSchemes validate = AbstractDOMParser::Val_Auto)
      throw (InvalidArgument, Exception, eh::Exception);

    /**
     * Returns a pointer to the DOM tree of the parsed text or file,
     * as DOMDocument.
     *
     * @return Pointer to the DOMDocument of the parsed text or file;
     * NULL if the document is empty or the document has not been parsed.
     */
    DOMDocument*
    root() throw (NotParsed, Exception, eh::Exception);

    /** Determines whether a document has been parsed into a DOM tree.
     *
     * @return <code>true</code> if the document has been parsed;
     * <code>false</code> otherwise.
     */
    bool
    parsed() throw ();

    /** Destroys the parser and clears the DOM tree.
     *
     */
    void
    clear() throw (Exception, eh::Exception);

    /** Sets the location of XML Schema that must be used for validating the
     * parsed XML texts/files.
     *
     * @param value The XML Schema location path.
     */
    void
    schema_location(const char* value) throw (eh::Exception);

    /** Retrieves the location of XML Schema.
     *
     * @return XML Schema location path as as null-terminated string.
     */
    const char*
    schema_location() const throw (eh::Exception);

  protected:
    typedef std::unique_ptr<XercesDOMParser> XercesDOMParser_ptr;

    XercesDOMParser_ptr parser_;
    std::string schema_location_;
  };
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace XMLUtility
{
  //
  // DOMDocumentAdapter class
  //

  inline
  DOMDocumentAdapter::DOMDocumentAdapter()
    throw (DOMDocumentAdapter::Exception, eh::Exception)
  {
  }

  inline
  DOMDocumentAdapter::DOMDocumentAdapter(const char* text,
    AbstractDOMParser::ValSchemes validate)
    throw (InvalidArgument, Exception, eh::Exception)
  {
    parse(text, validate);
  }

  inline
  DOMDocumentAdapter::~DOMDocumentAdapter() throw ()
  {
  }

  inline
  bool
  DOMDocumentAdapter::parsed() throw ()
  {
    return parser_.get() != 0;
  }

  inline
  void
  DOMDocumentAdapter::clear() throw (Exception, eh::Exception)
  {
    parser_ = XercesDOMParser_ptr();
  }

  inline
  void
  DOMDocumentAdapter::schema_location(const char* value) throw (eh::Exception)
  {
    schema_location_ = value;
  }

  inline
  const char*
  DOMDocumentAdapter::schema_location() const throw (eh::Exception)
  {
    return schema_location_.c_str();
  }
}

#endif
