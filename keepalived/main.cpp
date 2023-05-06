#include <Windows.h>
#include <csignal>
#include <iostream>

using namespace std;

// keepalived.h
void keepalived();
void keepalived_exit(DWORD fdwControl);
// keepalived.h

BOOL console_handler(DWORD ctrl_type)
{
    cout << "exit(" << ctrl_type << ") keepalivedHA..." << endl;

    switch (ctrl_type)
    {
    case CTRL_CLOSE_EVENT:
    case CTRL_C_EVENT:
        keepalived_exit(ctrl_type);
        return TRUE;
    default:
        return FALSE;
    }

}


int main(int argc, const char* argv[])
{
    keepalived();
    return 0;
}