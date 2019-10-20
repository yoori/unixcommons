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



#include <CORBACommons/ObjectPool.hpp>


struct ConfigInt : public CORBACommons::ObjectPoolConfiguration<int, int>
{
  struct Resolver
  {
    template <typename T>
    T
    resolve(int ref) throw ()
    {
      return -ref;
    }
  };

  Resolver resolver;
};


typedef CORBACommons::ObjectPool<int, ConfigInt,
  CORBACommons::ObjectPlainVar<int>> Pool;

void
test() throw (eh::Exception)
{
  ConfigInt conf;

  conf.timeout = Generics::Time::ONE_HOUR;
  conf.iors_list.push_back(ConfigInt::RefAndNumber(1));
  conf.iors_list.push_back(ConfigInt::RefAndNumber(2));

  Pool pool(conf);

  Pool::ObjectHandlerType o1 = pool.get_object();
  assert(*o1 == -2);
  Pool::ObjectHandlerType o2 = pool.get_object();
  assert(*o2 == -1);
  Pool::ObjectHandlerType o3 = std::move(o2);
  o1.release_bad();
  o3.release();
  o2 = pool.get_object();
  assert(*o2 == -1);
}

void
test2() throw (eh::Exception)
{
  ConfigInt conf;

  conf.timeout = Generics::Time::ONE_SECOND;
  conf.iors_list.push_back(ConfigInt::RefAndNumber(1));
  conf.iors_list.push_back(ConfigInt::RefAndNumber(2));

  Pool pool(conf, CORBACommons::ChoosePolicyType::PT_PRECISE);

  {
    Pool::ObjectHandlerType o1 = pool.get_object(0);
    assert(*o1 == -1);
    Pool::ObjectHandlerType o2 = pool.get_object(1);
    assert(*o2 == -2);
    o1.release_bad();
    o2.release();
  }

  {
    try
    {
      Pool::ObjectHandlerType o1 = pool.get_object(0);
      assert(false);
    }
    catch (const Pool::NoGoodReference&)
    {
    }
    Pool::ObjectHandlerType o2 = pool.get_object(1);
    assert(*o2 == -2);
    o2.release();
  }

  sleep(1);

  {
    Pool::ObjectHandlerType o1 = pool.get_object(0);
    assert(*o1 == -1);
    Pool::ObjectHandlerType o2 = pool.get_object(1);
    assert(*o2 == -2);
    o1.release();
    o2.release_bad();
  }
}

int
main()
{
  try
  {
    test();
    test2();
  }
  catch (const CORBA::Exception& e)
  {
    std::cerr << "CORBA::Exception:" << e;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "eh::Exception:" << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception" << std::endl;
  }

  return -1;
}
