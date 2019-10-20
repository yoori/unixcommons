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





#ifndef GENERICS_REFLECTION_HPP
#define GENERICS_REFLECTION_HPP

#include <list>

#include <String/SubString.hpp>


namespace Generics
{
  namespace Reflection
  {
    /**
     * Additional class for every member.
     * Just registers the member, but occupies additional 8 bytes.
     */
    template <typename MembersBase, typename Aggregator,
      const char* (* const NAME)(),
      MembersBase& (Aggregator::* const MEMBER)(),
      const MembersBase& (Aggregator::* const CONST_MEMBER)() const>
    class MemberRegistrator
    {
    public:
      MemberRegistrator() throw ();

    private:
      /**
       * Adds information about the data member to
       * Aggregator::reflection_info.
       */
      static
      bool
      add_info_() throw (eh::Exception);

      static const bool INITIALIZER_;
    };

    /**
     * Child class for every member.
     * Inherits from Member which is the original member type.
     */
    template <typename Member, typename MembersBase,
      typename Aggregator, const char* (* const NAME)(),
      MembersBase& (Aggregator::* const MEMBER)(),
      const MembersBase& (Aggregator::* const CONST_MEMBER)() const>
    class Child :
      public Member,
      private MemberRegistrator<MembersBase, Aggregator,
        NAME, MEMBER, CONST_MEMBER>
    {
    public:
      Child() throw (eh::Exception);
      template <typename T>
      explicit
      Child(const T& data) throw (eh::Exception);
      template <typename T1, typename T2>
      Child(const T1& data1, const T2& data2) throw (eh::Exception);
      template <typename T1, typename T2, typename T3>
      Child(const T1& data1, const T2& data2, const T3& data3)
        throw (eh::Exception);
      template <typename T1, typename T2, typename T3, typename T4>
      Child(const T1& data1, const T2& data2, const T3& data3,
        const T4& data4) throw (eh::Exception);
      template <typename T1, typename T2, typename T3, typename T4,
        typename T5>
      Child(const T1& data1, const T2& data2, const T3& data3,
        const T4& data4, const T5& data5)
        throw (eh::Exception);
      template <typename T1, typename T2, typename T3, typename T4,
        typename T5, typename T6>
      Child(const T1& data1, const T2& data2, const T3& data3,
        const T4& data4, const T5& data5, const T6& data6)
        throw (eh::Exception);

      template <typename T>
      Child&
      operator =(const T& data) throw (eh::Exception);
    };

    /**
     * Base class for every inheritance.
     * Inherits from Base which is the original base type.
     */
    template <typename Inheritance, typename Aggregator>
    class Base : public Inheritance
    {
    public:
      Base() throw (eh::Exception);

    private:
      /**
       * Adds information about all Inheritance data members to
       * Aggregator::reflection_info.
       */
      static
      bool
      add_info_() throw (eh::Exception);

      static const bool INITIALIZER_;
    };
  }
}

//
// Implementation
//

namespace Generics
{
  namespace Reflection
  {
    //
    // MemberRegistration class
    //

    template <typename MembersBase, typename Aggregator,
      const char* (* const NAME)(),
      MembersBase& (Aggregator::* const MEMBER)(),
      const MembersBase& (Aggregator::* const CONST_MEMBER)() const>
    MemberRegistrator<MembersBase, Aggregator, NAME, MEMBER, CONST_MEMBER>::
      MemberRegistrator() throw ()
    {
      (void)INITIALIZER_;
    }

    template <typename MembersBase, typename Aggregator,
      const char* (* const NAME)(),
      MembersBase& (Aggregator::* const MEMBER)(),
      const MembersBase& (Aggregator::* const CONST_MEMBER)() const>
    bool
    MemberRegistrator<MembersBase, Aggregator, NAME, MEMBER, CONST_MEMBER>::
      add_info_() throw (eh::Exception)
    {
      typename Aggregator::ReflectionMemberInfo info;
      info.name = String::SubString(NAME());
      info.member = MEMBER;
      info.const_member = CONST_MEMBER;
      const_cast<typename Aggregator::ReflectionMembersInfo&>(
        Aggregator::reflection_info()).push_back(info);
      return true;
    }

    template <typename MembersBase, typename Aggregator,
      const char* (* const NAME)(),
      MembersBase& (Aggregator::* const MEMBER)(),
      const MembersBase& (Aggregator::* const CONST_MEMBER)() const>
    const bool MemberRegistrator<MembersBase, Aggregator,
      NAME, MEMBER, CONST_MEMBER>::INITIALIZER_ =
      MemberRegistrator<MembersBase, Aggregator,
        NAME, MEMBER, CONST_MEMBER>::add_info_();

    //
    // Child class
    //

    template <typename Member, typename MembersBase,
      typename Aggregator, const char* (* const NAME)(),
      MembersBase& (Aggregator::* const MEMBER)(),
      const MembersBase& (Aggregator::* const CONST_MEMBER)() const>
    Child<Member, MembersBase, Aggregator, NAME, MEMBER, CONST_MEMBER>::
      Child() throw (eh::Exception)
    {
    }

