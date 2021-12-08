#include <vector>
#include "ns3/network-module.h"

using namespace ns3;

std::vector<std::string> GetIps ();

struct Config 
{
  int k_leaf;
  int k_top;
  int redundancy;
  int n_pods;
  int servers_per_rack;
  bool tof_rings; 
  int n_planes;
  int n_tofs; 
  int n_spines_per_tof;
};

class NodeVft
{
public:
  int number;
  std::vector<std::string> *ips;
  std::vector<NodeVft*> *neighbors;
  Ptr<Node> node;

  NodeVft ();
};

class Tof;
class Spine;
class Leaf;
class Server;

class Pod
{
public:
  int number;  
  std::vector<Spine*> *spines;
  std::vector<Leaf*> *leaves;
  std::vector<Server*> *servers;

  Pod (int pod_number);
};

class FatTree 
{
public:
  std::vector<Pod*> *pods;
  std::vector<Tof*> *aggregaion_layer;
  void Create (struct Config config);
  void CreatePod (int pod_number, struct Config config);
  void CreateAggregationLayerPlane (FatTree *fat_tree, int plane_number, struct Config config);
  Leaf* CreateRack (int pod_number, int leaf_number, struct Config config);

  FatTree ();
};

class Tof : public NodeVft
{
public:
  int plane_number;

  Tof(int plane_number, int tof_number, struct Config conf, FatTree *fat_tree);
};

class Spine : public NodeVft
{
public:
  int pod_number;

  Spine(int pod_number, int spine_number, std::vector<Leaf*> *leaves);
};

class Leaf : public NodeVft
{
public:
  int pod_number;

  Leaf(int leaf_number, int pod_number, int leaves_per_pod);
};

class Server : public NodeVft
{
public:
  Server(Leaf* leaf, int pod_number, int leaves_per_pod, int server_number);
};
