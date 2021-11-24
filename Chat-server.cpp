#define _WINSOCK_DEPRECATED_NO_WARNINGS // 최신 VC++ 컴파일 시 경고 방지
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <list>
#include <string>

#define SERVERPORT 9000
#define REMOTEPORT 9010
#define BUFSIZE    512
#define MULTICASTIP "235.7.8.9"

struct Client_INFO {
    SOCKADDR_IN addr;
    char name[10];
};

void makeMassage(char* buf, char *name ) {

    char tmp[BUFSIZE + 1];
    strcpy_s(tmp, buf);
    
}

// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}





int main(int argc, char* argv[])
{
    int retval;


    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");


    // 지역 주소 바인딩 ( 서버 )
    SOCKADDR_IN testaddr;
    ZeroMemory(&testaddr, sizeof(testaddr));
    testaddr.sin_port = htons(SERVERPORT);
    testaddr.sin_family = AF_INET;
    testaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    retval = bind(sock, (SOCKADDR*)&testaddr, sizeof(testaddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // 전송 주소 ( 서버 -> 클라이언트 멀티캐스트 ) 
    SOCKADDR_IN serveraddr;
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_port = htons(REMOTEPORT);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(MULTICASTIP);

    // 통신 변수
    int addrlen;
    char buf[BUFSIZE + 1];
    char sendbuf[BUFSIZE + 1];
    SOCKADDR_IN clientaddr;
    std::list<Client_INFO> L_CINFO;
    std::list<Client_INFO> ::iterator iter;
    Client_INFO CINFO;
    int namelen;
    char ack[7] = "cntack";

    // 서버 통신 부분
    printf("******************* 서버 구동 *******************\n");
    while (1) {

        addrlen = sizeof(clientaddr);
        retval = recvfrom(sock, buf, BUFSIZE + 1, 0, (SOCKADDR*)&clientaddr, &addrlen);
        if (retval == SOCKET_ERROR) {
            err_display("recvfrom()");
            continue;
        }

        // 클라이언트 연결 정보 설정
        if (!strncmp(buf, "cnt", 3)) {
            Client_INFO CINFO;
            CINFO.addr = clientaddr;

            //닉네임 설정           연결 메세지 구조 cnt name \0 
            
            strcpy_s(CINFO.name, buf + 3);

            L_CINFO.push_back(CINFO);
            strncpy_s(buf, "cntack\0", 7);

            printf("접속 addr = %s, 접속 닉네임 = %s , 추가 리스트 수 : %d\n", inet_ntoa(CINFO.addr.sin_addr), CINFO.name,L_CINFO.size());

            retval = sendto(sock, buf, BUFSIZE, 0, (SOCKADDR*)&clientaddr, sizeof(clientaddr));
            if (retval == SOCKET_ERROR) {
                err_display("sendto()");
            }
            continue;
        }

        buf[retval - 1] = '\0';

        // 추가 기능 부분
        if (!strcmp(buf, "game start")) {
            printf("게임스타트\n");
        }


        printf("[받음 | IP : %s | PORT : %d] %s\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), buf);
        
        for (iter = L_CINFO.begin(); iter != L_CINFO.end(); iter++) {
            Client_INFO sendInfo = *iter;
            clientaddr = sendInfo.addr;

            //strcpy_s(sendbuf, sendInfo.name);
            printf("[전송 | IP : %s | NAME : %s] %s\n", inet_ntoa(clientaddr.sin_addr), sendInfo.name, buf);

            retval = sendto(sock, buf, BUFSIZE+1, 0, (SOCKADDR*)&clientaddr, sizeof(clientaddr));
            if (retval == SOCKET_ERROR) {
                err_display("sendto()");
            }

        }
        
    }

    closesocket(sock);
    printf("******************* 서버 종료 *******************\n");
    // 윈속 종료
    WSACleanup();
    return 0;
}
