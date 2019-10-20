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



#ifndef POLYGLOT_TOKENIZER_HPP
#define POLYGLOT_TOKENIZER_HPP

#include <vector>
#include <cassert>

#include <Language/Polyglot/DictionaryLoader.hpp>

//#define P_DEBUG

namespace Polyglot
{
  bool
  is_asian_char(wchar_t ch) throw ();

  template <typename DictionaryNodeType, typename SuffixDictionaryNodeType>
  struct SumWeightCollector
  {
    typedef long long WeightType;

    SumWeightCollector(
      const DictionaryTraits& dict_traits,
      const DictionaryTraits& suffix_dict_traits) throw ();

    WeightType
    start() const throw ();

    WeightType
    unknown_symbol(WeightType in) const throw ();

    WeightType
    unknown_word_start() const throw ();

    WeightType
    unknown_word_start(WeightType in) const throw ();

    WeightType
    collect(WeightType in, const DictionaryNodeType& in2) const throw ();

    WeightType
    collect(WeightType in,
      const typename SuffixDictionaryNodeType::Suffix& in2) const throw ();

    WeightType
    collect(WeightType in, WeightType in2) const throw ();

  protected:
    long long min_weight_;
  };

  struct NullNormalizeStrategy
  {
    void
    operator()(const wchar_t* begin, const wchar_t* end,
      const DictionaryNode* /*node*/, std::string& out) const
      throw (eh::Exception);
  };

  struct WordNormalizeStrategy
  {
    void
    operator()(const wchar_t* begin, const wchar_t* end,
      const DictionaryNodeWithNorm* node, std::string& out) const
      throw (eh::Exception);
  };

  /**X
   * GenericNGramTokenizer
   * WeightCollectorType must implement next methods:
   *   WeightType start():
   *     start weight will be used for segment continuous text block
   *   WeightType unknown_symbol(WeightType)
   *     method will be used for transform weight if found unknown symbol
   *     it will be called for each symbol of continuous block without any
   *     segmentation.
   *   WeightType unknown_word_start()
   *     method will be used for transform weight if found unknown word
   *     it will be used one time for continuous block without any
   *     segmentation.
   *   WeightType collect(WeightType, const DictionaryNodeType& dict_node)
   *     method will be used for transform weight if found word (presented
   *     in dict_node)
   *   WeightType collect(WeightType in, WeightType in2)
   *     method will be used for combine two weights for independed blocks.
   */
  template <
    typename WeightCollectorType,
    typename DictionaryType,
    typename SuffixDictionaryType,
    typename NormalizeStrategyType>
  class GenericNGramTokenizer
  {
  public:
    typedef typename DictionaryType::Node DictionaryNode;
    typedef typename WeightCollectorType::WeightType WeightType;

    struct TokenizePoint
    {
      bool word_begin;
      typename WeightCollectorType::WeightType weight;
      std::wstring::const_iterator sep_pos;
    };

    struct BiTokenizePoint
    {
      struct Variant
      {
        Variant(const DictionaryNode* node_val,
          const typename WeightCollectorType::WeightType& weight_val,
          std::wstring::const_iterator sep_pos_val,
          const DictionaryNode* next_node_val) throw ();

        const DictionaryNode* node;
        typename WeightCollectorType::WeightType weight;
        std::wstring::const_iterator sep_pos;
        const DictionaryNode* next_node;
      };

      struct SuffixVariant
      {
        SuffixVariant(const SuffixDictionaryNode::Suffix* node_val,
          const typename WeightCollectorType::WeightType& weight_val,
          std::wstring::const_iterator sep_pos_val) throw ();

        const SuffixDictionaryNode::Suffix* node;
        typename WeightCollectorType::WeightType weight;
        std::wstring::const_iterator sep_pos;
      };

      typedef std::list<Variant> VariantList;
      typedef std::list<SuffixVariant> SuffixVariantList;

      /* weight that equal max of (unk_weight, variants, suffixes) */
      typename WeightCollectorType::WeightType weight;

      typename WeightCollectorType::WeightType unk_weight;
      VariantList variants;
      SuffixVariantList suffix_variants;

      /* only for debug */
      SuffixVariantList erased_suffix_variants;
    };

    typedef std::list<std::string> Result;

    GenericNGramTokenizer(const DictionaryType& dict,
      const SuffixDictionaryType& suffix_dict) throw ();

