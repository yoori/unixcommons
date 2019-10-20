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



// IsProperty.cpp : Defines the entry point for the console application.
//

#include <memory>
#include <set>

#include <eh/Exception.hpp>

#include <String/AsciiStringManip.hpp>
#include <String/UTF8IsProperty.hpp>
#include <String/UTF8AllProperties.hpp>

#include "../Common/UTF8TreeLoader.hpp"
#include "../Common/UTF8CategoryPrint.hpp"

DECLARE_EXCEPTION(TestException, eh::DescriptiveException);

using namespace String;
using namespace UnicodeProperty;

typedef bool (*IsFunction)(const char*);

struct PropertyDescription
{
  const char* type;
  IsFunction function;
  CODE_UNIT_PROPERTY mask_value;
};

const PropertyDescription PROPERTIES[] =
{
  { "space", String::is_space, CUP_SPACE },
  { "digit", String::is_digit, CUP_DIGIT },
  { "letter", String::is_letter, CUP_LETTER },
  { "letter_lower", String::is_lower_letter, CUP_LOWER_LETTER },
  { "letter_title", String::is_title_letter, CUP_TITLE_LETTER },
  { "letter_upper", String::is_upper_letter, CUP_UPPER_LETTER },
};

struct TestAllProperties : public AllProperties
{
  TestAllProperties(const AllProperties& val) throw ();

  uint8_t
  value() const throw ();
};

const int NUMBER_OF_PROPERTIES = sizeof(PROPERTIES) / sizeof(*PROPERTIES);

struct DynamicTrees
{
  typedef std::set<UnicodeSymbol> Utf8PropertiesDictionary;

  struct IsProperty
  {
    Utf8PropertiesDictionary set;
    String::Utf8Set::Utf8Chars chars;
    std::unique_ptr<String::Utf8Category> category;

    void
    insert(const UnicodeSymbol& first, const UnicodeSymbol& second)
      throw (eh::Exception);
  };

  IsProperty properties[NUMBER_OF_PROPERTIES];
  IsProperty all_properties;
  typedef std::map<UnicodeSymbol, uint8_t> SymbolProperties;
  SymbolProperties add_info;

  DynamicTrees() throw (eh::Exception);

  void
  load_data(const char* filename, IsProperty& tree)
    throw (eh::Exception);

  void
  generate_all_properties_map() throw (eh::Exception);

  /**
   * Fill add_info
   */
  void
  load_extended_data() throw (eh::Exception);

};

std::unique_ptr<DynamicTrees> dynamic_trees;

class TestContext
{
public:

  TestContext() throw ();

  void
  check_reference(const char* name, bool result)
    throw (eh::Exception);

  void
  property_check(const DynamicTrees::IsProperty& property,
    IsFunction is_property, const char* name)
    throw (eh::Exception);

  void
  set_all_checks_mode(bool new_value) throw ();

  void
  set_symbol(const UnicodeSymbol& new_symbol) throw ();
private:
  UnicodeSymbol symbol_;
  std::string operation_;
  bool do_all_checks_;

  bool reference_value_;
} test_context;

const std::string&
get_root_path() throw (eh::Exception);

//////////////////////////////////////////////////////////////////////////
//  Implementations
//////////////////////////////////////////////////////////////////////////

TestAllProperties::TestAllProperties(const AllProperties& val) throw ()
  : AllProperties(val)
{
}

uint8_t
TestAllProperties::value() const throw ()
{
  return cumulative_value_;
}

const std::string&
get_root_path() throw (eh::Exception)
{
  static std::string root_path;
  if (root_path.empty())
  {
    char* ev = getenv("TEST_TOP_SRC_DIR");
    root_path = ev ? ev : ".";
    root_path += "/tests/String/Utf8/Data";
  }
  return root_path;
}


TestContext::TestContext() throw () : do_all_checks_(false)
{
}

void
TestContext::check_reference(const char* name, bool result)
  throw (eh::Exception)
{
  if (result != reference_value_)
  {
    std::cerr << test_context.operation_ << " FAILED on " <<
      test_context.symbol_ << " " << name << "=" << result <<
      " but reference = " << reference_value_ << std::endl;
  }
}

void
TestContext::property_check(const DynamicTrees::IsProperty& property,
  IsFunction is_property, const char* name)
  throw (eh::Exception)
{
  const char RESULT_ON_STATIC_TREE[] = "algorithm on function call";
  const char RESULT_ON_COMPRESSED_SET[] = "algorithm on compressed set";
  const char RESULT_ON_CATEGORY[] = "algorithm on category";

  operation_ = name;

  reference_value_ = is_property(symbol_.c_str());

  check_reference(RESULT_ON_STATIC_TREE,
    property.set.find(symbol_) != property.set.end());

  if (do_all_checks_)
  {
    check_reference(RESULT_ON_COMPRESSED_SET,
      property.chars.belongs(String::Utf8Set::get_char(symbol_.c_str())));
  }

  check_reference(RESULT_ON_CATEGORY,
    property.category->is_owned(symbol_.c_str()));
}

