#include "vftgen-classes.h"

int IP_COUNTER = 0;

std::vector<std::string> GetIps ()
{
  int x = IP_COUNTER * 4 + 1;
  IP_COUNTER++;
  int x1 = x / 65536;
  int x2 = (x % 65536) / 256;
  int x3 = x % 256;
  std::string ip1 = "10." + std::to_string (x1) + std::string(".") +
                            std::to_string (x2) + std::string(".") +
                            std::to_string (x3) + std::string("/30");
  std::string ip2 = "10." + std::to_string (x1) + std::string(".") +
                            std::to_string (x2) + std::string(".") +
                            std::to_string (x3 + 1) + std::string("/30");
  return {ip1, ip2};
}

NodeVft::NodeVft ()
{
  node = CreateObject<Node> ();
  ips = new std::vector<std::string> ();
  neighbors = new std::vector<NodeVft*> ();
}

Pod::Pod (int pod_number)
{
  number = pod_number;
  spines = new std::vector<Spine*> ();
  leaves = new std::vector<Leaf*> ();
  servers = new std::vector<Server*> ();
}

FatTree::FatTree ()
{
  pods = new std::vector<Pod*> ();
  aggregaion_layer = new std::vector<Tof*> (); 
}

Leaf::Leaf(int leaf_number, int pod_number, int leaves_per_pod)
{
  number = leaf_number;
  this->pod_number = pod_number;
  int num = (pod_number * leaves_per_pod) + number;
  std::string ip = "200." + std::to_string (num / 256) + "." +
                    std::to_string (num % 256) + std::string (".1/24");
  ips->push_back(ip);
}

Server::Server(Leaf* leaf, int pod_number, int leaves_per_pod, int server_number)
{
  leaf->neighbors->push_back(this);
  int num = (pod_number * leaves_per_pod) + leaf->number;
  std::string ip = "200." + std::to_string (num / 256) + "." +
                    std::to_string (num % 256) + "." + 
                    std::to_string(server_number + 2) + std::string("/24");
  ips->push_back(ip);
  number = server_number;
}


void FatTree::Create (struct Config config) 
{
  for (int i = 0; i < config.n_pods; i++) 
    CreatePod (i, config); 
  for (int i = 0; i < config.n_planes; i++) 
    CreateAggregationLayerPlane (this, i, config);     
}

void FatTree::CreatePod (int pod_number, struct Config config)
{
  Pod* pod = new Pod (pod_number);
  pods->push_back (pod);
  
  std::vector<Leaf*> leaves;
  for (int i = 0; i < config.k_top; i++)
    leaves.push_back (CreateRack (pod_number, i, config));
  
  for (int i = 0; i < config.k_leaf; i++)
  {
    Spine* spine = new Spine (pod_number, i, &leaves); 
    pod->spines->push_back(spine);
  }
}

void FatTree::CreateAggregationLayerPlane (FatTree *fat_tree, int plane_number, struct Config config) 
{
  for (int i = 0; i < config.k_top; i++)
  {
    Tof* tof = new Tof(plane_number, i, config, fat_tree);
    aggregaion_layer->push_back(tof);
  }
}

Leaf* FatTree::CreateRack (int pod_number, int leaf_number, struct Config config) 
{
  Leaf* leaf = new Leaf(leaf_number, pod_number, config.k_top);
  pods->at (pod_number)->leaves->push_back (leaf);

  for (int i = 0; i < config.servers_per_rack; i++)
  {
    Server *server = new Server (leaf, pod_number, config.k_top, i);
    pods->at (pod_number)->servers->push_back (server);
  }
  return leaf;
}

Tof::Tof(int plane_number, int tof_number, struct Config conf, FatTree *fat_tree)
{
  this->plane_number = plane_number;
  number = tof_number;
  for (int i = 0; i < fat_tree->pods->size(); i++)
    //for (int j = plane_number; j < conf.redundancy + plane_number; j++)
    for (int j = 0; j < conf.redundancy; j++)
    {
      std::vector<std::string> ips_link = GetIps ();
      ips->push_back (ips_link[0]);
      
      Pod* podi = fat_tree->pods->at (i);
      //Spine* spinej = podi->spines->at (j % conf.k_leaf);
      Spine* spinej = podi->spines->at (conf.redundancy * plane_number + j);
      spinej->ips->push_back (ips_link[1]);

      neighbors->push_back(spinej);
    }
}

Spine::Spine(int pod_number, int spine_number, std::vector<Leaf*> *leaves) 
{
  number = spine_number;
  this->pod_number = pod_number;
  for (int i = 0; i < leaves->size(); i++)
  {
    std::vector<std::string> ips_link = GetIps ();
    ips->push_back (ips_link[0]);
    leaves->at (i)->ips->push_back (ips_link[1]);
    neighbors->push_back((*leaves)[i]);
  }
}