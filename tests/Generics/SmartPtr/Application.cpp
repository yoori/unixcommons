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

#include <ReferenceCounting/ReferenceCounting.hpp>
#include <ReferenceCounting/Vector.hpp>
#include <ReferenceCounting/List.hpp>
#include <ReferenceCounting/Deque.hpp>
#include <ReferenceCounting/Map.hpp>
#include <ReferenceCounting/HashTable.hpp>
#include <ReferenceCounting/PtrHolder.hpp>

#include <Generics/HashTableAdapters.hpp>


class A : public ReferenceCounting::AtomicImpl
{
public:
  explicit
  A() throw ()
    : rc_(0), adds_(0)
  {
  }
  void
  rc(int inc = 1) const throw ()
  {
    rc_ += inc;
    assert(ref_count_ == rc_);
    //std::cout << rc_ << " " << adds_ << std::endl;
  }
  virtual
  void
  add_ref() const throw ()
  {
    ReferenceCounting::AtomicImpl::add_ref();
    adds_++;
  }
protected:
  virtual
  ~A() throw ()
  {
    std::cout << adds_ << "\n";
  }

  mutable int rc_;
  mutable int adds_;
};
typedef ReferenceCounting::SmartPtr<A> A_var;
typedef ReferenceCounting::SmartPtr<const A> CA_var;
typedef ReferenceCounting::FixedPtr<A> AFtr;
typedef ReferenceCounting::FixedPtr<const A> CAFtr;
typedef ReferenceCounting::QualPtr<A> APtr;
typedef ReferenceCounting::QualPtr<const A> CAPtr;
typedef ReferenceCounting::ConstPtr<A> CACtr;

class B : public A
{
protected:
  virtual
  ~B() throw ()
  {
  }
};
typedef ReferenceCounting::SmartPtr<B> B_var;
typedef ReferenceCounting::SmartPtr<const B> CB_var;
typedef ReferenceCounting::FixedPtr<B> BFtr;
typedef ReferenceCounting::FixedPtr<const B> CBFtr;
typedef ReferenceCounting::QualPtr<B> BPtr;
typedef ReferenceCounting::QualPtr<const B> CBPtr;
typedef ReferenceCounting::ConstPtr<B> CBCtr;

A_var
a_var() throw (eh::Exception)
{
  return new A;
}

AFtr
a_ftr() throw (eh::Exception)
{
  return new A;
}

APtr
a_ptr() throw (eh::Exception)
{
  return new A;
}

B_var
b_var() throw (eh::Exception)
{
  return new B;
}

BFtr
b_ftr() throw (eh::Exception)
{
  return new B;
}

BPtr
b_ptr() throw (eh::Exception)
{
  return new B;
}

