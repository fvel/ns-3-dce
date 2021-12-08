/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Hajime Tazaki, NICT
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Hajime Tazaki <tazaki@nict.go.jp>
 */

#include "ns3/object-factory.h"
#include "frr-helper.h"
#include "ns3/names.h"
#include "ns3/ipv4-l3-protocol.h"
#include <fstream>
#include <sys/stat.h>
#include "ns3/log.h"
#include <arpa/inet.h>
#include <iostream>
#include <string>


NS_LOG_COMPONENT_DEFINE ("FrrHelper");

namespace ns3 {

class FrrConfig : public Object
{
private:
  static int index;
  std::string router_id;
  std::map<std::string, uint32_t> *networks;
public:
  FrrConfig ()
    : m_zebradebug (false),
      m_usemanualconf (false)
  {
    m_radvd_if = new std::map<std::string, std::string> ();
    m_haflag_if = new std::vector<std::string> ();
  }
  ~FrrConfig ()
  {
  }

  static TypeId
  GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::FrrConfig")
      .SetParent<Object> ()
      .AddConstructor<FrrConfig> ()
    ;
    return tid;
  }
  TypeId
  GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  void
  SetFilename (const std::string &filename)
  {
    m_filename = filename;
  }

  std::string
  GetFilename () const
  {
    return m_filename;
  }

  bool m_zebradebug;
  bool m_usemanualconf;
  std::map<std::string, std::string> *m_radvd_if;
  std::vector<std::string> *m_haflag_if;
  std::string m_filename;
  std::vector<uint32_t> iflist;

  virtual void
  Print (std::ostream& os) const
  {
    os << "hostname zebra" << std::endl
       << "password zebra" << std::endl
       << "log stdout" << std::endl;
  }
};
std::ostream& operator << (std::ostream& os, FrrConfig const& config)
{
  config.Print (os);
  return os;
}

class BgpConfig : public Object
{
public:
  std::string m_givenconfig;
  int m_asn;
  int m_num_tor;
  int m_num_fabric;
  int m_nodeid;
  bool m_only_bgp;
  std::vector<std::string> *m_ips;
  std::string m_network;
  Time stop_time;
  Time delayed_time;

  BgpConfig ()
  {
    m_givenconfig = "";
    m_network = "";
    m_asn = 0;
    m_only_bgp = false;
    stop_time = Time ();
    delayed_time = Time ();
  }
  ~BgpConfig () {}

  static TypeId
  GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::BgpConfig")
      .SetParent<Object> ()
      .AddConstructor<BgpConfig> ()
    ;
    return tid;
  }
  TypeId
  GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  virtual void
  Print (std::ostream& os) const
  {
    int idx = m_num_fabric ? 0 : 1;
    std::vector<std::string> *neighbors_ips;
    if (m_only_bgp) {
      neighbors_ips = new std::vector<std::string> ();
      for (int i = 0; i < m_ips->size(); i++) {
        std::string temp = m_ips->at(i);
        temp.erase(temp.length()-3,3);
          
        std::stringstream test(temp);
        std::string last;
        while(std::getline(test, last, '.')) {}
        
        int num = std::stoi( last );
        num = num % 2 ? ++num : --num;
        temp.erase(temp.length()-last.length(),last.length());
        temp += std::to_string(num);
          
        neighbors_ips->push_back(temp);
      }
    }

    os << "frr version 7.7-dev-frr-ns3-dce" << std::endl
       << "frr defaults datacenter" << std::endl
       << "!" << std::endl
       << "log stdout" << std::endl
       << "!" << std::endl
       << "debug bgp updates in" << std::endl
       << "debug bgp updates out" << std::endl
       << "!" << std::endl;

    if (m_asn) {
    os << "ip prefix-list DC_LOCAL_SUBNET seq 5 permit 10.0.0.0/8 le 30" << std::endl 
       << "ip prefix-list DC_LOCAL_SUBNET seq 10 permit 200.0.0.0/8 le 24" << std::endl
       << "route-map ACCEPT_DC_LOCAL permit 10" << std::endl
       << " match ip address prefix-list DC_LOCAL_SUBNET" << std::endl
       << "!" << std::endl
       << "router bgp " << m_asn << std::endl
       << " timers bgp 3 9" << std::endl
       << " bgp router-id 192.168." << m_nodeid / 256 << "." << m_nodeid % 256 << std::endl
       << " no bgp ebgp-requires-policy" << std::endl
       << " bgp bestpath as-path multipath-relax" << std::endl
       << " bgp bestpath compare-routerid" << std::endl
       << "!" << std::endl;

      if (m_num_tor) // Leaf or Spine
      {
        os << "neighbor TOR peer-group" << std::endl
           << " neighbor TOR remote-as external" << std::endl
           << " neighbor TOR advertisement-interval 0" << std::endl
           << " neighbor TOR timers connect 5" << std::endl;
        if (!m_num_fabric) // Leaf
        {
          for (int i = 0; i < m_num_tor; i++)
          {
            if (m_only_bgp)
              os << " neighbor " << neighbors_ips->at (idx++) << " peer-group TOR" << std::endl;
            else
              os << " neighbor sim" << i + 1 << " interface peer-group TOR" << std::endl;
          }
        }
        os << "!" << std::endl;
      }

      if (m_num_fabric) // Spine or Tof
      {
        os << "neighbor fabric peer-group" << std::endl
           << " neighbor fabric remote-as external" << std::endl
           << " neighbor fabric advertisement-interval 0" << std::endl
           << " neighbor fabric timers connect 5" << std::endl;
        for (int i = 0; i < m_num_tor; i++)
        {
          if (m_only_bgp)
            os << " neighbor " << neighbors_ips->at (idx++) << " peer-group TOR" << std::endl;
          else
            os << " neighbor sim" << i << " interface peer-group TOR" << std::endl;
        }
        for (int j = m_num_tor; j < m_num_fabric + m_num_tor; j++)
        {
          if (m_only_bgp)
            os << " neighbor " << neighbors_ips->at (idx++) << " peer-group fabric" << std::endl;
          else
            os << " neighbor sim" << j << " interface peer-group fabric" << std::endl;
        }      
      }

      os << "address-family ipv4 unicast" << std::endl;
      if (m_num_fabric) // Spine of Tof
        os << " neighbor fabric activate" << std::endl;
      if (m_num_tor) // Leaf or Spine
        os << " neighbor TOR activate" << std::endl;
      if (!m_num_fabric) // Leaf
        os << " network " << m_network << std::endl;
      os << " maximum-paths 64" << std::endl
         << "exit-address-family" << std::endl;
    }
  }
};