    template <typename Member, typename MembersBase,
      typename Aggregator, const char* (* const NAME)(),
      MembersBase& (Aggregator::* const MEMBER)(),
      const MembersBase& (Aggregator::* const CONST_MEMBER)() const>
    template <typename T>
    Child<Member, MembersBase, Aggregator, NAME, MEMBER, CONST_MEMBER>::
      Child(const T& data) throw (eh::Exception)
      : Member(data)
    {
    }

    template <typename Member, typename MembersBase,
      typename Aggregator, const char* (* const NAME)(),
      MembersBase& (Aggregator::* const MEMBER)(),
      const MembersBase& (Aggregator::* const CONST_MEMBER)() const>
    template <typename T1, typename T2>
    Child<Member, MembersBase, Aggregator, NAME, MEMBER, CONST_MEMBER>::
      Child(const T1& data1, const T2& data2) throw (eh::Exception)
      : Member(data1, data2)
    {
    }

    template <typename Member, typename MembersBase,
      typename Aggregator, const char* (* const NAME)(),
      MembersBase& (Aggregator::* const MEMBER)(),
      const MembersBase& (Aggregator::* const CONST_MEMBER)() const>
    template <typename T1, typename T2, typename T3>
    Child<Member, MembersBase, Aggregator, NAME, MEMBER, CONST_MEMBER>::
      Child(const T1& data1, const T2& data2, const T3& data3)
      throw (eh::Exception)
      : Member(data1, data2, data3)
    {
    }

    template <typename Member, typename MembersBase,
      typename Aggregator, const char* (* const NAME)(),
      MembersBase& (Aggregator::* const MEMBER)(),
      const MembersBase& (Aggregator::* const CONST_MEMBER)() const>
    template <typename T1, typename T2, typename T3, typename T4>
    Child<Member, MembersBase, Aggregator, NAME, MEMBER, CONST_MEMBER>::
      Child(const T1& data1, const T2& data2, const T3& data3,
        const T4& data4) throw (eh::Exception)
      : Member(data1, data2, data3, data4)
    {
    }

    template <typename Member, typename MembersBase,
      typename Aggregator, const char* (* const NAME)(),
      MembersBase& (Aggregator::* const MEMBER)(),
      const MembersBase& (Aggregator::* const CONST_MEMBER)() const>
    template <typename T1, typename T2, typename T3, typename T4,
      typename T5>
    Child<Member, MembersBase, Aggregator, NAME, MEMBER, CONST_MEMBER>::
      Child(const T1& data1, const T2& data2, const T3& data3,
      const T4& data4, const T5& data5) throw (eh::Exception)
      : Member(data1, data2, data3, data4, data5)
    {
    }

    template <typename Member, typename MembersBase,
      typename Aggregator, const char* (* const NAME)(),
      MembersBase& (Aggregator::* const MEMBER)(),
      const MembersBase& (Aggregator::* const CONST_MEMBER)() const>
    template <typename T1, typename T2, typename T3, typename T4,
      typename T5, typename T6>
    Child<Member, MembersBase, Aggregator, NAME, MEMBER, CONST_MEMBER>::
      Child(const T1& data1, const T2& data2, const T3& data3, const T4& data4,
        const T5& data5, const T6& data6) throw (eh::Exception)
      : Member(data1, data2, data3, data4, data5, data6)
    {
    }

    template <typename Member, typename MembersBase,
      typename Aggregator, const char* (* const NAME)(),
      MembersBase& (Aggregator::* const MEMBER)(),
      const MembersBase& (Aggregator::* const CONST_MEMBER)() const>
    template <typename T>
    Child<Member, MembersBase, Aggregator, NAME, MEMBER, CONST_MEMBER>&
    Child<Member, MembersBase, Aggregator, NAME, MEMBER, CONST_MEMBER>::
      operator =(const T& other) throw (eh::Exception)
    {
      Member::operator =(other);
      return *this;
    }


    //
    // Base class
    //

    template <typename Inheritance, typename Aggregator>
    Base<Inheritance, Aggregator>::Base() throw (eh::Exception)
    {
      (void)INITIALIZER_;
    }

    template <typename Inheritance, typename Aggregator>
    bool
    Base<Inheritance, Aggregator>::add_info_() throw (eh::Exception)
    {
      const typename Inheritance::ReflectionMembersInfo&
        inheritance_reflection_info(Inheritance::reflection_info());
      if (inheritance_reflection_info.empty())
      {
        abort();
      }
      typename Aggregator::ReflectionMembersInfo&
        aggregator_reflection_info(
          const_cast<typename Aggregator::ReflectionMembersInfo&>(
            Aggregator::reflection_info()));
      for (typename Inheritance::ReflectionMembersInfo::const_iterator itor(
        inheritance_reflection_info.begin());
        itor != inheritance_reflection_info.end(); ++itor)
      {
        typename Aggregator::ReflectionMemberInfo info;
        info.name = itor->name;
        info.member = itor->member;
        info.const_member = itor->const_member;
        aggregator_reflection_info.push_back(info);
      }
      return true;
    }

