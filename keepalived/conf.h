#pragma once
#include <vector>
#include <string>
using namespace std;

struct vrrp_instance {
    string state;
    string interface_v;
    string interface_name;
    uint8_t virtual_router_id;
    uint8_t priority;
    uint8_t advert_int;
    vector<string> virtual_ipaddress;
};

struct vrrp_global_defs {
    string vrrp_mcast_group4;// ∂‡≤•µÿ÷∑
};

struct vrrp_conf {
    int fd;
    bool preempt;	/* true if a higher prio preempt a lower one */
    vrrp_global_defs global_defs;
    vrrp_instance instance;
};

vrrp_conf read_vrrp_conf(const string& filename);

void print_vrrp_conf(vrrp_conf& conf);