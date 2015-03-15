#ifndef _SERVER_FRAMEWORK_IMPL_H_
#define _SERVER_FRAMEWORK_IMPL_H_

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

#define USE_CPP_EXCEPTION

#ifdef USE_CPP_EXCEPTION
#   define TRY_BLOCK_BEGIN try {
#   define CATCH_ALL_EXCEPTIONS } catch (...) {
#   define CATCH_BLOCK_END }
#else
#   define TRY_BLOCK_BEGIN
#   define CATCH_ALL_EXCEPTIONS if (0) {
#   define CATCH_BLOCK_END }
#endif

#define ACCEPT_BUF_SIZE 1024

#define OVERLAPPED_BUF_SIZE 4096
#define RECV_CACHE_LIMIT_SIZE 32767

namespace iocp {
    namespace mp {
        template <typename _T> using vector = std::vector<_T, Allocator<_T> >;
        template <typename _T> using list = std::list<_T, Allocator<_T> >;
        template <typename _T> using deque = std::deque<_T, Allocator<_T> >;
    }

    namespace _impl {
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
        private:
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

            const char *getIp() const { return _ip; }
            uint16_t getPort() const { return _port; }

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

            bool startup(const char *ip, uint16_t port);
            void shutdown();

        public:
            static bool initialize();
            static bool uninitialize();

            const char *getIp() const { return _ip; }
            uint16_t getPort() const { return _port; }
            size_t getClientCount() const { return _clientCount; }

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

            CRITICAL_SECTION _clientCriticalSection;
            mp::list<_ClientContext *> _clientList;
            size_t _clientCount = 0;

        protected:
            std::function<_ClientContext *()> _allocateCtx;
            std::function<void (_ClientContext *ctx)> _deallocateCtx;

            // Returns the number of bytes processed.
            // If the return value less than len, the remainder bytes will be cached.
            std::function<size_t (_ClientContext *ctx, const char *buf, size_t len)> _onRecv;

            std::function<void (_ClientContext *ctx)> _onDisconnect;

        private:
            _ServerFramework(const _ServerFramework &) = delete;
            _ServerFramework(_ServerFramework &&) = delete;
            _ServerFramework &operator=(const _ServerFramework &) = delete;
            _ServerFramework &operator=(_ServerFramework &&) = delete;
        };
    }  // end of namespace _impl
}  // endof namespace iocp

#endif
