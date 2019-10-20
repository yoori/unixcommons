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



#include <list>

#include <String/UTF8Case.hpp>

#include <Generics/Function.hpp>

#include <Stream/MemoryStream.hpp>

#include <Language/BLogic/NormalizeTrigger.hpp>


namespace
{
  using namespace Language::Trigger;


  struct Split
  {
    typedef std::pair<std::string, bool> Part;
    typedef std::list<Part> Parts;

    bool exact;
    Parts parts;
  };


  const char*
  find_quote(const char* cur, const char* const END) throw ()
  {
    for (;; cur++)
    {
      if (cur == END || *cur == '"')
      {
        break;
      }

      if (*cur == '[' || *cur == ']')
      {
        return 0;
      }
    }

    return cur;
  }

  const char*
  find_space_or_quote(const char* cur, const char* const END, const bool EXACT)
    throw ()
  {
    for (;; cur++)
    {
      if (cur == END)
      {
        break;
      }

      if (*cur == ' ' || *cur == '\t' || *cur == '"')
      {
        break;
      }

      if (*cur == '[')
      {
        return 0;
      }
      if (*cur == ']')
      {
        if (EXACT)
        {
          break;
        }

        return 0;
      }
    }

    return cur;
  }

  bool
  skip_spaces(const char*& cur, const char* END) throw ()
  {
    do
    {
      cur++;
      if (cur == END)
      {
        return true;
      }
    }
    while (*cur == ' ');

    return false;
  }

  void
  shrink(std::string& str) throw ()
  {
    char* out = &str[0];
    const char* cur = out;
    const char* const END = cur + str.size();

    if (*cur == ' ')
    {
      if (skip_spaces(cur, END))
      {
        str.clear();
        return;
      }
    }

    for (;;)
    {
      do
      {
        *out++ = *cur++;

        if (cur == END)
        {
          str.resize(out - str.data());
          return;
        }
      }
      while (*cur != ' ');

      if (skip_spaces(cur, END))
      {
        break;
      }

      *out++ = ' ';
    }

    str.resize(out - str.data());
  }

  void
  simplify_common(const String::SubString& trigger, const char* name,
    const String::SubString& str, std::string& result,
    const Language::Segmentor::SegmentorInterface* segmentor)
    throw (eh::Exception, Exception)
  {
    const bool HAS_SEGMENTOR = segmentor;

    std::string res;
    if (!String::case_change<String::Simplify>(str,
      HAS_SEGMENTOR ? res : result))
    {
      Stream::Error ostr;
      ostr << FNS << "invalid UTF-8 symbol in " << name << " >" <<
        trigger << "<";
      throw Exception(ostr);
    }

    if (HAS_SEGMENTOR)
    {
      if (res.empty())
      {
        result.clear();
      }
      else
      {
        segmentor->put_spaces(result, res.data(), res.size());
      }
    }
  }

  void
  simplify(const String::SubString& trigger, const char* name,
    const String::SubString& str, std::string& result,
    const Language::Segmentor::SegmentorInterface* segmentor)
    throw (eh::Exception, Exception)
  {
    simplify_common(trigger, name, str, result, segmentor);

    if (!result.empty())
    {
      shrink(result);
    }
  }

  bool
  next(const char*& cur, const char* const END) throw ()
  {
    while (*cur == ' ' || *cur == '\t')
    {
      if (++cur == END)
      {
        return true;
      }
    }

    return false;
  }