    void
    print_bi_tokenize_seq(const std::wstring& orig,
      std::vector<BiTokenizePoint>& vec, std::ostream& ostr) const
      throw (eh::Exception);

    void
    bi_tokenize(const std::wstring& word,
      std::vector<BiTokenizePoint>& vec) const
      throw (eh::Exception);

    void
    bi_tokenize_reconstruct(const std::wstring& original_phrase,
      const std::vector<BiTokenizePoint>& vec, Result& res) const
      throw (eh::Exception);

    void
    segment(const String::SubString& in, Result& res) const
      throw (eh::Exception);

    void
    put_spaces(std::string& result, const String::SubString& in) const
      throw (eh::Exception);

  protected:
    const DictionaryType& dict_;
    const SuffixDictionaryType& suffix_dict_;
    const WeightCollectorType coll_;
  };

  typedef
    GenericNGramTokenizer<
      SumWeightCollector<Dictionary::Node, SuffixDictionary::Node>,
      Dictionary,
      SuffixDictionary,
      NullNormalizeStrategy>
    Tokenizer;

  typedef
    GenericNGramTokenizer<
      SumWeightCollector<DictionaryWithNorm::Node, SuffixDictionary::Node>,
      DictionaryWithNorm,
      SuffixDictionary,
      WordNormalizeStrategy>
    NormalizeTokenizer;
}

//
// INLINES
//

namespace Polyglot
{
  inline
  bool
  is_asian_char(wchar_t ch) throw ()
  {
    return
      (ch >= 0x1100 && ch < 0x11FA) ||
      (ch >= 0x2E80 && ch < 0x2EF3) ||
      (ch >= 0x2F00 && ch < 0x2FD6) ||
      (ch >= 0x2FF0 && ch < 0x2FFC) ||
      (ch >= 0x3041 && ch < 0x3100) ||
      (ch >= 0x3105 && ch < 0x312E) ||
      (ch >= 0x3131 && ch < 0x318F) ||
      (ch >= 0x3190 && ch < 0x31B8) ||
      (ch >= 0x31C0 && ch < 0x31E4) ||
      (ch >= 0x31F0 && ch < 0x4DB6) ||
      (ch >= 0x4E00 && ch < 0x9FBC) ||
      (ch >= 0xAC00 && ch < 0xD7A4) ||
      (ch >= 0xF900 && ch < 0xFADA) ||
      (ch >= 0xFE10 && ch < 0xFE1A) ||
      (ch >= 0xFE30 && ch < 0xFE50) ||
      (ch >= 0x2F800 && ch < 0x2FA1E) ||
      (ch >= 0x20000 && ch < 0x2A6D6)
#ifdef P_DEBUG
      ||
      (ch >= 'A' && ch <= 'Z') ||
      (ch >= 'a' && ch <= 'z')
#endif
      ;
  }


  //
  // SumWeightCollector class
  //

  template <typename DictionaryNodeType, typename SuffixDictionaryNodeType>
  SumWeightCollector<DictionaryNodeType, SuffixDictionaryNodeType>::
    SumWeightCollector(const DictionaryTraits& dict_traits,
      const DictionaryTraits& /*suffix_dict_traits*/) throw ()
    : min_weight_(dict_traits.min_el)
  {
  }

  template <typename DictionaryNodeType, typename SuffixDictionaryNodeType>
  typename SumWeightCollector<DictionaryNodeType,
    SuffixDictionaryNodeType>::WeightType
  SumWeightCollector<DictionaryNodeType, SuffixDictionaryNodeType>::
    start() const throw ()
  {
    return 0;
  }

  template <typename DictionaryNodeType, typename SuffixDictionaryNodeType>
  typename SumWeightCollector<DictionaryNodeType,
    SuffixDictionaryNodeType>::WeightType
  SumWeightCollector<DictionaryNodeType, SuffixDictionaryNodeType>::
    unknown_symbol(WeightType in) const throw ()
  {
    return 1LL * min_weight_ + 1 + in;
  }

  template <typename DictionaryNodeType, typename SuffixDictionaryNodeType>
  typename SumWeightCollector<DictionaryNodeType,
    SuffixDictionaryNodeType>::WeightType
  SumWeightCollector<DictionaryNodeType, SuffixDictionaryNodeType>::
    unknown_word_start() const throw ()
  {
//  return 4LL * min_weight_;
    return 1LL * min_weight_ + 1;
  }

