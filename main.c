#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include <locale.h>
#pragma comment(lib,"ws2_32.lib")

DWORD WINAPI recv_thread(LPVOID lpParam)
{
    SOCKET client_socket = *(SOCKET*)lpParam;
    char recvbuf[1024] = {0};
    int ret;
    while ((ret = recv(client_socket, recvbuf, sizeof(recvbuf) - 1, 0)) > 0) {
        recvbuf[ret] = '\0';
        printf("\n[Server]: %s\n", recvbuf);
        printf("please input message:"); // 保持输入提示
        fflush(stdout);
    }
    printf("\n与服务器断开连接。\n");
    exit(0);
    return 0;
}

int main()
{
    setlocale(LC_ALL, "");
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2),&wsaData);

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(INVALID_SOCKET == client_socket)
    {
        printf("create socket failed !!!\n");
        return -1;
    }

    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(8080);
    target.sin_addr.s_addr = inet_addr("0.0.0.0");
    if (-1 == connect(client_socket, (struct sockaddr*)&target, sizeof(target)))
    {
        printf("connect server failed !!!\n");
        closesocket(client_socket);
        return -1;
    }

    DWORD id = GetCurrentProcessId();

    // 启动接收线程
    HANDLE hThread = CreateThread(NULL, 0, recv_thread, &client_socket, 0, NULL);

    while (1)
    {
        wchar_t wbuffer[512] = { 0 };
        char sendbuf[1024] = { 0 };
        printf("please input message:");
        fgetws(wbuffer, 512, stdin);
        size_t wlen = wcslen(wbuffer);
        if (wlen > 0 && wbuffer[wlen - 1] == L'\n') wbuffer[wlen - 1] = L'\0';

        char idbuf[32] = { 0 };
        sprintf_s(idbuf, sizeof(idbuf), "%lu:", id);

        int len = WideCharToMultiByte(CP_ACP, 0, wbuffer, -1, sendbuf + strlen(idbuf), sizeof(sendbuf) - (int)strlen(idbuf), NULL, NULL);
        memcpy(sendbuf, idbuf, strlen(idbuf));
        int sendlen = (int)strlen(idbuf) + (len > 0 ? len - 1 : 0);

        send(client_socket, sendbuf, sendlen, 0);
    }
    closesocket(client_socket);
    WSACleanup();
    return 0;
}