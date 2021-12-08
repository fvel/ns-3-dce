#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/csma-module.h"
#include "ns3/dce-module.h"
#include "ns3/frr-helper.h"
#include "vftgen-utils.h"
#include "frr-utils.h"
#include <vector>

struct Config init_config (int k_leaf, int k_top, int redundancy,
                           int n_pods, int servers_per_rack)
{
  /* 
   * Formulas used:
   * Number of Planes: k_leaf / r
   * Number of PoDs: (k_leaf + k_top) / r
   * Number of Leaf per PoD: k_top
   * Number of ToP per PoD: k_leaf
   * Number of ToFs: k_top * (k_leaf / r)
   */ 

  struct Config config;
  config.k_leaf = k_leaf;
  config.k_top = k_top;
  config.redundancy = redundancy;
  config.n_pods = n_pods ? n_pods : (k_leaf + k_top) / redundancy;
  config.servers_per_rack = servers_per_rack;
  config.n_planes = k_leaf / redundancy;
  config.n_tofs = config.k_top * (config.k_leaf / config.redundancy); 
  config.n_spines_per_tof = config.redundancy * config.n_pods;
  return config;
}

void CreateCollisionsDomains (FatTree *fat_tree, struct Config config,
                     CsmaHelper csma, PointToPointHelper pointToPoint)
{
  for (int i = 0; i < config.n_pods; i++)
  {
    for (int j = 0; j < config.k_top; j++)
    {
      NodeContainer nodes;
      Pod* podi = (*fat_tree->pods)[i];
      Leaf* leafj = (*podi->leaves)[j];
      for (int k = 0; k < config.servers_per_rack; k++)
      {
        Server* serverk = (Server*) (*leafj->neighbors)[k];
        nodes.Add (serverk->node);
      }
      nodes.Add (leafj->node);
      csma.Install (nodes);
    } 
  }

  for (int i = 0; i < config.n_pods; i++)
    for (int j = 0; j < config.k_leaf; j++)
    {   
      Pod* podi = (*fat_tree->pods)[i];
      Spine* spinej = (*podi->spines)[j];
      for (int k = 0; k < config.k_top; k++)
      {
        NodeContainer nodes;
        Leaf* leafk = (Leaf*) (*spinej->neighbors)[k];
        nodes.Add (leafk->node);
        nodes.Add (spinej->node);
        pointToPoint.Install (nodes);
      }
      
    }

  int n_tofs = config.k_top * (config.k_leaf / config.redundancy); 
  int n_spines_per_tof = config.redundancy * config.n_pods;
  for (int i = 0; i < n_tofs; i++)
  {
    Tof* tofi = (*fat_tree->aggregaion_layer)[i];
    for (int j = 0; j < n_spines_per_tof; j++)
    {
      NodeContainer nodes;
      Spine* spinej = (Spine*) (*tofi->neighbors)[j];
      nodes.Add (tofi->node);
      nodes.Add (spinej->node);
      pointToPoint.Install (nodes);
    }
  }
}

void SetNetworkStack (FatTree *fat_tree, struct Config config)
{  
  DceManagerHelper dce;  
  dce.SetTaskManagerAttribute ("FiberManagerType", EnumValue (0));
  dce.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));

  for (int i = 0; i < config.n_tofs; i++)
  {
    Tof* tofi = (*fat_tree->aggregaion_layer)[i];
    dce.Install (tofi->node);
  }
  for (int i = 0; i < config.n_pods; i++)
  {
    Pod* podi = (*fat_tree->pods)[i];
    for (int j = 0; j < config.k_leaf; j++)
    {         
      Spine* spinej = (*podi->spines)[j];
      dce.Install (spinej->node);
    }
    for (int j = 0; j < config.k_top; j++)
    {
      Leaf* leafj = (*podi->leaves)[j];
      dce.Install (leafj->node);
    }
    for (int j = 0; j < (config.servers_per_rack * config.k_top); j++)
    {
      Server* serverk = (*podi->servers)[j];
      dce.Install (serverk->node);
    }   
  }
}

