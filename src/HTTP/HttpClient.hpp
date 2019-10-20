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



#ifndef HTTP_HTTPCLIENT_HPP
#define HTTP_HTTPCLIENT_HPP

#include <Sync/MutexPtr.hpp>

#include <HTTP/HTTPCookie.hpp>
#include <HTTP/HttpAsync.hpp>


namespace HTTP
{
  typedef Sync::MutexRefPtr<HTTP::ClientCookieFacility>
    CookiePoolPtr;
  typedef ReferenceCounting::QualPtr<CookiePoolPtr> CookiePool_var;

  HttpInterface*
  CreateCookieClient(HttpInterface* pool, CookiePoolPtr* cookie)
    throw (eh::Exception);
}

#endif