  template <typename DictionaryNodeType, typename SuffixDictionaryNodeType>
  typename SumWeightCollector<DictionaryNodeType,
    SuffixDictionaryNodeType>::WeightType
  SumWeightCollector<DictionaryNodeType, SuffixDictionaryNodeType>::
    unknown_word_start(WeightType in) const throw ()
  {
//  return 4LL * min_weight_ + in;
    return 1LL * min_weight_ + 1 + in;
  }

  template <typename DictionaryNodeType, typename SuffixDictionaryNodeType>
  typename SumWeightCollector<DictionaryNodeType,
    SuffixDictionaryNodeType>::WeightType
  SumWeightCollector<DictionaryNodeType, SuffixDictionaryNodeType>::
    collect(WeightType in, const DictionaryNodeType& in2) const throw ()
  {
    return in + in2.freq;
  }

  template <typename DictionaryNodeType, typename SuffixDictionaryNodeType>
  typename SumWeightCollector<DictionaryNodeType,
    SuffixDictionaryNodeType>::WeightType
  SumWeightCollector<DictionaryNodeType, SuffixDictionaryNodeType>::
    collect(WeightType in,
      const typename SuffixDictionaryNodeType::Suffix& in2) const throw ()
  {
    return in + in2.freq;
  }

  template <typename DictionaryNodeType, typename SuffixDictionaryNodeType>
  typename SumWeightCollector<DictionaryNodeType,
    SuffixDictionaryNodeType>::WeightType
  SumWeightCollector<DictionaryNodeType, SuffixDictionaryNodeType>::
    collect(WeightType in, WeightType in2) const throw ()
  {
    return in + in2;
  }


  //
  // NullNormalizeStrategy class
  //

  inline
  void
  NullNormalizeStrategy::operator()(const wchar_t* begin,
    const wchar_t* end, const DictionaryNode* /*node*/,
    std::string& out) const throw (eh::Exception)
  {
    String::StringManip::wchar_to_utf8(String::WSubString(begin, end), out);
  }


  //
  // WordNormalizeStrategy class
  //

  inline
  void
  WordNormalizeStrategy::operator()(const wchar_t* begin,
    const wchar_t* end, const DictionaryNodeWithNorm* node,
    std::string& out) const throw (eh::Exception)
  {
    if (node)
    {
      out = node->norm_form;
    }
    else
    {
      String::StringManip::wchar_to_utf8(
        String::WSubString(begin, end), out);
    }
  }


  //
  // GenericNGramTokenizer::BiTokenizePoint::Variant class
  //

  template <typename WeightCollectorType, typename DictionaryType,
    typename SuffixDictionaryType, typename NormalizeStrategyType>
  GenericNGramTokenizer<WeightCollectorType, DictionaryType,
    SuffixDictionaryType, NormalizeStrategyType>::BiTokenizePoint::Variant::
      Variant(const DictionaryNode* node_val,
        const typename WeightCollectorType::WeightType& weight_val,
        std::wstring::const_iterator sep_pos_val,
        const DictionaryNode* next_node_val) throw ()
    : node(node_val), weight(weight_val), sep_pos(sep_pos_val),
      next_node(next_node_val)
  {
  }


  //
  // GenericNGramTokenizer::BiTokenizePoint::SuffixVariant class
  //

  template <typename WeightCollectorType, typename DictionaryType,
    typename SuffixDictionaryType, typename NormalizeStrategyType>
  GenericNGramTokenizer<WeightCollectorType, DictionaryType,
    SuffixDictionaryType, NormalizeStrategyType>::BiTokenizePoint::
      SuffixVariant::SuffixVariant(
        const SuffixDictionaryNode::Suffix* node_val,
        const typename WeightCollectorType::WeightType& weight_val,
        std::wstring::const_iterator sep_pos_val) throw ()
    : node(node_val), weight(weight_val), sep_pos(sep_pos_val)
  {
  }


  //
  // GenericNGramTokenizer class
  //

  template <typename WeightCollectorType, typename DictionaryType,
    typename SuffixDictionaryType, typename NormalizeStrategyType>
  GenericNGramTokenizer<WeightCollectorType, DictionaryType,
    SuffixDictionaryType, NormalizeStrategyType>::
    GenericNGramTokenizer(const DictionaryType& dict,
      const SuffixDictionaryType& suffix_dict) throw ()
    : dict_(dict), suffix_dict_(suffix_dict),
      coll_(dict.traits(), suffix_dict.traits())
  {
#ifdef _DEBUG_PRINT
    std::cerr << "DEBUG POLYGLOT: dictionary traits: min = " <<
      dict.traits().min_el << ", max = " << dict.traits().max_el <<
      ", suffix-min = " << suffix_dict.traits().min_el <<
      ", suffix-max = " << suffix_dict.traits().max_el << std::endl;
#endif
  }