void IpConfig (FatTree *fat_tree, struct Config config, bool ipv6)
{
  for (int i = 0; i < config.n_tofs; i++)
  {
    Tof* tofi = (*fat_tree->aggregaion_layer)[i];
    AddAdressesList (tofi->node, *tofi->ips, ipv6);
  }

  for (int i = 0; i < config.n_pods; i++)
  {
    Pod* podi = (*fat_tree->pods)[i];
    for (int j = 0; j < config.k_leaf; j++)
    {         
      Spine* spinej = (*podi->spines)[j];
      AddAdressesList (spinej->node, *spinej->ips, ipv6);
    }
    for (int j = 0; j < config.k_top; j++)
    {
      Leaf* leafj = (*podi->leaves)[j];
      AddAdressesList (leafj->node, *leafj->ips, ipv6);
      for (int k = 0; k < config.servers_per_rack; k++)
      {
        Server* serverk = (Server*) (*leafj->neighbors)[k];
        AddAdressesList (serverk->node, *serverk->ips, ipv6);
        std::string ip = (*leafj->ips)[0];
        ip.erase(ip.length()-3,3);
        std::string str = "route add default via " + ip + " dev sim0";
        RunIp (serverk->node, Seconds (0.20), str);
      }
    }   
  }
}

void ConfigFrr (FatTree *fat_tree, struct Config config, FrrHelper frr, bool only_bgp)
{
  int private_asn = 64512;
  for (int i = 0; i < config.n_tofs; i++)
    {
      Tof* tofi = (*fat_tree->aggregaion_layer)[i];
      frr.EnableBgp (tofi->node);
      if (only_bgp) frr.RunBgpWithoutZebra (tofi->node);
      frr.FatTreeBgpConfig (tofi->node, private_asn, 0, config.n_spines_per_tof, "", tofi->ips);
      frr.Install (tofi->node);
      RunIp (tofi->node, Seconds (59.0), "route list");
    }
    for (int i = 0; i < config.n_pods; i++)
    {
      private_asn++;
      Pod* podi = (*fat_tree->pods)[i];
      for (int j = 0; j < config.k_leaf; j++)
      {         
        Spine* spinej = (*podi->spines)[j];
        frr.EnableBgp (spinej->node);
        if (only_bgp) frr.RunBgpWithoutZebra (spinej->node);
        frr.FatTreeBgpConfig (spinej->node, private_asn, config.k_top, 
                              config.k_top, "", spinej->ips);
        frr.Install (spinej->node);
        RunIp (spinej->node, Seconds (59.0), "route list");
      }
      for (int j = 0; j < config.k_top; j++)
      {
        private_asn++;
        Leaf* leafj = (*podi->leaves)[j];
        frr.EnableBgp (leafj->node);
        if (only_bgp) frr.RunBgpWithoutZebra (leafj->node);
        std::string network = leafj->ips->at(0);
        network.erase(network.length()-4,4);
        network += "0/24";
        frr.FatTreeBgpConfig (leafj->node, private_asn, config.k_leaf, 0, network, leafj->ips);
        frr.Install (leafj->node);
        RunIp (leafj->node, Seconds (59.0), "route list");
      } 
    }
}

void RenameFolders (FatTree *fat_tree, struct Config config)
{
  std::ofstream rename_script;
  rename_script.open ("rename.sh");
  rename_script << "rm -rf files-tof*" << std::endl;
  rename_script << "rm -rf files-spine*" << std::endl;
  rename_script << "rm -rf files-leaf*" << std::endl;
  rename_script << "rm -rf files-server*" << std::endl;
  for (int i = 0; i < config.n_tofs; i++)
  {
    Tof* tofi = (*fat_tree->aggregaion_layer)[i];
    rename_script << "mv files-" << tofi->node->GetId () << 
                     " files-tof_" << tofi->plane_number + 1 << 
                     "_2_" << tofi->number + 1 << std::endl;
  }
  for (int i = 0; i < config.n_pods; i++)
  {
    Pod* podi = (*fat_tree->pods)[i];
    for (int j = 0; j < config.k_leaf; j++)
    {         
      Spine* spinej = (*podi->spines)[j];
      rename_script << "mv files-" << spinej->node->GetId () << 
                       " files-spine_" << spinej->pod_number + 1 << 
                       "_1_" << spinej->number + 1 << std::endl;
    }
    for (int j = 0; j < config.k_top; j++)
    {
      Leaf* leafj = (*podi->leaves)[j];      
      rename_script << "mv files-" << leafj->node->GetId () << 
                       " files-leaf_" << leafj->pod_number + 1 << 
                       "_0_" << leafj->number + 1<< std::endl;
      for (int k = 0; k < config.servers_per_rack; k++)
      {
        Server* serverk = (Server*) (*leafj->neighbors)[k];
        rename_script << "mv files-" << serverk->node->GetId () << 
                         " files-server_" << leafj->pod_number + 1 << 
                         "_X_" << leafj->number + 1 << 
                         "_" << serverk->number + 1 << std::endl;                        
      }
    }  
  }
  rename_script.close ();
  system ("bash rename.sh 1>/dev/null 2>/dev/null");
}

