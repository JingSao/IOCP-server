#ifndef _IOCP_SERVER_FRAMEWORK_H_
#define _IOCP_SERVER_FRAMEWORK_H_

#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <stdint.h>
#include <vector>
#include <list>
#include <deque>
#include <utility>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define ACCEPT_BUF_SIZE 1024

#define OVERLAPPED_BUF_SIZE 4096
#define RECV_CACHE_LIMIT_SIZE 32767

namespace iocp {

    class ClientContext;

    enum class CLIENT_CONTEXT_INT_PTR { USERDATA, TAG, IP, PORT };
    intptr_t getClientContextIntPtr(const ClientContext *ctx, CLIENT_CONTEXT_INT_PTR idx);
    intptr_t setClientContextIntPtr(ClientContext *ctx, CLIENT_CONTEXT_INT_PTR idx, intptr_t newInt);

    enum class POST_RESULT { SUCCESS, FAIL, CACHED };
    POST_RESULT postSend(ClientContext *ctx, const char *buf, size_t len);
    POST_RESULT postRecv(ClientContext *ctx);

    struct PER_IO_OPERATION_DATA;

    class ServerFramework final
    {
    public:
        ServerFramework();
        ~ServerFramework();

        bool start(const char *ip, uint16_t port,
            const std::function<size_t (ClientContext *ctx, const char *buf, size_t len)> &clientRecvCallback,
            const std::function<void (ClientContext *ctx)> &clientDisconnectCallback);
        void end();

        static bool startup();
        static bool cleanup();

    private:
        bool beginAccept();
        bool getFunctionPointers();

        bool doAccept(PER_IO_OPERATION_DATA *ioData);
        bool postAccept(PER_IO_OPERATION_DATA *ioData);

        void worketThreadProc();

        size_t doRecv(ClientContext *ctx, const char *buf, size_t len) const;
        static void doSend(ClientContext *ctx);

    private:
        char _ip[16];
        uint16_t _port = 0;

        HANDLE _ioCompletionPort = NULL;
        std::vector<std::thread *> _workerThreads;
        volatile bool _shouldQuit = false;

        SOCKET _listenSocket = INVALID_SOCKET;
        std::thread *_acceptThread = nullptr;
        std::vector<PER_IO_OPERATION_DATA *> _allAcceptIOData;

        std::vector<SOCKET> _freeSocketPool;
        CRITICAL_SECTION _poolCriticalSection;

        LPFN_ACCEPTEX _acceptEx = nullptr;
        LPFN_GETACCEPTEXSOCKADDRS _getAcceptExSockAddrs = nullptr;
        LPFN_DISCONNECTEX _disconnectEx = nullptr;

        std::function<size_t (ClientContext *ctx, const char *buf, size_t len)> _clientRecvCallback;
        std::function<void (ClientContext *ctx)> _clientDisconnectCallback;

        CRITICAL_SECTION _clientCriticalSection;
        std::list<ClientContext *> _clientList;

    private:
        ServerFramework(const ServerFramework &) = delete;
        ServerFramework(ServerFramework &&) = delete;
        ServerFramework &operator=(const ServerFramework &) = delete;
        ServerFramework &operator=(ServerFramework &&) = delete;
    };
}

#endif