  template <typename WeightCollectorType, typename DictionaryType,
    typename SuffixDictionaryType, typename NormalizeStrategyType>
  void
  GenericNGramTokenizer<WeightCollectorType, DictionaryType,
    SuffixDictionaryType, NormalizeStrategyType>::
      print_bi_tokenize_seq(const std::wstring& orig,
        std::vector<BiTokenizePoint>& vec, std::ostream& ostr) const
        throw (eh::Exception)
  {
    unsigned long i = 0;
    for (typename std::vector<BiTokenizePoint>::const_iterator it =
      vec.begin(); it != vec.end(); ++it, ++i)
    {
      ostr << "+ POINT #" << i << " (" << vec[i].weight << ")" <<
        std::endl << "  unk-variant: weight = " << vec[i].unk_weight <<
        std::endl << "  variants: ";

      for (typename BiTokenizePoint::VariantList::const_iterator sit =
        it->variants.begin(); sit != it->variants.end(); ++sit)
      {
        ostr << "( id = " << sit->node->id << ", word-weight = " <<
          sit->node->freq << ", weight = " << sit->weight << ", len = " <<
          (sit->sep_pos - (orig.begin() + i)) << ", next-id = " <<
          (sit->next_node ? sit->next_node->id : 0) << ") ";
      }

      ostr << std::endl << "  suffix_variants: ";

      for (typename BiTokenizePoint::SuffixVariantList::const_iterator sit =
        it->suffix_variants.begin();
        sit != it->suffix_variants.end(); ++sit)
      {
        ostr << "( len = " << sit->node->length << ", suff-weight = " <<
          sit->node->freq << ", weight = " << sit->weight <<
          ", unknown-word-len = " << (sit->sep_pos - (orig.begin() + i)) <<
          ") ";
      }

#ifdef _DEBUG
      ostr << std::endl << "  erased_suffix_variants: ";

      for (typename BiTokenizePoint::SuffixVariantList::const_iterator sit =
        it->erased_suffix_variants.begin();
        sit != it->erased_suffix_variants.end(); ++sit)
      {
        ostr << "( len = " << sit->node->length << ", weight = " <<
          sit->weight << ", unknown-word-len = " <<
          (sit->sep_pos - (orig.begin() + i)) << ") ";
      }
#endif

      ostr << std::endl;
    }
  }

