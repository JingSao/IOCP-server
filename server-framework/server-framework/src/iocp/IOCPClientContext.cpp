#define _CRT_SECURE_NO_WARNINGS

#include "IOCPClientContext.h"
#include "IOCPSocketMacros.h"

namespace iocp {

    ClientContext::ClientContext(SOCKET s, const char *ip, uint16_t port, std::function<void (ClientContext *context, const char *buf, size_t len)> recvCallback)
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
            if (len < 4)
            {
                _recvCache.resize(len);
                memcpy(&_recvCache[0], buf, len);
            }
            else
            {
                size_t bodySize = MAKE_BODY_SIZE(buf[0], buf[1], buf[2], buf[3]);
                size_t packetLen = bodySize + 4;
                if (packetLen <= len)
                {
                    _recvCallback(this, buf, len);
                }
                else
                {
                    _recvCache.resize(len);
                    memcpy(&_recvCache[0], buf, len);
                }
            }
        }
        else
        {
            size_t size = _recvCache.size();
            _recvCache.resize(size + len);
            memcpy(&_recvCache[size], buf, len);

            if (_recvCache.size() >= 4)
            {
                size_t bodySize = MAKE_BODY_SIZE(_recvCache[0], _recvCache[1], _recvCache[2], _recvCache[3]);
                size_t packetLen = bodySize + 4;
                if (packetLen <= _recvCache.size())
                {
                    _recvCallback(this, &_recvCache[0], _recvCache.size());
                    _recvCache.clear();
                }
            }
        }
        postRecv();
    }

    int ClientContext::postSend(const char *buf, size_t len)
    {
        if (!_sendCache.empty())
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

        if (len <= OVERLAPPED_BUF_SIZE)
        {
            memcpy(_sendOverlapped.buf, buf, len);
            _sendOverlapped.wsaBuf.len = len;
            return ::WSASend(_socket, &_sendOverlapped.wsaBuf, 1, &sendBytes, 0, (LPOVERLAPPED)&_sendOverlapped, nullptr);
        }
        else
        {
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
            if (_sendQueue.empty())
            {
                return;
            }

            _sendCache = std::move(_sendQueue.front());
            _sendQueue.pop_front();
        }

        memset(&_sendOverlapped, 0, sizeof(CUSTOM_OVERLAPPED));
        _sendOverlapped.wsaBuf.buf = _sendOverlapped.buf;
        _sendOverlapped.wsaBuf.len = OVERLAPPED_BUF_SIZE;
        _sendOverlapped.type = OPERATION_TYPE::SEND_POSTED;
        DWORD sendBytes = 0;

        if (_sendCache.size() <= OVERLAPPED_BUF_SIZE)
        {
            memcpy(_sendOverlapped.buf, &_sendCache[0], _sendCache.size());
            _sendOverlapped.wsaBuf.len = _sendCache.size();
            ::WSASend(_socket, &_sendOverlapped.wsaBuf, 1, &sendBytes, 0, (LPOVERLAPPED)&_sendOverlapped, nullptr);
            _sendCache.clear();
        }
        else
        {
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
            doRecv(customOverlapped->wsaBuf.buf, bytesTransfered);
            break;
        case OPERATION_TYPE::SEND_POSTED:
            doSend();
            break;
        case OPERATION_TYPE::NULL_POSTED:
            break;
        default:
            break;
        }
    }

}
