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





#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include <Generics/FileCache.hpp>

#include <String/TextTemplate.hpp>


using namespace String;

class CallBack : public TextTemplate::ArgsCallback
{
public:
  CallBack(const char* click_url, const char* ad_image) throw ();
  ~CallBack() throw ();

  std::string
  get_argument(const char* key) const
    throw (TextTemplate::UnknownName, eh::Exception);

private:
  std::string click_url_;
  std::string ad_image_;
};

CallBack::CallBack(const char* click_url, const char* ad_image) throw ()
  : click_url_(click_url ? click_url : ""),
    ad_image_(ad_image ? ad_image : "")
{
  std::cout << "CallBack:\n" << "  click_url: " << click_url << std::endl <<
    "  ad_image: " << ad_image << std::endl << std::endl;
}

CallBack::~CallBack() throw ()
{
}

std::string
CallBack::get_argument(const char* key) const
  throw (TextTemplate::UnknownName, eh::Exception)
{
  if (!key)
  {
    throw TextTemplate::UnknownName("CallBack::get_argument(): invalid key.");
  }

  if (strcmp(key, "CLICKURL") == 0)
  {
    return click_url_;
  }
  else if (strcmp(key, "ADIMAGE") == 0)
  {
    return ad_image_;
  }
  else
  {
    Stream::Error ostr;
    ostr << "CallBack::get_argument(): unknown key '" << key << "'";
    throw TextTemplate::UnknownName(ostr);
  }
}

class TestTextTemplateUpdateStrategy : public TextTemplate::UpdateStrategy
{
public:
  TestTextTemplateUpdateStrategy(const char* fname) throw (eh::Exception);

  virtual
  ~TestTextTemplateUpdateStrategy() throw ();

  virtual String::SubString
  start_lexeme() const throw (eh::Exception);
  virtual String::SubString
  end_lexeme() const throw (eh::Exception);
};


int
main(int argc, char* argv[])
{
  if(argc < 2)
  {
    std::cerr << "Usage:\n" << argv[0] <<
      " filename [iterations] [keys_filename]\n";
    return 1;
  }

  const char* file_name = argv[1];

  unsigned long iterations = 0;

  std::string output;

  if (argc > 2)
  {
    iterations = atol(argv[2]);
  }

  std::cout << "Processing " << file_name << " ...\n\n";

  typedef Generics::FileCacheManager<TestTextTemplateUpdateStrategy>
    TextTemplateCacheManager;

  try
  {
    TextTemplateCacheManager manager;

#if 0
    CallBack callback("http://upsa.ocslab.com/bugzilla/",
                      "bugzilla/image.jpg");
#else
    //TextTemplateArgs callback;
    TextTemplate::Args callback(true, 200, true,
      TextTemplate::Args::EI_JS_UNICODE);
    if (argc > 3)
    {
      std::ifstream file(argv[3]);
      while (file)
      {
        std::string str;
        std::getline(file, str);
        switch (std::string::size_type pos = str.find('='))
        {
        case 0:
          break;
        case std::string::npos:
          callback[str] = std::string();
          break;
        default:
          callback[str.substr(0, pos)] = str.substr(pos + 1);
          break;
        }
      }
    }
#endif

    if (argc > 4)
    {
      std::ifstream in(argv[4]);
      std::getline(in, output, '\0');
    }

    for (unsigned int i = 0; iterations ? (i < iterations) : true; i++)
    {
      TextTemplateCacheManager::BufferHolder_var text_template =
        manager.get(file_name);

      std::cout << "Instantiating template (" << i << "):\n";
      {
        TextTemplate::Keys keys;
        (*text_template)->keys(callback, keys);
        std::cout << "Keys:";
        for (TextTemplate::Keys::const_iterator itor(keys.begin());
          itor != keys.end(); ++itor)
        {
          std::cout << " " << *itor;
        }
        std::cout << std::endl;
      }
      std::string out((*text_template)->instantiate(callback));
      std::cout << out << std::endl << std::endl;

      if (output.length() && out != output)
      {
        std::cerr << "Unexpected result of template instantiation" <<
          std::endl;
      }

      sleep(1);
    }
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "main: eh::Exception caught. Description:" << std::endl <<
      e.what() << std::endl;
    return -1;
  }
  catch (...)
  {
    std::cerr << "main: Unknown Exception caught\n";
    return -1;
  }

  return 0;
}

//
// TestTextTemplateUpdateStrategy class
//

TestTextTemplateUpdateStrategy::TestTextTemplateUpdateStrategy(
  const char* fname) throw (eh::Exception)
  : TextTemplate::UpdateStrategy(fname)
{
}

String::SubString
TestTextTemplateUpdateStrategy::start_lexeme() const throw (eh::Exception)
{
  return String::TextTemplate::Basic::DEFAULT_LEXEME;
}

String::SubString
TestTextTemplateUpdateStrategy::end_lexeme() const throw (eh::Exception)
{
  return String::TextTemplate::Basic::DEFAULT_LEXEME;
}

TestTextTemplateUpdateStrategy::~TestTextTemplateUpdateStrategy() throw ()
{
}