void RenamePcapFiles (FatTree *fat_tree, struct Config config)
{
  std::ofstream rename_script;
  rename_script.open ("rename2.sh");
  for (int i = 0; i < config.n_tofs; i++)
  {
    Tof* tofi = (*fat_tree->aggregaion_layer)[i];
    for (int j = 0; j < config.n_pods * config.redundancy; j++)
    {
      rename_script << "mv vftgen-" << tofi->node->GetId () <<
                       "-" << j << ".pcap " <<
                       "tof_" << tofi->plane_number + 1 <<
                       "_2_" << tofi->number + 1 << "-sim" << j <<
                       ".pcap" << std::endl;
    }
  }
  for (int i = 0; i < config.n_pods; i++)
  {
    Pod* podi = (*fat_tree->pods)[i];
    for (int j = 0; j < config.k_leaf; j++)
    {         
      Spine* spinej = (*podi->spines)[j];
      for (int k = 0; k < config.k_top * 2; k++)
      {
        rename_script << "mv vftgen-" << spinej->node->GetId () <<
                         "-" << k << ".pcap " << 
                         "spine_" << spinej->pod_number + 1 <<
                         "_1_" << spinej->number + 1<< "-sim" << k <<
                         ".pcap" << std::endl;
      }
    }
    for (int j = 0; j < config.k_top; j++)
    {
      Leaf* leafj = (*podi->leaves)[j];
      for (int k = 0; k < config.k_leaf + 1; k++)
      {
        rename_script << "mv vftgen-" << leafj->node->GetId () <<
                         "-" << k << ".pcap " <<
                         "leaf_" << leafj->pod_number + 1 <<
                         "_0_" << leafj->number + 1 << "-sim" << k <<
                         ".pcap" << std::endl;
      }
      for (int k = 0; k < config.servers_per_rack; k++)
      {
        Server* serverk = (Server*) (*leafj->neighbors)[k];
        rename_script << "mv vftgen-" << serverk->node->GetId () <<
                         "-0.pcap server_" << leafj->pod_number + 1 <<
                         "_X_" << leafj->number + 1 << 
                         "_" << serverk->number <<
                         "-sim0.pcap" << std::endl;                         
      }
    }  
  }
  rename_script.close ();
  system ("bash rename2.sh 1>/dev/null 2>/dev/null");
}

