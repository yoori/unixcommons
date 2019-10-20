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
#include <cmath>
#include <vector>

#include <String/StringManip.hpp>
#include <String/Tokenizer.hpp>

#include <Generics/Function.hpp>
#include <Generics/CommonDecimal.hpp>

#include <Stream/MMapStream.hpp>

#include <Language/Polyglot/DictionaryLoader.hpp>


namespace
{
  long MAX_FREQ = 100000;
}

/*
 * single Dictionary format
 *   <ID:NUMBER> <WORD:UTF8-STRING> <FREQ:NUMBER[0..MAX-FREQ]>
 * single Dictionary with normal form format
 *   <ID:NUMBER> <WORD:UTF8-STRING> <FREQ:NUMBER[0..MAX-FREQ]> <NORM-WORD:UTF8-STRING>
 * bi gram Dictionary format
 *   <ID1:NUMBER> <ID2:NUMBER> <FREQ:NUMBER>
 */
namespace
{
  int
  read_int(const String::SubString& str)
  {
    int value;
    return String::StringManip::str_to_int(str, value) ? value : 0;
  }

  void
  parse_dictionary_string(const String::SubString& str, unsigned long& id,
    std::wstring& word, long& freq, std::string* norm_word)
    throw (eh::Exception, Polyglot::DictionaryLoader::InvalidParameter)
  {
    String::SubString part;

    String::StringManip::Splitter<> tokenizer(str);

    for (;;)
    {
      if (tokenizer.get_token(part))
      {
        id = read_int(part);

        if (id == 0)
        {
          // invalid line
          Stream::Error ostr;
          ostr << FNS << "can't parse dictionary line '" << str << "'";
          throw Polyglot::DictionaryLoader::InvalidParameter(ostr);
        }

        if (tokenizer.get_token(part))
        {
          Generics::ArrayWChar res;

          try
          {
            res = String::StringManip::utf8_to_wchar(part);
          }
          catch (const eh::Exception& ex)
          {
            Stream::Error ostr;
            ostr << FNS << "can't parse dictionary line - "
              "incorrect utf8 in '" << str << "'";
            throw Polyglot::DictionaryLoader::InvalidParameter(ostr);
          }

          word = res.get();

          if (tokenizer.get_token(part))
          {
            freq = read_int(part);

            if (freq == 0)
            {
              // invalid line
              Stream::Error ostr;
              ostr << FNS << "can't parse dictionary line '" << str << "'";
              throw Polyglot::DictionaryLoader::InvalidParameter(ostr);
            }

            if (tokenizer.get_token(part))
            {
              if (norm_word)
              {
                part.assign_to(*norm_word);

                if (!tokenizer.get_token(part))
                {
                  break;
                }
              }
            }
            else
            {
              break;
            }
          }
        }
      }

      // invalid line
      Stream::Error ostr;
      ostr << FNS << "can't parse dictionary line '" << str <<
        "': incorrect parts number";
      throw Polyglot::DictionaryLoader::InvalidParameter(ostr);
    }

    if (freq > MAX_FREQ)
    {
      freq = MAX_FREQ;
    }

    freq = -freq;
  }

  void
  parse_suffix_dictionary_string(const String::SubString& str,
    std::wstring& suffix, unsigned long& len, long& freq)
    throw (eh::Exception, Polyglot::DictionaryLoader::InvalidParameter)
  {
    String::SubString part;

    String::StringManip::Splitter<> tokenizer(str);


    for (;;)
    {
      if (tokenizer.get_token(part))
      {
        Generics::ArrayWChar res;

        try
        {
          res = String::StringManip::utf8_to_wchar(part);
        }
        catch (const eh::Exception& ex)
        {
          Stream::Error ostr;
          ostr << FNS << "can't parse dictionary line - "
            "incorrect utf8 in '" << str << "'";
          throw Polyglot::DictionaryLoader::InvalidParameter(ostr);
        }

        suffix = res.get();

        if (tokenizer.get_token(part))
        {
          len = read_int(part);

          if (len == 0)
          {
            // invalid line
            Stream::Error ostr;
            ostr << FNS << "can't parse dictionary line '" << str << "'";
            throw Polyglot::DictionaryLoader::InvalidParameter(ostr);
          }

          if (tokenizer.get_token(part))
          {
            freq = read_int(part);

            if (freq == 0)
            {
              // invalid line
              Stream::Error ostr;
              ostr << FNS << "can't parse dictionary line '" << str << "'";
              throw Polyglot::DictionaryLoader::InvalidParameter(ostr);
            }

            if (!tokenizer.get_token(part))
            {
              break;
            }
          }
        }
      }

      // invalid line
      Stream::Error ostr;
      ostr << FNS << "can't parse dictionary line '" << str <<
        "': incorrect parts number";
      throw Polyglot::DictionaryLoader::InvalidParameter(ostr);
    }

#if 0
    if (freq > MAX_FREQ)
    {
      freq = MAX_FREQ;
    }
#endif

    freq = -freq;
  }


