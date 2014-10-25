#include <algorithm>
#include "IOCPServerController.h"
#include "IOCPClientContext.h"

#define CONTINUE_IF(_cond_) if (_cond_) continue

namespace iocp {

    bool ServerController::startup()
    {
        WSADATA data;
        WORD ver = MAKEWORD(2, 2);
        int ret = ::WSAStartup(ver, &data);
        if (ret != 0)
        {
            LOG_DEBUG("WSAStartup failed: last error %d", ::WSAGetLastError());
            return false;
        }
        return true;
    }

    bool ServerController::cleanup()
    {
        return ::WSACleanup() == 0;
    }

    ServerController::ServerController(std::function<size_t (ClientContext *context, const char *buf, size_t len)> clientRecvCallback,
            std::function<void (ClientContext *context)> clientDisconnectCallback)
        : _clientRecvCallback(clientRecvCallback)
        , _clientDisconnectCallback(clientDisconnectCallback)
    {
        _ip[0] = '\0';
    }

    ServerController::~ServerController()
    {
        end();
    }

    bool ServerController::start(const char *ip, uint16_t port)
    {
        _shouldQuit = false;

        _ioCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
        if (_ioCompletionPort == NULL)
        {
            return false;
        }

        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);
        DWORD workerThreadCnt = systemInfo.dwNumberOfProcessors * 2;
        LOG_DEBUG("workerThreadCnt = %u\n", workerThreadCnt);

        while (workerThreadCnt-- > 0)
        {
            _workerThreads.push_back(new std::thread([this](){ worketThreadProc(); }));
        }

        _listenSocket = ::WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
        if (_listenSocket == INVALID_SOCKET)
        {
            _shouldQuit = true;
            return false;
        }

        _port = port;
        struct sockaddr_in serverAddr = { 0 };
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = ::htons(_port);
        if (ip != nullptr)
        {
            strncpy(_ip, ip, 16);
            serverAddr.sin_addr.s_addr = ::inet_addr(ip);
        }

        if (::bind(_listenSocket, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) == INVALID_SOCKET)
        {
            _shouldQuit = true;
            return false;
        }

        if (::listen(_listenSocket, SOMAXCONN) == INVALID_SOCKET)
        {
            _shouldQuit = true;
            return false;
        }

        _listenContext = new ClientContext(INVALID_SOCKET, "", 0, _clientList.begin(), nullptr);
        ::CreateIoCompletionPort((HANDLE)_listenSocket, _ioCompletionPort, (ULONG_PTR)_listenContext, 0);

        if (!getFunctionPointers())
        {
            _shouldQuit = true;
            return false;
        }

        //_acceptThread = new std::thread([this](){ acceptThreadProc(); });
        //return true;

        std::function<bool()> func = [this](){
            ACCEPT_OVERLAPPED *ol = new ACCEPT_OVERLAPPED;
            memset(ol, 0, sizeof(ACCEPT_OVERLAPPED));
            _acceptOverlappeds.push_back(ol);

            if (postAccept(ol))
            {
                return true;
            }
            _acceptOverlappeds.pop_back();
            delete ol;
            return false;
        };

