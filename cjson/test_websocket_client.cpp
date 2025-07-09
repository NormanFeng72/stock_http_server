/*
 * websocket client
 *
 * @build   make examples
 * @server  bin/websocket_server_test 8888
 * @client  bin/websocket_client_test ws://127.0.0.1:8888/
 * @js      html/websocket_client.html
 *
 */

#include "WebSocketClient.h"

using namespace hv;

int test_websocket_client() {
    const char* url = "ws://127.0.0.1:9999";

    WebSocketClient ws;
    ws.onopen = [&ws]() {
        printf("onopen\n");
        ws.send("hello");
    };
    ws.onclose = []() { printf("onclose\n"); };
    ws.onmessage = [](const std::string& msg) { printf("onmessage: %s\n", msg.c_str()); };

    // reconnect: 1,2,4,8,10,10,10...
    ReconnectInfo reconn;
    reconn.min_delay = 1000;
    reconn.max_delay = 30000;
    reconn.delay_policy = 2;
    ws.setReconnect(&reconn);

    ws.open(url);
    hv_delay(1000);

    // while (1) hv_delay(1000);
    return 0;
}
