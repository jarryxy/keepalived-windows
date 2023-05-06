#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Iphlpapi.h>	
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Ws2_32.lib")
using namespace std;

void add_vip(const string ip, const string mask, const string interface_name) {
	string command = "netsh interface ipv4 add address \"" + interface_name +
		"\" " + ip + " " + mask + " st=ac";
	// 执行命令
	int result = system(command.c_str());

	if (result == 0) {
		std::cout << "IP address added successfully." << std::endl;
	}
	else {
		std::cerr << "Failed to add IP address. Error code: " << result << std::endl;
	}

}


void remove_vip(const string ip, const string interface_name) {
	// 构造 netsh 命令
	std::string command = "netsh interface ipv4 delete address \"" + interface_name +
		"\" addr=" + ip;

	// 执行命令
	int result = system(command.c_str());

	if (result == 0) {
		std::cout << "IP address removed successfully." << std::endl;
	}
	else {
		std::cerr << "Failed to remove IP address. Error code: " << result << std::endl;
	}
}

bool has_string(const vector<string>& str_vec, const string& target_str) {
	// 使用find()算法查找指定字符串
	auto it = find(str_vec.begin(), str_vec.end(), target_str);

	// 判断是否找到指定字符串
	return (it != str_vec.end());
}


int maskToBits(const string& mask) {
	stringstream ss(mask);
	int a, b, c, d;
	char ch;
	ss >> a >> ch >> b >> ch >> c >> ch >> d;
	int bits = 0;
	bits += __popcnt(a);
	bits += __popcnt(b);
	bits += __popcnt(c);
	bits += __popcnt(d);
	return bits;
}

string bitsToMask(int bits) {
	unsigned long mask = 0xFFFFFFFFUL << (32 - bits);
	stringstream ss;
	ss << ((mask >> 24) & 0xFF) << '.' << ((mask >> 16) & 0xFF) << '.' << ((mask >> 8) & 0xFF) << '.' << (mask & 0xFF);
	return ss.str();
}