  template <typename WeightCollectorType, typename DictionaryType,
    typename SuffixDictionaryType, typename NormalizeStrategyType>
  void
  GenericNGramTokenizer<WeightCollectorType, DictionaryType,
    SuffixDictionaryType, NormalizeStrategyType>::
      bi_tokenize(const std::wstring& word,
      std::vector<BiTokenizePoint>& vec) const
      throw (eh::Exception)
  {
    const DictionaryType& dict = dict_;
    const SuffixDictionaryType& suffix_dict = suffix_dict_;

    assert(word.length() > 0);

    long len = word.length() - 1;
    std::wstring::const_iterator next_sep_pos = word.end();

    vec.resize(len + 2);

    vec[len + 1].unk_weight = coll_.unknown_word_start();
    vec[len + 1].weight = coll_.start();

    for (long i = 0; i <= len; ++i)
    {
      vec[i].unk_weight = coll_.unknown_word_start();
      vec[i].weight = vec[i].unk_weight;
    }

    for (long word_i = len; word_i >= 0; --word_i)
    {
      /* check separators */
      if (!is_asian_char(word[word_i]))
      {
        next_sep_pos = word.begin() + word_i;
      }

      /* calculate unknown weight */
      if (vec[word_i + 1].variants.empty() && word_i != len)
      {
        vec[word_i].unk_weight =
          coll_.unknown_symbol(vec[word_i + 1].weight);
      }
      else
      {
        vec[word_i].unk_weight =
          coll_.unknown_word_start(vec[word_i + 1].weight);
      }

      /* clear separated suffixes and recalculate point weight */
      typename WeightCollectorType::WeightType new_weight =
        vec[word_i].unk_weight;

      for (typename BiTokenizePoint::SuffixVariantList::iterator it =
        vec[word_i].suffix_variants.begin();
        it != vec[word_i].suffix_variants.end(); )
      {
        if (it->sep_pos > next_sep_pos)
        {
#ifdef _DEBUG
          vec[word_i].erased_suffix_variants.push_back(*it);
#endif
          vec[word_i].suffix_variants.erase(it++);
        }
        else
        {
          new_weight = std::max(new_weight, it->weight);
          ++it;
        }
      }

      vec[word_i].weight = new_weight;

      /* existing words search */
      typename DictionaryType::ConstFinder dict_it = dict.finder();

      for (long word_j = word_i; word_j <= len; ++word_j)
      {
        bool cont = dict_it.find(word[word_j]);

        if (dict_it.element())
        {
          const DictionaryNode* next_node = 0;
          const DictionaryNode& node = *dict_it.element();
          long next_word_pos = word_j + 1;
          long next_i = word_j + 1;

          while (vec[next_word_pos].variants.empty() &&
            next_word_pos <= len)
          {
            ++next_word_pos;
          }

          typename WeightCollectorType::WeightType weight =
            coll_.collect(vec[next_i].weight, node);

          vec[word_i].weight = std::max(vec[word_i].weight, weight);

#ifdef POLYGLOT_USE_BF
          for (typename BiTokenizePoint::VariantList::const_iterator it =
            vec[next_word_pos].variants.begin();
            it != vec[next_word_pos].variants.end(); ++it)
          {
            typename DictionaryNode::BiFrequencyMap::const_iterator sit =
              node.bi_freq_map.find(it->node->id);

            if (sit != node.bi_freq_map.end())
            {
              // add bi-gram weight
              typename WeightCollectorType::WeightType sw =
                coll_.collect(weight, sit->second);

              if (weight < sw)
              {
                next_node = it->node;
                weight = sw;
              }
            }
          }
#endif
          vec[word_i].variants.emplace_back(
            &node, weight, word.begin() + word_j + 1, next_node);
        }

        if (!cont)
        {
          break;
        }
      } /* word_j */

      /* suffix search */
      typename SuffixDictionaryType::ConstFinder
        suffix_dict_it = suffix_dict.finder();

      for(long word_j = word_i; word_j <= len; ++word_j)
      {
        bool cont = suffix_dict_it.find(word[word_j]);

        if (suffix_dict_it.element())
        {
          const SuffixDictionaryNode& node = *suffix_dict_it.element();

          for (SuffixDictionaryNode::SuffixList::const_iterator s_it =
            node.suffixes.begin(); s_it != node.suffixes.end(); ++s_it)
          {
            long len = static_cast<long>(s_it->length);
            if (word_j + 1 >= len)
            {
              typename WeightCollectorType::WeightType weight =
                coll_.collect(vec[word_j + 1].weight, s_it->freq);

              vec[word_j + 1 - len].weight =
                std::max(vec[word_j + 1 - len].weight, weight);

              vec[word_j + 1 - len].suffix_variants.emplace_back(
                  &(*s_it), weight, word.begin() + word_j + 1);
            }
          }
        }

        if (!cont)
        {
          break;
        }
      } /* word_j */
    } /* word_i */
  }

