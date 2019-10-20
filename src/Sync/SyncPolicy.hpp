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





#ifndef SYNC_SYNC_POLICY_HPP
#define SYNC_SYNC_POLICY_HPP

#include <Sync/PosixLock.hpp>


namespace Sync
{
  namespace Policy
  {
    template <typename AdoptedMutex, typename AdoptedReadGuard,
      typename AdoptedWriteGuard>
    class PolicyAdapter
    {
    public:
      typedef AdoptedMutex Mutex;
      typedef AdoptedReadGuard ReadGuard;
      typedef AdoptedWriteGuard WriteGuard;
    };

    typedef PolicyAdapter<PosixMutex, PosixGuard, PosixGuard>
      PosixThread;
    typedef PolicyAdapter<PosixSpinLock, PosixSpinGuard, PosixSpinGuard>
      PosixSpinThread;
    typedef PolicyAdapter<PosixRWLock, PosixRGuard, PosixWGuard>
      PosixThreadRW;
  }
}

#endif
