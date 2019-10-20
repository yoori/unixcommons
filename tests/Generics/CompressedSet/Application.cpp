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



#include <iostream>
#include <set>
#include <assert.h>
#include <stdlib.h>

#include <Generics/CompressedSet.hpp>


template <typename Integer>
class DebugCompressedSet : private Generics::CompressedSet<Integer>
{
public:
  void
  add(Integer low, Integer high) throw (eh::Exception)
  {
    std::cout << "Add " << low << " " << high << "\n";
    check();
    Generics::CompressedSet<Integer>::add(low, high);
  }

  void
  remove(Integer low, Integer high) throw (eh::Exception)
  {
    std::cout << "Remove " << low << " " << high << "\n";
    check();
    Generics::CompressedSet<Integer>::remove(low, high);
  }

  using Generics::CompressedSet<Integer>::belongs;
  using Generics::CompressedSet<Integer>::check_presence;

  void
  check() const throw (eh::Exception)
  {
    if (this->holder_.empty())
    {
      return;
    }

    typename Generics::CompressedSet<Integer>::Holder::const_iterator
      itor(this->holder_.begin());
    assert(itor->first <= itor->second);
    std::cout << itor->first << ":" << itor->second;
    for (typename Generics::CompressedSet<Integer>::Holder::const_iterator
      prev(itor); ++itor != this->holder_.end(); prev = itor)
    {
      std::cout << " " << itor->first << ":" << itor->second;
      assert(itor->first > prev->second + 1);
    }
    std::cout << std::endl;
  }
};

template <typename Integer>
class SimpleSet
{
public:
  void
  add(Integer low, Integer high) throw (eh::Exception)
  {
    for (; low <= high; low++)
    {
      holder_.insert(low);
    }
  }

  void
  add(Integer value) throw (eh::Exception)
  {
    holder_.insert(value);
  }

  void
  remove(Integer low, Integer high) throw (eh::Exception)
  {
    for (; low <= high; low++)
    {
      holder_.erase(low);
    }
  }

  void
  remove(Integer value) throw (eh::Exception)
  {
    holder_.erase(value);
  }

  bool
  belongs(Integer value) const throw (eh::Exception)
  {
    return holder_.find(value) != holder_.end();
  }

  typename Generics::CompressedSet<Integer>::CheckStatus
  check_presence(Integer low, Integer high) const throw (eh::Exception)
  {
    bool all = true;
    bool none = true;
    for (; low <= high; low++)
    {
      bool status = belongs(low);
      all = all && status;
      none = none && !status;
      if (!all && !none)
      {
        return Generics::CompressedSet<Integer>::CS_SOME;
      }
    }

    return none ? Generics::CompressedSet<Integer>::CS_NONE :
      Generics::CompressedSet<Integer>::CS_ALL;
  }

private:
  std::set<Integer> holder_;
};

int
main()
{
  const int MIN = 0, MAX = 50;

  DebugCompressedSet<int> set1;
  SimpleSet<int> set2;

  for (int i = 0; i < 1000; i++)
  {
    for (int j = rand() % 10; j > 0; j--)
    {
      int min = rand() % (MAX - MIN) + MIN;
      int max = rand() % (MAX - min) + min;
      set1.add(min, max);
      set2.add(min, max);
    }
    for (int j = rand() % 10; j > 0; j--)
    {
      int min = rand() % (MAX - MIN) + MIN;
      int max = rand() % (MAX - min) + min;
      set1.remove(min, max);
      set2.remove(min, max);
    }

    std::cout << "Checking match" << std::endl;
    set1.check();
    bool fail = false;
    for (int i = MIN; i < MAX; i++)
    {
      bool res1 = set1.belongs(i);
      bool res2 = set2.belongs(i);
      if (res1 != res2)
      {
        std::cerr << "For " << i << " compressed = " << res1 <<
          " but normal = " << res2 << std::endl;
        fail = true;
      }
    }

    for (int i = MIN; i < MAX; i++)
    {
      for (int j = i; j < MAX; j++)
      {
        Generics::CompressedSet<int>::CheckStatus res1 =
          set1.check_presence(i, j);
        Generics::CompressedSet<int>::CheckStatus res2 =
          set2.check_presence(i, j);
        if (res1 != res2)
        {
          std::cerr << "For " << i << ", " << j << " compressed = " <<
            res1 << " but normal = " << res2 << std::endl;
          fail = true;
        }
      }
    }

    if (fail)
    {
      return -1;
    }
  }

  std::cout << "Test complete" << std::endl;

  return 0;
}
