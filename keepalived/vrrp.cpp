#include <iostream>  
#include <stdio.h>
#include <winsock2.h>  
#include <ws2tcpip.h>  
#include <sstream>
#include <iomanip>
#include <thread>
#include "vrrp.h"
#include "vrrp_interface.h"
#include "conf.h"
#pragma comment(lib, "ws2_32.lib")



int open_send_vrrp(vrrp_conf conf) {
	int iRet = 0;
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	int iFlag = 1;	// 0-同一台主机 1-夸主机
	iRet = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&iFlag, sizeof(iFlag));

	if (iRet != 0) {
		printf("setsockopt fail:%d", WSAGetLastError());
		return -1;
	}

	// 绑定网卡
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	//addr.sin_addr.S_un.S_addr = INADDR_ANY;
	inet_pton(AF_INET, conf.instance.interface_v.c_str(), &addr.sin_addr.S_un.S_addr);
	iRet = bind(sock, (sockaddr*)&addr, sizeof(addr));
	if (iRet != 0) {
		printf("bind fail:%d", WSAGetLastError());
		return -1;
	}

	return sock;
}

int open_recv_vrrp(vrrp_conf conf) {
	int iRet = 0;

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	//addr.sin_addr.S_un.S_addr = INADDR_ANY;
	inet_pton(AF_INET, conf.instance.interface_v.c_str(), &addr.sin_addr.S_un.S_addr);
	//addr.sin_addr.S_un.S_addr = inet_addr("192.168.0.104");
	addr.sin_port = htons(9112);

	bool bOptval = true;
	iRet = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&bOptval, sizeof(bOptval));
	if (iRet != 0) {
		printf("setsockopt fail: SOL_SOCKET SO_REUSEADDR %d", WSAGetLastError());
		return -1;
	}

	iRet = bind(sock, (sockaddr*)&addr, sizeof(addr));
	if (iRet != 0) {
		printf("bind fail:%d", WSAGetLastError());
		return -1;
	}

	// 加入组播  
	ip_mreq multiCast;
	//multiCast.imr_interface.S_un.S_addr = INADDR_ANY;
	inet_pton(AF_INET, conf.instance.interface_v.c_str(), &multiCast.imr_interface.S_un.S_addr);
	inet_pton(AF_INET, conf.global_defs.vrrp_mcast_group4.c_str(), &multiCast.imr_multiaddr.S_un.S_addr);
	iRet = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&multiCast, sizeof(multiCast));
	if (iRet != 0) {
		printf("setsockopt fail:IP_ADD_MEMBERSHIP %d", WSAGetLastError());
		return -1;
	}
	return sock;
}

int vrrp_send_pkt(vrrp_conf conf, char* buffer, int buflen) {
	cout << "send" << endl;
	sockaddr_in addr;
	//addr.sin_addr.S_un.S_addr = inet_addr("224.0.0.100");
	inet_pton(AF_INET, conf.global_defs.vrrp_mcast_group4.c_str(), &addr.sin_addr.S_un.S_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9112);
	int ret = 0;
	ret = sendto(conf.fd, buffer, buflen, 0, (sockaddr*)&addr, sizeof(sockaddr));
	if (ret <= 0) {
		printf("send fail:%d", WSAGetLastError());
	}
	return ret;
}

void vrrp_build_pkt(vrrp_instance instance, char* buffer, uint8_t priority) {
	//vrrp_hdr hdr;
	//hdr.vrid = instance.virtual_router_id;
	//hdr.priority = instance.priority;
	//hdr.naddr = instance.virtual_ipaddress.size();
	//hdr.adver_int = instance.advert_int;
	//hdr.chksum = 0;
	//hdr.ip_addr = instance.virtual_ipaddress;
	//return struct_to_char(hdr);
	//buffer[0] = 167;//a7
	//buffer[1] = 197;///c5
	//buffer[2] = 226;//e2
	//buffer[3] = 8;//08
	buffer[0] = instance.virtual_router_id;
	buffer[1] = priority;
	buffer[2] = instance.virtual_ipaddress.size();
	buffer[3] = instance.advert_int;
	uint16_t chksum = 256;
	buffer[4] = (chksum >> 8) & 0xff;; // chksum 保留位 取高8位
	buffer[5] = chksum & 0xff;; // chksum 保留位 取低8位
	for (size_t i = 0; i < instance.virtual_ipaddress.size(); i++) {
		std::string ip = instance.virtual_ipaddress[i];
		int pos1 = ip.find_first_of('.');
		int pos2 = ip.find_first_of('.', pos1 + 1);
		int pos3 = ip.find_first_of('.', pos2 + 1);
		int pos4 = ip.find_first_of('/', pos3 + 1);
		std::string s1 = ip.substr(0, pos1);
		std::string s2 = ip.substr(pos1 + 1, pos2 - pos1 - 1);
		std::string s3 = ip.substr(pos2 + 1, pos3 - pos2 - 1);
		std::string s4 = ip.substr(pos3 + 1, pos4 - pos3 - 1);
		int i1 = std::stoi(s1);
		int i2 = std::stoi(s2);
		int i3 = std::stoi(s3);
		int i4 = std::stoi(s4);
		int mask = std::stoi(ip.substr(pos4 + 1));
		buffer[4 * (i + 1) + 2] = i1;
		buffer[4 * (i + 1) + 3] = i2;
		buffer[4 * (i + 1) + 4] = i3;
		buffer[4 * (i + 1) + 5] = i4;
	}

}