  struct Word
  {
    Word(const std::wstring& word_val, unsigned long freq_val)
      throw (eh::Exception);

    std::wstring word;
    unsigned long freq;
  };

  Word::Word(const std::wstring& word_val, unsigned long freq_val)
    throw (eh::Exception)
    : word(word_val), freq(freq_val)
  {
  }

  typedef Generics::GnuHashTable<
    Generics::NumericHashAdapter<unsigned long>, Word> IdWordMap;
}

namespace Polyglot
{
  void
  DictionaryLoader::load(const char* dict_base_path, Dictionary& out_dict)
    throw (eh::Exception, InvalidParameter)
  {
    load((std::string(dict_base_path) + "s-dict").c_str(),
      (std::string(dict_base_path) + "bi-dict").c_str(), out_dict);
  }

  void
  DictionaryLoader::load(const char* dict_base_path,
    DictionaryWithNorm& out_dict)
    throw (eh::Exception, InvalidParameter)
  {
    load((std::string(dict_base_path) + "sn-dict").c_str(),
      (std::string(dict_base_path) + "bi-dict").c_str(), out_dict);
  }

  void
  DictionaryLoader::load(const char* dict_file, const char* bi_dict_file,
    Dictionary& out_dict)
    throw (eh::Exception, InvalidParameter)
  {
    try
    {
      Stream::FileParser dict(dict_file);
      Stream::FileParser bi_dict(bi_dict_file);

      load(dict, bi_dict, out_dict);
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't open dictionaries '" << dict_file << "', '" <<
        bi_dict_file << "': " << ex.what();
      throw InvalidParameter(ostr);
    }
  }

  void
  DictionaryLoader::load(const char* dict_file, const char* bi_dict_file,
    DictionaryWithNorm& out_dict)
    throw (eh::Exception, InvalidParameter)
  {
    try
    {
      Stream::FileParser dict(dict_file);
      Stream::FileParser bi_dict(bi_dict_file);

      load(dict, bi_dict, out_dict);
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't open dictionaries '" << dict_file << "', '" <<
        bi_dict_file << "': " << ex.what();
      throw InvalidParameter(ostr);
    }
  }

  void
  DictionaryLoader::load(std::istream& dict, std::istream& /*bi_dict*/,
    Dictionary& out_dict)
    throw (eh::Exception, InvalidParameter)
  {
    std::string line;

//  IdWordMap id_word_map;

    // id dictionary load
    while (std::getline(dict, line))
    {
      unsigned long id;
      std::wstring word;
      long freq;

      parse_dictionary_string(line, id, word, freq, 0);

//    id_word_map.insert(std::make_pair(id, Word(word, freq)));

      out_dict.traits_.max_el = std::max(out_dict.traits_.max_el, freq);
      out_dict.traits_.min_el = std::min(out_dict.traits_.min_el, freq);
      out_dict.traits_.sum_el += freq;
      ++out_dict.traits_.count_el;

      out_dict.insert(std::make_pair(word, DictionaryNode(id, freq)));
    }

#if 0
    std::cout << "AVG: " << out_dict.traits_.sum_el * 1.0 /
      out_dict.traits_.count_el << std::endl;

    // bi dictionary load
    while (std::getline(bi_dict, line))
    {
      std::string::size_type sep_pos1 = line.find(' ');
      if (sep_pos1 == std::string::npos)
      {
        // invalid line
        Stream::Error ostr;
        ostr << FNS << "can't parse dictionary line '" << line << "'";
        throw InvalidParameter(ostr);
      }

      std::string::size_type sep_pos2 = line.find(' ', sep_pos1 + 1);
      if (sep_pos2 == std::string::npos)
      {
        // invalid line
        Stream::Error ostr;
        ostr << FNS << "can't parse dictionary line '" << line << "'";
        throw InvalidParameter(ostr);
      }

      std::string part1(line.begin(), line.begin() + sep_pos1);
      std::string part2(line.begin() + sep_pos1 + 1, line.begin() + sep_pos2);
      std::string part3(line.c_str() + sep_pos2 + 1);

      Stream::Parser ipart1(part1);
      Stream::Parser ipart2(part2);
      Stream::Parser ipart3(part3);

      unsigned long id1 = 0;
      unsigned long id2 = 0;
      unsigned long freq = 1;

      ipart1 >> id1;
      if (!ipart1.eof() || ipart1.bad())
      {
        // invalid line
        Stream::Error ostr;
        ostr << FNS << "can't parse bi dictionary line '" << line << "'";
        throw InvalidParameter(ostr);
      }

      ipart2 >> id2;
      if (!ipart2.eof() || ipart2.bad())
      {
        // invalid line
        Stream::Error ostr;
        ostr << FNS << "can't parse bi dictionary line '" << line << "'";
        throw InvalidParameter(ostr);
      }

      ipart3 >> freq;
      if (!ipart3.eof() || ipart3.bad())
      {
        // invalid line
        Stream::Error ostr;
        ostr << FNS << "can't parse bi dictionary line '" << line << "'";
        throw InvalidParameter(ostr);
      }

      Generics::ArrayWChar res;

      IdWordMap::const_iterator it1 = id_word_map.find(id1);
      IdWordMap::const_iterator it2 = id_word_map.find(id2);

      if (it1 == id_word_map.end() || it2 == id_word_map.end())
      {
        // invalid line
        Stream::Error ostr;
        ostr << FNS << "bi dictionary - "
          "invalid id reference in line '" << line << "'";
        throw InvalidParameter(ostr);
      }

      Dictionary::Iterator res_it = out_dict.find(
        it1->second.begin(), it1->second.end());

      if (res_it == out_dict.end() || !res_it.is_element())
      {
        // invalid line
        Stream::Error ostr;
        ostr << FNS << "bi dictionary - "
          "invalid word by id reference in line '" << line << "'";
        throw InvalidParameter(ostr);
      }

      freq = (freq + out_dict.traits_.max_el) *
        (it1->second.length() + it2->second.length());

      res_it.element().bi_freq_map.insert(std::make_pair(id2, freq));

      out_dict.traits_.bi_max_el = std::max(out_dict.traits_.bi_max_el, freq);
      out_dict.traits_.bi_min_el = std::min(out_dict.traits_.bi_min_el, freq);
      out_dict.traits_.bi_sum_el += freq;
      ++out_dict.traits_.bi_count_el;
    }
#endif
  }

