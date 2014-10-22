#include "IOCPClientContext.h"

namespace iocp {

    ClientContext::ClientContext(SOCKET s, const char *ip, uint16_t port, std::function<size_t (ClientContext *context, const char *buf, size_t len)> recvCallback)
        : _socket(s)
        , _port(port)
        , _completionKey(0)
        , _userData(nullptr)
        , _recvCallback(recvCallback)
    {
        strncpy(_ip, ip, 16);
    }

    ClientContext::~ClientContext()
    {
        closesocket(_socket);
    }

    int ClientContext::postRecv()
    {
        memset(&_recvOverlapped, 0, sizeof(CUSTOM_OVERLAPPED));
        _recvOverlapped.wsaBuf.buf = _recvOverlapped.buf;
        _recvOverlapped.wsaBuf.len = OVERLAPPED_BUF_SIZE;
        _recvOverlapped.type = OPERATION_TYPE::RECV_POSTED;
        DWORD recvBytes = 0, flags = 0;

        return ::WSARecv(_socket, &_recvOverlapped.wsaBuf, 1, &recvBytes, &flags, (LPOVERLAPPED)&_recvOverlapped, nullptr);
    }

    void ClientContext::doRecv(const char *buf, size_t len)
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
        postRecv();
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
        _sendOverlapped.wsaBuf.buf = _sendOverlapped.buf;
        _sendOverlapped.wsaBuf.len = OVERLAPPED_BUF_SIZE;
        _sendOverlapped.type = OPERATION_TYPE::SEND_POSTED;
        DWORD sendBytes = 0;

        if (len <= OVERLAPPED_BUF_SIZE)  // is overlapped enough to copy?
        {
            memcpy(_sendOverlapped.buf, buf, len);
            _sendOverlapped.wsaBuf.len = len;
            return ::WSASend(_socket, &_sendOverlapped.wsaBuf, 1, &sendBytes, 0, (LPOVERLAPPED)&_sendOverlapped, nullptr);
        }
        else
        {
            // cache the left bytes and send the overlapped
            _sendCache.resize(len - OVERLAPPED_BUF_SIZE);
            memcpy(&_sendCache[0], buf + OVERLAPPED_BUF_SIZE, len - OVERLAPPED_BUF_SIZE);

            memcpy(_sendOverlapped.buf, buf, OVERLAPPED_BUF_SIZE);
            return ::WSASend(_socket, &_sendOverlapped.wsaBuf, 1, &sendBytes, 0, (LPOVERLAPPED)&_sendOverlapped, nullptr);
        }
    }

    void ClientContext::doSend()
    {
        if (_sendCache.empty())
        {
            return;  // there is nothing to sent
        }

        memset(&_sendOverlapped, 0, sizeof(CUSTOM_OVERLAPPED));
        _sendOverlapped.wsaBuf.buf = _sendOverlapped.buf;
        _sendOverlapped.wsaBuf.len = OVERLAPPED_BUF_SIZE;
        _sendOverlapped.type = OPERATION_TYPE::SEND_POSTED;
        DWORD sendBytes = 0;

        if (_sendCache.size() <= OVERLAPPED_BUF_SIZE)  // is overlapped enough to copy?
        {
            memcpy(_sendOverlapped.buf, &_sendCache[0], _sendCache.size());
            _sendOverlapped.wsaBuf.len = _sendCache.size();
            ::WSASend(_socket, &_sendOverlapped.wsaBuf, 1, &sendBytes, 0, (LPOVERLAPPED)&_sendOverlapped, nullptr);

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
            ::WSASend(_socket, &_sendOverlapped.wsaBuf, 1, &sendBytes, 0, (LPOVERLAPPED)&_sendOverlapped, nullptr);

            memmove(&_sendCache[0], &_sendCache[OVERLAPPED_BUF_SIZE], _sendCache.size() - OVERLAPPED_BUF_SIZE);
            _sendCache.resize(_sendCache.size() - OVERLAPPED_BUF_SIZE);
        }
    }

    void ClientContext::processIO(DWORD bytesTransfered, LPOVERLAPPED overlapped)
    {
        CUSTOM_OVERLAPPED *customOverlapped = (CUSTOM_OVERLAPPED *)overlapped;
        switch (customOverlapped->type)
        {
        case OPERATION_TYPE::ACCEPT_POSTED:
            break;
        case OPERATION_TYPE::RECV_POSTED:
            _recvMutex.lock();
            doRecv(customOverlapped->wsaBuf.buf, bytesTransfered);
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
    }
}