void
test0() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  A_var sa1(new A);
  sa1->rc();
  A_var sa2(new B);
  sa2->rc();
  A_var sa3(sa1);
  sa1->rc();
  const A_var sa4(sa2);
  sa2->rc();
  A_var sa5(sa4);
  sa2->rc();
  CA_var sa6(sa3);
  sa1->rc();
  const CA_var sa7(sa4);
  sa2->rc();
  CA_var sa8(sa7);
  sa2->rc();

  AFtr fa1(new A);
  fa1->rc();
  AFtr fa2(new B);
  fa2->rc();
  AFtr fa3(fa1);
  fa1->rc();
  const AFtr fa4(fa2);
  fa2->rc();
  //AFtr fa5(fa4);
  CAFtr fa6(fa2);
  fa2->rc();
  const CAFtr fa7(fa4);
  fa2->rc();
  CAFtr fa8(fa7);
  fa2->rc();

  APtr pa1(new A);
  pa1->rc();
  APtr pa2(new B);
  pa2->rc();
  APtr pa3(pa1);
  pa1->rc();
  const APtr pa4(pa2);
  pa2->rc();
  //APtr pa5(pa4);
  CAPtr pa6(pa2);
  pa2->rc();
  const CAPtr pa7(pa4);
  pa2->rc();
  CAPtr pa8(pa7);
  pa2->rc();

  CACtr ca1(new A);
  ca1->rc();
  CACtr ca2(new B);
  ca2->rc();
  CACtr ca3(ca1);
  ca1->rc();
  const CACtr ca4(ca2);
  ca2->rc();
  CACtr ca5(ca4);
  ca2->rc();
  CACtr ca6(ca2);
  ca2->rc();
  const CACtr ca7(ca4);
  ca2->rc();
  CACtr ca8(ca7);
  ca2->rc();

  A_var sa9(fa1);
  fa1->rc();
  //A_var sa10(fa4);
  //CA_var sa11(fa4);
  //fa2->rc();
  A_var sa12(pa1);
  pa1->rc();
  //A_var sa13(pa4);
  //CA_var sa14(pa4);
  //pa2->rc();
  //A_var sa15(ca1);
  CA_var sa16(ca1);
  ca1->rc();
  //A_var sa17(ca4);
  //CA_var sa18(ca4);
  //ca2->rc();

  AFtr fa9(sa1);
  sa1->rc();
  AFtr fa10(sa4);
  sa2->rc();
  CAFtr fa11(sa2);
  sa2->rc();
  CAFtr fa12(sa4);
  sa2->rc();
  //AFtr fa15(ca1);
  CAFtr fa16(ca1);
  ca1->rc();
  //AFtr fa17(ca4);
  CAFtr fa18(ca4);
  ca2->rc();

  APtr pa9(sa1);
  sa1->rc();
  APtr pa10(sa4);
  sa2->rc();
  CAPtr pa11(sa2);
  sa2->rc();
  CAPtr pa12(sa4);
  sa2->rc();
  //APtr pa15(ca1);
  CAPtr pa16(ca1);
  ca1->rc();
  //APtr pa17(ca4);
  CAPtr pa18(ca4);
  ca2->rc();
}

void
test00() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  A_var sa1(a_var());
  sa1->rc();
  A_var sa2(a_ftr());
  sa2->rc();
  A_var sa3(a_ptr());
  sa3->rc();
  A_var sa4 = a_var();
  sa4->rc();
  A_var sa5 = a_ftr();
  sa5->rc();
  A_var sa6 = a_ptr();
  sa6->rc();
  CA_var sa7(a_var());
  sa7->rc();
  CA_var sa8(a_ftr());
  sa8->rc();
  CA_var sa9(a_ptr());
  sa9->rc();
  CA_var sa10 = a_var();
  sa10->rc();
  CA_var sa11 = a_ftr();
  sa11->rc();
  CA_var sa12 = a_ptr();
  sa12->rc();

  AFtr fa1(a_var());
  fa1->rc();
  AFtr fa2(a_ftr());
  fa2->rc();
  AFtr fa3(a_ptr());
  fa3->rc();
  AFtr fa4 = a_var();
  fa4->rc();
  AFtr fa5 = a_ftr();
  fa5->rc();
  AFtr fa6 = a_ptr();
  fa6->rc();
  CAFtr fa7(a_var());
  fa7->rc();
  CAFtr fa8(a_ftr());
  fa8->rc();
  CAFtr fa9(a_ptr());
  fa9->rc();
  CAFtr fa10 = a_var();
  fa10->rc();
  CAFtr fa11 = a_ftr();
  fa11->rc();
  CAFtr fa12 = a_ptr();
  fa12->rc();

  APtr pa1(a_var());
  pa1->rc();
  APtr pa2(a_ftr());
  pa2->rc();
  APtr pa3(a_ptr());
  pa3->rc();
  APtr pa4 = a_var();
  pa4->rc();
  APtr pa5 = a_ftr();
  pa5->rc();
  APtr pa6 = a_ptr();
  pa6->rc();
  CAPtr pa7(a_var());
  pa7->rc();
  CAPtr pa8(a_ftr());
  pa8->rc();
  CAPtr pa9(a_ptr());
  pa9->rc();
  CAPtr pa10 = a_var();
  pa10->rc();
  CAPtr pa11 = a_ftr();
  pa11->rc();
  CAPtr pa12 = a_ptr();
  pa12->rc();
}

