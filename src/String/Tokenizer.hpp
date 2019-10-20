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



// @file String/Tokenizer.hpp
#ifndef STRING_TOKENIZER_HPP
#define STRING_TOKENIZER_HPP

#include <String/AsciiStringManip.hpp>

#include <Generics/Uncopyable.hpp>


namespace String
{
  namespace StringManip
  {
    typedef const AsciiStringManip::Char3Category<' ', '\n', '\t'>
      TokenizerDefaultSeparators;

    namespace Helper
    {
      template <typename Category>
      struct SplitterState : private Generics::Uncopyable
      {
        explicit
        SplitterState(const SubString& str) throw (eh::Exception);
        SplitterState(const SubString& str, Category category)
          throw (eh::Exception);

        Category category;

        const char* str;
        const char* const END;

        const char* separator;
        bool error;
      };

      template <const bool EMPTY>
      struct GetToken
      {
        template <typename SplitterState>
        static
        bool
        get_token(SplitterState& state, SubString& token)
          throw (eh::Exception);
      };

      template <>
      struct GetToken<false>
      {
        template <typename SplitterState>
        static
        bool
        get_token(SplitterState& state, SubString& token)
          throw (eh::Exception);
      };
    };

    template <typename Category = TokenizerDefaultSeparators,
      const bool EMPTY = false>
    class Splitter
    {
    public:
      /**
       * Default constructor using default category constructor
       * @param str SubString to split
       */
      explicit
      Splitter(const SubString& str)
        throw (eh::Exception);

      /**
       * Constructor
       * @param str SubString to split
       * @param category Category to determine separation
       */
      Splitter(const SubString& str, Category category)
        throw (eh::Exception);

      /**
       * Searches for the next token and returns substring describing it
       * @param token resulted token
       * @return if a new token has been found or not
       */
      bool
      get_token(SubString& token) throw (eh::Exception);

      /**
       * Returns separator at the end of found token
       * @return pointer to the separator symbol in the string
       */
      const char*
      get_separator() const throw ();

      /**
       * If get_token() returned negative it may mean category found
       * the error in the string
       * @return whether or not error has been found in the string
       */
      bool
      is_error() const throw ();

    private:
      Helper::SplitterState<Category> state_;
    };

    typedef Splitter<const AsciiStringManip::CharCategory&> CharSplitter;

    typedef Splitter<AsciiStringManip::SepColon> SplitColon;
    typedef Splitter<AsciiStringManip::SepComma> SplitComma;
    typedef Splitter<AsciiStringManip::SepPeriod> SplitPeriod;
    typedef Splitter<AsciiStringManip::SepMinus> SplitMinus;
    typedef Splitter<AsciiStringManip::SepSemCol> SplitSemCol;
    typedef Splitter<AsciiStringManip::SepAmp> SplitAmp;
    typedef Splitter<AsciiStringManip::SepSpace> SplitSpace;
    typedef Splitter<AsciiStringManip::SepEq> SplitEq;
    typedef Splitter<AsciiStringManip::SepSlash> SplitSlash;
    typedef Splitter<AsciiStringManip::SepHash> SplitHash;
    typedef Splitter<AsciiStringManip::SepBar> SplitBar;
    typedef Splitter<AsciiStringManip::SepNL> SplitNL;
    typedef Splitter<AsciiStringManip::SepTab> SplitTab;

    class Tokenizer :
      private AsciiStringManip::CharCategory,
      public CharSplitter
    {
    public:
      Tokenizer(const String::SubString& str, const char* symbols)
        throw (eh::Exception);
    };


    template <typename Category, typename Callback>
    bool
    divide(const String::SubString& str, Category category,
      Callback callback) throw (eh::Exception);
  }
}

namespace String
{
  namespace StringManip
  {
    namespace Helper
    {
      //
      // SplitterState class
      //

      template <typename Category>
      SplitterState<Category>::SplitterState(const SubString& str)
        throw (eh::Exception)
      : category(),
        str(str.begin()), END(str.end()), error(false)
      {
      }

      template <typename Category>
      SplitterState<Category>::SplitterState(const SubString& str,
        Category category)
        throw (eh::Exception)
        : category(category),
          str(str.begin()), END(str.end()), error(false)
      {
      }


      //
      // GetToken class
      //

      template <const bool EMPTY>
      template <typename SplitterState>
      bool
      GetToken<EMPTY>::get_token(SplitterState& state, SubString& token)
        throw (eh::Exception)
      {
        if (state.str == state.END)
        {
          return false;
        }

        unsigned long octets_length = 0;
        if ((state.separator =
          state.category.find_owned(state.str, state.END, &octets_length)))
        {
          token.assign(state.str, state.separator);
          state.str = state.separator == state.END ? state.END :
            state.separator + octets_length;
          return true;
        }

        state.error = true;
        return false;
      }

      template <typename SplitterState>
      bool
      GetToken<false>::get_token(SplitterState& state, SubString& token)
        throw (eh::Exception)
      {
        if (const char* begin = state.category.find_nonowned(
          state.str, state.END))
        {
          if (const char* end = state.category.find_owned(begin, state.END))
          {
            if (begin == end)
            {
              return false;
            }

            token.assign(begin, end);
            state.separator = state.str = end;
            return true;
          }
        }

        state.error = true;
        return false;
      }
    }


    //
    // Splitter class
    //

    template <typename Category, const bool EMPTY>
    Splitter<Category, EMPTY>::Splitter(const SubString& str)
      throw (eh::Exception)
      : state_(str)
    {
    }

    template <typename Category, const bool EMPTY>
    Splitter<Category, EMPTY>::Splitter(const SubString& str,
      Category category)
      throw (eh::Exception)
      : state_(str, category)
    {
    }

    template <typename Category, const bool EMPTY>
    bool
    Splitter<Category, EMPTY>::get_token(SubString& token)
      throw (eh::Exception)
    {
      return Helper::GetToken<EMPTY>::get_token(state_, token);
    }

    template <typename Category, const bool EMPTY>
    const char*
    Splitter<Category, EMPTY>::get_separator() const throw ()
    {
      return state_.separator;
    }

    template <typename Category, const bool EMPTY>
    bool
    Splitter<Category, EMPTY>::is_error() const throw ()
    {
      return state_.error;
    }


    //
    // Tokenizer class
    //

    inline
    Tokenizer::Tokenizer(const String::SubString& str, const char* symbols)
      throw (eh::Exception)
      : AsciiStringManip::CharCategory(symbols), CharSplitter(str, *this)
    {
    }


    template <typename Category, typename Callback>
    bool
    divide(const String::SubString& str, Category category,
      Callback callback) throw (eh::Exception)
    {
      const char* last = str.begin();
      const char* const END = str.end();
      for (;;)
      {
        const char* cur = category.find_owned(last, END);
        if (!cur)
        {
          return false;
        }
        if (cur != last)
        {
          callback.nonowned(String::SubString(last, cur));
        }
        if (cur == END)
        {
          break;
        }
        last = cur;
        cur = category.find_nonowned(last, END);
        if (!cur)
        {
          return false;
        }
        if (cur != last)
        {
          callback.owned(String::SubString(last, cur));
        }
        if (cur == END)
        {
          break;
        }
        last = cur;
      }
      return true;
    }
  }
}

#endif
