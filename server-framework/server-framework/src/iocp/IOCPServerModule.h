#ifndef _IOCP_SERVER_MODULE_H_
#define _IOCP_SERVER_MODULE_H_

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

namespace iocp {

    class ClientContext;

    class ServerModule final
    {
    public:
        ServerModule();
        ~ServerModule();

        bool start(const char *ip, uint16_t port);
        void end();

    protected:
        void worketThreadProc();
        void acceptThreadProc();

        char _ip[16];
        uint16_t _port;

        HANDLE _ioCompletionPort;
        std::vector<std::thread *> _workerThreads;
        volatile bool _shouldQuit;

        SOCKET _listenSocket;
        std::thread *_acceptThread;

        std::vector<SOCKET> _freeSocketPool;
        LPFN_ACCEPTEX _acceptEx;
    public:
        std::function<void (ClientContext *context, const char *buf, size_t len)> _clientRecvCallback;
        std::function<void (ClientContext *context)> _clientDisconnectCallback;
    protected:
        std::mutex *_clientMutex;
        std::list<ClientContext *> _clinetList;

        typedef std::pair<std::list<ClientContext *>::iterator, bool> COMPLETION_KEY;

        DECLEAR_NO_COPY_CLASS(ServerModule);
    };
}

#endif
