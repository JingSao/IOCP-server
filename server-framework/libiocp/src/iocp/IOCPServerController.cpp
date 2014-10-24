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
        : _port(0)
        , _ioCompletionPort(NULL)
        , _shouldQuit(false)
        , _listenSocket(INVALID_SOCKET)
        , _acceptThread(nullptr)
        , _acceptEx(nullptr)
        , _clientRecvCallback(clientRecvCallback)
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

        GUID guidAcceptEx = WSAID_ACCEPTEX;
        DWORD bytes = 0;

        if (::WSAIoctl(_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guidAcceptEx, sizeof(guidAcceptEx),
            &_acceptEx, sizeof(_acceptEx),
            &bytes, nullptr, nullptr) == SOCKET_ERROR)
        {
            _shouldQuit = true;
            return false;
        }

        _acceptThread = new std::thread([this](){ acceptThreadProc(); });

        // TODO:
        //SOCKET clientSocket = INVALID_SOCKET;
        //if (_freeSocketPool.empty())
        //{
        //    clientSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        //}
        //else
        //{
        //    clientSocket = _freeSocketPool.back();
        //    _freeSocketPool.pop_back();
        //}

        //if (clientSocket == INVALID_SOCKET)
        //{
        //    continue;
        //}

        //char lpOutputBuf[1024];
        //int outBufLen = 1024;
        //DWORD bytes = 0;
        //WSAOVERLAPPED overlap = { 0 };

        //BOOL r = _acceptEx(_listenSocket, clientSocket, lpOutputBuf,
        //    outBufLen - ((sizeof(sockaddr_in)+16) * 2),
        //    sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16,
        //    &bytes, &overlap);

        //if (r)
        //{
        //}

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

        std::for_each(_clinetList.begin(), _clinetList.end(), [](ClientContext *context) {
            COMPLETION_KEY *completionKey = (COMPLETION_KEY *)context->getCompletionKey();
            if (completionKey != nullptr)
            {
                delete completionKey;
            }
            delete context;
        });
        _clinetList.clear();

        ::CloseHandle(_ioCompletionPort);
        _ioCompletionPort = NULL;

        std::for_each(_freeSocketPool.begin(), _freeSocketPool.end(), &::closesocket);
        _freeSocketPool.clear();
    }

    void ServerController::worketThreadProc()
    {
        LPOVERLAPPED overlapped = nullptr;
        ULONG_PTR completionKey = (ULONG_PTR)nullptr;
        DWORD bytesTransfered = 0;

        std::function<void (COMPLETION_KEY *)> removeCompletionKey = [this](COMPLETION_KEY *completionKey) {
            std::list<ClientContext *>::iterator &it = completionKey->first;

            _clientDisconnectCallback(*it);

            SOCKET s = (*it)->recycleSocket();
            _freeSocketPool.push_back(s);
            delete *it;
            _clinetList.erase(it);
            delete completionKey;
        };

        while (!_shouldQuit)
        {
            BOOL ret = ::GetQueuedCompletionStatus(_ioCompletionPort, &bytesTransfered, &completionKey, &overlapped, INFINITE);
            if (!ret)
            {
                DWORD lastError = ::GetLastError();
                if (lastError == ERROR_NETNAME_DELETED)
                {
                    _clientMutex.lock();
                    removeCompletionKey((COMPLETION_KEY *)completionKey);
                    _clientMutex.unlock();
                    continue;
                }

                CONTINUE_IF(lastError == WAIT_TIMEOUT);
                CONTINUE_IF(overlapped != nullptr);
                break;
            }

            if (bytesTransfered == 0)
            {
                if (completionKey != (ULONG_PTR)nullptr)
                {
                    _clientMutex.lock();
                    removeCompletionKey((COMPLETION_KEY *)completionKey);
                    _clientMutex.unlock();
                }
                continue;
            }

            std::list<ClientContext *>::iterator &it = ((COMPLETION_KEY *)completionKey)->first;
            if ((*it)->processIO(bytesTransfered, overlapped) != bytesTransfered)
            {
                _clientMutex.lock();
                removeCompletionKey((COMPLETION_KEY *)completionKey);
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
                _clinetList.push_front(nullptr);
                std::list<ClientContext *>::iterator it = _clinetList.begin();
                _clientMutex.unlock();

                COMPLETION_KEY *completionKey = new COMPLETION_KEY(std::make_pair(it, false));
                ClientContext *context = new ClientContext(clientSocket, ip, port, (ULONG_PTR)completionKey, _clientRecvCallback);;
                *completionKey->first = context;

                ::CreateIoCompletionPort((HANDLE)clientSocket, _ioCompletionPort, (ULONG_PTR)completionKey, 0);

                int ret = context->postRecv();
                if (ret == SOCKET_ERROR && ::WSAGetLastError() == ERROR_IO_PENDING)
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
