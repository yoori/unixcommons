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



#include "CommonClasses.hpp"
#include <iostream>

namespace
{
  const String::SubString CANNOT_ALLOCATE(
    "NotificationCallback::get_semaphore(). Can't allocate memory.");
}

//
// class NotificationCallback
//

NotificationCallback::NotificationCallback(HTTP::PoolPolicy_var policy,
  unsigned int notify_number) throw (eh::Exception)
  : SimpleCounterCallback(policy.in(), EventLog::ELS_LOG_EVERYTHING),
    sem_(),
    notify_number_(notify_number),
    waits_number_(0)
{
}

void
NotificationCallback::on_response(const HTTP::ResponseInformation& data) throw ()
{
  SimpleCounterCallback::on_response(data);
  check();
}

void
NotificationCallback::on_error(const String::SubString& descr,
  const HTTP::RequestInformation& data) throw ()
{
  SimpleCounterCallback::on_error(descr, data);
  check();
}

inline
void
NotificationCallback::check() throw()
{
  if (get_counter().succeeded() + get_counter().failed() >= notify_number_ 
      && waits_number_)
  {
    for (int i = 0; i < waits_number_; ++i)
    {
      sem_->release();
    }
  }
  //else
  //  std::cout << notify_number_ << " : " << get_counter().succeeded() + get_counter().failed() << std::endl;
}

Sync::Semaphore&
NotificationCallback::get_semaphore() throw()
{
  if (waits_number_++ == 0)
  {
    sem_.reset(new (std::nothrow) Sync::Semaphore(0));
    if (!sem_.get())
    {
      policy_->error(CANNOT_ALLOCATE);
      --waits_number_;
    }
  }
  return *sem_;
}

NotificationCallback::~NotificationCallback() throw ()
{
}

//
// class VSTestInterface
//

VSTestInterface::VSTestInterface(Sync::Semaphore& finish_sem)
    throw(eh::Exception):
  finish_sem_(finish_sem)
{
}

VSTestInterface::~VSTestInterface() throw ()
{
}

//
// struct InfoToCallback
//

InfoToCallback::InfoToCallback(HTTP::PoolPolicy_var new_policy,
  size_t new_type, const std::vector</*const */std::string>& new_requests,
  Sync::Semaphore& thrs_sem, Sync::Semaphore& sem)
    throw(eh::Exception):
  policy(new_policy),
  type(new_type),
  requests_by_type(new_requests),
  threads_sem(thrs_sem),
  main_sem(sem)
{
}
