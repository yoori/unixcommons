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
#include <sstream>
#include <fstream>

#include <unistd.h>

#include <String/StringManip.hpp>

#include <Generics/Time.hpp>
#include <String/AsciiStringManip.hpp>
#include <Generics/AppUtils.hpp>

#include <Language/GenericSegmentor/Polyglot.hpp>
//#include <Language/ChineeseSegmentor/NLPIR.hpp>
//#include <Language/JapaneseSegmentor/Mecab.hpp>
//#include <Language/KoreanSegmentor/Klt.hpp>
//#include <Language/KoreanSegmentor/Moran.hpp>
#include <Language/SegmentorManager/SegmentorManager.hpp>
#include <Language/BLogic/NormalizeTrigger.hpp>

#include "Application.hpp"

using namespace Language::Segmentor;

namespace
{
  const char USAGE[] =
    "[OPTIONS] ( help | parse-input | parse-lines | put-spaces TEXT | "
      "segment TEXT )\n"
    "OPTIONS:\n";
}

std::ostream&
print_mime(const String::SubString& str, std::ostream& out)
  throw (eh::Exception)
{
  std::string res;
  String::StringManip::mime_url_encode(str, res);
  out << res;
  return out;
}

std::ostream&
print_bin(const String::SubString& str, std::ostream& out) throw (eh::Exception)
{
  for (const char* cur = str.begin(), * const END = str.end(); cur != END;)
  {
    if (cur != str.begin())
    {
      out << ", ";
    }

    unsigned long octets;
    if (!String::UTF8Handler::is_correct_utf8_sequence(cur, octets) ||
      static_cast<size_t>(END - cur) < octets)
    {
      std::cout << "??? (" <<
        static_cast<int>(static_cast<unsigned char>(*cur++)) << ")";
      continue;
    }

    char buf[7];
    memcpy(buf, cur, octets);
    buf[octets] = '\0';
    out << buf << "(";
    for (unsigned long i = 0; i < octets; i++)
    {
      if (i)
      {
        out << ' ';
      }
      out << static_cast<int>(static_cast<unsigned char>(*cur++));
    }
    out << ")";
  }

  return out;
}

void
put_spaces_wrap(
  std::string& res,
  String::SubString src,
  Language::Segmentor::SegmentorInterface* segmentor,
  bool normalize)
{
  if(normalize)
  {
    Language::Trigger::normalize_phrase(src, res, segmentor);
  }
  else
  {
    segmentor->put_spaces(res, src.data(), src.length());
  }
}

void
segment_wrap(
  Language::Segmentor::WordsList& res,
  Language::Segmentor::SegmentorInterface& segmentor,
  String::SubString str,
  bool normalize)
{
  if(normalize)
  {
    std::string nstr;
    Language::Trigger::normalize_phrase(str, nstr, 0);
    segmentor.segmentation(res, nstr.data(), nstr.size());
  }
  else
  {
    segmentor.segmentation(res, str.data(), str.length());
  }
}

void put_spaces_i(
  std::ostream& out,
  Language::Segmentor::SegmentorInterface& ling_server,
  const String::SubString& istr,
  bool normalize)
{
  std::string str;
  //String::StringManip::trim(t);
  out << "t: " << istr.size() << std::endl;

  put_spaces_wrap(str, istr, &ling_server, normalize);

  out
    << "  in string: '" << istr << "'" << std::endl
    << "  in binary view: ";
  print_bin(istr, out);
  out
    << std::endl
    << "  out string: '" << str << "'" << std::endl
    << "  out mime view: '";
  print_mime(str, out);
  out
    << "'" << std::endl
    << "  out binary view: ";
  print_bin(str, out);
  out << std::endl;
  out << "  out size: " << str.size() << std::endl;
  out << "  input and output are " <<
    (str == istr ? "equal" : "non equal") << std::endl;
}

void segment_i(
  Language::Segmentor::SegmentorInterface& ling_server,
  String::SubString str,
  std::ostream& out,
  bool normalize)
{
  Language::Segmentor::WordsList res;
  segment_wrap(res, ling_server, str, normalize);

  size_t i = 0;
  for(Language::Segmentor::WordsList::const_iterator it =
        res.begin();
      it != res.end(); ++it, ++i)
  {
    out << "  " << i << ": '" << *it << "'" << std::endl;
    out << "    ";
    print_mime(*it, out);
    out << std::endl;
  }
}

