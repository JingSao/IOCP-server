#include "src/ClientConnection.h"

int main() {
    ClientConnection cc;
    cc.connentToServer("192.168.1.102", 8899);
    //cc.connentToServer("127.0.0.1", 8899);

    char str[1024] = "Hello World!";
    while (1) {
        cc.sendBuf(str, strlen(str));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    cc.quit();

    return 0;
}
