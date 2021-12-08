#include "vftgen-classes.h"

using namespace ns3;

struct Config init_config (int k_leaf, int k_top, int redundancy,
              int n_pods, int servers_per_rack);

void CreateCollisionsDomains (FatTree *fat_tree, struct Config config,
                     CsmaHelper csma, PointToPointHelper pointToPoint);

void SetNetworkStack (FatTree *fat_tree, struct Config config);

void IpConfig (FatTree *fat_tree, struct Config config, bool ipv6);

void ConfigFrr (FatTree *fat_tree, struct Config config, FrrHelper frr, bool only_bgp);

void RenameFolders (FatTree *fat_tree, struct Config config);

void RenamePcapFiles (FatTree *fat_tree, struct Config config);

void GenerateGraphImg (FatTree *fat_tree, struct  Config config);