int vrrp_send_adv(vrrp_conf& conf, uint8_t priority) {
	int buflen = 6 + 4 * conf.instance.virtual_ipaddress.size();
	char* buffer = (char*)malloc(sizeof(char) * buflen);
	vrrp_build_pkt(conf.instance, buffer, priority);
	int i = vrrp_send_pkt(conf, buffer, buflen);
	return i;
}


void preempt_watch(int* count, vrrp_conf* conf) {
	while (true)
	{
		cout << "preempt_watch: " << *count << " state: " << conf->instance.state << endl;
		Sleep(1000);
		if (*count <= 0) {
			if (conf->instance.state == "BACKUP" || conf->instance.state == BACKUP) {
				(*conf).preempt = false;
				conf->instance.state = MASTER;
				for (int i = 0; i < (*conf).instance.virtual_ipaddress.size(); i++) {
					//remove_vip(conf->instance.virtual_ipaddress[i], conf->instance.interface_name);
					//Sleep(300);
					Sleep(100);
					add_vip((*conf).instance.virtual_ipaddress[i], "255.255.255.0", (*conf).instance.interface_name);
				}
			}
			*count = 0;
			continue;
		}
		(*count)--;
	}
}

void vrrp_recv_adv(vrrp_conf& conf) {
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, conf.instance.interface_v.c_str(), &addr.sin_addr.S_un.S_addr);
	addr.sin_port = htons(9411);

	int ret = 0;
	int len = sizeof(sockaddr);
	char recv[1024] = { 0 };
	int count = 2 + conf.instance.advert_int;
	std::thread t(preempt_watch, &count, &conf);
	t.detach();
	cout << "recv start" << endl;
	while (true)
	{
		memset(recv, 0, sizeof(recv));
		ret = recvfrom(conf.fd, recv, sizeof(recv) - 1, 0, (sockaddr*)&addr, &len);
		if (ret <= 0) {
			printf("recvfrom fail:%d", WSAGetLastError());
			return;
		}
		vrrp_hdr hdr = vrrp_recv_parse(recv);
		// Only message with the same vrid are accepted
		if (hdr.vrid == conf.instance.virtual_router_id) {
			if (hdr.priority > conf.instance.priority) {
				int count = 2 + conf.instance.advert_int;
				conf.preempt = true;
				if (conf.instance.state == "MASTER" || conf.instance.state == MASTER) {
					conf.instance.state = BACKUP;
					for (int i = 0; i < conf.instance.virtual_ipaddress.size(); i++) {
						Sleep(100);
						remove_vip(conf.instance.virtual_ipaddress[i], conf.instance.interface_name);
					}
				}
				char str[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(addr.sin_addr), str, INET_ADDRSTRLEN);
				cout << "preepmt: priority " << hdr.priority << " ip " << str << endl;
			}
			print_vrrp_hdr(hdr);
		}
	}
}


vrrp_hdr vrrp_recv_parse(char* recv) {
	vrrp_hdr hdr;
	hdr.vrid = static_cast<uint8_t>(recv[0]);
	hdr.priority = static_cast<uint8_t>(recv[1]);
	hdr.naddr = static_cast<uint8_t>(recv[2]);
	hdr.adver_int = static_cast<uint8_t>(recv[3]);
	hdr.chksum = (static_cast<uint16_t>(recv[4]) << 8) | static_cast<uint16_t>(recv[5]);
	std::ostringstream oss;
	for (int i = 0; i < hdr.naddr; i++) {
		uint8_t str[] = { static_cast<uint8_t>(recv[4 * (i + 1) + 2]),
			static_cast<uint8_t>(recv[4 * (i + 1) + 3]), static_cast<uint8_t>(recv[4 * (i + 1) + 4]),
			static_cast<uint8_t>(recv[4 * (i + 1) + 5]) };
		oss << std::dec << static_cast<int>(str[0]) << "." << static_cast<int>(str[1]) << "."
			<< static_cast<int>(str[2]) << "." << static_cast<int>(str[3]);
		hdr.ip_addr.push_back(oss.str());
		oss.str("");
	}
	return hdr;
}

void print_vrrp_hdr(const vrrp_hdr& hdr) {
	std::cout << "vrid: " << (int)hdr.vrid << std::endl;
	std::cout << "priority: " << (int)hdr.priority << std::endl;
	std::cout << "naddr: " << (int)hdr.naddr << std::endl;
	std::cout << "adver_int: " << (int)hdr.adver_int << std::endl;
	std::cout << "chksum: " << hdr.chksum << std::endl;
	std::cout << "ip_addr: ";
	for (const auto& ip : hdr.ip_addr) {
		std::cout << ip << " ";
	}
	std::cout << std::endl;
}