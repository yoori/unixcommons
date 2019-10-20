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
#include <memory>

#include <SNMPAgent/SNMPAgentX.hpp>

#include <Logger/StreamLogger.hpp>


typedef SNMPAgentX::ValuesProcessor<Generics::Values> ValuesProcessor;
class Processor : public ValuesProcessor
{
public:
  explicit
  Processor(unsigned id) throw ()
    : ValuesProcessor(id)
  {
  }

  void
  register_ids(SNMPAgentX::GenericSNMPAgent* agent) const
    throw (eh::Exception)
  {
    ValuesProcessor::register_ids(agent);
    if (const SNMPAgentX::GenericSNMPAgent::RootInfo* root =
      agent->get_rootinfo("SeqTable.SeqEntry"))
    {
      unsigned ids[2] = { 777, id_ };
      root->register_index(2, ids);
    }
    else
    {
      std::cerr << "Failed to find SeqTable\n";
    }
    if (const SNMPAgentX::GenericSNMPAgent::RootInfo* root =
      agent->get_rootinfo("Seq2Table.Seq2Entry.Index1"))
    {
      unsigned ids[2] = { 555, id_ };
      root->register_index(2, ids);
    }
    else
    {
      std::cerr << "Failed to find Seq2Table\n";
    }
  }

  bool
  process_variable(void* variable,
    const SNMPAgentX::GenericSNMPAgent::VariableInfo& info,
    unsigned size, const unsigned* ids, const Generics::Values* values) const
    throw (eh::Exception)
  {
#if 0
    const std::string& name = size == 2 ?
      info.name.text() + "::" + info.root->indices[0].find(*ids)->second :
      info.name.text();
    std::cout << name << std::endl;
#else
    std::cout << size << " " << info.name.text() << std::endl;
#endif
    return ValuesProcessor::process_variable(
      variable, info, size, ids, values);
  }
};
typedef SNMPAgentX::SNMPStatsGen<Generics::Values, Processor> SNMPStatsImpl;
typedef ReferenceCounting::FixedPtr<SNMPStatsImpl> FSNMPStatsImpl_var;

int
main(int argc, char* argv[])
{
  try
  {
    Logging::FLogger_var logger(
      new Logging::OStream::Logger(Logging::OStream::Config(std::cout)));
    srand(time(0));
    Generics::Values_var stats(new Generics::Values);
    unsigned long pid = getpid();
    stats->set("PID", pid);
    stats->set("Name", argv[argc > 1 ? 1 : 0]);
    if (rand() & 32)
    {
      stats->set("Random", static_cast<long>(rand() % 21 - 10));
    }
    static const char* NODES[6] =
    {
      "Node1",
      "Node2",
      "Node3",
      "Node1.Node4",
      "Node2.Node5",
      "Node2.Node5.Node6",
    };
    for (int i = 10; i < 30; i++)
    {
      if (rand() & 32)
      {
        Stream::Stack<256> ostr;
        ostr << NODES[(i - 10) % 6] << ".Random" << i;
        stats->set(ostr.str(), static_cast<long>(rand() % 21 - 10));
      }
    }
    stats->set("Data2.one", 111l);
    stats->set("Data2.two", 222l);
    stats->set("Data3.two", 332l);
    stats->set("Data3.ugarwx", 666l);
    stats->set("Data4.one", 411l);

    stats->set("Data23.ten", 2310l);
    stats->set("Data24.eleven", 2411l);

    const char* src = getenv("TEST_SRC_DIR");
    const char* top_src = getenv("TEST_TOP_SRC_DIR");

    for (int i = 0; i < 1; i++)
    {
      std::cout << "\n\n" << i << "\n\n";
      FSNMPStatsImpl_var snmp(
        new SNMPStatsImpl(stats.in(), pid, logger.in(), "Test",
          "SNMPSimple-MIB:SNMPSimple",
          (std::string("/usr/share/snmp/mibs:") +
            std::string(top_src ? top_src : ".") +
            "/share/snmp/mibs:" +
            (src ? src : "tests/SNMP/SNMPSimple")).c_str(),
            argc > 2 ? argv[2] : 0));

      std::cout << "..." << std::endl;

      std::cin.get();
    }

    return 0;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "main(): " << e.what() << std::endl;    
  }
  catch (...)
  {
    std::cerr << "main(): unknown exception caught" << std::endl;
  }

  return 1;
}
