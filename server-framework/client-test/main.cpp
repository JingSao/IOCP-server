#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <functional>
#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <utility>
#include <assert.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define LOG_DEBUG printf

class ClientConnection final {

public:
    static bool startup() {
        WSADATA data;
        WORD ver = MAKEWORD(2, 2);
        int ret = ::WSAStartup(ver, &data);
        if (ret != 0) {
            LOG_DEBUG("WSAStartup failed: last error %d", ::WSAGetLastError());
            return false;
        }
        return true;
    }

    static bool cleanup() {
        return ::WSACleanup() == 0;
    }

    ClientConnection() { }
    ~ClientConnection() { }

    void connentToServer(const char *ip, unsigned short port) {
        if (_isConnectSuccess || _connectThread != nullptr) {
            return;
        }

        _socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (_socket == INVALID_SOCKET) {
            return;
        }

        _connectThread = new std::thread(std::bind(&ClientConnection::_connectToServer, this, ip, port));
    }

    void quit() {
        if (_socket != INVALID_SOCKET) {
            closesocket(_socket);
            _socket = INVALID_SOCKET;
        }

        if (_connectThread != nullptr) {
            _connectThread->join();
            delete _connectThread;
            _connectThread = nullptr;
        }

        if (_sendThread != nullptr) {
            _sendNeedQuit = true;
            _sendThread->join();
            delete _sendThread;
            _sendThread = nullptr;
        }

        if (_recvThread != nullptr) {
            _recvNeedQuit = true;
            _recvThread->join();
            delete _recvThread;
            _recvThread = nullptr;
        }
        _isConnectSuccess = false;
    }

    void sendBuf(const char *buf, int len) {
        _sendMutex.lock();
        _sendQueue.push_back(std::move(std::vector<char>(buf, buf + len)));
        _sendMutex.unlock();
    }

    void peekBuf(const std::function<void (std::vector<char> &&buf)> &callback) {
        _recvMutex.lock();
        if (_recvQueue.empty()) {
            _recvMutex.unlock();
        }
        callback(std::move(_recvQueue.front()));
        _recvQueue.pop_front();
        _recvMutex.unlock();
    }

private:
    ClientConnection(const ClientConnection &) = delete;
    ClientConnection(ClientConnection &&) = delete;
    ClientConnection &operator=(const ClientConnection &) = delete;
    ClientConnection &operator=(ClientConnection &&) = delete;

    SOCKET _socket = INVALID_SOCKET;
    std::thread *_connectThread = nullptr;
    bool _isConnectSuccess = false;

    std::thread *_sendThread = nullptr;
    std::mutex _sendMutex;
    std::deque<std::vector<char> > _sendQueue;
    volatile bool _sendNeedQuit = false;

    std::thread *_recvThread = nullptr;
    std::mutex _recvMutex;
    std::deque<std::vector<char> > _recvQueue;
    volatile bool _recvNeedQuit = false;

    void _connectToServer(const char *ip, unsigned short port) {
        struct sockaddr_in serverAddr = { 0 };
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = ::inet_addr(ip);
        serverAddr.sin_port = ::htons(port);

        int ret = ::connect(_socket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr));
        if (ret == SOCKET_ERROR) {
            LOG_DEBUG("Connot connect to %s:%hu\n", ip, port);
            return;
        }
        LOG_DEBUG("connect to %s:%hu\n", ip, port);

        _isConnectSuccess = true;
        _sendThread = new std::thread(std::bind(&ClientConnection::_sendThreadFunc, this));
        _recvThread = new std::thread(std::bind(&ClientConnection::_recvThreadFunc, this));
    }

    void _recvThreadFunc() {
        char buf[4];
        while (!_recvNeedQuit) {
            if (::recv(_socket, buf, 4, 0) == SOCKET_ERROR) {
                LOG_DEBUG("Recv error\n");
                break;
            }
            size_t len = buf[0];
            len <<= 8;
            len |= buf[1];
            len <<= 8;
            len |= buf[2];
            len <<= 8;
            len |= buf[3];

            std::vector<char> cache;
            cache.resize(len);
            if (::recv(_socket, &cache[0], len, 0) == SOCKET_ERROR) {
                LOG_DEBUG("Recv error\n");
                break;
            }

            LOG_DEBUG("recv: %.*s\n", len, &cache[0]);
            continue;

            _recvMutex.lock();
            _recvQueue.push_back(std::move(cache));
            _recvMutex.unlock();
        }
    }

    void _sendThreadFunc() {
        char buf[4];
        while (!_sendNeedQuit) {
            _sendMutex.lock();
            if (_sendQueue.empty()) {
                _sendMutex.unlock();
            } else {
                std::vector<char> packet(std::move(_sendQueue.front()));
                _sendQueue.pop_front();
                _sendMutex.unlock();

                size_t len = packet.size();
                assert(len <= INT_MAX);

                buf[0] = (len >> 24) & 0xFF;
                buf[1] = (len >> 16) & 0xFF;
                buf[2] = (len >> 8) & 0xFF;
                buf[3] = len & 0xFF;
                if (::send(_socket, buf, 4, 0) != 4) {
                    LOG_DEBUG("Send error\n");
                    break;
                }
                if (::send(_socket, &packet[0], len, 0) != len) {
                    LOG_DEBUG("Send error\n");
                    break;
                }
            }
        }
    }
};

int main() {
    ClientConnection::startup();

    ClientConnection cc;
    //cc.connentToServer("192.168.1.102", 8899);
    cc.connentToServer("127.0.0.1", 8899);

    char str[1024] = "Hello World!";
    //while (scanf("%1024[^\n]%*c", str) != EOF) {
    //while (scanf("%1024s", str) != EOF) {
    while (1) {
        cc.sendBuf(str, strlen(str));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    cc.quit();

    ClientConnection::cleanup();
    return 0;
}
