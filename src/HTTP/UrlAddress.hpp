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



// File   : UrlAddress.hpp
// Author : Karen Aroutiounov

#ifndef HTTP_URLADDRESS_HPP
#define HTTP_URLADDRESS_HPP

#include <String/AsciiStringManip.hpp>


namespace HTTP
{
  extern const String::AsciiStringManip::Caseless HTTP_SCHEME;
  extern const String::AsciiStringManip::Caseless HTTPS_SCHEME;

  extern const String::AsciiStringManip::Caseless HTTP_PREFIX;
  extern const String::AsciiStringManip::Caseless HTTPS_PREFIX;

  extern const String::AsciiStringManip::Caseless HTTP_BEGIN;
  extern const String::AsciiStringManip::Caseless HTTPS_BEGIN;

  /**
   * Context object structure
   * URL is [scheme:][//[userinfo@]host[:port]][path][?query][#fragment]
   */
  struct UrlParts
  {
    bool has_scheme;
    String::SubString scheme;
    bool has_userinfo;
    String::SubString userinfo;
    bool has_host;
    String::SubString host;
    bool has_port;
    String::SubString port;
    bool has_path;
    String::SubString path;
    bool has_query;
    String::SubString query;
    bool has_fragment;
    String::SubString fragment;

    /**
     * Constructor
     * Initializes data members with default values
     */
    UrlParts() throw ();

    /**
     * Constructor
     * Initializes data members with the given values
     * @param scheme initial value for scheme data member
     * @param userinfo initial value for userinfo data member
     * @param host initial value for host data member
     * @param port initial value for port data member
     * @param path initial value for path data member
     * @param query initial value for query data member
     * @param fragment initial value for fragment data member
     */
    UrlParts(const String::SubString& scheme,
      const String::SubString& userinfo, const String::SubString& host,
      const String::SubString& port, const String::SubString& path,
      const String::SubString& query, const String::SubString& fragment)
      throw ();
  };

  /**
   * Extended context object with additional authority field and
   * function members.
   */
  struct ExtendedUrlParts : public UrlParts
  {
    String::SubString authority;

    /**
     * Resets data members to have default values
     */
    void
    clear() throw ();

    /**
     * Splits URL into its parts, parts are stored in data members.
     * @param url URL to split
     */
    void
    split_url(const String::SubString& url)
      throw (eh::Exception);
  };

  /**
   * Represent splitted URL. Verifies it upon splitting.
   */
  class URLAddress
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
    DECLARE_EXCEPTION(InvalidURL, Exception);

    /**
     * Constructor
     */
    URLAddress() throw ();

    /**
     * Constructor
     * Splits the supplied URL and verifies it.
     */
    explicit
    URLAddress(const String::SubString& url)
      throw (eh::Exception, InvalidURL);

    /**
     * Constructor
     * Uses previously splitted URL for initialization. Combines parts
     * to get full URL.
     * @param scheme scheme part of URL
     * @param userinfo userinfo part of URL
     * @param host host part of URL
     * @param port port part of URL
     * @param path path part of URL
     * @param query query part of URL
     * @param fragment fragment part of URL
     */
    URLAddress(const String::SubString& scheme,
      const String::SubString& userinfo, const String::SubString& host,
      const String::SubString& port, const String::SubString& path,
      const String::SubString& query, const String::SubString& fragment)
      throw (eh::Exception, InvalidURL);

    /**
     * Copy constructor
     * @param another source URL object
     */
    URLAddress(const URLAddress& another) throw (eh::Exception);

    /**
     * Destructor
     */
    virtual
    ~URLAddress() throw ();

    /**
     * Assignment operator
     * @param another source URL object
     * @return reference to the object
     */
    URLAddress&
    operator =(const URLAddress& another) throw (eh::Exception);

    /**
     * Assigns new URL to object
     * @param value new URL to be assigned to object
     */
    void
    url(const String::SubString& value)
      throw (eh::Exception, Exception, InvalidURL);