void
test1() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  A_var sa1; sa1 = new A;
  sa1->rc();
  A_var sa2; sa2 = new B;
  sa2->rc();
  A_var sa3; sa3 = sa1;
  sa1->rc();
  //const A_var sa4; sa4 = sa2;
  const A_var sa4(sa2);
  sa2->rc();
  A_var sa5; sa5 = sa4;
  sa2->rc();
  CA_var sa6(sa3);
  sa1->rc();
  CA_var sa7(sa4);
  sa2->rc();

  APtr pa1; pa1 = new A;
  pa1->rc();
  APtr pa2; pa2 = new B;
  pa2->rc();
  APtr pa3; pa3 = pa1;
  pa1->rc();
  //const APtr pa4; pa4 = pa2;
  const APtr pa4(pa2);
  pa2->rc();
  //APtr pa5; pa5 = pa4;
  CAPtr pa6; pa6 = pa2;
  pa2->rc();
  CAPtr pa7; pa7 = pa4;
  pa2->rc();

  CACtr ca1; ca1 = new A;
  ca1->rc();
  CACtr ca2; ca2 = new B;
  ca2->rc();
  CACtr ca3; ca3 = ca1;
  ca1->rc();
  //const CACtr ca4; ca4 = ca2;
  const CACtr ca4(ca2);
  ca2->rc();
  //CACtr ca5; ca5 = ca4;
  CACtr ca6; ca6 = ca2;
  ca2->rc();
  CACtr ca7; ca7 = ca4;
  ca2->rc();

  A_var sa8; sa8 = pa1;
  pa1->rc();
  //A_var sa9; sa9 = pa4;
  //CA_var sa10; sa10 = pa4;
  //pa2->rc();
  //A_var sa11; sa11 = ca4;
  //CA_var sa12; sa12 = ca4;
  //ca2->rc();

  APtr pa8; pa8 = sa1;
  sa1->rc();
  APtr pa9; pa9 = sa4;
  sa2->rc();
  CAPtr pa10; pa10 = sa2;
  sa2->rc();
  CAPtr pa11; pa11 = sa4;
  sa2->rc();
  //APtr pa12; pa12 = ca1;
  //APtr pa13; pa13 = ca4;
  CAPtr pa14; pa14 = ca2;
  ca2->rc();
  CAPtr pa15; pa15 = ca4;
  ca2->rc();

  CACtr ca8; ca8 = sa1;
  sa1->rc();
  CACtr ca9; ca9 = sa4;
  sa2->rc();
  CACtr ca10; ca10 = sa2;
  sa2->rc();
  CACtr ca11; ca11 = sa4;
  sa2->rc();
  CACtr ca12; ca12 = pa1;
  pa1->rc();
  CACtr ca13; ca13 = pa4;
  pa2->rc();
  CACtr ca14; ca14 = pa2;
  pa2->rc();
  CACtr ca15; ca15 = pa4;
  pa2->rc();

  const CA_var sa20(sa2);
  sa2->rc();
  CA_var sa21(sa20);
  sa2->rc();
  CA_var sa22; sa22 = sa20;
  sa2->rc();

  const CAPtr pa20(pa2);
  pa2->rc();
  CA_var pa21(pa20);
  pa2->rc();
  CAPtr pa22; pa22 = pa20;
  pa2->rc();

  const CAPtr ca20(ca2);
  ca2->rc();
  CA_var ca21(ca20);
  ca2->rc();
  CAPtr ca22; ca22 = ca20;
  ca2->rc();
}

