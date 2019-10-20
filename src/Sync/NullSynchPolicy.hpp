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



// file      : Utility/Synch/Policy/Null.hpp




#ifndef SYNC_NULL_SYNCH_POLICY_HPP
#define SYNC_NULL_SYNCH_POLICY_HPP

#include <Generics/Uncopyable.hpp>


namespace Sync
{
  namespace Policy
  {
    class NullMutex : private Generics::Uncopyable
    {
    };

    class NullGuard : private Generics::Uncopyable
    {
    public:
      explicit
      NullGuard(NullMutex&) throw ();
    };

    struct Null
    {
      typedef NullMutex Mutex;
      typedef NullGuard ReadGuard;
      typedef NullGuard WriteGuard;
    };
  }
}


namespace Sync
{
  namespace Policy
  {
    inline
    NullGuard::NullGuard (NullMutex&) throw ()
    {
    }
  }
}

#endif
