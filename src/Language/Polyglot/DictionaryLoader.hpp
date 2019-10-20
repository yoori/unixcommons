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



#ifndef POLYGLOT_DICTIONARYLOADER_HPP
#define POLYGLOT_DICTIONARYLOADER_HPP

#include <string>
#include <list>

#include <String/StringManip.hpp>

#include <Language/Polyglot/IncHashTable.hpp>


namespace Polyglot
{
  struct DictionaryNode
  {
    DictionaryNode(unsigned long id_val, long freq_val)
#ifdef POLYGLOT_USE_BF
      throw (eh::Exception);
#else
      throw ();
#endif

    unsigned long id;
    long freq;

#ifdef POLYGLOT_USE_BF
    typedef Generics::GnuHashTable<
      Generics::NumericHashAdapter<unsigned long>, unsigned long>
      BiFrequencyMap;
    BiFrequencyMap bi_freq_map;
#endif
  };

  struct SuffixDictionaryNode
  {
    struct Suffix
    {
      Suffix(unsigned long length_val, long freq_val) throw ();

      unsigned long length;
      long freq;
    };

    typedef std::list<Suffix> SuffixList;
    SuffixList suffixes;
  };

  struct DictionaryNodeWithNorm
  {
    DictionaryNodeWithNorm(unsigned long id_val, long freq_val,
      const char* norm_form_val) throw (eh::Exception);

    unsigned long id;
    long freq;
    std::string norm_form;

#ifdef POLYGLOT_USE_BF
    typedef Generics::GnuHashTable<
      Generics::NumericHashAdapter<unsigned long>, unsigned long>
    BiFrequencyMap bi_freq_map;
#endif
  };

  /**X
   * DictionaryTraits
   */
  struct DictionaryTraits
  {
    DictionaryTraits() throw ();

    unsigned long count_el;
    long min_el;
    long max_el;
    long sum_el;

    unsigned long bi_count_el;
    unsigned long bi_min_el;
    unsigned long bi_max_el;
    unsigned long bi_sum_el;
  };

  /**X
   * Dictionary
   */
  class Dictionary :
    public Generics::IncHashTable<wchar_t, DictionaryNode>
  {
    friend class DictionaryLoader;

  public:
    typedef DictionaryNode Node;

    const DictionaryTraits&
    traits() const throw ();

  protected:
    DictionaryTraits traits_;
  };

  /**X
   * SuffixDictionary
   */
  class SuffixDictionary :
    public Generics::IncHashTable<wchar_t, SuffixDictionaryNode>
  {
    friend class DictionaryLoader;

  public:
    typedef SuffixDictionaryNode Node;

    const DictionaryTraits&
    traits() const throw ();

  protected:
    DictionaryTraits traits_;
  };

  /**X
   * DictionaryWithNorm
   */
  class DictionaryWithNorm :
    public Generics::IncHashTable<wchar_t, DictionaryNodeWithNorm>
  {
    friend class DictionaryLoader;

  public:
    typedef DictionaryNodeWithNorm Node;

    const DictionaryTraits&
    traits() const throw ();

  protected:
    DictionaryTraits traits_;
  };

  /**X
   * DictionaryLoader
   */
  class DictionaryLoader
  {
  public:
    DECLARE_EXCEPTION(InvalidParameter, eh::DescriptiveException);

    static
    void
    load(const char* dict_base_path, Dictionary& out_dict)
      throw (eh::Exception, InvalidParameter);

    static
    void
    load(const char* dict_base_path, DictionaryWithNorm& out_dict)
      throw (eh::Exception, InvalidParameter);

    static
    void
    load_suffixes(const char* dict_base_path, SuffixDictionary& out_dict)
      throw (eh::Exception, InvalidParameter);

    static
    void
    load(const char* dict, const char* bidict, Dictionary& out_dict)
      throw (eh::Exception, InvalidParameter);

    static
    void
    load(const char* dict, const char* bidict, DictionaryWithNorm& out_dict)
      throw (eh::Exception, InvalidParameter);

    static
    void
    load(std::istream& dict, std::istream& bidict, Dictionary& out_dict)
      throw (eh::Exception, InvalidParameter);

    static
    void
    load(std::istream& dict, std::istream& bidict,
      DictionaryWithNorm& out_dict)
      throw (eh::Exception, InvalidParameter);

    static
    void
    load_suffixes(std::istream& suffix_dict, SuffixDictionary& out_dict)
      throw (eh::Exception, InvalidParameter);
  };
}

//
// INLINES
//

namespace Polyglot
{
  //
  // DictionaryNode class
  //

  inline
  DictionaryNode::DictionaryNode(unsigned long id_val, long freq_val)
#ifdef POLYGLOT_USE_BF
    throw (eh::Exception)
#else
    throw ()
#endif
    : id(id_val), freq(freq_val)
  {
  }


  //
  // SuffixDictionaryNode::Suffix class
  //

  inline
  SuffixDictionaryNode::Suffix::Suffix(
    unsigned long length_val, long freq_val) throw ()
    : length(length_val), freq(freq_val)
  {
  }


  //
  // DictionaryNodeWithNorm class
  //

  inline
  DictionaryNodeWithNorm::DictionaryNodeWithNorm(unsigned long id_val,
    long freq_val, const char* norm_form_val) throw (eh::Exception)
    : id(id_val), freq(freq_val), norm_form(norm_form_val)
  {
  }


  //
  // DictionaryTraits class
  //

  inline
  DictionaryTraits::DictionaryTraits() throw ()
    : count_el(0), min_el(0xFFFFFFFF), max_el(0), sum_el(0),
      bi_count_el(0), bi_min_el(0xFFFFFFFF), bi_max_el(0), bi_sum_el(0)
  {
  }


  //
  // Dictionary class
  //

  inline
  const DictionaryTraits&
  Dictionary::traits() const throw ()
  {
    return traits_;
  }


  //
  // DictionaryWithNorm class
  //

  inline
  const DictionaryTraits&
  DictionaryWithNorm::traits() const throw ()
  {
    return traits_;
  }


  //
  // SuffixDictionary class
  //

  inline
  const DictionaryTraits&
  SuffixDictionary::traits() const throw ()
  {
    return traits_;
  }
}

#endif