class OspfConfig : public Object
{
private:
  std::string router_id;
  std::map<std::string, uint32_t> *networks;
public:
  OspfConfig ()
    : m_ospfdebug (false)
  {
    networks = new std::map<std::string, uint32_t> ();
    iflist = new std::vector<uint32_t> ();
  }
  ~OspfConfig ()
  {
    delete networks;
    delete iflist;
  }

  bool m_ospfdebug;

  static TypeId
  GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::OspfConfig")
      .SetParent<Object> ()
      .AddConstructor<OspfConfig> ()
    ;
    return tid;
  }
  TypeId
  GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  void
  addNetwork (std::string prefix, uint32_t area)
  {
    networks->insert (std::map<std::string, uint32_t>::value_type (prefix, area));
  }

  void
  SetRouterId (const char * router_id)
  {
    this->router_id = std::string (router_id);
  }

  void
  SetFilename (const std::string &filename)
  {
    m_filename = filename;
  }

  std::string
  GetFilename () const
  {
    return m_filename;
  }

  virtual void
  Print (std::ostream& os) const
  {
    os << "hostname zebra" << std::endl
       << "password zebra" << std::endl
       << "log stdout" << std::endl;
    if (m_ospfdebug)
      {
        os << "debug ospf event " << std::endl;
        os << "debug ospf nsm " << std::endl;
        os << "debug ospf ism " << std::endl;
        os << "debug ospf packet all " << std::endl;
      }

    for (std::vector<uint32_t>::iterator i = iflist->begin ();
         i != iflist->end (); ++i)
      {
        os << "interface ns3-device" << (*i) << std::endl;
      }

    os << "router ospf " << std::endl;
    
    for (std::map<std::string, uint32_t>::iterator i = networks->begin ();
         i != networks->end (); ++i)
      {
        os << "  network " << (*i).first << " area " << (*i).second << std::endl;
      }
    os << " redistribute connected" << std::endl;
    if (router_id != "") {
      os << " ospf router-id " << router_id << std::endl;
    }
    os << "!" << std::endl;
  }
  std::vector<uint32_t> *iflist;
  std::string m_filename;
  uint32_t m_routerId;
};

FrrHelper::FrrHelper ()
{
}

void
FrrHelper::EnableZebraDebug (NodeContainer nodes)
{
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<FrrConfig> zebra_conf = nodes.Get (i)->GetObject<FrrConfig> ();
      if (!zebra_conf)
        {
          zebra_conf = new FrrConfig ();
          nodes.Get (i)->AggregateObject (zebra_conf);
        }
      zebra_conf->m_zebradebug = true;
    }
  return;
}

