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



#ifndef GENERICS_MT19937_HPP
#define GENERICS_MT19937_HPP

#include <cstdint>

#include <unistd.h>
#include <fcntl.h>

#include <Generics/Time.hpp>
#include <Generics/Uncopyable.hpp>


namespace Generics
{
  /**
   * Mersenne-Twister random number generation with period of 2^19937-1
   * Not thread safe
   */
  class MT19937 : private Uncopyable
  {
  public:
    static const size_t STATE_SIZE = 624;

    /**
     * Up limit for random numbers range.
     * This generator range is [0, RAND_MAXIMUM]
     */
    static const uint32_t RAND_MAXIMUM = ~static_cast<uint32_t>(0);

    /**
     * Constructor
     * Uses /dev/urandom for initialization
     */
    MT19937() throw ();
    /**
     * Constructor
     * @param value initial seed number
     */
    explicit
    MT19937(const uint32_t value) throw ();
    /**
     * Constructor
     * @param value pointer to data for initial seed
     * @param size data size
     */
    explicit
    MT19937(const uint32_t* value, size_t size = STATE_SIZE) throw ();

    /**
     * Initializes object
     * Uses /dev/urandom for initialization
     */
    void
    seed() throw ();
    /**
     * Initializes object
     * @param value initial seed number
     */
    void
    seed(uint32_t value) throw ();
    /**
     * Initializes object
     * @param value pointer to data for initial seed
     * @param size data size
     */
    void
    seed(const uint32_t* value, size_t size = STATE_SIZE) throw ();

    /**
     * Creates next random number in the sequence
     * @return random number in [0..2^32-1] range
     */
    uint32_t
    rand() throw ();

  protected:
    /**
     * Initializes state
     * @param value initial seed number
     */
    void
    initialize(register uint32_t value) throw ();

    /**
     * Refreshes state after each pass
     */
    void
    reinit() throw ();

  private:
    uint32_t State_[STATE_SIZE];
    const uint32_t* Next_;
    int Left_;
  };
}

//
// INLINES
//

namespace Generics
{
  inline
  MT19937::MT19937() throw ()
  {
    seed();
  }

  inline
  MT19937::MT19937(const uint32_t value) throw ()
  {
    seed(value);
  }

  inline
  MT19937::MT19937(const uint32_t* value, size_t size) throw ()
  {
    seed(value, size);
  }

  inline
  uint32_t
  MT19937::rand() throw ()
  {
    if (!Left_)
    {
      reinit();
    }
    Left_--;

    register uint32_t res = *Next_++;
    res ^= (res >> 11);
    res ^= (res << 7) & 0x9D2C5680ul;
    res ^= (res << 15) & 0xEFC60000ul;
    return (res ^ (res >> 18));
  }

  namespace MT19937Helper
  {
    inline
    uint32_t
    MT19937_hash(const void* data, size_t size) throw ()
    {
      const uint8_t* ptr = static_cast<const uint8_t*>(data);
      uint32_t value = 0;
      while (size-- > 0)
      {
        value *= 257;
        value += *ptr++;
      }
      return value;
    }

    template <typename T>
    inline
    uint32_t
    MT19937_hash(const T data) throw ()
    {
      return MT19937_hash(&data, sizeof(data));
    }

    inline
    uint32_t
    MT19937_mix(const register uint32_t m, const register uint32_t s0,
      const register uint32_t s1) throw ()
    {
      return m ^
        (((s0 & 0x80000000ul) | (s1 & 0x7FFFFFFFul)) >> 1) ^
        (-(s1 & 0x00000001ul) & 0x9908B0DFul);
    }
  }

  inline
  void
  MT19937::reinit() throw ()
  {
    static const size_t PERIOD_LENGTH = 397;

    register uint32_t* p = State_;
    for (register int i = STATE_SIZE - PERIOD_LENGTH; i--; ++p)
    {
      *p = MT19937Helper::MT19937_mix(p[PERIOD_LENGTH], p[0], p[1]);
    }
    for (register int i = PERIOD_LENGTH; --i; ++p)
    {
      *p = MT19937Helper::MT19937_mix(p[PERIOD_LENGTH - STATE_SIZE],
        p[0], p[1]);
    }
    *p = MT19937Helper::MT19937_mix(p[PERIOD_LENGTH - STATE_SIZE],
      p[0], State_[0]);

    Left_ = STATE_SIZE;
    Next_ = State_;
  }

  inline
  void
  MT19937::seed() throw ()
  {
    int urandom = open("/dev/urandom", O_RDONLY);
    if (urandom >= 0)
    {
      uint32_t value[STATE_SIZE];
      bool success;
      for (;;)
      {
        ssize_t res = read(urandom, value, sizeof(value));
        success = res == sizeof(value);
        if (res <= 0 || success)
        {
          break;
        }
      }
      close(urandom);
      if (success)
      {
        seed(value, STATE_SIZE);
        return;
      }
    }

    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    seed(
      MT19937Helper::MT19937_hash(hostname, sizeof(hostname)) ^
      MT19937Helper::MT19937_hash(Time::get_time_of_day()) ^
      MT19937Helper::MT19937_hash(clock()) ^
      MT19937Helper::MT19937_hash(getpid()));
  }

  inline
  void
  MT19937::seed(uint32_t value) throw ()
  {
    initialize(value);
    reinit();
  }

  inline
  void
  MT19937::seed(const uint32_t* value, size_t size) throw ()
  {
    initialize(0x21414B53ul);
    register size_t i = 1;
    register uint32_t j = 0;
    for (register size_t k = (STATE_SIZE > size ? STATE_SIZE : size);
      k; --k)
    {
      State_[i] ^= (State_[i - 1] ^ (State_[i - 1] >> 30)) * 1664525ul;
      State_[i] += value[j] + j;
      if (++i >= STATE_SIZE)
      {
        State_[0] = State_[STATE_SIZE - 1];
        i = 1;
      }
      if (++j >= size)
      {
        j = 0;
      }
    }
    for (register size_t k = STATE_SIZE - 1; k; --k)
    {
      State_[i] ^= (State_[i - 1] ^ (State_[i - 1] >> 30)) * 1566083941ul;
      State_[i] -= i;
      if (++i >= STATE_SIZE)
      {
        State_[0] = State_[STATE_SIZE - 1];
        i = 1;
      }
    }
    State_[0] = 0x80000000ul;
    reinit();
  }

  inline
  void
  MT19937::initialize(register uint32_t value) throw ()
  {
    register uint32_t* s = State_;
    *s++ = value;
    for (register size_t i = 1; i < STATE_SIZE; ++i)
    {
      value = 1812433253ul * (value ^ (value >> 30)) + i;
      *s++ = value;
    }
  }
}

#endif
