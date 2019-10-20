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



// @file String/SubStringFind.tpp

namespace String
{
  //
  // Find char forward
  //

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find(ValueType ch,
    SizeType pos) const
    throw ()
  {
    if (pos < length_)
    {
      ConstPointer ptr = Traits::find(begin_ + pos, length_ - pos, ch);
      if (ptr)
      {
        return ptr - begin_;
      }
    }
    return NPOS;
  }

  //
  // Find substring forward
  //

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find(const BasicSubString& str,
    SizeType pos) const
    throw ()
  {
    if (!str.length_)
    {
      return pos >= length_ ? NPOS : 0;
    }

    SizeType length = get_available_length_(pos, length_);
    if (str.length_ > length)
    {
      return NPOS;
    }

    const ConstPointer END = begin_ + length_;
    const ConstPointer FOUND = std::search(begin_ + pos, END,
      str.begin_, str.begin_ + str.length_, Traits::eq);
    return FOUND != END ? FOUND - begin_ : NPOS;
  }

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find(ConstPointer ptr,
    SizeType pos) const
    throw (LogicError)
  {
    return find(BasicSubString<CharType, Traits, Checker>(ptr), pos);
  }

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find(ConstPointer ptr,
    SizeType pos, SizeType count) const
    throw (LogicError)
  {
    return find(BasicSubString<CharType, Traits, Checker>(ptr, count), pos);
  }

  //
  // Find char backward
  //

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::rfind(ValueType ch,
    SizeType pos) const
    throw ()
  {
    if (length_)
    {
      ConstPointer last = begin_ + std::min(length_ - 1, pos);
      for (;; --last)
      {
        if (Traits::eq(*last, ch))
        {
          return last - begin_;
        }
        if (last == begin_)
        {
          return NPOS;
        }
      }
    }
    return NPOS;
  }

  //
  // Find substring backward
  //

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::rfind(const BasicSubString& str,
    SizeType pos) const
    throw ()
  {
    if (str.length_ > length_)
    {
      return NPOS;
    }

    if (pos > length_)
    {
      pos = length_;
    }

    if (!str.length_)
    {
      return pos;
    }

    if (pos > length_ - str.length_)
    {
      pos = length_ - str.length_;
    }

    for (ConstPointer data = begin_ + pos; ; data--)
    {
      if (!Traits::compare(data, str.begin_, str.length_))
      {
        return data - begin_;
      }
      if (data == begin_)
      {
        break;
      }
    }

    return NPOS;
  }

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::rfind(ConstPointer ptr,
    SizeType pos) const
    throw (LogicError)
  {
    return rfind(BasicSubString<CharType, Traits, Checker>(ptr), pos);
  }

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::rfind(ConstPointer ptr,
    SizeType pos, SizeType count) const
    throw (LogicError)
  {
    return rfind(BasicSubString<CharType, Traits, Checker>(ptr), pos);
  }

  //
  // Find char forward
  //

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_first_of(ValueType ch,
    SizeType pos) const
    throw ()
  {
    return find(ch, pos);
  }

  //
  // Find chars forward
  //

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_first_of(
    const BasicSubString& str, SizeType pos) const
    throw ()
  {
    for (; pos < length_; pos++)
    {
      if (Traits::find(str.begin_, str.length_, begin_[pos]))
      {
        return pos;
      }
    }
    return NPOS;
  }

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_first_of(
    ConstPointer ptr, SizeType pos) const
    throw (LogicError)
  {
    return find_first_of(BasicSubString<CharType, Traits, Checker>(ptr),
      pos);
  }

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_first_of(
    ConstPointer ptr, SizeType pos, SizeType count) const
    throw (LogicError)
  {
    return find_first_of(
      BasicSubString<CharType, Traits, Checker>(ptr, count), pos);
  }

  //
  // Find not char forward
  //

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_first_not_of(ValueType ch,
    SizeType pos) const
    throw ()
  {
    for (; pos < length_; pos++)
    {
      if (!Traits::eq(begin_[pos], ch))
      {
        return pos;
      }
    }
    return NPOS;
  }

  //
  // Find not chars forward
  //

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_first_not_of(
    const BasicSubString& str, SizeType pos) const
    throw ()
  {
    for (; pos < length_; pos++)
    {
      if (!Traits::find(str.begin_, str.length_, begin_[pos]))
      {
        return pos;
      }
    }
    return NPOS;
  }

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_first_not_of(
    ConstPointer ptr, SizeType pos) const
    throw (LogicError)
  {
    return find_first_not_of(BasicSubString<CharType, Traits, Checker>(ptr),
      pos);
  }

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_first_not_of(
    ConstPointer ptr, SizeType pos, SizeType count) const
    throw (LogicError)
  {
    return find_first_not_of(
      BasicSubString<CharType, Traits, Checker>(ptr, count), pos);
  }

  //
  // Find char backward
  //

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_last_of(ValueType ch,
    SizeType pos) const
    throw ()
  {
    return rfind(ch, pos);
  }

  //
  // Find chars backward
  //

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_last_of(
    const BasicSubString& str, SizeType pos) const
    throw ()
  {
    if (length_ && str.length_)
    {
      if (pos >= length_)
      {
        pos = length_ - 1;
      }
      do
      {
        if (Traits::find(str.begin_, str.length_, begin_[pos]))
        {
          return pos;
        }
      }
      while (pos--);
    }
    return NPOS;
  }

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_last_of(
    ConstPointer ptr, SizeType pos) const
    throw (LogicError)
  {
    return find_last_of(BasicSubString<CharType, Traits, Checker>(ptr),
      pos);
  }

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_last_of(
    ConstPointer ptr, SizeType pos, SizeType count) const
    throw (LogicError)
  {
    return find_last_of(
      BasicSubString<CharType, Traits, Checker>(ptr, count), pos);
  }

  //
  // Find not chars backward
  //

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_last_not_of(
    const BasicSubString& str, SizeType pos) const
    throw ()
  {
    if (length_ && str.length_)
    {
      if (pos >= length_)
      {
        pos = length_ - 1;
      }
      do
      {
        if (!Traits::find(str.begin_, str.length_, begin_[pos]))
        {
          return pos;
        }
      }
      while (pos--);
    }
    return NPOS;
  }

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_last_not_of(
    ConstPointer ptr, SizeType pos) const
    throw (LogicError)
  {
    return find_last_not_of(BasicSubString<CharType, Traits, Checker>(ptr),
      pos);
  }

  template <typename CharType, typename Traits, typename Checker>
  typename BasicSubString<CharType, Traits, Checker>::SizeType
  BasicSubString<CharType, Traits, Checker>::find_last_not_of(
    ConstPointer ptr, SizeType pos, SizeType count) const
    throw (LogicError)
  {
    return find_last_not_of(
      BasicSubString<CharType, Traits, Checker>(ptr, count), pos);
  }
}
