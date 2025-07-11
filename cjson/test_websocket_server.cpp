/*
 * websocket server
 *
 * @build   make examples
 * @server  bin/websocket_server_test 9999
 * @client  bin/websocket_client_test ws://127.0.0.1:9999/
 * @js      html/websocket_client.html
 *
 */

#include "WebSocketServer.h"
#include "EventLoop.h"
#include "htime.h"
#include "hssl.h"

using namespace hv;

int test_websocket_server() {
    int port = 9999;
    printf("==========>websocket_server listening at port: %d\n", port);
    WebSocketServerCallbacks ws;
    ws.onopen = [](const WebSocketChannelPtr& channel, const std::string& url) {
        printf("==========>websocket_server onopen: GET %s\n", url.c_str());
        // send(time) every 1s
        setInterval(500, [channel](TimerID id) {
            if (channel->isConnected()) {
                char str[DATETIME_FMT_BUFLEN] = {0};
                datetime_t dt = datetime_now();
                datetime_fmt(&dt, str);
                channel->send(str);
            }
            else {
                killTimer(id);
            }
        });
    };
    ws.onmessage = [](const WebSocketChannelPtr& channel, const std::string& msg) { printf("==========>websocket_server onmessage: %s\n", msg.c_str()); };
    ws.onclose = [](const WebSocketChannelPtr& channel) { printf("==========>websocket_server onclose\n"); };

    websocket_server_t server;
    server.port = port;
#if 0
    server.ssl = 1;
    hssl_ctx_init_param_t param;
    memset(&param, 0, sizeof(param));
    param.crt_file = "cert/server.crt";
    param.key_file = "cert/server.key";
    if (hssl_ctx_init(&param) == NULL) {
        fprintf(stderr, "SSL certificate verify failed!\n");
        return -20;
    }
#endif
    server.ws = &ws;

    websocket_server_run(&server);
    return 0;
}
