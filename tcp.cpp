#include <windows.h>
#include "pluginapi.h" // nsis plugin

HINSTANCE g_hInstance;
HWND g_hwndParent;

// To work with Unicode version of NSIS, please use TCHAR-type
// functions for accessing the variables and the stack.

extern "C"
void __declspec(dllexport) CheckPort(HWND hwndParent, int string_size, 
                                      LPTSTR variables, stack_t **stacktop,
                                      extra_parameters *extra, ...)
{
  EXDLL_INIT();
  g_hwndParent = hwndParent;

  // note if you want parameters from the stack, pop them off in order.
  // i.e. if you are called via exdll::myFunction file.dat read.txt
  // calling popstring() the first time would give you file.dat,
  // and the second time would give you read.txt. 
  // you should empty the stack of your parameters, and ONLY your
  // parameters.

  // do your stuff here
  // get and parse port number
    TCHAR ports[11];
    popstringn(ports,10);
    int port = _ttoi(ports);
    if (port <= 0 || port > 65536)
    {
        pushstring(TEXT("socket_error"));
        return;
    }

    // start winsock
    WSADATA wsa_data;
    WORD req_version = MAKEWORD(1,1);
    if (WSAStartup(req_version, &wsa_data) != 0) {
        pushstring(TEXT("winsock_error"));
        return;
    }

    // create and bind a socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        pushstring(TEXT("socket_error"));
        return;
    }

    struct sockaddr_in server = {0};
    server.sin_port = htons((unsigned short)port);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (struct sockaddr *)&server, sizeof server) == SOCKET_ERROR)
    {
        if (GetLastError() == WSAEADDRINUSE)
        {
            closesocket(sock);
            WSACleanup();
            pushstring(TEXT("inuse"));
            return;
        }

        closesocket(sock);
        WSACleanup();
        pushstring(TEXT("bind_error"));
        return;
    }

    closesocket(sock);
    WSACleanup();
    pushstring(TEXT("free"));
    return;
}


BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
  g_hInstance = hInst;
  return TRUE;
}