void
FrrHelper::UseManualZebraConfig (NodeContainer nodes)
{
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<FrrConfig> zebra_conf = nodes.Get (i)->GetObject<FrrConfig> ();
      if (!zebra_conf)
        {
          zebra_conf = new FrrConfig ();
          nodes.Get (i)->AggregateObject (zebra_conf);
        }
      zebra_conf->m_usemanualconf = true;
    }
  return;
}

void
FrrHelper::GenerateConfigBasic (Ptr<Node> node)
{
  // Create folders
  std::stringstream conf_dir;
  conf_dir << "files-" << node->GetId () << "";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/usr/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/local/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/etc/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);

  // generate folders /var/tmp and /var/run
  conf_dir.str ("");
  conf_dir << "files-" << node->GetId () << "/var/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/tmp/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir.str ("");
  conf_dir << "files-" << node->GetId () << "/var/run/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
}

void
FrrHelper::GenerateConfigZebra (Ptr<Node> node)
{
  Ptr<FrrConfig> zebra_conf = node->GetObject<FrrConfig> ();

  // config generation
  std::stringstream conf_dir, conf_file;
  conf_dir << "files-" << node->GetId () << "/usr/local/etc/";
  conf_file << conf_dir.str () << "/zebra.conf";
  zebra_conf->SetFilename ("/usr/local/etc/zebra.conf");


  // copy binary 
  std::stringstream command_cp; 
  command_cp.str ("");
  command_cp << "ln -nsf /usr/lib/frr/zebra ~/bake/build/bin_dce/zebra" <<  node->GetId ();
  system (command_cp.str ().c_str ());

  if (zebra_conf->m_usemanualconf)
    {
      return;
    }

  std::ofstream conf;
  conf.open (conf_file.str ().c_str ());
  conf << *zebra_conf;
  if (zebra_conf->m_zebradebug)
    {
      conf << "debug zebra kernel" << std::endl;
      conf << "debug zebra events" << std::endl;
      conf << "debug zebra packet" << std::endl;
    }

  conf.close ();
}

// BGP
void
FrrHelper::EnableBgp (NodeContainer nodes)
{
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<BgpConfig> bgp_conf = nodes.Get (i)->GetObject<BgpConfig> ();
      if (!bgp_conf)
        {
          bgp_conf = CreateObject<BgpConfig> ();
          nodes.Get (i)->AggregateObject (bgp_conf);
        }
    }

  return;
}

void
FrrHelper::RunBgpWithoutZebra (NodeContainer nodes)
{
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<BgpConfig> bgp_conf = nodes.Get (i)->GetObject<BgpConfig> ();
      if (!bgp_conf)
        {
          bgp_conf = CreateObject<BgpConfig> ();
          nodes.Get (i)->AggregateObject (bgp_conf);
        }
      bgp_conf->m_only_bgp = true;
    }

  return;
}

void
FrrHelper::UseGivenBgpConfig (Ptr<Node> node, std::string file)
{
  Ptr<BgpConfig> bgp_conf = node->GetObject<BgpConfig> ();
  if (!bgp_conf)
    {
      bgp_conf = new BgpConfig ();
      node->AggregateObject (bgp_conf);
    }
  bgp_conf->m_givenconfig = file;
}

void
FrrHelper::FatTreeBgpConfig (Ptr<Node> node, int asn, int num_tor, int num_fabric,
                             std::string network, std::vector<std::string> *ips)
{
  Ptr<BgpConfig> bgp_conf = node->GetObject<BgpConfig> ();
  if (!bgp_conf)
    {
      bgp_conf = new BgpConfig ();
      node->AggregateObject (bgp_conf);
    }
  bgp_conf->m_asn = asn;
  bgp_conf->m_num_tor = num_tor;
  bgp_conf->m_num_fabric = num_fabric;
  bgp_conf->m_nodeid = node->GetId ();
  bgp_conf->m_network = network;
  bgp_conf->m_ips = ips;
}

void
FrrHelper::BgpNodeFailure (Ptr<Node> node, Time at)
{  
  Ptr<BgpConfig> bgp_conf = node->GetObject<BgpConfig> ();
  if (!bgp_conf)
    {
      bgp_conf = new BgpConfig ();
      node->AggregateObject (bgp_conf);
    }
  bgp_conf->stop_time = at;
}