    template <typename Inheritance, typename Aggregator>
    const bool Base<Inheritance, Aggregator>::INITIALIZER_ =
      Base<Inheritance, Aggregator>::add_info_();
  }
}

//
// Reflection macros
//

/**
 * Per-class initializer
 * Must be used inside class definition firstly.
 * @param Aggregator the name of the class-aggregator
 * @param MembersBase the common base for all reflected members
 */
#define REFLECTION_INIT(Aggregator, MembersBase) \
  typedef Aggregator ReflectionAggregator; \
  typedef MembersBase ReflectionMembersBase; \
  \
  struct ReflectionMemberInfo \
  { \
    String::SubString name; \
    ReflectionMembersBase& (ReflectionAggregator::*member)(); \
    const ReflectionMembersBase& \
      (ReflectionAggregator::*const_member)() const; \
  }; \
  \
  typedef std::list<ReflectionMemberInfo> ReflectionMembersInfo; \
  \
  static \
  const ReflectionMembersInfo& \
  reflection_info() throw (eh::Exception) \
  { \
    static ReflectionMembersInfo reflection_info; \
    return reflection_info; \
  } \
  \
  template <typename MembersBaseType, typename AggregatorType, \
    const char* (* const NAME)(), \
    MembersBaseType& (AggregatorType::* const MEMBER)(), \
    const MembersBaseType& (AggregatorType::* const CONST_MEMBER)() const> \
  friend class ::Generics::Reflection::MemberRegistrator; \
  \
  template <typename InheritanceType, typename AggregatorType> \
  friend class ::Generics::Reflection::Base;

/**
 * Copies all inherited members information
 * XXX: Does not work yet.
 * @param Inheritance class to copy from
 * @param name unique name to use as identifier
 */
#define REFLECTION_INHERITS_NAME(Inheritance, name) \
  ::Generics::Reflection::Base<Inheritance, ReflectionAggregator> \
    reflection_inherits_##name;

/**
 * Copies all inherited members information
 * @param Inheritance class to copy from
 */
#define REFLECTION_INHERITS(Inheritance) \
  REFLECTION_INHERITS_NAME(Inheritance, Inheritance)

#define REFLECTION_MEMBER_COMMON(Member, member_name, string_name) \
  static \
  const char* \
  reflection_##member_name##_name() throw () \
  { \
    return string_name; \
  } \
  ReflectionMembersBase& \
  reflection_##member_name##_member() throw ()\
  { \
    return member_name; \
  }\
  const ReflectionMembersBase& \
  reflection_##member_name##_const_member() const throw () \
  { \
    return member_name; \
  } \

/**
 * Member definer and registrator (with child type)
 * Defines a new member of the specified type with a custom name
 * @param Member type of the member.
 * @param member_name name of the member.
 * @param string_name string name of the member.
 */
#define REFLECTION_MEMBER1_NAME(Member, member_name, string_name) \
  REFLECTION_MEMBER_COMMON(Member, member_name, string_name) \
  ::Generics::Reflection::Child<Member, ReflectionMembersBase, \
    ReflectionAggregator, \
    &ReflectionAggregator::reflection_##member_name##_name, \
    &ReflectionAggregator::reflection_##member_name##_member, \
    &ReflectionAggregator::reflection_##member_name##_const_member> \
    member_name;

/**
 * Defines a new member of the specified type with a custom name
 * @param Member type of the member.
 * @param member_name name of the member.
 * @param string_name string name of the member.
 */
#define REFLECTION_MEMBER2_NAME(Member, member_name, string_name) \
  REFLECTION_MEMBER_COMMON(Member, member_name, string_name) \
  Member member_name; \
  ::Generics::Reflection::MemberRegistrator<ReflectionMembersBase, \
    ReflectionAggregator, \
    &ReflectionAggregator::reflection_##member_name##_name, \
    &ReflectionAggregator::reflection_##member_name##_member, \
    &ReflectionAggregator::reflection_##member_name##_const_member> \
    reflection_##member_name##_registrator;

/**
 * Member definer and registrator (with child type)
 * Defines a new member of the specified type
 * @param Member type of the member.
 * @param member_name name of the member. Will be used as a string
 * name of the member also.
 */
#define REFLECTION_MEMBER1(Member, member_name) \
  REFLECTION_MEMBER1_NAME(Member, member_name, #member_name)

/**
 * Member definer and registrator (with additional member)
 * Defines a new member of the specified type
 * @param Member type of the member.
 * @param member_name name of the member. Will be used as a string
 * name of the member also.
 */
#define REFLECTION_MEMBER2(Member, member_name) \
  REFLECTION_MEMBER2_NAME(Member, member_name, #member_name)

#endif