void
TestContext::set_all_checks_mode(bool new_value) throw ()
{
  do_all_checks_ = new_value;
}

void
TestContext::set_symbol(const UnicodeSymbol& new_symbol) throw ()
{
  symbol_ = new_symbol;
}

void
DynamicTrees::IsProperty::insert(const UnicodeSymbol& first,
  const UnicodeSymbol& second) throw (eh::Exception)
{
  String::Utf8Set::add_symbols(chars, first.c_str(), second.c_str());
  for (UnicodeSymbol octet = first; octet != second; ++octet)
  {
    set.insert(octet);
  }
  set.insert(second);
}

void
DynamicTrees::load_data(const char* filename, IsProperty& property)
  throw (eh::Exception)
{
  Utf8Loading::load_properties(filename, property);

  property.category.reset(new String::Utf8Category(property.chars));
}

void
DynamicTrees::load_extended_data() throw (eh::Exception)
{
  for (int i = 0; i < NUMBER_OF_PROPERTIES; ++i)
  {
    all_properties.chars.add(properties[i].chars);
  }
  all_properties.category.reset(
    new String::Utf8Category(all_properties.chars));

  const UnicodeSymbol LAST("\xF4\x8F\xBF\xBF");
  for (UnicodeSymbol symbol(L'\0');
    symbol <= LAST; ++symbol)
  {
    if (!all_properties.category->is_owned(symbol.c_str()))
    {
      continue;
    }
    uint8_t all_properties = 0;
    for (int i = 0; i < NUMBER_OF_PROPERTIES; i++)
    {
      // Calculate cumulative mask for all symbol properties
      if (properties[i].category->is_owned(symbol.c_str()))
      {
        all_properties |= PROPERTIES[i].mask_value;
      }
    }
    add_info.insert(SymbolProperties::value_type(symbol, all_properties));
  }
}

DynamicTrees::DynamicTrees() throw (eh::Exception)
{
  for (int i = 0; i < NUMBER_OF_PROPERTIES; ++i)
  {
    load_data((get_root_path() + "/" + PROPERTIES[i].type + ".txt").c_str(),
      properties[i]);
  }
}

void
is_subsets_test() throw (TestException, eh::Exception)
{
  using namespace String;

  const UnicodeSymbol LAST("\xF4\x8F\xBF\xBF");
  for (UnicodeSymbol symbol(L'\0');
     symbol <= LAST; ++symbol)
  {
    test_context.set_symbol(symbol);
    uint8_t all_properties = 0;
    for (int i = 0; i < NUMBER_OF_PROPERTIES; i++)
    {
      test_context.property_check(dynamic_trees->properties[i],
        PROPERTIES[i].function, PROPERTIES[i].type);
      // Calculate cumulative mask for all symbol properties
      if (PROPERTIES[i].function(symbol.c_str()))
      {
        all_properties |= PROPERTIES[i].mask_value;
      }
    }
    TestAllProperties val(String::all_properties(symbol.c_str()));
    // and check get_properties function
    if (val.value() != all_properties)
    {
      std::cerr << "Symbol: " << symbol <<
        ", String::is_* = " << std::hex <<
        static_cast<int>(all_properties) << ", String::all_properties = " <<
        std::hex << static_cast<int>(val.value()) << std::endl;
    }
  }
}

void
generate_source() throw (eh::Exception)
{
  for (int i = 0; i < NUMBER_OF_PROPERTIES; i++)
  {
    const char* type = PROPERTIES[i].type;
    std::cout << "/////////////////////////////////////"
      << "/////////////////////////////////////" << std::endl
      << "// Static N-arc tree definition for is_" << type << " property"
      << std::endl << std::endl;
    Utf8CategoryPrintable printable_category;
    dynamic_trees->properties[i].category->swap(printable_category);
    printable_category.print_to_cpp(type);
  }
}

void
generate_all_properties_source() throw (eh::Exception)
{
  const char* type = "all_properties";

  std::cout << std::endl << std::endl;
  Utf8CategoryExtendedPrintable printable_category(dynamic_trees->add_info);
  dynamic_trees->load_extended_data();
  dynamic_trees->all_properties.category->swap(printable_category);
  printable_category.print_to_cpp(type);
  printable_category.print_finishers_to_cpp();
}

int
main(int argc, char* argv[])
{
  try
  {
    dynamic_trees.reset(new DynamicTrees);

    if (argc > 1)
    {
      if (!strcmp(argv[1], "gen"))
      {
        generate_source();
        return 0;
      }

      if (!strcmp(argv[1], "gen_all"))
      {
        generate_all_properties_source();
        return 0;
      }

      // Set this flag into true only if you need test new UTF8 subset test.
      if (!strcmp(argv[1], "all"))
      {
        test_context.set_all_checks_mode(true);
      }
    }

    std::cout << "IsProperty test started.." << std::endl;
    is_subsets_test();
    std::cout << "SUCCESS" << std::endl;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "Exception occurred: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "Unknown exception occurred!" << std::endl;
  }

  return 0;
}
