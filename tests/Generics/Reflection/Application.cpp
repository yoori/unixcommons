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
#include <string>

#include <Generics/Reflection.hpp>

//#define INHERITANCE

// Base class for all members
struct MemberBase
{
  virtual
  ~MemberBase()
  {
  }

  virtual
  void
  print(std::ostream& ostr) = 0;
};

// General member
template <typename Data>
class MemberCommon : public MemberBase
{
public:
  Data&
  data()
  {
    return data_;
  }

  const Data&
  data() const
  {
    return data_;
  }

  virtual
  void
  print(std::ostream& ostr)
  {
    ostr << data_;
  }

private:
  Data data_;
};

// Certain member class
typedef MemberCommon<std::string> MemberString;

// Certain member class
typedef MemberCommon<int> MemberInt;

#define LIST(type) \
  REFLECTION_INIT(type, MemberBase) \
  REFLECTION_MEMBER1(MemberString, string1) \
  REFLECTION_MEMBER1(MemberString, string2) \
  REFLECTION_MEMBER1(MemberInt, int1) \
  REFLECTION_MEMBER1_NAME(MemberInt, int2, "int3")

#define INIT(sa) \
{ \
  sa.string1.data() = "1"; \
  sa.string2.data() = "2"; \
  sa.int1.data() = 3; \
  sa.int2.data() = 4; \
}

#define PRINT(type, sa) \
{ \
  std::cout << #type << "\n"; \
  for (type::ReflectionMembersInfo::const_iterator \
    itor(sa.reflection_info().begin()); \
    itor != sa.reflection_info().end(); ++itor) \
  { \
    std::cout << itor->name << "="; \
    (sa.*(itor->member))().print(std::cout); \
    std::cout << "\n"; \
  } \
  std::cout << "\n"; \
}

// The final class definition
struct SA1
{
  LIST(SA1)
};

class SA2
{
  LIST(SA2)
  public:
  SA2()
  {
    INIT((*this));
  }
  void
  print()
  {
    PRINT(SA2, (*this));
  }
};

template <typename T>
struct SA3
{
  LIST(SA3)
};

#ifdef INHERITANCE
struct SA4 : SA1
{
  REFLECTION_INIT(SA4, MemberBase)
  REFLECTION_INHERITS(SA1)
  REFLECTION_MEMBER1(MemberString, string5)
};

struct SA5 : SA1
{
  REFLECTION_INIT(SA5, MemberBase)
  REFLECTION_INHERITS(SA1)
  REFLECTION_MEMBER2(MemberString, string5)
};
#endif

struct SimpleStrings
{
  std::string s1;
  std::string s2;
  std::string s3;
  std::string s4;
  std::string s5;
  std::string s6;
  std::string s7;
};

#define LIST_ \
  s5("5"), \
  s6(static_cast<const char*>("6")), \
  s7(std::string("7"))

#define INIT_(sa) \
{ \
  sa.s1 = "1"; \
  sa.s2 = static_cast<const char*>("2"); \
  sa.s3 = '3'; \
  sa.s4 = std::string("4"); \
}

#define PRINT_(type, sa) \
{ \
  std::cout << #type << "\n" << \
    sizeof(type) << " " << sizeof(SimpleStrings) << "\n"; \
  for (type::ReflectionMembersInfo::const_iterator \
    itor(sa.reflection_info().begin()); \
    itor != sa.reflection_info().end(); ++itor) \
  { \
    std::cout << itor->name << "=" << (sa.*(itor->member))() << "\n"; \
  } \
  std::cout << "\n"; \
}

struct Strings1
{
  REFLECTION_INIT(Strings1, std::string)
  REFLECTION_MEMBER1(std::string, s1)
  REFLECTION_MEMBER1(std::string, s2)
  REFLECTION_MEMBER1(std::string, s3)
  REFLECTION_MEMBER1(std::string, s4)
  REFLECTION_MEMBER1(std::string, s5)
  REFLECTION_MEMBER1(std::string, s6)
  REFLECTION_MEMBER1(std::string, s7)
  Strings1()
    : LIST_
  {
  }
};

struct Strings2
{
  REFLECTION_INIT(Strings2, std::string)
  REFLECTION_MEMBER2(std::string, s1)
  REFLECTION_MEMBER2(std::string, s2)
  REFLECTION_MEMBER2(std::string, s3)
  REFLECTION_MEMBER2(std::string, s4)
  REFLECTION_MEMBER2(std::string, s5)
  REFLECTION_MEMBER2(std::string, s6)
  REFLECTION_MEMBER2(std::string, s7)
  Strings2()
    : LIST_
  {
  }
};

int
main()
{
  {
    SA1 sa;
    INIT(sa);
    PRINT(SA1, sa);
  }
  {
    SA2 sa;
    sa.print();
  }
  {
    SA3<int> sa;
    INIT(sa);
    PRINT(SA3<int>, sa);
  }
#ifdef INHERITANCE
  {
    SA4 sa;
    INIT(sa);
    sa.string5.data() = "5";
    PRINT(SA4, sa);
  }
  {
    SA5 sa;
    INIT(sa);
    sa.string5.data() = "5";
    PRINT(SA5, sa);
  }
#endif
  {
    Strings1 sa;
    INIT_(sa);
    PRINT_(Strings1, sa);
  }
  {
    Strings2 sa;
    INIT_(sa);
    PRINT_(Strings2, sa);
  }
  return 0;
}