  void
  add_part(const char* begin, const char* end, bool quotes, bool exact,
    const String::SubString& trigger, Split& split,
    const Language::Segmentor::SegmentorInterface* segmentor,
    unsigned& parts, unsigned& size)
    throw (eh::Exception)
  {
    if (begin != end)
    {
      std::string tmp;

      simplify(trigger, "trigger", String::SubString(begin, end),
        tmp, segmentor);

      if (!tmp.empty())
      {
        if (exact)
        {
          begin = tmp.data();
          end = begin + tmp.size();
          for (const char* cur = begin; ; ++cur)
          {
            if (cur == end || *cur == ' ')
            {
              parts++;
              size += (cur - begin) + 1;

              split.parts.emplace_back();
              Split::Part& part = split.parts.back();
              part.first.assign(begin, cur);
              part.second = false;

              if (cur == end)
              {
                break;
              }
              begin = ++cur;
            }
          }
        }
        else
        {
          parts++;
          size += tmp.size() + 3;

          split.parts.emplace_back();
          Split::Part& part = split.parts.back();
          part.first.swap(tmp);
          part.second = quotes;
        }
      }
    }
  }

  void
  divide(const String::SubString& trigger, Split& split,
    const Language::Segmentor::SegmentorInterface* segmentor,
    unsigned& parts, unsigned& size)
    throw (eh::Exception, Exception)
  {
    if (trigger.size() > 1024)
    {
      Stream::Error ostr;
      ostr << FNS << "trigger >" << trigger << "< is too large";
      throw Exception(ostr);
    }

    parts = 0;
    size = 0;

    split.exact = false;
    split.parts.clear();

    if (trigger.empty())
    {
      return;
    }

    const char* cur = trigger.data();
    const char* const END = cur + trigger.size();

    if (next(cur, END))
    {
      return;
    }

    const bool EXACT = *cur == '[';
    if (EXACT)
    {
      if (++cur == END || next(cur, END))
      {
        Stream::Error ostr;
        ostr << FNS << "no right bracket in trigger >" <<
          trigger << "<";
        throw Exception(ostr);
      }
      size = 1;
    }
    split.exact = EXACT;

    for (;;)
    {
      if (EXACT && *cur == ']')
      {
        if (++cur != END && !next(cur, END))
        {
          Stream::Error ostr;
          ostr << FNS << "symbols after right bracket in trigger >" <<
            trigger << "<";
          throw Exception(ostr);
        }
        break;
      }

      const char* begin;
      const char* end;
      const bool QUOTES = *cur == '"';

      if (!EXACT && QUOTES)
      {
        begin = ++cur;
        end = find_quote(cur, END);
        if (end == END)
        {
          Stream::Error ostr;
          ostr << FNS << "unpaired quote in trigger >" << trigger << "<";
          throw Exception(ostr);
        }
        if (!end)
        {
          Stream::Error ostr;
          ostr << FNS << "unexpected bracket in trigger >" <<
            trigger << "<";
          throw Exception(ostr);
        }
        cur = end + 1;
      }
      else
      {
        begin = cur;
        if (EXACT && QUOTES)
        {
          ++cur;
        }
        end = find_space_or_quote(cur, END, EXACT);
        if (!end)
        {
          Stream::Error ostr;
          ostr << FNS << "unexpected bracket in trigger >" <<
            trigger << "<";
          throw Exception(ostr);
        }
        cur = end;
      }

      add_part(begin, end, QUOTES, EXACT, trigger, split, segmentor,
        parts, size);

      if (cur == END || next(cur, END))
      {
        if (EXACT)
        {
          Stream::Error ostr;
          ostr << FNS << "no right bracket in trigger >" <<
            trigger << "<";
          throw Exception(ostr);
        }
        break;
      }
    }
  }

  bool
  is_substr(const String::SubString& small, const String::SubString& big)
    throw ()
  {
    String::SubString::SizeType pos = big.find(small);
    if (pos == String::SubString::NPOS)
    {
      return false;
    }
    if (pos && big[pos - 1] != ' ')
    {
      return false;
    }
    pos += small.size();
    if (pos != big.size() && big[pos] != ' ')
    {
      return false;
    }
    return true;
  }