void GenerateGraphImg (FatTree *fat_tree, struct  Config config)
{
  // Create graph image
  // Create a .dot file and a .png file

  //Each Tofs-Spines links different color for each tof plane
  std::vector <std::string> colors = {"cornflowerblue", "darkorange", "darkolivegreen1", "fuchsia", "gold1", 
                                      "deeppink4", "chartreuse4", "darkorange4", "aquamarine", "deeppink1"}; 
  std::string graph = "graph {\n  splines=false;\n\n";

  //To make a separation in the image betwen tofs and spines
  int separation = 3;
  std::string auxLevel = "  subgraph AuxLevel {\n    edge[style=invis]\n    Aux [style=invis, height = " + 
                              std::to_string (separation) + std::string("]\n");
  std::string tofsOrdered = " {\n    rank = same\n    edge[style=invis] \n    ";
  std::string spinesOrdered = " {\n    rank = same\n    edge[style=invis] \n    ";
  std::string leavesOrdered = " {\n    rank = same\n    edge[style=invis] \n    ";

  std::string linksLeafServers = "";
  std::string serverRack = "\n"; 

  // We create the auxiliar level, the ordered tofs and the tof-spine links with colors.
  int tofs_per_plane = config.n_tofs / config.n_planes;
  for (int i = 0; i < config.n_planes; i++)
  {
    for (int j = 0; j < tofs_per_plane; j++) 
    {
      Tof* tofj = (*fat_tree->aggregaion_layer)[i*tofs_per_plane  + j];
      std::string tofjNode = std::string("Plane") + std::to_string (tofj->plane_number) +
                    std::string("_Tof") + std::to_string (tofj->number) ; 
      
      auxLevel += std::string ("    ") + tofjNode + std::string (" -- Aux;\n");
      (i == 0 && j == 0) ? tofsOrdered += tofjNode : tofsOrdered += std::string(" -- ") + tofjNode;

      for (int k = 0; k < config.n_spines_per_tof; k++)
      {
        Spine* spinek = (Spine*) (*tofj->neighbors)[k];
        std::string spinekNode = std::string("Pod") + std::to_string (spinek->pod_number) + 
                      std::string("_Spine") + std::to_string (spinek->number);
        graph += std::string("  ") + tofjNode + std::string(" -- ") + spinekNode + 
                      std::string(" [color=") + std::string (colors[i%colors.size()]) + std::string("];\n");
        auxLevel += std::string ("    Aux -- ") + spinekNode + std::string (";\n");
      }
    }
  }
  auxLevel += std::string ("  }\n\n");
  tofsOrdered += std::string(";\n  }\n\n");
  graph += std::string("\n") + tofsOrdered + auxLevel;

  //We create the cluster pods and spine-leaf links, and spines ordered
  for (int i = 0; i < config.n_pods; i++)
  {
    Pod* podi = (*fat_tree->pods)[i];
    for (int j = 0; j < config.k_leaf; j++)
    {         
      Spine* spinej = (*podi->spines)[j];
      std::string spinejNode = std::string("Pod") + std::to_string (i) + 
                    std::string("_Spine") + std::to_string (spinej->number);

      (i == 0 && j == 0) ? spinesOrdered += spinejNode : spinesOrdered += std::string(" -- ") + spinejNode;

      for (int k = 0; k < config.k_top; k++)
      {
        Leaf* leafk = (*podi->leaves)[k];
        std::string leafkNode = std::string("Pod") + std::to_string (i) + 
                      std::string("_Leaf") + std::to_string (leafk->number);
        if (i%2 ==0) {
          graph += std::string("    ") + spinejNode + std::string("[ fillcolor=bisque  style=filled];\n");
          graph += std::string("    ") + leafkNode + std::string("[ fillcolor=bisque  style=filled];\n");
        } else {
          graph += std::string("    ") + spinejNode + std::string("[ fillcolor=azure2  style=filled];\n");
          graph += std::string("    ") + leafkNode + std::string("[ fillcolor=azure2  style=filled];\n");

        }
        (i == 0 && j == 0 && k == 0) ? leavesOrdered += leafkNode : leavesOrdered += std::string(" -- ") + leafkNode;
        graph += std::string("    ") + spinejNode + std::string(" -- ") + leafkNode + std::string(";\n");
      }
    }
  
  }

  spinesOrdered += std::string(";\n  }\n\n");
  leavesOrdered += std::string(";\n  }\n\n");
  graph += std::string("\n") + spinesOrdered + std::string("\n") + leavesOrdered;

  // We create leaf-server links, and servers in a column-rack 
  for (int i = 0; i < config.n_pods; i++)
  {
    Pod* podi = (*fat_tree->pods)[i];
      for (int j = 0; j < config.k_top; j++)
      {
        Leaf* leafj = (*podi->leaves)[j];
        std::string leafjNode = std::string("Pod") + std::to_string (i) + 
                      std::string("_Leaf") + std::to_string (leafj->number);
        
        serverRack += std::string("  subgraph cluster_") + leafjNode + std::string(" {\n") +
                            std::string("    edge[style=invis]\n    peripheries=0\n    ");

        for (int k = 0; k < config.servers_per_rack; k++)
        { 
          Server* serverk = (*podi->servers)[j*config.servers_per_rack + k];
         
          std::string serverkNode = leafjNode + 
                      std::string("_Server") + 
                      std::to_string (serverk->number);
          linksLeafServers += std::string("  ") + leafjNode + std::string(" -- ") + serverkNode + std::string(";\n");
        

          if (k != 0){ serverRack += std::string(" -- "); }            
          serverRack += serverkNode;
        } 
        serverRack += std::string("\n  }\n");
      }
  }

  graph += linksLeafServers + serverRack + std::string("\n}");
  std::ofstream graphFile;
  graphFile.open ("graph.txt");
  graphFile << graph << std::endl;
  graphFile.close ();
  system ("dot -Tpng graph.txt -o graph.png");
}
