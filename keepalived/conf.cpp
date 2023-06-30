#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include "vrrp.h"
#include "conf.h"
using namespace std;

std::string trim(const std::string& str)
{
	std::size_t first_non_space = str.find_first_not_of(" \t\r\n");
	std::size_t last_non_space = str.find_last_not_of(" \t\r\n");

	if (first_non_space == std::string::npos)
	{
		return "";
	}

	return str.substr(first_non_space, last_non_space - first_non_space + 1);
}

std::string extract_quoted_content(const std::string& line)
{
	std::size_t first_quote_pos = line.find_first_of("\"");
	std::size_t last_quote_pos = line.find_last_of("\"");

	if (first_quote_pos != std::string::npos && last_quote_pos != std::string::npos && first_quote_pos < last_quote_pos 
		&& first_quote_pos == 0 && last_quote_pos == line.length()-1)
	{
		return line.substr(first_quote_pos + 1, last_quote_pos - first_quote_pos - 1);
	}

	return line;
}

vrrp_instance read_vrrp_instance(ifstream& file) {
	vrrp_instance instance;
	string line;
	while (getline(file, line)) {
		stringstream ss(line);
		string key;
		string tmp;
		if (ss >> key) {
			if (key == "{" || key == "}") {
				break;
			}
			if (key == "state") {
				ss >> instance.state;
			}
			else if (key == "interface") {
				ss >> instance.interface_v;
			}
			else if (key == "interface_name") {
				string tmp;
				getline(ss, tmp);
				instance.interface_name = trim(tmp);
				instance.interface_name = extract_quoted_content(instance.interface_name);
			}
			else if (key == "virtual_router_id") {
				ss >> tmp;
				instance.virtual_router_id = stoi(tmp);
			}
			else if (key == "priority") {
				ss >> tmp;
				instance.priority = stoi(tmp);
			}
			else if (key == "advert_int") {
				ss >> tmp;
				instance.advert_int = stoi(tmp);
			}
			else if (key == "virtual_ipaddress") {
				string address;
				do {
					getline(file, line);
					stringstream ss(line);
					ss >> address;
					if (address != "{" && address != "}") {
						instance.virtual_ipaddress.push_back(address);
					}
				} while (address != "}");
			}
		}
		else {
			break;
		}
	}
	return instance;
}

vrrp_global_defs read_vrrp_global_defs(ifstream& file) {
	vrrp_global_defs global_defs;
	string line;
	while (getline(file, line)) {
		stringstream ss(line);
		string key;
		if (ss >> key) {
			if (key == "{" || key == "}") {
				break;
			}
			if (key == "vrrp_mcast_group4") {
				ss >> global_defs.vrrp_mcast_group4;
			}
		}
		else {
			break;
		}
	}
	return global_defs;
}

vrrp_conf read_vrrp_conf(const string& filename) {
	vrrp_conf conf;
	ifstream file(filename);
	string line;
	while (getline(file, line)) {
		stringstream ss(line);
		string key;
		if (ss >> key) {
			if (key == "global_defs") {
				conf.global_defs = read_vrrp_global_defs(file);
			}
			else if (key == "vrrp_instance") {
				vrrp_instance instance = read_vrrp_instance(file);
				conf.instance = instance;
			}
		}
	}
	return conf;
}

void print_vrrp_conf(vrrp_conf& conf) {
	cout << "vrrp_conf:" << endl;
	cout << "global_defs.vrrp_mcast_group4 = " << conf.global_defs.vrrp_mcast_group4 << endl;
	cout << "instance.state = " << conf.instance.state << endl;
	cout << "instance.interface_v = " << conf.instance.interface_v << endl;
	cout << "instance.interface_name = " << conf.instance.interface_name << endl;
	cout << "instance.virtual_router_id = " << (int)conf.instance.virtual_router_id << endl;
	cout << "instance.priority = " << (int)conf.instance.priority << endl;
	cout << "instance.advert_int = " << (int)conf.instance.advert_int << endl;
	cout << "instance.virtual_ipaddress = [";
	for (int i = 0; i < conf.instance.virtual_ipaddress.size(); ++i) {
		if (i > 0) {
			cout << ", ";
		}
		cout << conf.instance.virtual_ipaddress[i];
	}
	cout << "]" << endl;
}