  bool
  narrow_one(Split::Parts& parts, Split::Parts::iterator itor) throw ()
  {
    Split::Parts::iterator next(itor);

    for (++next; next != parts.end();)
    {
      if (itor->first.size() == next->first.size())
      {
        if (itor->first == next->first)
        {
          if (next->second)
          {
            itor->second = true;
          }
          next = parts.erase(next);
          continue;
        }
      }
      else if (itor->first.size() > next->first.size())
      {
        if (is_substr(next->first, itor->first))
        {
          next = parts.erase(next);
          continue;
        }
      }
      else
      {
        if (is_substr(itor->first, next->first))
        {
          return true;
        }
      }
      ++next;
    }

    return false;
  }

  void
  narrow(Split::Parts& parts) throw ()
  {
    for (Split::Parts::iterator itor = parts.begin(); itor != parts.end();)
    {
      if (narrow_one(parts, itor))
      {
        itor = parts.erase(itor);
      }
      else
      {
        ++itor;
      }
    }
    parts.sort();
  }

  void
  combine(const Split& split, std::string& result)
    throw (eh::Exception, Exception)
  {
    const bool EXACT = split.exact;

    if (EXACT)
    {
      result.push_back('[');
    }

    for (Split::Parts::const_iterator itor = split.parts.begin();
      itor != split.parts.end(); ++itor)
    {
      if (itor != split.parts.begin())
      {
        result.push_back(' ');
      }

      const bool QUOTES = !EXACT &&
        itor->first.find(' ') != std::string::npos;
      if (QUOTES)
      {
        result.push_back('\"');
      }

      result.append(itor->first);

      if (QUOTES)
      {
        result.push_back('\"');
      }
    }

    if (EXACT)
    {
      result.push_back(']');
    }
  }

  void
  combine(const Split& split, Trigger& result)
    throw (eh::Exception, Exception)
  {
    const bool EXACT = split.exact;

    if (EXACT)
    {
      result.trigger.push_back('[');
    }

    for (Split::Parts::const_iterator itor = split.parts.begin();
      itor != split.parts.end(); ++itor)
    {
      if (itor != split.parts.begin())
      {
        result.trigger.push_back(' ');
      }

      const bool QUOTES = !EXACT &&
        itor->first.find(' ') != std::string::npos;
      if (QUOTES)
      {
        result.trigger.push_back('\"');
      }

      const char* const CUR = result.trigger.data() + result.trigger.size();
      result.trigger.append(itor->first);
      Trigger::Part part =
        {
          String::SubString(CUR, itor->first.size()),
          QUOTES || itor->second
        };
      result.parts.push_back(part);

      if (QUOTES)
      {
        result.trigger.push_back('\"');
      }
    }

    if (EXACT)
    {
      result.trigger.push_back(']');
    }
  }
}

namespace Language
{
  namespace Trigger
  {
    void
    normalize(const String::SubString& trigger, std::string& result,
      const Segmentor::SegmentorInterface* segmentor)
      throw (eh::Exception, Exception)
    {
      Split split;
      unsigned parts, size;

      divide(trigger, split, segmentor, parts, size);

      result.clear();
      if (!parts)
      {
        return;
      }

      if (!split.exact)
      {
        narrow(split.parts);
      }

      result.reserve(size);

      combine(split, result);
    }

    void
    normalize(const String::SubString& trigger, Trigger& result,
      const Segmentor::SegmentorInterface* segmentor)
      throw (eh::Exception, Exception)
    {
      Split split;
      unsigned parts, size;

      divide(trigger, split, segmentor, parts, size);

      result.exact = false;
      result.parts.clear();
      result.trigger.clear();

      if (!parts)
      {
        return;
      }

      if (!split.exact)
      {
        narrow(split.parts);
      }

      result.exact = split.exact;
      result.parts.reserve(parts);
      result.trigger.reserve(size);

      combine(split, result);
    }

    void
    normalize_phrase(const String::SubString& phrase, std::string& result,
      const Language::Segmentor::SegmentorInterface* segmentor)
      throw (eh::Exception, Exception)
    {
      simplify(phrase, "phrase", phrase, result, segmentor);
    }
  }
}