void
test1_() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  A_var sa1; sa1 = new A;
  sa1->rc();
  sa1 = sa1;
  sa1->rc(0);
  sa1 = const_cast<const A_var&>(sa1);
  sa1->rc(0);
  sa1 = ReferenceCounting::add_ref(sa1);
  sa1->rc(0);
  sa1 = ReferenceCounting::add_ref(const_cast<const A_var&>(sa1));
  sa1->rc(0);
  CA_var sa2(sa1);
  sa1->rc();
  sa2 = sa2;
  sa2->rc(0);
  sa2 = const_cast<const CA_var&>(sa2);
  sa2->rc(0);
  sa2 = std::move(sa2);
  sa2->rc(0);
  sa2 = std::move(const_cast<const CA_var&>(sa2));
  sa2->rc(0);
  sa2 = ReferenceCounting::add_ref(sa2);
  sa2->rc(0);
  sa2 = ReferenceCounting::add_ref(const_cast<const CA_var&>(sa2));
  sa2->rc(0);

  APtr pa1; pa1 = new A;
  pa1->rc();
  pa1 = pa1;
  pa1->rc(0);
  //pa1 = const_cast<const APtr&>(pa1);
  pa1 = std::move(pa1);
  pa1->rc(0);
  //pa1 = std::move(const_cast<const APtr&>(pa1));
  pa1 = ReferenceCounting::add_ref(pa1);
  pa1->rc(0);
  //pa1 = ReferenceCounting::add_ref(const_cast<const APtr&>(pa1));
  CAPtr pa2; pa2 = pa1;
  pa2->rc();
  pa2 = pa2;
  pa2->rc(0);
  pa2 = const_cast<const CAPtr&>(pa2);
  pa2->rc(0);
  pa2 = std::move(pa2);
  pa2->rc(0);
  pa2 = std::move(const_cast<const CAPtr&>(pa2));
  pa2->rc(0);
  pa2 = ReferenceCounting::add_ref(pa2);
  pa2->rc(0);
  pa2 = ReferenceCounting::add_ref(const_cast<const CAPtr&>(pa2));
  pa2->rc(0);

  CACtr ca1; ca1 = new A;
  ca1->rc();
  ca1 = ca1;
  ca1->rc(0);
  ca1 = const_cast<const CACtr&>(ca1);
  ca1->rc(0);
  ca1 = std::move(ca1);
  ca1->rc(0);
  ca1 = std::move(const_cast<const CACtr&>(ca1));
  ca1->rc(0);
  ca1 = ReferenceCounting::add_ref(ca1);
  ca1->rc(0);
  ca1 = ReferenceCounting::add_ref(const_cast<const CACtr&>(ca1));
  ca1->rc(0);
}

void
test10() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  A_var sa1;
  sa1 = a_var();
  sa1->rc();
  sa1 = a_ftr();
  sa1->rc();
  sa1 = a_ptr();
  sa1->rc();
  CA_var sa2;
  sa2 = a_var();
  sa2->rc();
  sa2 = a_ftr();
  sa2->rc();
  sa2 = a_ptr();
  sa2->rc();

  APtr pa1;
  pa1 = a_var();
  pa1->rc();
  pa1 = a_ftr();
  pa1->rc();
  pa1 = a_ptr();
  pa1->rc();
  CAPtr pa2;
  pa2 = a_var();
  pa2->rc();
  pa2 = a_ftr();
  pa2->rc();
  pa2 = a_ptr();
  pa2->rc();

  CACtr ca;
  ca = a_var();
  ca->rc();
  ca = a_ftr();
  ca->rc();
  ca = a_ptr();
  ca->rc();
}