  template <typename WeightCollectorType, typename DictionaryType,
    typename SuffixDictionaryType, typename NormalizeStrategyType>
  void
  GenericNGramTokenizer<WeightCollectorType, DictionaryType,
    SuffixDictionaryType, NormalizeStrategyType>::
      bi_tokenize_reconstruct(const std::wstring& original_phrase,
        const std::vector<BiTokenizePoint>& vec, Result& res) const
        throw (eh::Exception)
  {
    NormalizeStrategyType norm_strategy;
    int unknown_seq_i = -1;
    const DictionaryNode* next_node = 0;
    unsigned long len = original_phrase.size() - 1;

    unsigned long word_i = 0;

    while (word_i <= len)
    {
      if (vec[word_i].unk_weight == vec[word_i].weight)
      {
#if 0
        if (vec[word_i].variants.empty() &&
          vec[word_i].suffix_variants.empty())
#endif
        if (unknown_seq_i == -1)
        {
          unknown_seq_i = word_i;
        }
        ++word_i;
      }
      else
      {
        if (unknown_seq_i != -1)
        {
          std::string word_utf8;
          std::wstring word(original_phrase.begin() + unknown_seq_i,
            original_phrase.begin() + word_i);
          String::StringManip::wchar_to_utf8(word, word_utf8);
          res.push_back(std::move(word_utf8));
          unknown_seq_i = -1;
        }

        /* check normal word variants */
        typename BiTokenizePoint::VariantList::const_iterator max_it =
          vec[word_i].variants.begin();

        if (next_node == 0)
        {
          for (typename BiTokenizePoint::VariantList::const_iterator it =
            ++vec[word_i].variants.begin();
            it != vec[word_i].variants.end(); ++it)
          {
            if (max_it->weight < it->weight)
            {
              max_it = it;
            }
          }
        }
        else
        {
          for (typename BiTokenizePoint::VariantList::const_iterator it =
            ++vec[word_i].variants.begin();
            it != vec[word_i].variants.end(); ++it)
          {
            if (it->node == next_node)
            {
              max_it = it;
            }
          }
        }

        /* check suffixes */
        bool suffix_selected = false;
        typename BiTokenizePoint::SuffixVariantList::const_iterator
          max_suffix_it = vec[word_i].suffix_variants.begin();

        if (max_it != vec[word_i].variants.end())
        {
          WeightType cmp_weight = max_it->weight;

          for (typename BiTokenizePoint::SuffixVariantList::const_iterator
            it = vec[word_i].suffix_variants.begin();
            it != vec[word_i].suffix_variants.end(); ++it)
          {
            if (cmp_weight < it->weight)
            {
              max_suffix_it = it;
              cmp_weight = max_suffix_it->weight;
              suffix_selected = true;
            }
          }
        }
        else
        {
          suffix_selected = true;

          for (typename BiTokenizePoint::SuffixVariantList::const_iterator
            it = ++vec[word_i].suffix_variants.begin();
            it != vec[word_i].suffix_variants.end(); ++it)
          {
            if (max_suffix_it->weight < it->weight)
            {
              max_suffix_it = it;
            }
          }
        }

        if (!suffix_selected)
        {
          unsigned long word_end =
            max_it->sep_pos - original_phrase.begin();

          std::string word_utf8;

          norm_strategy(original_phrase.data() + word_i,
            original_phrase.data() + word_end, max_it->node, word_utf8);

          if (!word_utf8.empty())
          {
            res.push_back(std::move(word_utf8));
          }

          word_i = word_end;
          next_node = max_it->next_node;
        }
        else
        {
          unsigned long word_end =
            max_suffix_it->sep_pos - original_phrase.begin();

          std::string word_utf8;

          norm_strategy(original_phrase.data() + word_i,
            original_phrase.data() + word_end, 0, word_utf8);

          if (!word_utf8.empty())
          {
            res.push_back(std::move(word_utf8));
          }

          word_i = word_end;
        }
      }
    } /* word_i <= len */

    if (unknown_seq_i != -1)
    {
      std::string word_utf8;
      std::wstring word(original_phrase.begin() + unknown_seq_i,
        original_phrase.end());
      String::StringManip::wchar_to_utf8(word, word_utf8);
      res.push_back(std::move(word_utf8));
    }
  }

  template <typename WeightCollectorType, typename DictionaryType,
    typename SuffixDictionaryType, typename NormalizeStrategyType>
  void
  GenericNGramTokenizer<WeightCollectorType, DictionaryType,
    SuffixDictionaryType, NormalizeStrategyType>::
      segment(const String::SubString& in, Result& res) const
      throw (eh::Exception)
  {
    Generics::ArrayWChar w_in = String::StringManip::utf8_to_wchar(in);

    std::wstring wstr(w_in.get());
    std::vector<BiTokenizePoint> sres;

    if (!wstr.empty())
    {
      bi_tokenize(wstr, sres);
//    print_bi_tokenize_seq(wstr, sres, std::cout);
      bi_tokenize_reconstruct(wstr, sres, res);
    }
  }

  template <typename WeightCollectorType, typename DictionaryType,
    typename SuffixDictionaryType, typename NormalizeStrategyType>
  void
  GenericNGramTokenizer<WeightCollectorType, DictionaryType,
    SuffixDictionaryType, NormalizeStrategyType>::
      put_spaces(std::string& res, const String::SubString& in) const
      throw (eh::Exception)
  {
    Result vres;
    segment(in, vres);

    res.clear();
    bool sep = true;

    for (Result::const_iterator it = vres.begin(); it != vres.end(); ++it)
    {
      if (!sep && !it->empty() && *it->begin() != ' ')
      {
        res += ' ';
      }
      res += *it;
      sep = (*it->rbegin() == ' ');
    }
  }
}

#endif