    /**
     * Full URL stored
     * @return full URL
     */
    const std::string&
    url() const throw ();

    /**
     * Scheme part of the URL
     * @return scheme part
     */
    const String::SubString&
    scheme() const throw ();

    /**
     * Authority part of the URL
     * @return authority part
     */
    const String::SubString&
    authority() const throw ();

    /**
     * Path part of the URL
     * @return path part
     */
    const String::SubString&
    path() const throw ();

    /**
     * Query part of the URL
     * @return query part
     */
    const String::SubString&
    query() const throw ();

    /**
     * Fragment part of the URL
     * @return fragment part
     */
    const String::SubString&
    fragment() const throw ();

    /**
     * Userinfo part of the URL
     * @return userinfo part
     */
    const String::SubString&
    userinfo() const throw ();

    /**
     * Host part of the URL
     * @return host part
     */
    const String::SubString&
    host() const throw ();

    /**
     * Port part of the URL
     * @return port part
     */
    const String::SubString&
    port() const throw ();


    /**
     * Creates object of URLAddress class or its descendant depending on the
     * scheme of the URL.
     * @param url URL to use
     * @return an object of the corresponding class
     */
    static
    URLAddress*
    create_address(const String::SubString& url)
      throw (InvalidURL, Exception, eh::Exception);

  protected:
    virtual
    void
    assign_(const String::SubString& url)
      throw (eh::Exception, Exception, InvalidURL);

    /**
     * Assembly URL from url parts and save it to url_
     * parts will set according new url_ value.
     */
    void
    assign_url_parts_(const UrlParts& parts, bool check)
      throw (eh::Exception, InvalidURL);

    void
    url_without_check_(const String::SubString& url)
      throw (eh::Exception);

    virtual
    void
    specific_checks_() throw (InvalidURL, Exception, eh::Exception);