void
test2() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  B_var sb1(new B);
  sb1->rc();
  const B_var sb2(sb1);
  sb1->rc();
  CB_var sb3(sb2);
  sb1->rc();

  BPtr pb1(sb1);
  sb1->rc();
  const BPtr pb2(sb2);
  sb1->rc();
  CBPtr pb3(sb2);
  sb1->rc();

  CBCtr cb1(sb1);
  sb1->rc();
  const CBCtr cb2(sb2);
  sb1->rc();
  CBCtr cb3(sb2);
  sb1->rc();

  A_var sa1(sb1);
  sb1->rc();
  A_var sa2(sb2);
  sb1->rc();
  //A_var sa3(sb3);
  CA_var sa4(sb1);
  sb1->rc();
  CA_var sa5(sb2);
  sb1->rc();
  CA_var sa6(sb3);
  sb1->rc();
  A_var sa7(pb1);
  sb1->rc();
  //A_var sa8(pb2);
  //A_var sa9(pb3);
  CA_var sa10(pb1);
  sb1->rc();
  //CA_var sa11(pb2);
  //sb1->rc();
  CA_var sa12(pb3);
  sb1->rc();
  //A_var sa13(cb1);
  //A_var sa14(cb2);
  //A_var sa15(cb3);
  CA_var sa16(cb1);
  sb1->rc();
  //CA_var sa17(cb2);
  //sb1->rc();
  CA_var sa18(cb3);
  sb1->rc();

  APtr pa1(sb1);
  sb1->rc();
  APtr pa2(sb2);
  sb1->rc();
  //APtr pa3(sb3);
  CAPtr pa4(sb1);
  sb1->rc();
  CAPtr pa5(sb2);
  sb1->rc();
  CAPtr pa6(sb3);
  sb1->rc();
  APtr pa7(pb1);
  sb1->rc();
  //APtr pa8(pb2);
  //APtr pa9(pb3);
  CAPtr pa10(pb1);
  sb1->rc();
  CAPtr pa11(pb2);
  sb1->rc();
  CAPtr pa12(pb3);
  sb1->rc();
  //APtr pa13(cb1);
  //APtr pa14(cb2);
  //APtr pa15(cb3);
  CAPtr pa16(cb1);
  sb1->rc();
  CAPtr pa17(cb2);
  sb1->rc();
  CAPtr pa18(cb3);
  sb1->rc();
}

void
test20() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  A_var sa1(b_var());
  sa1->rc();
  A_var sa2(b_ftr());
  sa2->rc();
  A_var sa3(b_ptr());
  sa3->rc();
  A_var sa4 = b_var();
  sa4->rc();
  A_var sa5 = b_ftr();
  sa5->rc();
  A_var sa6 = b_ptr();
  sa6->rc();
  CA_var sa7(b_var());
  sa7->rc();
  CA_var sa8(b_ftr());
  sa8->rc();
  CA_var sa9(b_ptr());
  sa9->rc();
  CA_var sa10 = b_var();
  sa10->rc();
  CA_var sa11 = b_ftr();
  sa11->rc();
  CA_var sa12 = b_ptr();
  sa12->rc();

  AFtr fa1(b_var());
  fa1->rc();
  AFtr fa2(b_ftr());
  fa2->rc();
  AFtr fa3(b_ptr());
  fa3->rc();
  AFtr fa4 = b_var();
  fa4->rc();
  AFtr fa5 = b_ftr();
  fa5->rc();
  AFtr fa6 = b_ptr();
  fa6->rc();
  CAFtr fa7(b_var());
  fa7->rc();
  CAFtr fa8(b_ftr());
  fa8->rc();
  CAFtr fa9(b_ptr());
  fa9->rc();
  CAFtr fa10 = b_var();
  fa10->rc();
  CAFtr fa11 = b_ftr();
  fa11->rc();
  CAFtr fa12 = b_ptr();
  fa12->rc();

  APtr pa1(b_var());
  pa1->rc();
  APtr pa2(b_ftr());
  pa2->rc();
  APtr pa3(b_ptr());
  pa3->rc();
  APtr pa4 = b_var();
  pa4->rc();
  APtr pa5 = b_ftr();
  pa5->rc();
  APtr pa6 = b_ptr();
  pa6->rc();
  CAPtr pa7(b_var());
  pa7->rc();
  CAPtr pa8(b_ftr());
  pa8->rc();
  CAPtr pa9(b_ptr());
  pa9->rc();
  CAPtr pa10 = b_var();
  pa10->rc();
  CAPtr pa11 = b_ftr();
  pa11->rc();
  CAPtr pa12 = b_ptr();
  pa12->rc();
}

