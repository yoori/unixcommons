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



#include <memory>
#include <string>
#include <iostream>
#include <algorithm>

#include <eh/Exception.hpp>
#include <Generics/ArrayAutoPtr.hpp>
#include <Generics/MemBuf.hpp>
#include <Generics/Rand.hpp>

#include <Stream/MemoryStream.hpp>

using namespace Generics;

namespace
{
  // changing required additional code modification.
  const std::size_t BUF_SIZE = 1024;

  DECLARE_EXCEPTION(Exception, eh::DescriptiveException);
}

// Check respawn buffer into new one, with using old content.
// Checking correct size and data structure, after
// doubling MemBuf object.

void
do_test_fill_with_merge(MemBuf& buf) throw (eh::Exception)
{
  {
    MemBuf new_buf(BUF_SIZE * 2);

    const unsigned char* ptr =
      buf.get<const unsigned char>();
    std::merge(ptr, ptr + BUF_SIZE,
      ptr, ptr + BUF_SIZE, new_buf.get<unsigned char>());

    buf.swap(new_buf);
  }

  if (buf.size() != BUF_SIZE * 2)
  {
    std::cerr << "Fail: result buffer size=" << buf.size()
      << " instead " << BUF_SIZE * 2 << std::endl;
  }

  for (std::size_t i = 0; i < buf.size(); ++i)
  {
    const unsigned char* ptr = buf.get<const unsigned char>();
    if (ptr[i] != (i / 8) % 256)
    {
      std::cerr << "Fail: cannot produce merged buffer with right content."
        " Position " << i << " contain "
        << static_cast<std::size_t>(ptr[i]) <<
        " instead " << (i / 8) % 256 << std::endl;
    }
  }
}

void
do_test_resize(MemBuf& buf) throw (eh::Exception)
{
  buf.resize(0);
  buf.resize(BUF_SIZE);
  buf.resize(BUF_SIZE * 2);
  try
  {
    buf.resize(BUF_SIZE * 3);
    throw Exception("Buffer overflows allowed");
  }
  catch (const Generics::MemBuf::RangeError&)
  {
  }
}

void
do_test_copyconstructible(MemBuf& buf, MemBuf copy_buf) throw (eh::Exception)
{
  const char FUN[] = "do_test_copyconstructible: ";
  Stream::Error ostr;
  if (buf.size() != copy_buf.size())
  {
    ostr << " Unequal size: left=" << buf.size()
      << ", right=" << copy_buf.size() << std::endl;
  }
  if (memcmp(buf.data(), copy_buf.data(), buf.size()) != 0)
  {
    ostr << " Unequal content " << buf.size() << " ";
    ostr.write(buf.get<const char>(), buf.size());
    ostr << "\ncopy:\n";
    ostr.write(copy_buf.get<const char>(), copy_buf.size());
  }
  const String::SubString& str = ostr.str();
  if (str.size())
  {
    std::cerr << FUN << str << std::endl;
  }
}

void
do_test_assignable(MemBuf& will_assign) throw (eh::Exception)
{
  const char FUN[] = "do_test_assignable: ";
  ::memset(will_assign.data(), 0xFF, will_assign.size());
  MemBuf buf = will_assign;
  Stream::Error ostr;
  if (buf.size() != will_assign.size())
  {
    ostr << " Unequal size ";
  }
  if (memcmp(buf.data(), will_assign.data(), buf.size()) != 0)
  {
    ostr << " Unequal content " << buf.size() << " ";
    ostr.write(buf.get<const char>(), buf.size());
    ostr << "\ncopy:\n";
    ostr.write(will_assign.get<const char>(), will_assign.size());
  }
  const String::SubString& str = ostr.str();
  if (str.size())
  {
    std::cerr << FUN << str << std::endl;
  }
}

void
do_usable_test() throw (eh::Exception)
{
  const char FUN[] = "do_usable_test: ";

  for (std::size_t i = 0; i < 10000; ++i)
  {
    std::size_t size = safe_rand(0, 10240);
    Generics::MemBuf tmp(size);
    memset(tmp.data(), 0xFF, size);
    {
      Generics::MemBuf tmp2(size + 377);
      memset(tmp2.data(), 0x0, size + 377);
      tmp2 = std::move(tmp);
      tmp = Generics::MemBuf(tmp2);
      if (memcmp(tmp.data(), tmp2.data(), tmp.size()))
      {
        std::cerr << FUN << "Buffer content check failed" << std::endl;
        return;
      }
    }
    Generics::MemBuf tmp3(tmp);
    if (memcmp(tmp.data(), tmp3.data(), tmp.size()))
    {
      std::cerr << FUN << "Buffer content check failed" << std::endl;
      return;
    }
  }
}

void
smart_membuf() throw (eh::Exception)
{
  SmartMemBuf_var s1(new SmartMemBuf(100));
  SmartMemBuf_var s2 = s1;

  // does not compile
  //ConstSmartMemBuf_var c1(s1);
  //ConstSmartMemBuf_var c1(new ConstSmartMemBuf(s1));
  //ConstSmartMemBuf_var c1(new ConstSmartMemBuf(*s1));

  // create a copy
  ConstSmartMemBuf_var c1(new ConstSmartMemBuf(s1->membuf()));
  if (c1->membuf().empty() || s1->membuf().empty() ||
    s2->membuf().empty())
  {
    std::cerr << FNS << "Copy construction error" << std::endl;
    return;
  }

  // move ownership
  ConstSmartMemBuf_var c2(Generics::transfer_membuf(s1));
  if (c2->membuf().empty() || !s1->membuf().empty() ||
    !s2->membuf().empty())
  {
    std::cerr << FNS << "Ownership transfer error" << std::endl;
    return;
  }
}

int
main()
{
  std::cout << "MemBuf test started" << std::endl;

  try
  {
    MemBuf buf(BUF_SIZE);
    for (std::size_t i = 0; i < BUF_SIZE; ++i)
    {
      buf.get<unsigned char>()[i] = i % 256;
    }
    unsigned char* ptr = buf.get<unsigned char>();
    std::sort(ptr, ptr + buf.size());

    do_test_fill_with_merge(buf);
    do_test_copyconstructible(buf, buf);
    do_test_assignable(buf);
    do_test_resize(buf);

    buf.resize(BUF_SIZE / 2);
    do_test_copyconstructible(buf, buf);
    do_test_assignable(buf);
    do_test_resize(buf);

    buf.resize(0);
    do_test_copyconstructible(buf, buf);
    do_test_assignable(buf);
    do_test_resize(buf);

    do_usable_test();

    {
      MemBuf buf(BUF_SIZE);
      MemBuf buf1(BUF_SIZE);
      buf.clear();
      buf.clear();
      //buf = buf1;
      MemBuf buf2(buf1);
    }

    smart_membuf();

    std::cout << "Test complete" << std::endl;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "FAIL:" << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "unknown exception" << std::endl;
  }

  return 0;
}