void
FrrHelper::BgpDelayedStart (Ptr<Node> node, Time at)
{  
  Ptr<BgpConfig> bgp_conf = node->GetObject<BgpConfig> ();
  if (!bgp_conf)
    {
      bgp_conf = new BgpConfig ();
      node->AggregateObject (bgp_conf);
    }
  bgp_conf->delayed_time = at;
}

// OSPF
void
FrrHelper::EnableOspf (NodeContainer nodes, const char *network)
{
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<OspfConfig> ospf_conf = nodes.Get (i)->GetObject<OspfConfig> ();
      if (!ospf_conf)
        {
          ospf_conf = new OspfConfig ();
          nodes.Get (i)->AggregateObject (ospf_conf);
        }

      ospf_conf->addNetwork (std::string (network), 0);
    }
  return;
}

void
FrrHelper::SetOspfRouterId (Ptr<Node> node, const char * routerid)
{
  Ptr<OspfConfig> ospf_conf = node->GetObject<OspfConfig> ();
  if (!ospf_conf)
    {
      ospf_conf = new OspfConfig ();
      node->AggregateObject (ospf_conf);
    }
  ospf_conf->SetRouterId (routerid);
  return;
}

void
FrrHelper::EnableOspfDebug (NodeContainer nodes)
{
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<OspfConfig> ospf_conf = nodes.Get (i)->GetObject<OspfConfig> ();
      if (!ospf_conf)
        {
          ospf_conf = new OspfConfig ();
          nodes.Get (i)->AggregateObject (ospf_conf);
        }
      ospf_conf->m_ospfdebug = true;
    }
  return;
}

ApplicationContainer
FrrHelper::Install (Ptr<Node> node)
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
FrrHelper::Install (std::string nodeName)
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
FrrHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

void
FrrHelper::GenerateConfigBgp (Ptr<Node> node)
{
  Ptr<BgpConfig> bgp_conf = node->GetObject<BgpConfig> ();

  // config generation
  std::stringstream conf_dir, conf_file;

  // generate folders /usr/lib/x86_64-linux-gnu/libyang1/extensions and user_types
  conf_dir.str ("");
  conf_dir << "files-" << node->GetId () << "/usr/lib/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/x86_64-linux-gnu/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/libyang1/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/extensions/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir.str ("");
  conf_dir << "files-" << node->GetId () << "/usr/lib/x86_64-linux-gnu/libyang1/user_types";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  // copy libyang plugins
  std::stringstream command_cp;
  command_cp << "ln -nsf /usr/lib/x86_64-linux-gnu/libyang1/extensions/metadata.so ~/bake/source/ns-3-dce/files-";
  command_cp << node->GetId ();
  command_cp << "/usr/lib/x86_64-linux-gnu/libyang1/extensions/metadata.so";
  system (command_cp.str ().c_str ());
  command_cp.str ("");
  command_cp << "ln -nsf /usr/lib/x86_64-linux-gnu/libyang1/extensions/nacm.so ~/bake/source/ns-3-dce/files-";
  command_cp << node->GetId ();
  command_cp << "/usr/lib/x86_64-linux-gnu/libyang1/extensions/nacm.so";
  system (command_cp.str ().c_str ());
  command_cp.str ("");
  command_cp << "ln -nsf /usr/lib/x86_64-linux-gnu/libyang1/extensions/yangdata.so ~/bake/source/ns-3-dce/files-";
  command_cp << node->GetId ();
  command_cp << "/usr/lib/x86_64-linux-gnu/libyang1/extensions/yangdata.so";
  system (command_cp.str ().c_str ());
  command_cp.str ("");
  command_cp << "ln -nsf /usr/lib/x86_64-linux-gnu/libyang1/user_types/user_inet_types.so ~/bake/source/ns-3-dce/files-";
  command_cp << node->GetId ();
  command_cp << "/usr/lib/x86_64-linux-gnu/libyang1/user_types/user_inet_types.so";
  system (command_cp.str ().c_str ());
  command_cp.str ("");
  command_cp << "ln -nsf /usr/lib/x86_64-linux-gnu/libyang1/user_types/user_yang_types.so ~/bake/source/ns-3-dce/files-";
  command_cp << node->GetId ();
  command_cp << "/usr/lib/x86_64-linux-gnu/libyang1/user_types/user_yang_types.so";
  system (command_cp.str ().c_str ());

  // copy binary 
  command_cp.str ("");
  command_cp << "ln -nsf /usr/lib/frr/bgpd ~/bake/build/bin_dce/bgpd" << node->GetId ();
  
  system (command_cp.str ().c_str ());

  conf_dir.str ("");
  conf_dir << "files-" << node->GetId () << "/usr/local/etc/";
  conf_file << conf_dir.str () << "/bgpd.conf";
  
  if (!bgp_conf->m_givenconfig.empty())
    {
      command_cp.str ("");
      command_cp << "cp " << bgp_conf->m_givenconfig;
      command_cp << " files-" << node->GetId () << "/usr/local/etc/bgpd.conf";
      system (command_cp.str ().c_str ());
      return;
    }

  if (bgp_conf->m_asn)
    {
      std::ofstream conf;
      conf.open (conf_file.str ().c_str ());
      bgp_conf->Print (conf);
      conf.close ();
    }
}