void
test3() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  B_var sb1(new B);
  sb1->rc();
  const B_var sb2(sb1);
  sb1->rc();
  CB_var sb3(sb2);
  sb1->rc();

  BPtr pb1(sb1);
  sb1->rc();
  const BPtr pb2(sb2);
  sb1->rc();
  CBPtr pb3(sb2);
  sb1->rc();

  A_var sa1; sa1 = sb1;
  sb1->rc();
  A_var sa2; sa2 = sb2;
  sb1->rc();
  //A_var sa3; sa3 = sb3;
  CA_var sa4; sa4 = sb1;
  sb1->rc();
  CA_var sa5; sa5 = sb2;
  sb1->rc();
  CA_var sa6; sa6 = sb3;
  sb1->rc();
  A_var sa7; sa7 = pb1;
  sb1->rc();
  //A_var sa8; sa8 = pb2;
  //A_var sa9; sa9 = pb3;
  CA_var sa10; sa10 = pb1;
  sb1->rc();
  //CA_var sa11; sa11 = pb2;
  //sb1->rc();
  CA_var sa12; sa12 = pb3;
  sb1->rc();

  APtr pa1; pa1 = sb1;
  sb1->rc();
  APtr pa2; pa2 = sb2;
  sb1->rc();
  //APtr pa3; pa3 = sb3;
  CAPtr pa4; pa4 = sb1;
  sb1->rc();
  CAPtr pa5; pa5 = sb2;
  sb1->rc();
  CAPtr pa6; pa6 = sb3;
  sb1->rc();
  APtr pa7; pa7 = pb1;
  sb1->rc();
  //APtr pa8; pa8 = pb2;
  //APtr pa9; pa9 = pb3;
  CAPtr pa10; pa10 = pb1;
  sb1->rc();
  CAPtr pa11; pa11 = pb2;
  sb1->rc();
  CAPtr pa12; pa12 = pb3;
  sb1->rc();
}

void
test30() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  A_var sa1;
  sa1 = b_var();
  sa1->rc();
  sa1 = b_ftr();
  sa1->rc();
  sa1 = b_ptr();
  sa1->rc();
  CA_var sa2;
  sa2 = b_var();
  sa2->rc();
  sa2 = b_ftr();
  sa2->rc();
  sa2 = b_ptr();
  sa2->rc();

  APtr pa1;
  pa1 = b_var();
  pa1->rc();
  pa1 = b_ftr();
  pa1->rc();
  pa1 = b_ptr();
  pa1->rc();
  CAPtr pa2;
  pa2 = b_var();
  pa2->rc();
  pa2 = b_ftr();
  pa2->rc();
  pa2 = b_ptr();
  pa2->rc();

  CACtr ca;
  ca = b_var();
  ca->rc();
  ca = b_ftr();
  ca->rc();
  ca = b_ptr();
  ca->rc();
}

void
test4() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  typedef ReferenceCounting::Vector<APtr> V;

  APtr a(new A);
  a->rc();

  V v(5, a);
  a->rc(5);
  V v2(v);
  a->rc(5);
  V v3;
  v3 = v;
  a->rc(5);
  V v4(5);
  V v5(std::move(v));
  v = std::move(v5);
  V v6(v.begin(), v.end());
  a->rc(5);
  v.resize(7, a);
  a->rc(2);
  v4.swap(v);
  swap(v, v4);

  v.push_back(a);
  a->rc();
  v.push_back(A_var(a));
  a->rc();
  v.emplace_back(a);
  a->rc();
  v.emplace_back(A_var(a));
  a->rc();

  v.insert(v.begin(), a);
  a->rc();
  v.insert(v.begin(), A_var(a));
  a->rc();
  v.insert(v.begin(), 2, a);
  a->rc(2);
  v.insert(v.end(), 3, a);
  a->rc(3);
  v.insert(v.begin() + 3, 25, a);
  a->rc(25);
  v.insert(v.end() - 3, v2.begin(), v2.end());
  a->rc(5);
  v.erase(v.begin(), v.begin() + 2);
  a->rc(-2);
}