  void
  DictionaryLoader::load(std::istream& dict, std::istream& /*bi_dict*/,
    DictionaryWithNorm& out_dict)
    throw (eh::Exception, InvalidParameter)
  {
    std::string line;

//  IdWordWithMap id_word_map;

    // id dictionary load
    while (std::getline(dict, line))
    {
      unsigned long id;
      std::wstring word;
      long freq;
      std::string norm_word;

      parse_dictionary_string(line, id, word, freq, &norm_word);

//    id_word_map.insert(std::make_pair(id, Word(word, freq)));

      out_dict.traits_.max_el = std::max(out_dict.traits_.max_el, freq);
      out_dict.traits_.min_el = std::min(out_dict.traits_.min_el, freq);
      out_dict.traits_.sum_el += freq;
      ++out_dict.traits_.count_el;

      out_dict.insert(std::make_pair(word,
        DictionaryNodeWithNorm(id, freq, norm_word.c_str())));
    }
  }

  void
  DictionaryLoader::load_suffixes(const char* dict_base_path,
    SuffixDictionary& out_suffix_dict)
    throw (eh::Exception, InvalidParameter)
  {
    std::string suffix_dict_file(std::string(dict_base_path) + "suffix-dict");

    try
    {
      Stream::FileParser suffix_dict(suffix_dict_file.c_str());

      load_suffixes(suffix_dict, out_suffix_dict);
    }
    catch (const eh::Exception& ex)
    {
      Stream::Error ostr;
      ostr << FNS << "Can't open suffix dictionary '" << suffix_dict_file <<
        "': " << ex.what();
      throw InvalidParameter(ostr);
    }
  }

  void
  DictionaryLoader::load_suffixes(std::istream& suffix_dict,
    SuffixDictionary& out_dict)
    throw (eh::Exception, InvalidParameter)
  {
    std::string line;

//  IdWordWithMap id_word_map;
    DictionaryTraits& result_dict_traits = out_dict.traits_;

    // id dictionary load
    while (std::getline(suffix_dict, line))
    {
      std::wstring suffix;
      unsigned long len;
      long freq;

      parse_suffix_dictionary_string(line, suffix, len, freq);

      result_dict_traits.max_el = std::max(result_dict_traits.max_el, freq);
      result_dict_traits.min_el = std::min(result_dict_traits.min_el, freq);
      result_dict_traits.sum_el += freq;
      ++result_dict_traits.count_el;

      SuffixDictionary::iterator it =
        out_dict.find(suffix.begin(), suffix.end());
      if (it != out_dict.end())
      {
        it->second.suffixes.emplace_back(len, freq);
      }
      else
      {
        SuffixDictionaryNode suffix_node;
        suffix_node.suffixes.emplace_back(len, freq);
        out_dict.insert(std::make_pair(suffix, suffix_node));
      }
    }
  }
}