void
FrrHelper::GenerateConfigOspf (Ptr<Node> node)
{
  NS_LOG_FUNCTION (node);

  Ptr<OspfConfig> ospf_conf = node->GetObject<OspfConfig> ();
  ospf_conf->m_routerId = 1 + node->GetId ();

  // config generation
  std::stringstream conf_dir, conf_file;
  
  conf_dir << "files-" << node->GetId () << "";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/usr/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/local/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/etc/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);

  // copy binary 
  std::stringstream command_cp;
  command_cp.str ("");
  command_cp << "cp /usr/lib/frr/ospfd ~/bake/build/bin_dce/ospfd" << node->GetId ();
  system (command_cp.str ().c_str ());

  conf_file << conf_dir.str () << "/ospfd.conf";
  ospf_conf->SetFilename ("/usr/local/etc/ospfd.conf");

  std::ofstream conf;
  conf.open (conf_file.str ().c_str ());
  ospf_conf->Print (conf);
  conf.close ();

}

ApplicationContainer
FrrHelper::InstallPriv (Ptr<Node> node)
{
  DceApplicationHelper process;
  ApplicationContainer apps;
  int idx = 0;
  GenerateConfigBasic (node);

  // BGP
  Ptr<BgpConfig> bgp_conf = node->GetObject<BgpConfig> ();
  if (bgp_conf)
    {
      GenerateConfigBgp (node);
      process.ResetArguments ();
      process.SetBinary ("bgpd" + std::to_string (node->GetId ()));
      process.AddArguments ("-f", "/usr/local/etc/bgpd.conf");
      process.AddArguments ("-i", "/usr/local/etc/bgpd.pid");
      if (bgp_conf->m_only_bgp) process.AddArguments ("--no_zebra", "");

      process.SetUid (0);
      process.SetStackSize (1 << 20);
      apps = process.Install (node);
      if (bgp_conf->delayed_time.IsZero()) {
        apps.Get (idx)->SetStartTime (Seconds (1.0 + 0.0001 * node->GetId ()));
      } else {
        apps.Get (idx)->SetStartTime (bgp_conf->delayed_time);
      }
      if (!bgp_conf->stop_time.IsZero ()) apps.Get (idx)->SetStopTime (bgp_conf->stop_time);
      node->AddApplication (apps.Get (idx));
      idx++;

    }

  // zebra
  if (bgp_conf && !bgp_conf->m_only_bgp) {
    Ptr<FrrConfig> zebra_conf = node->GetObject<FrrConfig> ();
    if (!zebra_conf)
      {
        zebra_conf = new FrrConfig ();
        node->AggregateObject (zebra_conf);
      }
    GenerateConfigZebra (node);
    process.ResetArguments ();
    process.SetBinary ("zebra" + std::to_string (node->GetId ()));
    process.AddArguments ("-f", zebra_conf->GetFilename ());
    process.AddArguments ("-i", "/usr/local/etc/zebra.pid");
    process.SetUid (0);
    process.SetStackSize (1 << 20);
    apps.Add (process.Install (node));
    apps.Get (idx)->SetStartTime (Seconds (0.2 + 0.0001 * node->GetId ()));
    node->AddApplication (apps.Get (idx));
    idx++;
  }

  // OSPF
  Ptr<OspfConfig> ospf_conf = node->GetObject<OspfConfig> ();
  if (ospf_conf)
    {
      GenerateConfigOspf (node);
      process.ResetArguments ();
      process.SetBinary ("ospfd" + std::to_string (node->GetId ()));
      process.AddArguments ("-f", ospf_conf->GetFilename ());
      process.AddArguments ("-i", "/usr/local/etc/ospfd.pid");
      apps.Add (process.Install (node));
      apps.Get (idx)->SetStartTime (Seconds (1.0 + 0.0001 * node->GetId ()));
      node->AddApplication (apps.Get (idx));
      idx++;
    }

  return apps;
}

} // namespace ns3
