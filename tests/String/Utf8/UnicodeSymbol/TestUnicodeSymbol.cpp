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



// TestUnicodeSymbol.cpp

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>

#include <String/UnicodeSymbol.hpp>
#include <Stream/BzlibStreams.hpp>

using namespace String;

namespace
{
  DECLARE_EXCEPTION(IOException, eh::DescriptiveException);
  std::string root_path;
}

//
// Testing input / output UnicodeSymbols from stl streams.
//
void
unicode_symbol_test() throw (eh::Exception, IOException)
{
  std::ostringstream ostr;
  UnicodeSymbol symbol, symbol_middle(0x10FFFF / 2),
    symbol_last("\xF4\x8F\xBF\xBF");

  std::cout << "Put information:\nText mode\n"
    << symbol << " " << symbol_middle << " " << symbol_last
    << UnicodeSymbol::binary << "\nBinary mode\n"
    << symbol << " " << symbol_middle << " " << symbol_last
    << "\nEnd of information."
    << std::endl;

  ostr << symbol << symbol_middle << symbol_last
    << UnicodeSymbol::binary
    << symbol << symbol_middle << symbol_last;

  std::cout << "Stream content: " << ostr.str() << std::endl;

  const std::string& str = ostr.str();
  Stream::Parser istr(str);

  UnicodeSymbol got_symbol, got_middle_symbol, got_last_symbol;
  UnicodeSymbol got_bin_symbol, got_bin_middle_symbol, got_bin_last_symbol;

  // Read back UnicodeSymbols.
  char buf_for_null[5];
  istr.get(buf_for_null, 5);
  if (strcmp(buf_for_null, "null"))
  {
    throw IOException("Did not put NULL Unicode Stream");
  }

  istr >> UnicodeSymbol::nobinary
    /*>> got_symbol*/ >> got_middle_symbol >> got_last_symbol;
  istr >> UnicodeSymbol::binary;
  istr /*>> got_bin_symbol*/ >> got_bin_middle_symbol >> got_bin_last_symbol;

  if (symbol != got_symbol || symbol != got_bin_symbol ||
    symbol_middle != got_middle_symbol ||
    symbol_middle != got_bin_middle_symbol ||
    symbol_last != got_last_symbol ||
    symbol_last != got_bin_last_symbol)
  {
    std::cout << "Stream contain: " << ostr.str() << std::endl;
    std::cout << "Got information:\nText symbols: " 
      << UnicodeSymbol::nobinary
      << got_symbol << " " << got_middle_symbol << " " << got_last_symbol
      << "\nBinary symbols: " << got_bin_symbol << " " 
      << got_bin_middle_symbol << " " << got_bin_last_symbol << std::endl;
    throw IOException("Unicode symbol binary input/output error");
  }

  if (istr.eof())
  {
    throw IOException("EOF must NOT be reached");
  }
  istr >> got_symbol;
  if (!istr.eof())
  {
    throw IOException("EOF must be reached");
  }
  
}

void
not_trimmed_input_test() throw (eh::Exception, IOException)
{
  Stream::Parser istr("  41");
  UnicodeSymbol symbol;
  istr >> symbol;
  if (symbol != UnicodeSymbol("A"))
  {
    std::cout << symbol << std::endl;
    throw IOException("Spaces isn't scrolled");
  }
}

void
text_format_check() throw (eh::Exception, IOException)
{
  UnicodeSymbol symbol(L'A'), symbol_last("\xF4\x8F\xBF\xBF");
  std::cout << symbol << std::endl;
  symbol = 5;
  std::cout << symbol << std::endl;
  std::cout << std::left << symbol << std::endl;
  std::cout << symbol_last << std::uppercase << symbol_last << std::endl;
}

void
construction_test() throw (eh::Exception)
{
  UnicodeSymbol symbol(L'\0');
  const UnicodeSymbol LAST("\xF4\x8F\xBF\xBF");
  for (; symbol <= LAST; ++symbol)
  {
    try
    {
      UnicodeSymbol new_symbol(symbol.c_str());
    }
    catch (...)
    {
      std::cerr << "Cannot create symbol on well-formed sequence: "
        << symbol << std::endl;
      break; 
    }
  }

  std::string fn = root_path;
  fn += "/String/Utf8/Data/bad_UTF8_octets.txt.bz2";
  try
  {
    Stream::BzlibInStream ifs(fn.c_str());
    char buf[32];
    ifs.getline(buf, 32); // pass through well-formed BOM mark
    while (!ifs.eof())
    {
      ifs.getline(buf, 32);
      if (buf[0] == 0)
      {
        continue;
      }
      try
      {
        UnicodeSymbol new_symbol(buf);
        std::cerr << "Created symbol on ill-formed sequence: "
          << buf << std::endl;
      }
      catch (...)
      {
      }
    }
  }
  catch (const eh::Exception& ex)
  {
    std::cerr << "File " << fn << " open error. "
      << ex.what() << std::endl;
    return;
  }
}

int
main(int /*argc*/, char* /*argv*/[])
{
  try
  {
    std::cout << "UnicodeSymbol test started.." << std::endl;
    char* ev = getenv("TEST_TOP_SRC_DIR");
    root_path = ev ? ev : ".";
    root_path += "/tests";

    construction_test();
    text_format_check();
    not_trimmed_input_test();
    unicode_symbol_test();
    std::cout << "SUCCESS" << std::endl;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "Exception occurred: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception occurred!" << std::endl;
    throw;
  }

  return 0;
}
