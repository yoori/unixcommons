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



#ifndef CORBA_CORBACOMMONS_STATSIMPL_HPP
#define CORBA_CORBACOMMONS_STATSIMPL_HPP

#include <CORBACommons/Stats_s.hpp>

#include <Generics/Values.hpp>

#include <CORBACommons/CorbaAdapters.hpp>


namespace CORBACommons
{
  /**
   * Helper class for conversion of Values (Generics::Values, for example)
   * into CORBA Any and Sequence of Any.
   */
  class ValuesConverter : private Generics::Uncopyable
  {
  public:
    DECLARE_EXCEPTION(Exception, eh::DescriptiveException);

    /**
     * Returns a sequence of all stored values
     * @param values generalized values
     * @return CORBA sequence of stored values
     */
    template <typename Values>
    static
    StatsValueSeq*
    get_stats(Values& values)
      throw (CORBA::Exception,
        CORBACommons::ProcessStatsControl::ImplementationException);

    /**
     * Generics getter
     * @param values generalized values
     * @param key key to search
     * @return value associated with the key (if any)
     */
    template <typename Values>
    static
    CORBA::Any_ptr
    get_any(Values& values, const Generics::Values::Key& key)
      throw (eh::Exception, CORBA::Exception,
        Generics::Values::KeyNotFound);

  private:
    class AnyConverter : private Generics::Uncopyable
    {
    public:
      explicit
      AnyConverter(CORBA::Any& any) throw ();

      template <typename Type>
      void
      operator ()(const Generics::Values::Key& key, const Type& value)
        throw (eh::Exception);

      template <typename T>
      static
      void
      put_any(CORBA::Any& any, T value)
        throw (eh::Exception, CORBA::Exception);

      static
      void
      put_any(CORBA::Any& any, const Generics::Values::String& value)
        throw (eh::Exception, CORBA::Exception);

    private:
      CORBA::Any& any_;
    };

    class AllConverter : private Generics::Uncopyable
    {
    public:
      explicit
      AllConverter(StatsValueSeq& seq) throw ();

      void
      operator ()(size_t size) throw (eh::Exception);

      template <typename Type>
      void
      operator ()(const Generics::Values::Key& key, const Type& value)
        throw (eh::Exception);

    private:
      StatsValueSeq& seq_;
      size_t index_;
    };
  };

  template <typename Values>
  class ProcessStatsGen :
    public virtual POA_CORBACommons::ProcessStatsControl
  {
  public:
    explicit
    ProcessStatsGen(Values* stats) throw ();

    virtual
    StatsValueSeq*
    get_stats()
      throw (CORBA::Exception,
        CORBACommons::ProcessStatsControl::ImplementationException);

    Values&
    stats() throw ();

  protected:
    /**
     * Destructor
     */
    virtual
    ~ProcessStatsGen() throw () = default;

  private:
    ::ReferenceCounting::FixedPtr<Values> stats_;
  };

  typedef ProcessStatsGen<Generics::Values> ProcessStatsImpl;

  typedef PortableServer::Servant_var<POA_CORBACommons::ProcessStatsControl>
    POA_ProcessStatsControl_var;
}

//
// INLINES
//

namespace CORBACommons
{
  //
  // ValuesConverter::AnyConverter class
  //

  inline
  ValuesConverter::AnyConverter::AnyConverter(CORBA::Any& any) throw ()
    : any_(any)
  {
  }

  template <typename T>
  void
  ValuesConverter::AnyConverter::put_any(CORBA::Any& any, T value)
    throw (eh::Exception, CORBA::Exception)
  {
    any <<= value;
  }

  inline
  void
  ValuesConverter::AnyConverter::put_any(CORBA::Any& any,
    const Generics::Values::String& value)
    throw (eh::Exception, CORBA::Exception)
  {
    any <<= value.c_str();
  }

  template <typename Type>
  void
  ValuesConverter::AnyConverter::operator ()(const Generics::Values::Key&,
    const Type& value) throw (eh::Exception)
  {
    try
    {
      put_any(any_, value);
    }
    catch (...)
    {
      Stream::Error ostr;
      ostr << FNS << "Failed to convert value '" << value << "' to any";
      throw ValuesConverter::Exception(ostr);
    }
  }


  //
  // ValuesConverter::AllConverter class
  //

  inline
  ValuesConverter::AllConverter::AllConverter(StatsValueSeq& seq) throw ()
    : seq_(seq), index_(0)
  {
  }

  inline
  void
  ValuesConverter::AllConverter::operator ()(size_t size)
    throw (eh::Exception)
  {
    try
    {
      seq_.length(size);
    }
    catch (...)
    {
      throw ValuesConverter::Exception("");
    }
  }

  template <typename Type>
  void
  ValuesConverter::AllConverter::operator ()(
    const Generics::Values::Key& key, const Type& value)
    throw (eh::Exception)
  {
    try
    {
      StatsValue& stat = seq_[index_++];
      stat.key << key.text();
      AnyConverter::put_any(stat.value, value);
    }
    catch (...)
    {
      throw ValuesConverter::Exception("");
    }
  }


  //
  // ValuesConverter class
  //

  template <typename Values>
  StatsValueSeq*
  ValuesConverter::get_stats(Values& values)
    throw (CORBA::Exception,
      CORBACommons::ProcessStatsControl::ImplementationException)
  {
    try
    {
      StatsValueSeq_var seq(new StatsValueSeq);
      AllConverter all_converter(*seq);
      values.enumerate_all(all_converter);
      return seq._retn();
    }
    catch (...)
    {
      throw CORBACommons::ProcessStatsControl::ImplementationException();
    }
  }

  template <typename Values>
  CORBA::Any_ptr
  ValuesConverter::get_any(Values& values,
    const Generics::Values::Key& key)
    throw (eh::Exception, CORBA::Exception, Generics::Values::KeyNotFound)
  {
    {
      CORBA::Any_var any(new CORBA::Any);

      Sync::PosixGuard guard(values.mutex_);

      typename Values::Data::const_iterator itor(values.data_.find(key));
      if (itor != values.data_.end())
      {
        AnyConverter any_converter(*any);
        values.enumerate_one_(*itor, any_converter);
        return any._retn();
      }
    }

    Stream::Error ostr;
    ostr << FNS << "key  '" << key << "' is not found";
    throw Generics::Values::KeyNotFound(ostr);
  }

  //
  // ProcessStatsGen class
  //

  template <typename Values>
  ProcessStatsGen<Values>::ProcessStatsGen(Values* stats) throw ()
    : stats_(::ReferenceCounting::add_ref(stats))
  {
  }

  template <typename Values>
  Values&
  ProcessStatsGen<Values>::stats() throw ()
  {
    return *stats_;
  }

  template <typename Values>
  StatsValueSeq*
  ProcessStatsGen<Values>::get_stats()
    throw (CORBA::Exception,
      CORBACommons::ProcessStatsControl::ImplementationException)
  {
    return ValuesConverter::get_stats(*stats_);
  }
}

#endif
