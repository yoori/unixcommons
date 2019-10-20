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

#include <String/AnalyzerParams.hpp>
#include <String/Analyzer.hpp>
#include <String/BasicAnalyzer.hpp>


namespace
{
  class CerrCallback :
    public ReferenceCounting::AtomicImpl,
    public Generics::ActiveObjectCallback
  {
  public:
    virtual
    void
    report_error(Severity severity, const String::SubString& description,
      const char* error_code = 0) throw ();

  protected:
    virtual
    ~CerrCallback() throw ();
  };

  CerrCallback::~CerrCallback() throw ()
  {
  }

  void
  CerrCallback::report_error(Severity /*severity*/,
    const String::SubString& description,
    const char* /*error_code*/) throw ()
  {
    try
    {
      std::cerr << description << std::endl;
    }
    catch (...)
    {
    }
  }
}

namespace String
{
  namespace SequenceAnalyzer
  {
    template <typename ResultType>
    void
    interprete_base_seq(std::istream& istr, ResultType& result_arg)
      throw (BasicAnalyzerException, eh::Exception)
    {
      AnalyzerParams params;

      params.shield_symbol = '\\';
      params.main_separators = CharSet(", ");
      params.ignore_successive_separators = true;

      params.regular_symbs = CharSet("a-zA-Z0-9_.:-");
      params.regular_range_symbs = CharSet("a-zA-Z0-9_.:");

      params.allow_ignored_symbs = true;
      params.ignored_symbs = CharSet("\n");

      params.allow_recursion = true;
      params.recursion_max_depth = 10000;
      params.allow_repeat = true;
      params.num_retries_symb = CharPair('{', '}');
      params.retry_part_symb = CharPair('(', ')');
      params.allow_range = true;
      params.immediate_range_mode = false;
      params.range_part_symb = CharPair('[', ']');
      params.range_separators = CharSet(", ");
      params.range_symbol = '-';
      params.allow_padding = true;
      params.padding_symb = '0';
      params.use_char_range = false;
      params.use_int_range = true;
      params.int_range_bounds.add(0, 1000);
      params.default_int_range_start = 0;
      params.use_str_range = false;
      params.before_lexeme_out_str = "";
      params.after_lexeme_out_str = " ";

      try
      {
        Generics::ActiveObjectCallback_var callback(new CerrCallback);
        Analyzer base_analyzer(params, callback);
        base_analyzer.process_char_sequence(istr, result_arg);
      }
      catch (const Analyzer::Exception& e)
      {
        Stream::Error ostr;
        ostr << FNS <<
          "Got Generics::SequenceAnalyzer::Analyzer::Exception: " <<
          e.what();
        throw BasicAnalyzerException(ostr);
      }
      catch (const eh::Exception& e)
      {
        Stream::Error ostr;
        ostr << FNS << "Got eh::Exception: " << e.what();
        throw BasicAnalyzerException(ostr);
      }
    }

    void
    interprete_base_sequence(std::istream& istr, std::ostream& ostr)
      throw (BasicAnalyzerException, eh::Exception)
    {
      interprete_base_seq<std::ostream>(istr, ostr);
    }

    void
    interprete_base_sequence(std::istream& istr,
      std::list<std::string>& ret_list)
      throw (BasicAnalyzerException, eh::Exception)
    {
      interprete_base_seq<std::list<std::string> >(istr, ret_list);
    }
  } // namespace SequenceAnalyzer
}