void Application::run(int argc, char* argv[]) throw(Exception, eh::Exception)
{
  try
  {
    typedef Generics::AppUtils::Args::CommandList CommandList;

    Generics::AppUtils::Option<unsigned long> opt_count;
    Generics::AppUtils::Option<unsigned long> opt_sleep(0);

    //Generics::AppUtils::StringOption opt_klt_ini("/opt/KLT/hdic/KLT2000.ini");
    //Generics::AppUtils::StringOption opt_moran_ini("/opt/Moran/dic/moran.dbs");
    //Generics::AppUtils::StringOption opt_mecab_ini("/usr/etc/mecabrc");
    //Generics::AppUtils::StringOption opt_nlpir_ini("/usr/share/NLPIR");
    Generics::AppUtils::StringOption opt_gen_ini("/opt/oix/polyglot/dict/");

    /*
    Generics::AppUtils::CheckOption opt_klt;
    Generics::AppUtils::CheckOption opt_moran;
    Generics::AppUtils::CheckOption opt_mecab;
    Generics::AppUtils::CheckOption opt_nlpir;
    */
    Generics::AppUtils::CheckOption opt_gen;
    Generics::AppUtils::CheckOption opt_gen_norm;

    Generics::AppUtils::CheckOption opt_help;
    Generics::AppUtils::CheckOption opt_input_mime;
    Generics::AppUtils::CheckOption opt_output_mime;
    Generics::AppUtils::CheckOption opt_ini_time;
    Generics::AppUtils::CheckOption opt_normalize;

    Generics::AppUtils::Args args(-1);

    args.add(
      Generics::AppUtils::equal_name("sleep") ||
      Generics::AppUtils::short_name("s"),
      opt_sleep, "Sleep before action", "seconds");
    args.add(
      Generics::AppUtils::equal_name("count") ||
      Generics::AppUtils::short_name("c"),
      opt_count, "Perform action several times", "number");
    args.add(
      Generics::AppUtils::equal_name("ini-time") ||
      Generics::AppUtils::short_name("t"),
      opt_ini_time, "Print out initialization time");
    args.add(
      Generics::AppUtils::equal_name("help") ||
      Generics::AppUtils::short_name("h"),
      opt_help, "Print out help");
    args.add(
      Generics::AppUtils::equal_name("mime") ||
      Generics::AppUtils::short_name("m"),
      opt_input_mime, "Perform mime decoding on input first");
    args.add(
      Generics::AppUtils::equal_name("mime-out") ||
      Generics::AppUtils::short_name("mo"),
      opt_output_mime, "Perform mime encoding on text output");

    /*
    args.add(
      Generics::AppUtils::equal_name("klt") ||
      Generics::AppUtils::short_name("k"),
      opt_klt, "Use KLT");
    args.add(
      Generics::AppUtils::equal_name("moran") ||
      Generics::AppUtils::short_name("mn"),
      opt_moran, "Use Moran");
    args.add(
      Generics::AppUtils::equal_name("mecab") ||
      Generics::AppUtils::short_name("mb"),
      opt_mecab, "Use Mecab");
    args.add(
      Generics::AppUtils::equal_name("nlpir") ||
      Generics::AppUtils::short_name("nl"),
      opt_nlpir, "Use NLPIR");
    */
    args.add(
      Generics::AppUtils::equal_name("gen") ||
      Generics::AppUtils::short_name("g"),
      opt_gen, "Use Polyglot");
    args.add(
      Generics::AppUtils::equal_name("gen-norm") ||
      Generics::AppUtils::short_name("gn"),
      opt_gen_norm, "Use Normalized Polyglot");
    args.add(
      Generics::AppUtils::equal_name("norm") ||
      Generics::AppUtils::short_name("n"),
      opt_normalize, "Normalize trigger");

    /*
    args.add(
      Generics::AppUtils::equal_name("klt-ini"),
      opt_klt_ini, "Path to KLT initialization file", "filename");
    args.add(
      Generics::AppUtils::equal_name("moran-ini"),
      opt_moran_ini, "Path to Moran initialization file", "filename");
    args.add(
      Generics::AppUtils::equal_name("mecab-ini"),
      opt_mecab_ini, "Path to Mecab initialization file", "filename");
    args.add(
      Generics::AppUtils::equal_name("nlpir-ini"),
      opt_nlpir_ini, "Path to NLPIR initialization directory", "directory");
    */
    args.add(
      Generics::AppUtils::equal_name("gen-ini"),
      opt_gen_ini, "Path to Polyglot initialization file", "filename");

    args.parse(argc - 1, argv + 1);

    const CommandList& commands = args.commands();

    if (commands.empty() || opt_help.enabled())
    {
      args.usage(std::cout << USAGE);
      return;
    }

    std::string command = *commands.begin();

    /* init segmentor map */
    Language::Segmentor::CompositeSegmentor_var composite_segmentor(
      new Language::Segmentor::CompositeSegmentor());

    {
      Generics::CPUTimer ini_timer;

      if (opt_ini_time.enabled())
      {
        ini_timer.start();
      }

      /*
      if (opt_klt.enabled())
      {
        composite_segmentor->add_segmentor(
          Language::Segmentor::SegmentorInterface_var(
            new Language::Segmentor::Korean::KltSegmentor(
              opt_klt_ini->c_str(), "")));
      }

      if (opt_moran.enabled())
      {
        composite_segmentor->add_segmentor(
          Language::Segmentor::SegmentorInterface_var(
            new Language::Segmentor::Korean::MoranSegmentor(  
              opt_moran_ini->c_str())));
      }

      if (opt_mecab.enabled())
      {
        composite_segmentor->add_segmentor(
          Language::Segmentor::SegmentorInterface_var(
            new Language::Segmentor::Japanese::MecabSegmentor(
              opt_mecab_ini->c_str())));
      }

      if (opt_nlpir.enabled())
      {
        composite_segmentor->add_segmentor(
          Language::Segmentor::SegmentorInterface_var(
            new Language::Segmentor::Chineese::NlpirSegmentor(
              opt_nlpir_ini->c_str())));
      }
      */

      if (opt_gen.enabled())
      {
        composite_segmentor->add_segmentor(
          Language::Segmentor::SegmentorInterface_var(
            new Language::Segmentor::PolyglotSegmentor(
              opt_gen_ini->c_str())));
      }

      if (opt_gen_norm.enabled())
      {
        composite_segmentor->add_segmentor(
          Language::Segmentor::SegmentorInterface_var(
            new Language::Segmentor::NormalizePolyglotSegmentor(
              opt_gen_ini->c_str())));
      }

      if (opt_ini_time.enabled())
      {
        ini_timer.stop();

        std::cout << "Initialization time: " << ini_timer.elapsed_time() <<
          std::endl << std::endl;
      }
    }

    std::string input_text;
    CommandList::const_iterator first = ++commands.begin();
    for (CommandList::const_iterator it = first; it != commands.end(); ++it)
    {
      if (it != first)
      {
        input_text += " ";
      }

      input_text += *it;
    }

    if (opt_input_mime.enabled())
    {
      std::string in_s;
      String::StringManip::mime_url_decode(input_text, in_s);
      input_text.swap(in_s);
    }

    if (*opt_sleep)
    {
      ::sleep(*opt_sleep);
    }

    Generics::CPUTimer timer;

    if (command == "parse-input")
    {
      std::string str, line;
      while (std::getline(std::cin, line))
      {
        str += line;
        str += '\n';
      }

      std::string res_str;
      put_spaces_wrap(res_str, str, composite_segmentor, opt_normalize.enabled());

      if(opt_output_mime.enabled())
      {
        std::string mime_res_str;
        String::StringManip::mime_url_encode(res_str, mime_res_str);
        mime_res_str.swap(res_str);
      }
      std::cout << res_str;

      if (opt_count.installed())
      {
        timer.start();

        for (unsigned long i = 0; i < *opt_count; ++i)
        {
          std::string tmp;
          put_spaces_wrap(tmp, str, composite_segmentor, opt_normalize.enabled());
        }

        timer.stop();
      }
    }
    else if (command == "parse-lines")
    {
      std::string line;
      while (std::getline(std::cin, line))
      {
        std::string res;
        put_spaces_wrap(res, line, composite_segmentor, opt_normalize.enabled());

        if(opt_output_mime.enabled())
        {
          std::string mime_res_str;
          String::StringManip::mime_url_encode(res, mime_res_str);
          mime_res_str.swap(res);
        }
        std::cout << res << std::endl;

        if (opt_count.installed())
        {
          timer.start();

          for (unsigned long i = 0; i < *opt_count; ++i)
          {
            std::string tmp;
            put_spaces_wrap(tmp, line, composite_segmentor, opt_normalize.enabled());
          }

          timer.stop();
        }
      }
    }
    else if (command == "put-spaces")
    {
      std::cout << " result:" << std::endl;
      put_spaces_i(
        std::cout,
        *composite_segmentor,
        input_text,
        opt_normalize.enabled());

      if (opt_count.installed())
      {
        timer.start();

        for (unsigned long i = 0; i < *opt_count; ++i)
        {
          std::string tmp;
          put_spaces_wrap(tmp, input_text, composite_segmentor, opt_normalize.enabled());
        }

        timer.stop();
      }
    }
    else if (command == "segment")
    {
      std::cout << "segment result for string:" << std::endl;
      segment_i(*composite_segmentor, input_text, std::cout, opt_normalize.enabled());

      if (opt_count.installed())
      {
        timer.start();

        for (unsigned long i = 0; i < *opt_count; ++i)
        {
          Language::Segmentor::WordsList res;
          segment_wrap(res, *composite_segmentor, input_text, opt_normalize.enabled());
        }

        timer.stop();
      }
    }
    else
    {
      Stream::Error ostr;
      ostr << "Unknown command: " << command;
      throw Exception(ostr);
    }

    if (opt_count.installed())
    {
      std::cout << "Performance result - average time: " <<
        timer.elapsed_time() / *opt_count << std::endl;
    }
  }
  catch (const eh::Exception& ex)
  {
    Stream::Error ostr;
    ostr << FNS << "eh::Exception caught: " << ex.what();
    throw Exception(ostr);
  }
}

int
main(int argc, char** argv)
{
  try
  {
    Application app;

    app.run(argc, argv);

    return 0;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << FNS << "eh::Exception exception caught: " << e.what() <<
      std::endl;
  }
  catch (...)
  {
    std::cerr << FNS << "unknown exception caught" << std::endl;
  }

  return -1;
}
