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
#include "MemoryPool.h"

#pragma comment(lib, "ws2_32.lib")

#define ACCEPT_BUF_SIZE 1024

#define OVERLAPPED_BUF_SIZE 4096
#define RECV_CACHE_LIMIT_SIZE 32767

namespace iocp {

    namespace mp {
        template <typename _T> using vector = std::vector<_T, allocator<_T> >;
        template <typename _T> using list = std::list<_T, allocator<_T> >;
        template <typename _T> using deque = std::deque<_T, allocator<_T> >;
    }

    class _ServerFramework;

    enum class _OPERATION_TYPE
    {
        NULL_POSTED = 0,
        ACCEPT_POSTED,
        RECV_POSTED,
        SEND_POSTED,
    };

    // Extend OVERLAPPED structure. Typically, we set original OVERLAPPED as the first field.
    typedef struct _PER_IO_OPERATION_DATA
    {
        OVERLAPPED overlapped;
        _OPERATION_TYPE type;
        char buf[OVERLAPPED_BUF_SIZE];
    } _PER_IO_OPERATION_DATA;


    //
    // _ClientContext
    //

    class _ClientContext  // User-defined CompletionKey.
    {
    protected:
        // Socket is the 1st field, so that listenSocket can just use the socket as the CompletionKey.
        SOCKET _socket = INVALID_SOCKET;

        _PER_IO_OPERATION_DATA _sendIOData;
        _PER_IO_OPERATION_DATA _recvIOData;
        CRITICAL_SECTION _sendCriticalSection;
        CRITICAL_SECTION _recvCriticalSection;

        char _ip[16];
        uint16_t _port = 0;

        // The iterator for itself in the Server's ClientContext list, so that remove it expediently.
        // This is based on a characteristic that std::list's iterator won't become invalid when we erased other elements.
        mp::list<_ClientContext *>::iterator _iterator;

        mp::vector<char> _sendCache;
        mp::vector<char> _recvCache;
        mp::deque<mp::vector<char> > _sendQueue;

        friend class _ServerFramework;

    protected:
        _ClientContext();
        ~_ClientContext();

    public:
        enum class POST_RESULT { SUCCESS, FAIL, CACHED };

        POST_RESULT postRecv();
        POST_RESULT postSend(const char *buf, size_t len);

    private:
        _ClientContext(const _ClientContext &) = delete;
        _ClientContext(_ClientContext &&) = delete;
        _ClientContext &operator=(const _ClientContext &) = delete;
        _ClientContext &operator=(_ClientContext &&) = delete;
    };

    //
    // _ServerFramework
    //

    class _ServerFramework
    {
    protected:
        _ServerFramework();
        ~_ServerFramework();

        typedef std::function<size_t (_ClientContext *ctx, const char *buf, size_t len)> ClientRecvCallback;
        typedef std::function<void (_ClientContext *ctx)> ClientDisconnectCallback;

        bool startup(const char *ip, uint16_t port,
            const ClientRecvCallback &clientRecvCallback,
            const ClientDisconnectCallback &clientDisconnectCallback);
        void shutdown();

    public:
        static bool initialize();
        static bool uninitialize();

    private:
        bool beginAccept();
        bool getFunctionPointers();

        bool doAccept(_PER_IO_OPERATION_DATA *ioData);
        bool postAccept(_PER_IO_OPERATION_DATA *ioData);

        void worketThreadProc();

        bool doRecv(_ClientContext *ctx, const char *buf, size_t len) const;
        void doSend(_ClientContext *ctx) const;

    private:
        char _ip[16];
        uint16_t _port = 0;

        HANDLE _ioCompletionPort = NULL;
        mp::vector<std::thread *> _workerThreads;
        volatile bool _shouldQuit = false;

        SOCKET _listenSocket = INVALID_SOCKET;
        std::thread *_acceptThread = nullptr;
        mp::vector<_PER_IO_OPERATION_DATA *> _allAcceptIOData;

        mp::vector<SOCKET> _freeSocketPool;
        CRITICAL_SECTION _poolCriticalSection;

        LPFN_ACCEPTEX _acceptEx = nullptr;
        LPFN_GETACCEPTEXSOCKADDRS _getAcceptExSockAddrs = nullptr;
        LPFN_DISCONNECTEX _disconnectEx = nullptr;

        ClientRecvCallback _clientRecvCallback;
        ClientDisconnectCallback _clientDisconnectCallback;

        CRITICAL_SECTION _clientCriticalSection;
        mp::list<_ClientContext *> _clientList;

    private:
        _ServerFramework(const _ServerFramework &) = delete;
        _ServerFramework(_ServerFramework &&) = delete;
        _ServerFramework &operator=(const _ServerFramework &) = delete;
        _ServerFramework &operator=(_ServerFramework &&) = delete;
    };
}

#endif
