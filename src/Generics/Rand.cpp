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



// Generics/Rand.cpp
#include <Sync/PosixLock.hpp>

#include <Generics/ISAAC.hpp>
#include <Generics/MT19937.hpp>


namespace Generics
{
  namespace
  {
    Sync::PosixMutex mutex;
    ISAAC generator;
  }

  const size_t MT19937::STATE_SIZE;
  const uint32_t MT19937::RAND_MAXIMUM;

  const uint32_t ISAAC::RAND_MAXIMUM;
  const size_t ISAAC::SIZE;

  uint32_t
  safe_rand() throw ()
  {
    Sync::PosixGuard lock(mutex);
    return generator.rand() >> 1;
  }
}
