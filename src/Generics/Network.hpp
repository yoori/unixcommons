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



#ifndef GENERICS_NETWORK_HPP
#define GENERICS_NETWORK_HPP

#include <list>
#include <set>

#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>

#include <eh/Exception.hpp>

#include <Generics/Uncopyable.hpp>


namespace Generics
{
  namespace Network
  {
    /**
     * Class supplies static resolving functions and a set of exceptions
     */
    class Resolver
    {
    public:
      DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
      DECLARE_EXCEPTION(InvalidArgument, Exception);
      DECLARE_EXCEPTION(GetHostByNameFailed, Exception);
      DECLARE_EXCEPTION(UnresolvableAddress, Exception);

      /**
       * Translates a host name into set of addresses.
       * Throws an exception on error.
       * @param host_name name of the host to translate
       * @param addresses resulted addresses
       * @param buf buffer for host information storage
       * @param buf_size its size
       */
      static
      void
      get_host_by_name(const char* host_name, hostent& addresses,
        char* buf, size_t buf_size)
        throw (Exception);
    };

    /**
     * Provides list of IPs of local network interfaces
     */
    class LocalInterfaces : private Uncopyable
    {
    public:
      DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

      /**
       * Constructor
       */
      LocalInterfaces() throw (eh::Exception, Exception);

      /**
       * Destructor
       */
      ~LocalInterfaces() throw ();

      /**
       * Copies list of 'const sockaddr_in*' converted by functor into
       * the container
       * @param container container to fill with addresses
       * @param functor functor to convert address with for the container
       */
      template <typename Container, typename Functor>
      void
      list_all(Container& container, Functor functor)
        throw (eh::Exception, Exception);


    private:
      ifaddrs* addresses_;
    };

    /**
     * Test if a hostname resolves localhost
     */
    class IsLocalInterface : public Resolver
    {
    public:
      /**
       * Constructor
       */
      IsLocalInterface() throw (eh::Exception);

      /**
       * MT-safe method, that is intended for checking
       * if host name resolves local host.
       * If any of IPs got from the host_name is equal to any local
       * network interface IP match is recorded
       * @param host_name name of the host to resolve
       * @return match of host_name IPs and local IPs
       */
      bool
      check_host_name(const char* host_name) const
        throw (eh::Exception, InvalidArgument, GetHostByNameFailed,
          UnresolvableAddress);


    protected:
      static
      uint32_t
      ip_address(const sockaddr_in* address) throw ();

      typedef std::set<uint32_t> LocalAddresses;

      LocalAddresses local_addresses_;
    };
  }
}

//
// Implementation
//

namespace Generics
{
  namespace Network
  {
    template <typename Container, typename Functor>
    void
    LocalInterfaces::list_all(Container& container, Functor functor)
      throw (eh::Exception, Exception)
    {
      for (const ifaddrs* address = addresses_; address;
        address = address->ifa_next)
      {
        if (address->ifa_addr && address->ifa_addr->sa_family == AF_INET)
        {
          const sockaddr_in* addr =
            reinterpret_cast<const sockaddr_in*>(address->ifa_addr);
          if (addr->sin_addr.s_addr != INADDR_ANY)
          {
            container.insert(container.end(), functor(addr));
          }
        }
      }
    }
  }
}

#endif
