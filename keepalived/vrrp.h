#pragma once
#include <vector>
#include <string>
#include "conf.h"
using namespace std;

constexpr auto MASTER = "master";
constexpr auto BACKUP = "backup";

struct vrrp_hdr {
    uint8_t vrid; /* virtual router id */
    uint8_t priority; /* router priority */
    uint8_t naddr; /* address counter */
    uint8_t adver_int; /* advertissement interval(in sec) */
    uint16_t chksum; /* checksum (ip-like one) */
    vector<string> ip_addr;
};


int open_send_vrrp(vrrp_conf conf);

int open_recv_vrrp(vrrp_conf conf);

int vrrp_send_pkt(vrrp_conf conf, char* buffer, int buflen);

void vrrp_build_pkt(vrrp_instance instance, char* buffer, uint8_t priority);

int vrrp_send_adv(vrrp_conf& conf, uint8_t priority);

void vrrp_recv_adv(vrrp_conf& conf);

vrrp_hdr vrrp_recv_parse(char* recv);

void print_vrrp_hdr(const vrrp_hdr& hdr);

