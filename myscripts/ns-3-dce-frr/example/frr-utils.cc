#include "frr-utils.h"
#include "ns3/dce-module.h"
#include <string>

namespace ns3 {

  void RunIp (Ptr<Node> node, Time at, std::string str)
  {
    DceApplicationHelper process;
    ApplicationContainer apps;
    process.SetBinary ("ip");
    process.SetStackSize (1 << 16);
    process.ResetArguments ();
    process.ParseArguments (str.c_str ());
    apps = process.Install (node);
    apps.Start (at);
  }  

  void AddAddress (Ptr<Node> node, Time at, const char *name, const char *address)
  {
    std::ostringstream oss;
    oss << "-f inet addr add " << address << " dev " << name;
    RunIp (node, at, oss.str ());
  }

  void AddAdressesList (Ptr<Node> node, std::vector<std::string> adresses, bool ipv6)
  {
    for (int i = 0; i < adresses.size(); i++) {
      char * str;
      asprintf (&str, "sim%d", i);
      AddAddress (node, Seconds (0.1), str, adresses[i].c_str ());
    }
    RunIp (node, Seconds (0.1), "link set lo up");

    for (int i = 0; i < adresses.size(); i++) {
      RunIp (node, Seconds (0.11), "link set sim" + std::to_string(i) + std::string(" up"));
      // Flush Ipv6 addresses so that no ICMPv6 packet is sent
      if (!ipv6) RunIp (node, Seconds (0.12), "-6 address flush dev sim" + std::to_string(i));
    }
  }

  void RunPing (Ptr<Node> node, Time at, std::string str)
  {
    DceApplicationHelper process;
    ApplicationContainer apps;
    process.SetBinary ("ping");
    process.SetStackSize (1 << 16);
    process.ResetArguments ();
    process.ParseArguments (str.c_str ());
    apps = process.Install (node);
    apps.Start (at);
  }
  
}
