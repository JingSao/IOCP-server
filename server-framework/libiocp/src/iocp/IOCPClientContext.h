#ifndef _IOCP_CLIENT_CONTEXT_H_
#define _IOCP_CLIENT_CONTEXT_H_

#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <deque>
#include <functional>
#include <mutex>
#include <stdint.h>
#include "common/CommonMacros.h"

#pragma comment(lib, "ws2_32.lib")

#define OVERLAPPED_BUF_SIZE 1024
#define RECV_CACHE_MAX_SIZE 4096

namespace iocp {

    class ClientContext final
    {
    public:
        ClientContext(SOCKET s, const char *ip, uint16_t port, ULONG_PTR completionKey,
            const std::function<size_t (ClientContext *context, const char *buf, size_t len)> &recvCallback);
        ~ClientContext();

        int postRecv();
        int postSend(const char *buf, size_t len);

        DWORD processIO(DWORD bytesTransfered, LPOVERLAPPED overlapped);

        SOCKET recycleSocket() { SOCKET s = _socket; _socket = INVALID_SOCKET; return s; }

    protected:
        enum class OPERATION_TYPE
        {
            NULL_POSTED = 0,
            ACCEPT_POSTED,
            RECV_POSTED,
            SEND_POSTED,
        };

        typedef struct CUSTOM_OVERLAPPED
        {
            OVERLAPPED overlapped;
            WSABUF wsaBuf;
            OPERATION_TYPE type;
            char buf[OVERLAPPED_BUF_SIZE];
            void (*callback)(ClientContext *context);
        } CUSTOM_OVERLAPPED;

        CUSTOM_OVERLAPPED _sendOverlapped;
        CUSTOM_OVERLAPPED _recvOverlapped;
        SOCKET _socket;
        std::mutex _sendMutex;
        std::mutex _recvMutex;

        LAZY_SYNTHESIZE_C_ARRAY_READONLY(char, 16, _ip, IP);
        LAZY_SYNTHESIZE_READONLY(uint16_t, _port, Port);
        LAZY_SYNTHESIZE_READONLY(ULONG_PTR, _completionKey, CompletionKey);
        LAZY_SYNTHESIZE(void *, _userData, UserData);

    protected:
        std::vector<char> _sendCache;
        std::vector<char> _recvCache;
        std::deque<std::vector<char> > _sendQueue;

        std::function<size_t (ClientContext *, const char *, size_t)> _recvCallback;

        size_t doRecv(const char *buf, size_t len);
        void doSend();

        DECLEAR_NO_COPY_CLASS(ClientContext);
    };
}

#endif