    std::string url_;
    ExtendedUrlParts parts_;
  };

  /**
   * Expansion of URLAddress class to support HTTP URL details.
   * Uses strict checking.
   */
  class HTTPAddress : public URLAddress
  {
  public:
    /**
     * Constructor
     * Splits HTTP URL supplied and verifies it.
     * @param url HTTP URL
     */
    explicit
    HTTPAddress(const String::SubString& url = String::SubString())
      throw (InvalidURL, eh::Exception);

    /**
     * Constructor
     * Uses splitted HTTP URL for initialization. Combines parts
     * to get full URL.
     * @param host host part of HTTP URL
     * @param path path part of HTTP URL
     * @param query query part of HTTP URL
     * @param fragment fragment part of HTTP URL
     * @param port port number of HTTP URL (0 - use default port)
     * @param secure whether scheme https and port 443 or http and 80 to use
     * by default
     * @param userinfo userinfo part of URL
     */
    HTTPAddress(const String::SubString& host,
      const String::SubString& path,
      const String::SubString& query = String::SubString(),
      const String::SubString& fragment = String::SubString(),
      unsigned short port = 0,
      bool secure = false,
      const String::SubString& userinfo = String::SubString())
      throw (InvalidURL, eh::Exception);

    /**
     * Destructor
     */
    virtual
    ~HTTPAddress() throw () = default;

    /**
     * Port number of the HTTP URL
     * @return port number
     */
    unsigned short
    port_number() const throw ();

    /**
     * If HTTP URL uses secure scheme
     * @return whether secure or not
     */
    bool
    secure() const throw ();

    /**
     * If the port is default for the scheme
     * @return whether default port or not
     */
    bool
    is_default_port() const throw ();

    /**
     * Returns URL combined of the selected parts of the original URL
     * @param flags parts to combine
     * @param str string to place result to
     * @return reference to str
     */
    const std::string&
    get_view(unsigned long flags, std::string& str) const
      throw (eh::Exception);

    enum VW_FLAGS
    {
      VW_PROTOCOL = 0x01,
      VW_HOSTNAME = 0x02,
      VW_HOSTNAME_WWW = (VW_HOSTNAME + 0x100),
      VW_PORT = 0x04,
      VW_NDEF_PORT = 0x08,
      VW_PATH = 0x10,
      VW_QUERY = 0x20,
      VW_STRIP_PATH = (VW_PATH + 0x40),
      VW_FRAGMENT = 0x80,

      VW_FULL = VW_PROTOCOL | VW_HOSTNAME | VW_NDEF_PORT |
        VW_PATH | VW_QUERY | VW_FRAGMENT
    };

  protected:
    HTTPAddress(const String::SubString& url,
      bool strict_url)
      throw (InvalidURL, eh::Exception);

    static
    int
    get_default_port_(bool secure) throw ();

    virtual
    void
    assign_(const String::SubString& url)
      throw (eh::Exception, Exception, InvalidURL);

    void
    set_(bool secure, const String::SubString& userinfo,
      const String::SubString& host, unsigned short port,
      const String::SubString& path, const String::SubString& query,
      const String::SubString& fragment) throw (eh::Exception);

    virtual
    void
    specific_checks_() throw (InvalidURL, Exception, eh::Exception);

    virtual
    bool
    additional_checks_() throw (InvalidURL, Exception, eh::Exception);

    bool strict_;
    unsigned short port_number_;
    bool secure_;
    bool default_port_;
  };

  /**
   * Expansion of HTTPAddress class to act as url in web browser.
   * Supports IDN and always non-strict
   */
  class BrowserAddress : public HTTPAddress
  {
  public:
    DECLARE_EXCEPTION(IDNAError, InvalidURL);

    /**
     * Constructor
     * Splits HTTP URL supplied and verifies it.
     * Converts host name with punicode if required.
     * @param url HTTP URL
     */
    explicit
    BrowserAddress(const String::SubString& url = String::SubString())
      throw (InvalidURL, eh::Exception);

    /**
     * Constructor
     * Uses splitted HTTP URL for initialization. Combines parts
     * to get full URL.
     * @param host host part of HTTP URL. Encoded with punycode if required
     * @param path path part of HTTP URL
     * @param query query part of HTTP URL
     * @param fragment fragment part of HTTP URL
     * @param port port number of HTTP URL (0 - use default port)
     * @param secure whether scheme https and port 443 or http and 80 to use
     * by default
     * @param userinfo userinfo part of URL
     */
    BrowserAddress(const String::SubString& host,
      const String::SubString& path,
      const String::SubString& query = String::SubString(),
      const String::SubString& fragment = String::SubString(),
      unsigned short port = 0,
      bool secure = false,
      const String::SubString& userinfo = String::SubString())
      throw (InvalidURL, eh::Exception);

    /**
     * Destructor
     */
    virtual
    ~BrowserAddress() throw () = default;

    /**
     * Unicode host part of the URL
     * @return unicode host part
     */
    String::SubString
    unicode_host() const throw ();

  protected:
    virtual
    bool
    additional_checks_() throw (InvalidURL, Exception, eh::Exception);

    void
    process_host_(const String::SubString& host)
      throw (InvalidURL, eh::Exception);

    std::string decoded_host_;
    std::string encoded_host_;
  };


  /**
   * Checker of URL parts for validness
   */
  class URLPartsChecker
  {
  public:
    /**
     * Checks URL parts for validness
     * @param url URL to use in error messages
     * @param parts URL parts to check for validness
     * @param error error message if URL is not valid
     * @return if supplied URL parts are valid
     */
    bool
    operator ()(const String::SubString& url, const UrlParts& parts,
      std::string& error) throw (eh::Exception);
  };

  /**
   * Checker of URL for validness
   */
  class URLChecker : protected URLPartsChecker
  {
  public:
    /**
     * Checks URL for validness
     * @param url URL to check for validness
     * @return if supplied URL is valid
     */
    bool
    operator ()(const String::SubString& url) throw (eh::Exception);
  };

  /**
   * Checker of HTTP URL for validness
   */
  class HTTPChecker : protected URLPartsChecker
  {
  public:
    /**
     * Destructor
     */
    virtual
    ~HTTPChecker() throw ();

    /**
     * Checks HTTP URL for validness
     * @param url HTTP URL to check for validness
     * @param error error message if URL is not valid
     * @param strict if prefix http:// or https:// is required
     * @return if supplied HTTP URL is valid
     */
    bool
    operator ()(const String::SubString& url, std::string* error = 0,
      bool strict = true) throw (eh::Exception);

  protected:
    virtual
    bool
    process_parts_(const String::SubString& url, ExtendedUrlParts& parts,
      std::string& error, bool strict) throw (eh::Exception);
  };

  /**
   * Checker of HTTP URL with IDN for validness
   */
  class BrowserChecker : protected HTTPChecker
  {
  public:
    /**
     * Destructor
     */
    virtual
    ~BrowserChecker() throw () = default;

    /**
     * Checks HTTP URL for validness
     * @param url HTTP URL to check for validness
     * @param error error message if URL is not valid
     * @return if supplied HTTP URL is valid
     */
    bool
    operator ()(const String::SubString& url, std::string* error = 0)
      throw (eh::Exception);

  protected:
    virtual
    bool
    process_parts_(const String::SubString& url, ExtendedUrlParts& parts,
      std::string& error, bool strict) throw (eh::Exception);

    std::string encoded_host_;
  };


  /**
   * Converts url into pseudo-normal form.
   * @param url url to convert
   * @return normalized url
   */
  std::string
  normalize_http_address(const String::SubString& url)
    throw (eh::Exception);

  /**
   * Fetch keywords from url
   * @param url url to convert
   * @return keywords from url
   */
  std::string
  keywords_from_http_address(const String::SubString& url)
    throw (eh::Exception);
}