        func();
        return true;
    }

    bool ServerController::postAccept(ACCEPT_OVERLAPPED *ol)
    {
        SOCKET clientSocket = INVALID_SOCKET;
        if (_freeSocketPool.empty())
        {
            clientSocket = //::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
        }
        else
        {
            clientSocket = _freeSocketPool.back();
            _freeSocketPool.pop_back();
        }

        if (clientSocket == INVALID_SOCKET)
        {
            return false;
        }

        DWORD bytes = 0;
        ol->clientSocket = clientSocket;
        BOOL ret = _acceptEx(_listenSocket, clientSocket, ol->buf,
            0,//ACCEPT_BUF_SIZE - ((sizeof(sockaddr_in) + 16) * 2),
            sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16,
            &bytes, &ol->overlapped);

        if (!ret && ::WSAGetLastError() != ERROR_IO_PENDING)
        {
            return false;
        }

        return true;
    }

    void ServerController::end()
    {
        _shouldQuit = true;
        if (_listenSocket != INVALID_SOCKET)
        {
            ::closesocket(_listenSocket);
        }

        if (_acceptThread != nullptr)
        {
            _acceptThread->join();
            delete _acceptThread;
            _acceptThread = nullptr;
        }
        _listenSocket = INVALID_SOCKET;

        for (unsigned i = _workerThreads.size(); i > 0; --i)
        {
            ::PostQueuedCompletionStatus(_ioCompletionPort, 0, (ULONG_PTR)nullptr, nullptr);
        }

        std::for_each(_workerThreads.begin(), _workerThreads.end(), [](std::thread *t) {
            t->join();
            delete t;
        });
        _workerThreads.clear();

        std::for_each(_clientList.begin(), _clientList.end(), [](ClientContext *context) { delete context; });
        _clientList.clear();

        std::for_each(_acceptOverlappeds.begin(), _acceptOverlappeds.end(), [](ACCEPT_OVERLAPPED *ol) { delete ol; });
        _acceptOverlappeds.clear();

        ::CloseHandle(_ioCompletionPort);
        _ioCompletionPort = NULL;

        std::for_each(_freeSocketPool.begin(), _freeSocketPool.end(), &::closesocket);
        _freeSocketPool.clear();
        delete _listenContext;
        _listenContext = nullptr;
    }

    bool ServerController::getFunctionPointers()
    {
        GUID guidAcceptEx = WSAID_ACCEPTEX;
        DWORD bytes = 0;

        if (::WSAIoctl(_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guidAcceptEx, sizeof(guidAcceptEx),
            &_acceptEx, sizeof(_acceptEx),
            &bytes, nullptr, nullptr) == SOCKET_ERROR)
        {
            return false;
        }

        GUID guidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
        if (::WSAIoctl(_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guidGetAcceptExSockAddrs, sizeof(guidGetAcceptExSockAddrs),
            &_getAcceptExSockAddrs, sizeof(_getAcceptExSockAddrs),
            &bytes, nullptr, nullptr) == SOCKET_ERROR)
        {
            return false;
        }

        GUID guidDisconnectEx = WSAID_DISCONNECTEX;
        if (::WSAIoctl(_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guidDisconnectEx, sizeof(guidDisconnectEx),
            &_disconnectEx, sizeof(_disconnectEx),
            &bytes, nullptr, nullptr) == SOCKET_ERROR)
        {
            return false;
        }

        return true;
    }

    void ServerController::worketThreadProc()
    {
        LPOVERLAPPED overlapped = nullptr;
        ULONG_PTR completionKey = (ULONG_PTR)nullptr;
        DWORD bytesTransfered = 0;

        std::function<void (ClientContext *)> removeExceptional = [this](ClientContext *context) {
            _clientDisconnectCallback(context);

            SOCKET s = context->recycleSocket();
            _disconnectEx(s, nullptr, TF_REUSE_SOCKET, 0);
            _freeSocketPool.push_back(s);
            _clientList.erase(context->getIterator());
            delete context;
        };

        while (!_shouldQuit)
        {
            BOOL ret = ::GetQueuedCompletionStatus(_ioCompletionPort, &bytesTransfered, &completionKey, &overlapped, INFINITE);
            ClientContext *context = (ClientContext *)completionKey;
            if (!ret)
            {
                DWORD lastError = ::GetLastError();
                if (lastError == ERROR_NETNAME_DELETED)
                {
                    if (context != _listenContext)
                    {
                        _clientMutex.lock();
                        removeExceptional(context);
                        _clientMutex.unlock();
                    }
                    continue;
                }

                CONTINUE_IF(lastError == WAIT_TIMEOUT);
                CONTINUE_IF(overlapped != nullptr);
                break;
            }

            if (context == _listenContext)
            {
                std::vector<ACCEPT_OVERLAPPED *>::iterator it = std::find_if(_acceptOverlappeds.begin(), _acceptOverlappeds.end(),
                    [overlapped](ACCEPT_OVERLAPPED *ol) {
                    return &ol->overlapped == overlapped;
                });
                if (it != _acceptOverlappeds.end())
                {
                    struct sockaddr_in *remoteAddr = nullptr, *localAddr = nullptr;
                    int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);

                    _getAcceptExSockAddrs((*it)->buf, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
                        (sockaddr **)&localAddr, &localLen, (sockaddr **)&remoteAddr, &remoteLen);

                    const char *ip = ::inet_ntoa(remoteAddr->sin_addr);
                    uint16_t port = ::ntohs(remoteAddr->sin_port);

                    LOG_DEBUG("%s %hu", ::inet_ntoa(remoteAddr->sin_addr), ::ntohs(remoteAddr->sin_port));
                    LOG_DEBUG("%s %hu", ::inet_ntoa(localAddr->sin_addr), ::ntohs(localAddr->sin_port));

                    _clientMutex.lock();
                    _clientList.push_front(nullptr);
                    std::list<ClientContext *>::iterator it1 = _clientList.begin();
                    _clientMutex.unlock();

                    ClientContext *context = new ClientContext((*it)->clientSocket, ip, port, it1, _clientRecvCallback);
                    *it1 = context;
                    ::CreateIoCompletionPort((HANDLE)(*it)->clientSocket, _ioCompletionPort, (ULONG_PTR)context, 0);

                    bool ret = context->postRecv();
                    if (ret)
                    {
                        LOG_DEBUG("%16s:%5hu connected\n", ip, port);
                    }
                    else
                    {
                        LOG_DEBUG("%16s:%5hu post recv failed\n", ip, port);
                    }

                    postAccept(*it);
                }

                continue;
            }

            if (bytesTransfered == 0)
            {
                if (context != nullptr)
                {
                    _clientMutex.lock();
                    removeExceptional(context);
                    _clientMutex.unlock();
                }
                continue;
            }

            if (context->processIO(bytesTransfered, overlapped) != bytesTransfered)
            {
                _clientMutex.lock();
                removeExceptional(context);
                _clientMutex.unlock();
            }
        }
    }

    void ServerController::acceptThreadProc()
    {
        while (!_shouldQuit)
        {
            struct sockaddr_in clientAddr;
            int len = sizeof(struct sockaddr_in);

            // TODO: timeout
            SOCKET clientSocket = ::WSAAccept(_listenSocket, (struct sockaddr *)&clientAddr, &len, nullptr, 0);
            if (clientSocket != INVALID_SOCKET)
            {
                char *ip = ::inet_ntoa(clientAddr.sin_addr);
                uint16_t port = clientAddr.sin_port;

                _clientMutex.lock();
                _clientList.push_front(nullptr);
                std::list<ClientContext *>::iterator it = _clientList.begin();
                _clientMutex.unlock();

                ClientContext *context = new ClientContext(clientSocket, ip, port, it, _clientRecvCallback);
                *it = context;
                ::CreateIoCompletionPort((HANDLE)clientSocket, _ioCompletionPort, (ULONG_PTR)context, 0);

                bool ret = context->postRecv();
                if (ret)
                {
                    LOG_DEBUG("%16s:%5hu connected\n", ip, port);
                }
                else
                {
                    LOG_DEBUG("%16s:%5hu post recv failed\n", ip, port);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}
