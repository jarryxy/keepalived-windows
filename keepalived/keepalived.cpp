#include <iostream>
#include <pthread.h>
#include <winsock2.h>  
#include <ws2tcpip.h>  
#include <csignal>
#include <Windows.h>
#include <sstream>
#include <locale>
#include <codecvt>
#include "vrrp.h"
#include "conf.h"
#include "vrrp_interface.h"
#pragma comment(lib, "pthreadVC2.lib")
#pragma comment(lib, "ws2_32.lib")
using namespace std;


pthread_t tids[5];
vrrp_conf conf;
bool running = true;

void* send_adv(void* args) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    vrrp_conf* conf = (vrrp_conf*)args;
    int fd = open_send_vrrp(*conf);
    conf->fd = fd;
    while (true)
    {
        if (!conf->preempt) {
            vrrp_send_adv(*conf, conf->instance.priority);
        }
        Sleep(conf->instance.advert_int * 1000);
    }
    return 0;
}

void* recv_adv(void* args) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    vrrp_conf* conf = (vrrp_conf*)args;
    int fd = open_recv_vrrp(*conf);
    conf->fd = fd;
    vrrp_recv_adv(*conf);
    return 0;
}




// 获取当前可执行文件的路径
std::wstring getExecutablePath()
{
    wchar_t buffer[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    std::wstring path(buffer);
    return path.substr(0, path.find_last_of(L"\\/"));
}

// 拼接绝对路径
std::wstring getAbsolutePath(const std::wstring& relativePath)
{
    std::wstring absolutePath = getExecutablePath() + L"\\" + relativePath;
    return absolutePath;
}

std::string wstringToString(const std::wstring& wstr)
{
    // 创建一个宽字符转换器
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    // 将宽字符串转换为 UTF-8 编码的窄字符串
    std::string str = converter.to_bytes(wstr);
    return str;
}

// 程序主函数
void keepalived() {
    wstring path = getAbsolutePath(L"keepalived.conf");
    conf = read_vrrp_conf(wstringToString(path));
    print_vrrp_conf(conf);
    conf.preempt = false;

    if (conf.instance.state == MASTER || conf.instance.state == "MASTER") {
        for (int i = 0; i < conf.instance.virtual_ipaddress.size(); i++) {
            //remove_vip(conf.instance.virtual_ipaddress[i], conf.instance.interface_name);
            Sleep(100);
            add_vip(conf.instance.virtual_ipaddress[i], "255.255.255.0", conf.instance.interface_name);
        }
        pthread_create(&tids[0], NULL, send_adv, (void*)&conf);
        Sleep(500);
        pthread_create(&tids[1], NULL, recv_adv, (void*)&conf);
    }
    else {
        for (int i = 0; i < conf.instance.virtual_ipaddress.size(); i++) {
            remove_vip(conf.instance.virtual_ipaddress[i], conf.instance.interface_name);
        }
        pthread_create(&tids[0], NULL, send_adv, (void*)&conf);
        Sleep(500);
        pthread_create(&tids[1], NULL, recv_adv, (void*)&conf);
    }

    while (running) {
        Sleep(1000);
    }
}

void keepalived_exit(DWORD fdwControl) {
    cout << "exit(" << fdwControl << ") keepalivedHA..." << endl;
    pthread_cancel(tids[0]);
    pthread_cancel(tids[1]);
    for (int i = 0; i < conf.instance.virtual_ipaddress.size(); i++) {
        remove_vip(conf.instance.virtual_ipaddress[i], conf.instance.interface_name);
    }
    // 发送权重0的adv
    vrrp_send_adv(conf, 0);
    running = false;
}