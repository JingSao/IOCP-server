#include "src/ClientConnection.h"

#include <iostream>
#include "../lightweight-3rdparty/msgpack/msgpack.hpp"

int main() {
    //std::vector<std::string> vec;
    //vec.push_back("Hello");
    //vec.push_back("MessagePack");
    std::unordered_map<std::string, int> vec;
    vec.insert(std::make_pair("123", 123));

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, vec);

    msgpack::unpacked msg;
    msgpack::unpack(&msg, sbuf.data(), sbuf.size());

    msgpack::object obj = msg.get();
    std::cout << obj << std::endl;

    //std::vector<std::string> rvec;
    std::map<std::string, int> rvec;
    obj.convert(&rvec);

    ClientConnection cc;
    cc.connentToServer("192.168.1.102", 8899);
    //cc.connentToServer("127.0.0.1", 8899);

    std::thread t([&cc]() {
        std::vector<char> buf;
        while (1) {
            if (cc.peekBuf(&buf)) {
                msgpack::sbuffer sbuf;
                sbuf.write(&buf[0], buf.size());

                msgpack::unpacked msg;
                msgpack::unpack(&msg, sbuf.data(), sbuf.size());

                msgpack::object obj = msg.get();
                std::vector<std::string> rvec;
                obj.convert(&rvec);

                std::for_each(rvec.begin(), rvec.end(), [](const std::string &s){ std::cout << s << std::endl; });
            }
        }
    });

    {
        std::vector<std::string> vec;
        vec.push_back("Hello");
        vec.push_back("MessagePack");

        msgpack::sbuffer sbuf;
        msgpack::pack(sbuf, vec);

        const char *str = sbuf.data();
        size_t len = sbuf.size();
        //char str[1024] = "Hello World!";
        //size_t len = strlen(str);
        while (1) {
            cc.sendBuf(str, len);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    cc.quit();

    return 0;
}
