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





#ifndef STRING_BASIC_ANALIZER_HPP
#define STRING_BASIC_ANALIZER_HPP

#include <istream>
#include <ostream>
#include <list>

#include <eh/Exception.hpp>


namespace String
{
  namespace SequenceAnalyzer
  {
    /**
     * Exception that raised by prepared standard translator
     */
    DECLARE_EXCEPTION(BasicAnalyzerException, eh::DescriptiveException);

    /**
     * Translate through a prepared standard translator
     * @param istr Input data stream
     * @param ostr Stream to store translation results
     */
    void
    interprete_base_sequence(std::istream& istr, std::ostream& ostr)
      throw (BasicAnalyzerException, eh::Exception);

    /**
     * Translate through a prepared standard translator
     * @param istr Input data stream
     * @param ret_list Translation results stored in the form of a
     * std::list
     */
    void
    interprete_base_sequence(std::istream& istr,
      std::list<std::string>& ret_list)
      throw (BasicAnalyzerException, eh::Exception);
  } // namespace SequenceAnalyzer
}

#endif
