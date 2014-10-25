#ifndef _IOCP_SERVER_CONTROLLER_H_
#define _IOCP_SERVER_CONTROLLER_H_

#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include <utility>
#include <stdint.h>
#include "common/CommonMacros.h"

#define ACCEPT_BUF_SIZE 1024

namespace iocp {

    class ClientContext;

    class ServerController final
    {
    public:
        ServerController(std::function<size_t(ClientContext *context, const char *buf, size_t len)> clientRecvCallback,
            std::function<void (ClientContext *context)> clientDisconnectCallback);
        ~ServerController();

        bool start(const char *ip, uint16_t port);
        void end();

        static bool startup();
        static bool cleanup();

    protected:
        typedef struct ACCEPT_OVERLAPPED
        {
            OVERLAPPED overlapped;
            SOCKET clientSocket;
            char buf[ACCEPT_BUF_SIZE];
        } ACCEPT_OVERLAPPED;

        bool getFunctionPointers();

        bool postAccept(ACCEPT_OVERLAPPED *ol);

        void worketThreadProc();
        void acceptThreadProc();

        char _ip[16];
        uint16_t _port = 0;

        HANDLE _ioCompletionPort = NULL;
        std::vector<std::thread *> _workerThreads;
        volatile bool _shouldQuit = false;

        SOCKET _listenSocket = INVALID_SOCKET;
        std::thread *_acceptThread = nullptr;
        ClientContext *_listenContext = nullptr;
        std::vector<ACCEPT_OVERLAPPED *> _acceptOverlappeds;

        std::vector<SOCKET> _freeSocketPool;
        LPFN_ACCEPTEX _acceptEx = nullptr;
        LPFN_GETACCEPTEXSOCKADDRS _getAcceptExSockAddrs = nullptr;
        LPFN_DISCONNECTEX _disconnectEx = nullptr;

        std::function<size_t (ClientContext *context, const char *buf, size_t len)> _clientRecvCallback;
        std::function<void (ClientContext *context)> _clientDisconnectCallback;

        std::mutex _clientMutex;
        std::list<ClientContext *> _clientList;

        DECLEAR_NO_COPY_CLASS(ServerController);
    };
}

#endif
