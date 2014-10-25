#include "IOCPClientContext.h"

namespace iocp {

    ClientContext::ClientContext(SOCKET s, const char *ip, uint16_t port, const std::list<ClientContext *>::iterator &it,//ULONG_PTR completionKey,
        const std::function<size_t (ClientContext *context, const char *buf, size_t len)> &recvCallback)
        : _socket(s)
        , _port(port)
        , _iterator(it)
        , _userData(nullptr)
        , _recvCallback(recvCallback)
    {
        strncpy(_ip, ip, 16);
    }

    ClientContext::~ClientContext()
    {
        closesocket(_socket);
    }

    bool ClientContext::postRecv()
    {
        memset(&_recvOverlapped, 0, sizeof(CUSTOM_OVERLAPPED));
        _recvOverlapped.type = OPERATION_TYPE::RECV_POSTED;
        DWORD recvBytes = 0, flags = 0;

        WSABUF wsaBuf;
        wsaBuf.buf = _recvOverlapped.buf;
        wsaBuf.len = OVERLAPPED_BUF_SIZE;
        int ret = ::WSARecv(_socket, &wsaBuf, 1, &recvBytes, &flags, (LPOVERLAPPED)&_recvOverlapped, nullptr);
        if (ret == SOCKET_ERROR && ::WSAGetLastError() != ERROR_IO_PENDING)
        {
            return false;
        }
        return true;
    }

    size_t ClientContext::doRecv(const char *buf, size_t len)
    {
        if (_recvCache.empty())
        {
            size_t processed = _recvCallback(this, buf, len);
            if (processed < len)  // cache the left bytes
            {
                size_t left = len - processed;
                _recvCache.resize(left);
                memcpy(&_recvCache[0], buf + processed, left);
            }
        }
        else
        {
            size_t size = _recvCache.size();
            if (size + len > RECV_CACHE_LIMIT_SIZE)
            {
                return 0;
            }
            _recvCache.resize(size + len);
            memcpy(&_recvCache[size], buf, len);
            size_t processed = _recvCallback(this, &_recvCache[0], _recvCache.size());
            if (processed >= _recvCache.size())  // all cache bytes is processed
            {
                _recvCache.clear();
            }
            else if (processed > 0)  // cache the left bytes
            {
                size_t left = len - processed;
                memmove(&_recvCache[0], &_recvCache[processed], left);
                _recvCache.resize(left);
            }
        }

        return postRecv() ? len : 0;
    }

    int ClientContext::postSend(const char *buf, size_t len)
    {
        if (!_sendCache.empty())  // cached bytes is sending, put the current to queue
        {
            std::vector<char> temp;
            temp.resize(len);
            memcpy(&temp[0], buf, len);
            _sendQueue.push_back(std::move(temp));
            return -1;
        }

        memset(&_sendOverlapped, 0, sizeof(CUSTOM_OVERLAPPED));
        _sendOverlapped.type = OPERATION_TYPE::SEND_POSTED;

        DWORD sendBytes = 0;
        WSABUF wsaBuf;
        wsaBuf.buf = _sendOverlapped.buf;
        wsaBuf.len = OVERLAPPED_BUF_SIZE;
        if (len <= OVERLAPPED_BUF_SIZE)  // is overlapped enough to copy?
        {
            memcpy(_sendOverlapped.buf, buf, len);
            wsaBuf.len = len;
            int ret = ::WSASend(_socket, &wsaBuf, 1, &sendBytes, 0, (LPOVERLAPPED)&_sendOverlapped, nullptr);
            if (ret == SOCKET_ERROR && ::WSAGetLastError() != ERROR_IO_PENDING)
            {
                return 1;
            }
            return 0;
        }
        else
        {
            // cache the left bytes and send the overlapped
            _sendCache.resize(len - OVERLAPPED_BUF_SIZE);
            memcpy(&_sendCache[0], buf + OVERLAPPED_BUF_SIZE, len - OVERLAPPED_BUF_SIZE);

            memcpy(_sendOverlapped.buf, buf, OVERLAPPED_BUF_SIZE);
            int ret = ::WSASend(_socket, &wsaBuf, 1, &sendBytes, 0, (LPOVERLAPPED)&_sendOverlapped, nullptr);
            if (ret == SOCKET_ERROR && ::WSAGetLastError() != ERROR_IO_PENDING)
            {
                return 1;
            }
            return 0;
        }
    }

    void ClientContext::doSend()
    {
        if (_sendCache.empty())
        {
            return;  // there is nothing to sent
        }

        memset(&_sendOverlapped, 0, sizeof(CUSTOM_OVERLAPPED));
        _sendOverlapped.type = OPERATION_TYPE::SEND_POSTED;
        DWORD sendBytes = 0;

        WSABUF wsaBuf;
        wsaBuf.buf = _sendOverlapped.buf;
        wsaBuf.len = OVERLAPPED_BUF_SIZE;
        if (_sendCache.size() <= OVERLAPPED_BUF_SIZE)  // is overlapped enough to copy?
        {
            memcpy(_sendOverlapped.buf, &_sendCache[0], _sendCache.size());
            wsaBuf.len = _sendCache.size();
            ::WSASend(_socket, &wsaBuf, 1, &sendBytes, 0, (LPOVERLAPPED)&_sendOverlapped, nullptr);

            if (_sendQueue.empty())
            {
                _sendCache.clear();
            }
            else
            {
                _sendCache = std::move(_sendQueue.front());
                _sendQueue.pop_front();
            }
        }
        else
        {
            // cache the left bytes and send the overlapped
            memcpy(_sendOverlapped.buf, &_sendCache[0], OVERLAPPED_BUF_SIZE);
            ::WSASend(_socket, &wsaBuf, 1, &sendBytes, 0, (LPOVERLAPPED)&_sendOverlapped, nullptr);

            memmove(&_sendCache[0], &_sendCache[OVERLAPPED_BUF_SIZE], _sendCache.size() - OVERLAPPED_BUF_SIZE);
            _sendCache.resize(_sendCache.size() - OVERLAPPED_BUF_SIZE);
        }
    }

    DWORD ClientContext::processIO(DWORD bytesTransfered, LPOVERLAPPED overlapped)
    {
        DWORD ret = bytesTransfered;
        CUSTOM_OVERLAPPED *customOverlapped = (CUSTOM_OVERLAPPED *)overlapped;
        switch (customOverlapped->type)
        {
        case OPERATION_TYPE::ACCEPT_POSTED:
            break;
        case OPERATION_TYPE::RECV_POSTED:
            _recvMutex.lock();
            ret = doRecv(customOverlapped->buf, bytesTransfered);
            _recvMutex.unlock();
            break;
        case OPERATION_TYPE::SEND_POSTED:
            _sendMutex.lock();
            doSend();
            _sendMutex.unlock();
            break;
        case OPERATION_TYPE::NULL_POSTED:
            break;
        default:
            break;
        }

        return ret;
    }
}