void
test5() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  typedef ReferenceCounting::List<APtr> V;

  APtr a(new A);
  a->rc();

  V v(5, a);
  a->rc(5);
  V v2(v);
  a->rc(5);
  V v3;
  v3 = v;
  a->rc(5);
  V v4(5);
  V v5(std::move(v));
  v = std::move(v5);
  V v6(v.begin(), v.end());
  a->rc(5);
  v.emplace_back(a);
  v.emplace_front(a);
  a->rc(2);
  v4.swap(v);
  swap(v, v4);

  v.push_front(a);
  a->rc();
  v.push_front(A_var(a));
  a->rc();
  v.push_back(a);
  a->rc();
  v.push_back(A_var(a));
  a->rc();

  v.insert(v.begin(), a);
  a->rc();
  v.insert(v.begin(), A_var(a));
  a->rc();
  v.insert(v.begin(), 2, a);
  a->rc(2);
  v.insert(v.end(), 3, a);
  a->rc(3);
  V::iterator i(v.begin());
  std::advance(i, 3);
  v.insert(i, 25, a);
  a->rc(25);
  i = v.end();
  std::advance(i, -3);
  v.insert(i, v2.begin(), v2.end());
  a->rc(5);
  i = v.begin();
  std::advance(i, 2);
  v.erase(v.begin(), i);
  a->rc(-2);
}

void
test6() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  typedef ReferenceCounting::Deque<APtr> V;

  APtr a(new A);
  a->rc();

  V v(5, a);
  a->rc(5);
  V v2(v);
  a->rc(5);
  V v3;
  v3 = v;
  a->rc(5);
  V v4(5);
  V v5(std::move(v));
  v = std::move(v5);
  V v6(v.begin(), v.end());
  a->rc(5);
  v.resize(7, a);
  a->rc(2);
  v4.swap(v);
  swap(v, v4);

  v.push_front(a);
  a->rc(1);
  v.push_front(A_var(a));
  a->rc(1);
  v.push_back(a);
  a->rc(1);
  v.push_back(A_var(a));
  a->rc(1);

  v.insert(v.begin(), a);
  a->rc();
  v.insert(v.begin(), A_var(a));
  a->rc();
  v.insert(v.begin(), 2, a);
  a->rc(2);
  v.insert(v.end(), 3, a);
  a->rc(3);
  v.insert(v.begin() + 3, 25, a);
  a->rc(25);
  v.insert(v.end() - 3, v2.begin(), v2.end());
  a->rc(5);
  v.erase(v.begin(), v.begin() + 2);
  a->rc(-2);
}

void
test7() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  typedef ReferenceCounting::Map<int, APtr> V;

  APtr a(new A);
  a->rc();

  V v;
  v.insert(V::value_type(1, a));
  a->rc();
  v.insert(V::value_type(3, a));
  a->rc();
  v.insert(V::value_type(2, a));
  a->rc();
  {
    V::value_type vv(5, a);
    v.insert(vv);
  }
  a->rc();
  {
    V::value_type vv(4, A_var(a));
    v.insert(vv);
  }
  a->rc();
  V v2(v);
  a->rc(5);
  V v3;
  v3 = v;
  a->rc(5);

  V::iterator i(v.begin());
  std::advance(i, 2);
  v.insert(*i);
  a->rc(0);
  v.insert(i, *i);
  a->rc(0);
  v.insert(i, V::value_type(i->first, i->second));
  a->rc(0);
  v.erase(v.begin(), i);
  a->rc(-2);
  v[v.begin()->first] = static_cast<A*>(0);
  a->rc(-1);
  v[6] = a;
  a->rc();
}