//
// INLINES
//

namespace HTTP
{
  //
  // UrlParts class
  //

  inline
  UrlParts::UrlParts() throw ()
    : has_scheme(false), has_userinfo(false), has_host(false),
      has_port(false), has_path(false), has_query(false),
      has_fragment(false)
  {
  }


  //
  // URLAddress class
  //

  inline
  URLAddress::~URLAddress() throw ()
  {
  }

  inline
  const std::string&
  URLAddress::url() const throw ()
  {
    return url_;
  }

  inline
  const String::SubString&
  URLAddress::scheme() const throw ()
  {
    return parts_.scheme;
  }

  inline
  const String::SubString&
  URLAddress::authority() const throw ()
  {
    return parts_.authority;
  }

  inline
  const String::SubString&
  URLAddress::path() const throw ()
  {
    return parts_.path;
  }

  inline
  const String::SubString&
  URLAddress::query() const throw ()
  {
    return parts_.query;
  }

  inline
  const String::SubString&
  URLAddress::fragment() const throw ()
  {
    return parts_.fragment;
  }

  inline
  const String::SubString&
  URLAddress::userinfo() const throw ()
  {
    return parts_.userinfo;
  }

  inline
  const String::SubString&
  URLAddress::host() const throw ()
  {
    return parts_.host;
  }

  inline
  const String::SubString&
  URLAddress::port() const throw ()
  {
    return parts_.port;
  }


  //
  // HTTPAddress class
  //

  inline
  unsigned short
  HTTPAddress::port_number() const throw ()
  {
    return port_number_;
  }

  inline
  bool
  HTTPAddress::secure() const throw ()
  {
    return secure_;
  }

  inline
  bool
  HTTPAddress::is_default_port() const throw ()
  {
    return default_port_;
  }


  //
  // BrowserAddress
  //

  inline
  String::SubString
  BrowserAddress::unicode_host() const throw ()
  {
    return decoded_host_;
  }


  //
  // HTTPChecker class
  //

  inline
  HTTPChecker::~HTTPChecker() throw ()
  {
  }
}

#endif