void
test8() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";

  typedef ReferenceCounting::HashTable<
    Generics::NumericHashAdapter<int>, APtr> V;

  A_var a(new A);
  a->rc();

  V v;
  v.insert(V::value_type(1, a));
  a->rc();
  v.insert(V::value_type(3, a));
  a->rc();
  v.insert(V::value_type(2, a));
  a->rc();
  {
    V::value_type vv(5, a);
    v.insert(vv);
  }
  a->rc();
  {
    V::value_type vv(4, A_var(a));
    v.insert(vv);
  }
  a->rc();
  V v2(v);
  a->rc(5);
  V v3;
  v3 = v;
  a->rc(5);

  V::iterator i(v.begin());
  std::advance(i, 2);
  v.insert(*i);
  a->rc(0);
  v.insert(i, *i);
  a->rc(0);
  v.insert(i, V::value_type(i->first, i->second));
  a->rc(0);
  v.erase(v.begin(), i);
  a->rc(-2);
  v[v.begin()->first] = static_cast<A*>(0);
  a->rc(-1);
  v[6] = a;
  a->rc();
}

void
test9() throw (eh::Exception)
{
  std::cout << __FUNCTION__ << "\n";


  A_var sa1(nullptr);
  const A_var sa2(nullptr);
  CA_var sa3(nullptr);
  const CA_var sa4(nullptr);
  A_var sa5 = nullptr;
  const A_var sa6 = nullptr;
  CA_var sa7 = nullptr;
  const CA_var sa8 = nullptr;
  sa1 = nullptr;
  sa3 = nullptr;

  AFtr fa1(nullptr);
  const AFtr fa2(nullptr);
  CAFtr fa3(nullptr);
  const CAFtr fa4(nullptr);
  AFtr fa5 = nullptr;
  //const AFtr fa6 = nullptr;
  CAFtr fa7 = nullptr;
  const CAFtr fa8 = nullptr;
  //fa1 = nullptr;
  //fa3 = nullptr;

  APtr pa1(nullptr);
  const APtr pa2(nullptr);
  CAPtr pa3(nullptr);
  const CAPtr pa4(nullptr);
  APtr p5 = nullptr;
  //const APtr pa6 = nullptr;
  CAPtr pa7 = nullptr;
  const CAPtr pa8 = nullptr;
  pa1 = nullptr;
  pa3 = nullptr;

  CACtr ca1(nullptr);
  const CACtr ca2(nullptr);
  CACtr ca3 = nullptr;
  const CACtr ca4 = nullptr;
  ca1 = nullptr;
}

template <typename SmartPtr>
void
test_hp() throw (eh::Exception)
{
  std::cout << __PRETTY_FUNCTION__ << "\n";

  typedef ReferenceCounting::PtrHolder<SmartPtr> HPtr;

  HPtr h1;
  HPtr h2(new A);
  HPtr h3(SmartPtr(new A));
  SmartPtr s1;
  HPtr h4(s1);
  s1 = new A;
  HPtr h5(s1);
  HPtr h6(std::move(s1));
  HPtr h7;
  h7 = new A;
  h7 = new A;
  h7 = nullptr;
  SmartPtr s2 = h1.get();
  SmartPtr s3 = h2.get();
  const_cast<const HPtr&>(h1).get();
  const_cast<const HPtr&>(h2).get();
}

void
test_h() throw (eh::Exception)
{
  test_hp<A_var>();
  test_hp<APtr>();
  test_hp<CACtr>();
}

int
main()
{
  try
  {
    test0();
    test00();
    test1();
    test1_();
    test10();
    test2();
    test20();
    test3();
    test30();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test_h();
    std::cout << "Done\n";
    return 0;
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "eh::Exception: " << ex.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception" << std::endl;
  }

  return -1;
}
