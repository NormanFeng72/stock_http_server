/*
 * sample http server
 * more detail see examples/httpd
 *
 */

#include "HttpServer.h"
#include "cJSON.h"
#include "main.h"
#include "utils.h"
#include "string.h"
#include <string>
#include <unistd.h>
#include <sanitizer/asan_interface.h>
#include "signal.h"
#include <sys/wait.h>
#include "requests.h"
#include "hthread.h" // import hv_gettid
#include "hloop.h"
#include "hbase.h"
#include "hlog.h"
#include "nlog.h"
#include <stdio.h>
#include "http_client.h"
#include "HttpMessage.h"

#ifdef __cplusplus
extern "C" {
#endif
void __gcov_flush();
#ifdef __cplusplus
}
#endif

int port;
pid_t child_pid = -1;

extern "C" {
int test_cjson(char *content);
int start_tcp_echo_server();
int test_hloop();
int test_connect(int port);
int test_date();
int test_hatomic_main();
int test_hmutex();
int test_md5(char *data);
int test_sha1(char *data);
int test_socketpair();
}
int test_nmap();
int test_curl(int argc, char *argv[]);
int test_hstring();
int test_hthread();
int test_ifconfig();
int test_hdir();
int test_objectpool();
int test_sync();
int test_base64(char *data);
int test_parse_confile();
int test_websocket_server();
int test_websocket_client();

static int response_status(HttpResponse *resp, int code = 200, const char *message = NULL) {
    resp->Set("code", code);
    if (message == NULL) message = http_status_str((enum http_status)code);
    resp->Set("message", message);
    resp->DumpBody();
    return code;
}

int test_http_client(char *url) {
    HttpRequest req;
    req.method = HTTP_GET;
    req.url = url;
    req.timeout = 3;
    HttpResponse res;
    int ret = http_client_send(&req, &res);
    printf("%s\n", req.Dump(true, true).c_str());
    if (ret != 0) {
        printf("* Failed:%s:%d\n", http_client_strerror(ret), ret);
    }
    else {
        printf("%s\n", res.Dump(true, true).c_str());
    }
    return ret;
}

static void test_http_async_client(int *finished) {
    printf("test_http_async_client request thread tid=%ld\n", hv_gettid());
    char url[128];
    sprintf(url, "http://127.0.0.1:%d/echo", port);

    HttpRequestPtr req(new HttpRequest);
    req->method = HTTP_POST;
    // req->url = "127.0.0.1:9090/echo";
    req->url = strdup(url);
    req->headers["Connection"] = "keep-alive";
    req->body = "this is an async request.";
    req->timeout = 1;
    printf("=> test_http_async_client, url: %s\n", req->url.c_str());
    http_client_send_async(req, [finished](const HttpResponsePtr &resp) {
        printf("test_http_async_client response thread tid=%ld\n", hv_gettid());
        if (resp == NULL) {
            printf("request failed!\n");
        }
        else {
            printf("%d %s\r\n", resp->status_code, resp->status_message());
            printf("%s\n", resp->body.c_str());
        }
        *finished = 1;
    });
}

static void test_http_sync_get() {
    // auto resp = requests::get("http://www.example.com");
    //
    // make clean && make WITH_OPENSSL=yes
    // auto resp = requests::get("https://www.baidu.com");
    char url[128];
    sprintf(url, "http://127.0.0.1:%d/User", port);

    // auto resp = requests::get("http://127.0.0.1:9090/WithdrawOrder?order_id=2");
    auto resp = requests::get(url);
    if (resp == NULL) {
        printf("request failed!\n");
    }
    else {
        printf("%d %s\r\n", resp->status_code, resp->status_message());
        printf("%s\n", resp->body.c_str());
    }
}

static void test_http_sync_post() {
    char url[128];
    sprintf(url, "http://127.0.0.1:%d/echo", port);

    hv::Json jroot;
    jroot["user"] = "admin";
    http_headers headers;
    headers["Content-Type"] = "application/json";
    // resp = requests::post("127.0.0.1:9090/echo", jroot.dump(), headers);
    auto resp = requests::post(url, jroot.dump(), headers);
    if (resp == NULL) {
        printf("request failed!\n");
    }
    else {
        printf("%d %s\r\n", resp->status_code, resp->status_message());
        printf("%s\n", resp->body.c_str());
    }
}

/******************************************************************************\
 *
 *      Register sigacation
 *
\******************************************************************************/
static void sigterm_handler(int signum) {
    printf("received signal\n");
    __gcov_flush();
    if (child_pid != -1) {
        kill(child_pid, SIGTERM);    // 发送SIGTERM信号给子进程
        waitpid(child_pid, NULL, 0); // 等待子进程结束
    }
    __gcov_flush();
    exit(0);
}
void reg_sigterm_handler(void (*handler)(int s)) {

    struct sigaction new_action;
    /* setup signal hander */
    new_action.sa_handler = sigterm_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    if (sigaction(SIGINT, &new_action, NULL) == -1) {
        printf("register wrong\n");
        exit(0);
    }
    if (sigaction(SIGSEGV, &new_action, NULL) == -1) {
        printf("register wrong\n");
        exit(0);
    }
    if (sigaction(SIGQUIT, &new_action, NULL) == -1) {
        printf("register wrong\n");
        exit(0);
    }
}

char logTxt[2048];
char errorTxt[2048];
AccountClass *account;
QuoteClass *quote;
AcctListClass *acc_list;
HoldingsClass *holdings;
std::list<OrderClass> *order_list;
static long order_id;
char success_logfile[] = "log/success.txt";
char log_file_name[] = "log/log.txt";
int i_data_len = 66;

bool isValidMktID(int mktID) {
    if (mktID < SHA || mktID > SME) {
        return false;
    }
    return true;
}

int initial() {
    account = new AccountClass();
    quote = new QuoteClass();
    holdings = new HoldingsClass();
    order_list = new std::list<OrderClass>();
    memset(logTxt, 0, 2048);
    memset(errorTxt, 0, 2048);
    int ret_code;
    char acct_id[] = "1";
    // 读取账户列表，并（根据acct_id）取得测试账户
    acc_list = new AcctListClass();
    acc_list->read();
    ret_code = acc_list->getAcct(acct_id, account);
    if (ret_code != 0) {
        sprintf(errorTxt, "Error: 账户id=[%s]不存在!\n", acct_id);
        printf("Error: 账户id=[%s]不存在!\n", acct_id);
        // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
        return -1;
    }

    // 读取行情列表
    quote->read();

    // 读取持仓
    holdings->read();

    // 打印信息
    printf("============================acct============================\n");
    account->print();
    printf("============================holdings============================\n");
    holdings->print();

    // 初始化交易申报队列

    // 委托编号从1开始
    order_id = 1;

    // 删除之前的log文件
    // std::system("rm -f log/*.txt");

    return 0;
}

int queryStockbyCode(char *stock_code, StockClass *stock) {
    int ret = quote->queryStockbyCode(stock_code, stock);
    if (ret != 0) {

        printf("Error: 股票[%s]不存在!\n", stock_code);
        sprintf(errorTxt, "Error: 股票[%s]不存在!\n", stock_code);
        // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
        return -404;
    }
    memset(logTxt, 0, 2048);
    sprintf(logTxt, "%s, %s, %s, %s, %s\n", UtilsClass::getDateTimeString().c_str(), "用代码查股票", stock->stock_code.c_str(), stock->stock_name.c_str(),
            stock->mkt.c_str());
    // UtilsClass::writeMsg(success_logfile, logTxt);
    return 0;
}

int queryStock(char *stock_code, int mkt_id, StockClass *stock) {
    if (isValidMktID(mkt_id) == false) {
        printf("Error: mkt_id 无效[%d], 有效的范围是[大于等于 %d, 且小于等于 %d]!\n", mkt_id, SHA, SME);
        sprintf(errorTxt, "Error: mkt_id 无效[%d], 有效的范围是[大于等于 %d, 且小于等于 %d]!\n", mkt_id, SHA, SME);
        // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
        return -404;
    }

    int ret = quote->queryStock(stock_code, mkt_id, stock);
    if (ret != 0) {
        printf("Error: 股票[%s]不存在!\n", stock_code);
        sprintf(errorTxt, "Error: 股票[%s]不存在!\n", stock_code);
        // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
        return -404;
    }
    memset(logTxt, 0, 2048);
    sprintf(logTxt, "%s, %s, %s, %s, %s\n", UtilsClass::getDateTimeString().c_str(), "复合条件查股票", stock->stock_code.c_str(), stock->stock_name.c_str(),
            stock->mkt.c_str());
    // UtilsClass::writeMsg(success_logfile, logTxt);
    return 0;
}

int buyStock(StockClass *stock, float price, int qty) {

    stock->print();
    // 检查账户是否有开通该市场的交易许可
    if (account->mktPermission((char *)stock->mkt.c_str()) != true) {
        printf("Error: 账户Acct[name=%s]没有市场[%s]的交易许可,需先开户!\n", account->cust_name.c_str(), stock->mkt.c_str());
        sprintf(errorTxt, "Error: 账户Acct[name=%s]没有市场[%s]的交易许可,需先开户!\n", account->cust_name.c_str(), stock->mkt.c_str());
        // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
        return -501;
    }
    // 检查交易数量大于0
    if (qty <= 0) {
        printf("Error: 买入数量必须大于零!\n");
        sprintf(errorTxt, "Error: 买入数量必须大于零!\n");
        // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
        return -502;
    }

    // 检查报价
    if (price < 0.01 && price > 500) {
        printf("Error: 报价参数不正确! \n");
        return -510;
    }

    // 检查交易数量是否100的倍数
    // if (qty % 100 != 0) {
    //     printf("Error: 买入数量[%d]必须是100的整数倍!\n", qty);
    //     sprintf(errorTxt, "Error: 买入数量[%d]必须是100的整数倍!\n", qty);
    //     // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
    //     return -502;
    // }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //  以下部分新添加，是为了检验迭代的效果
    if (strcmp(stock->mkt.c_str(), "上海A股") == 0 && qty % 1000 != 0) {
        printf("=========Error: 上海A股, 买入数量[%d]必须是1000的整数倍!\n", qty);
        return -503;
    }
    else if (strcmp(stock->mkt.c_str(), "上海B股") == 0 && qty % 200 != 0) {
        printf("=========Error: 上海B股, 买入数量[%d]必须是200的整数倍!\n", qty);
        return -504;
    }
    else if (strcmp(stock->mkt.c_str(), "深圳A股") == 0 && qty % 500 != 0) {
        printf("=========Error: 深圳A股, 买入数量[%d]必须是500的整数倍!\n", qty);
        return -505;
    }
    else if (strcmp(stock->mkt.c_str(), "深圳B股") == 0 && qty % 300 != 0) {
        printf("=========Error: 深圳B股, 买入数量[%d]必须是300的整数倍!\n", qty);
        return -506;
    }
    else if (strcmp(stock->mkt.c_str(), "创业板") == 0 && qty % 600 != 0) {
        printf("=========Error: 创业板, 买入数量[%d]必须是600的整数倍!\n", qty);
        return -507;
    }
    else if (strcmp(stock->mkt.c_str(), "科创板") == 0 && qty % 700 != 0) {
        printf("=========Error: 创业板, 买入数量[%d]必须是700的整数倍!\n", qty);
        return -508;
    }
    else if (strcmp(stock->mkt.c_str(), "中小板") == 0 && qty % 800 != 0) {
        printf("=========Error: 创业板, 买入数量[%d]必须是800的整数倍!\n", qty);
        return -509;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 检查账户余额是否充足
    if (qty * price > account->balance) {
        printf("Error: 余额不足[%f], 买入[Code=%s, qty=%d, price=%f]!\n", account->balance, stock->stock_code.c_str(), qty, price);
        sprintf(errorTxt, "Error: 余额不足[%f], 买入[Code=%s, qty=%d, price=%f]!\n", account->balance, stock->stock_code.c_str(), qty, price);
        // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
        return -505;
    }

    // 检查报价是否大于昨日收盘价的10%（涨跌幅限制统一按10%计）
    if ((price > (std::atof(stock->close_price.c_str()) + 0.00001) * 1.1) || price < ((std::atof(stock->close_price.c_str()) + 0.00001) * 0.90)) {
        printf("Error: 报价超过涨跌幅限制, 报价[股票=%s, 昨收=%s, price=%f]!\n", stock->stock_code.c_str(), stock->close_price.c_str(), price);
        sprintf(errorTxt, "Error: 报价超过涨跌幅限制, 报价[股票=%s, 昨收=%s, price=%f]!\n", stock->stock_code.c_str(), stock->close_price.c_str(), price);
        // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
        return -506;
    }

    // 如果报价高于昨日收盘，则直接买入成交，如果低于昨日收盘则加入交易申报队列
    if (price > (std::atof(stock->close_price.c_str()) + 0.00001)) {
        printf("Success: 买入直接成交, 报价[股票=%s, 昨收=%s, price=%f]!\n", stock->stock_code.c_str(), stock->close_price.c_str(), price);
        holdings->update_holdings("买入", qty, *stock);
    }
    else {
        printf("Success: 报价低于昨日收盘价, 加入交易申报队列, 报价[股票=%s, 昨收=%s, price=%f]!\n", stock->stock_code.c_str(), stock->close_price.c_str(),
               price);
        OrderClass order = OrderClass();
        order.trade_date = UtilsClass::getDate();
        order.stock_code = stock->stock_code;
        order.stock_name = stock->stock_name;
        order.mkt = stock->mkt;
        order.order_type = "买入";
        order.price = std::to_string(price);
        order.qty = std::to_string(qty);
        order.status = "排队中";
        order.order_id = order_id++;
        order_list->push_back(order);
        printf("============================place the order into trading list============================\n");
        order.print();
    }
    memset(logTxt, 0, 2048);
    sprintf(logTxt, "%s, %s, %s, %s, %s\n", UtilsClass::getDateTimeString().c_str(), "买入股票", stock->stock_code.c_str(), stock->stock_name.c_str(),
            stock->mkt.c_str());
    // UtilsClass::writeMsg(success_logfile, logTxt);

    return 0;
}

int buyStockbyCode(char *stock_code, float price, int qty) {
    StockClass *stock = new StockClass();
    int ret = queryStockbyCode(stock_code, stock);
    if (ret != 0) {
        delete stock;
        return -404;
    }
    printf("============================found out stock============================\n");
    if (buyStock(stock, price, qty) != 0) {
        delete stock;
        return -500;
    }
    delete stock;
    return 0;
}

int sellStock(StockClass *stock, float price, int qty) {
    // 检查账户是否有开通该市场的交易许可
    if (account->mktPermission((char *)stock->mkt.c_str()) != true) {
        printf("Error: 账户Acct[name=%s]没有市场[%s]的交易许可,需先开户!\n", account->cust_name.c_str(), stock->mkt.c_str());
        sprintf(errorTxt, "Error: 账户Acct[name=%s]没有市场[%s]的交易许可,需先开户!\n", account->cust_name.c_str(), stock->mkt.c_str());
        // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
        return -501;
    }
    // 检查交易数量大于0
    if (qty <= 0) {
        printf("Error: 卖出数量必须大于零!\n");
        sprintf(errorTxt, "Error: 卖出数量必须大于零!\n");
        // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
        return -502;
    }
    // 检查账户是否持有该股票，股票余额是否充足
    if (holdings->query_holdings(*stock) < 0) {
        printf("Error: 账户Acct[name=%s]没有该股票[%s]!\n", account->cust_name.c_str(), stock->stock_code.c_str());
        sprintf(errorTxt, "Error: 账户Acct[name=%s]没有该股票[%s]!\n", account->cust_name.c_str(), stock->stock_code.c_str());
        // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
        return -601;
    }
    if (holdings->query_holdings(*stock) < qty) {
        printf("Error: 账户Acct[name=%s]股票[%s]的余额[%d]不足, 卖出的股票数量[%d]!\n", account->cust_name.c_str(), stock->stock_code.c_str(),
               holdings->query_holdings(*stock), qty);
        sprintf(errorTxt, "Error: 账户Acct[name=%s]股票[%s]的余额[%d]不足, 卖出的股票数量[%d]!\n", account->cust_name.c_str(), stock->stock_code.c_str(),
                holdings->query_holdings(*stock), qty);
        // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
        return -602;
    }
    // 检查报价是否小于昨日收盘价的10%（涨跌幅限制统一按10%计）
    if ((price > (std::atof(stock->close_price.c_str()) + 0.00001) * 1.1) || price < ((std::atof(stock->close_price.c_str()) + 0.00001) * 0.90)) {
        printf("Error: 报价超过涨跌幅限制, 报价[股票=%s, 昨收=%s, price=%f]!\n", stock->stock_code.c_str(), stock->close_price.c_str(), price);
        sprintf(errorTxt, "Error: 报价超过涨跌幅限制, 报价[股票=%s, 昨收=%s, price=%f]!\n", stock->stock_code.c_str(), stock->close_price.c_str(), price);
        // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
        return -506;
    }

    // 如果报价低于昨日收盘，则直接卖出成交，如果高于昨日收盘则加入交易申报队列
    if (price > (std::atof(stock->close_price.c_str()) + 0.00001)) {
        printf("Success: 卖出报价高于昨日收盘价, 加入交易申报队列, 报价[股票=%s, 昨收=%s, price=%f]!\n", stock->stock_code.c_str(), stock->close_price.c_str(),
               price);
        OrderClass order = OrderClass();
        order.trade_date = UtilsClass::getDate();
        order.stock_code = stock->stock_code;
        order.stock_name = stock->stock_name;
        order.mkt = stock->mkt;
        order.order_type = "卖出";
        order.price = std::to_string(price);
        order.qty = std::to_string(qty);
        order.status = "排队中";
        order.order_id = order_id++;
        order_list->push_back(order);
        printf("============================place the order into trading list============================\n");
        order.print();
    }
    else {
        printf("Success: 卖出直接成交, 报价[股票=%s, 昨收=%s, price=%f]!\n", stock->stock_code.c_str(), stock->close_price.c_str(), price);
        holdings->update_holdings("卖出", qty, *stock);
    }
    memset(logTxt, 0, 2048);
    sprintf(logTxt, "%s, %s, %s, %s, %s\n", UtilsClass::getDateTimeString().c_str(), "卖出股票", stock->stock_code.c_str(), stock->stock_name.c_str(),
            stock->mkt.c_str());
    // UtilsClass::writeMsg(success_logfile, logTxt);

    return 0;
}

int sellStockbyCode(char *stock_code, float price, int qty) {
    StockClass *stock = new StockClass();
    int ret = queryStockbyCode(stock_code, stock);
    if (ret != 0) {
        delete stock;
        return -404;
    }
    printf("============================found out stock============================\n");
    stock->print();
    if (sellStock(stock, price, qty) != 0) {
        delete stock;
        return -600;
    }

    delete stock;
    return 0;
}

int withdrawOrder(long id) {
    OrderClass item;
    std::list<OrderClass>::iterator iter_order;
    for (iter_order = order_list->begin(); iter_order != order_list->end(); iter_order++) {
        item = *iter_order;
        if (item.order_id == id && item.status != "已撤单") {
            iter_order->status = "已撤单";
            printf("Success: 撤单成功, 委托编号[id=%ld]!\n", id);
            memset(logTxt, 0, 2048);
            sprintf(logTxt, "%s, %s, %s, %s, %s\n", UtilsClass::getDateTimeString().c_str(), "撤单成功", item.stock_code.c_str(), item.stock_name.c_str(),
                    item.mkt.c_str());
            // UtilsClass::writeMsg(success_logfile, logTxt);
            return 0;
        }
        if (item.order_id == id && item.status == "已撤单") {
            printf("Error: 重复撤单, 委托编号[id=%ld]之前已经被撤单了!\n", id);
            sprintf(errorTxt, "Error: 重复撤单, 委托编号[id=%ld]之前已经被撤单了!\n", id);
            // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
            return -701;
        }
    }
    printf("Error: 撤单失败, 找不到委托单[id=%ld]!\n", id);
    sprintf(errorTxt, "Error: 撤单失败, 找不到委托单[id=%ld]!\n", id);
    // UtilsClass::writeLog(log_file_name, LOG_ERROR, errorTxt);
    return -704;
}

HV_EXPORT int handle_check_course(HttpRequest *req, HttpResponse *resp) {
    printf("##################################################\t/CheckCourse\t##################################################\n");
    resp->content_type = req->content_type;
    resp->body = req->body;
    printf("Request Body:%s\n", req->body.c_str());

    cJSON *cjson_body = cJSON_Parse(req->body.c_str());
    char *weekday = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "weekday"));
    float hours = cJSON_GetNumberValue(cJSON_GetObjectItem(cjson_body, "hours"));
    int qty = static_cast<int>(std::round(cJSON_GetNumberValue(cJSON_GetObjectItem(cjson_body, "qty"))));
    char *notes = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "notes"));

    printf("***\tCheckCourse, weekday:%s, hours:%.3f, qty:%d\n", weekday, hours, qty);
    if (weekday == NULL) {
        return 400;
    }
    if (strcmp(weekday, "MONDAY") == 0) {
        printf("MONDAY...\n");
        test_cjson((char *)req->body.c_str());
    }
    else if (strcmp(weekday, "TUESDAY") == 0) {
        printf("TUESDAY...\n");
        int finished = 0;
        test_http_async_client(&finished);
    }
    else if (strcmp(weekday, "WEDNESDAY") == 0) {
        printf("WEDNESDAY...\n");
        test_nmap();
    }
    else if (strcmp(weekday, "THURSDAY") == 0) {
        printf("THURSDAY...\n");
        char buff[256];
        sprintf(buff, "http://127.0.0.1:%d/echo", port);
        test_http_client(buff);
    }
    else if (strcmp(weekday, "FRIDAY") == 0) {
        printf("FRIDAY...\n");
        test_hloop();
    }
    else if (strcmp(weekday, "SATDAY") == 0) {
        printf("SATDAY...\n");
        // test_http_sync_get();
        int argc = 4;
        char *argv[5];
        argv[0] = strdup("./http_server_test");
        argv[1] = strdup("-v");
        argv[2] = strdup("localhost:6666");
        argv[3] = strdup("-d");
        argv[4] = strdup(req->body.c_str());
        test_curl(argc, argv);
        free(argv[0]);
        free(argv[1]);
        free(argv[2]);
        free(argv[3]);
        free(argv[4]);
    }
    else if (strcmp(weekday, "SUNDAY") == 0) {
        printf("SUNDAY...\n");
        test_http_sync_post();
    }
    else {
        printf("Others, qty=%d\n", qty);
        switch (qty) {
        case 1: test_connect(port); break;
        case 2: printf("b\n"); break;
        case 3: printf("c\n"); break;
        default: printf("others\n"); break;
        }
    }

    if (hours >= 8.30 && hours <= 18.30) {
        printf("Working hours...\n");
        test_date();
        test_hatomic_main();
    }
    else {
        printf("Off hours...\n");
        test_hmutex();
        test_md5(notes);
        test_sha1(notes);
    }

    if (cjson_body != NULL) {
        cJSON_Delete(cjson_body);
    }
    cjson_body = cJSON_CreateObject();
    cJSON_AddStringToObject(cjson_body, "return_code", "0");
    cJSON_AddStringToObject(cjson_body, "message", "成功！");
    cJSON_Delete(cjson_body);
    response_status(resp, 0, "OK");
    return 200;
}

HV_EXPORT int handle_insert_order(HttpRequest *req, HttpResponse *resp) {
    printf("##################################################\t/InsertOrder\t##################################################\n");
    resp->content_type = req->content_type;
    resp->body = req->body;
    // printf("Request Body:%s\n", req->body.c_str());

    cJSON *cjson_body = cJSON_Parse(req->body.c_str());
    char *code = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "ticker"));
    char *mkt = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "market"));
    char *side = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "side"));
    float price = cJSON_GetNumberValue(cJSON_GetObjectItem(cjson_body, "price"));
    int qty = static_cast<int>(std::round(cJSON_GetNumberValue(cJSON_GetObjectItem(cjson_body, "qty"))));

    cJSON *client_id = cJSON_GetObjectItem(cjson_body, "client_id");
    cJSON *business_type = cJSON_GetObjectItem(cjson_body, "business_type");
    printf("***\tInsertOrder, ticker:%s, market:%s, side:%s, price:%.3f, qty:%d\n", code, mkt, side, price, qty);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 加上检查传入参数
    if (code == NULL || mkt == NULL || side == NULL) {
        printf("ERROR:\tInsertOrder, required paramenter can't be null!\n");
        return 201;
    }
    if (qty > 65535 || qty < 0) {
        printf("ERROR:\tInsertOrder, qyt value is incorrect!\n");
        return 201;
    }
    if (std::isnan(price)) {
        printf("ERROR:\tInsertOrder, price value is nan!\n");
        return 201;
    }
    if (price > 65535 || qty < 0) {
        printf("ERROR:\tInsertOrder, price value is incorrect!\n");
        return 201;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int ret_code;

    if (strcmp(side, "买") == 0) {
        printf("买入\n");
        ret_code = buyStockbyCode(code, price, qty);
    }
    else {
        printf("卖出\n");
        printf("ticker:%s, market:%s, side:%s, price:%.3f, qty:%d\n", code, mkt, side, price, qty);
        ret_code = sellStockbyCode(code, price, qty);
    }
    if (cjson_body != NULL) {
        cJSON_Delete(cjson_body);
    }
    cjson_body = cJSON_CreateObject();
    cJSON_AddStringToObject(cjson_body, "return_code", std::to_string(ret_code).c_str());
    //__gcov_flush();
    if (ret_code != 0) {
        cJSON_AddStringToObject(cjson_body, "message", errorTxt);
        resp->body = cJSON_Print(cjson_body);
        return 400;
    }
    else {
        cJSON_AddStringToObject(cjson_body, "message", "成功！");
        resp->body = cJSON_Print(cjson_body);
        memset(logTxt, 0, 2048);
        sprintf(logTxt, "%s, %s, %s, %s, %s\n", UtilsClass::getDateTimeString().c_str(), "委托", side, code, mkt);
        UtilsClass::writeMsg(success_logfile, logTxt);
    }
    cJSON_Delete(cjson_body);
    response_status(resp, 0, "OK");
    return 200;
}

HV_EXPORT int handle_query_stock_bycode(HttpRequest *req, HttpResponse *resp) {
    printf("##################################################\t/QueryStockbycode\t##################################################\n");
    resp->content_type = req->content_type;
    resp->body = req->body;
    // printf("Request Body:%s\n", req->body.c_str());
    int ret_code;
    cJSON *cjson_body = cJSON_Parse(req->body.c_str());
    cJSON *code = cJSON_GetObjectItem(cjson_body, "stock_code");
    printf("***\tQueryStockbyCode, stock_code:%s\n", cJSON_GetStringValue(code));
    if (cJSON_GetStringValue(code) == NULL) {
        return 400;
    }
    StockClass *stock = new StockClass();
    ret_code = queryStockbyCode(cJSON_GetStringValue(code), stock);
    //__gcov_flush();

    if (ret_code == 0) {
        stock->print();
        if (cjson_body != NULL) {
            cJSON_Delete(cjson_body);
        }
        cjson_body = cJSON_CreateArray();
        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "trade_date", stock->trade_date.c_str());
        cJSON_AddStringToObject(item, "stock_code", stock->stock_code.c_str());
        cJSON_AddStringToObject(item, "stock_name", stock->stock_name.c_str());
        cJSON_AddStringToObject(item, "mkt", stock->mkt.c_str());
        cJSON_AddStringToObject(item, "yesterday_price", stock->yesterday_price.c_str());
        cJSON_AddStringToObject(item, "open_price", stock->open_price.c_str());
        cJSON_AddStringToObject(item, "high_price", stock->high_price.c_str());
        cJSON_AddStringToObject(item, "low_price", stock->low_price.c_str());
        cJSON_AddStringToObject(item, "close_price", stock->close_price.c_str());
        cJSON_AddStringToObject(item, "increase_rate", stock->increase_rate.c_str());
        cJSON_AddStringToObject(item, "qty", stock->qty.c_str());
        cJSON_AddStringToObject(item, "amount", stock->amount.c_str());
        cJSON_AddStringToObject(item, "per", stock->per.c_str());
        cJSON_AddItemToArray(cjson_body, item);
        resp->body = cJSON_Print(cjson_body);
        memset(logTxt, 0, 2048);
        sprintf(logTxt, "%s, %s, %s\n", UtilsClass::getDateTimeString().c_str(), "代码查股票", stock->stock_code.c_str());
        UtilsClass::writeMsg(success_logfile, logTxt);
    }
    else {
        response_status(resp, 400, "there are something wrong!");
        return 400;
    }
    delete stock;
    cJSON_Delete(cjson_body);
    response_status(resp, 0, "OK");
    return 200;
}

HV_EXPORT int handle_grade(HttpRequest *req, HttpResponse *resp) {
    printf("##################################################\t/Grade\t##################################################\n");
    printf("Request Body:%s\n", req->body.c_str());

    cJSON *cjson_body = cJSON_Parse(req->body.c_str());
    float grade = cJSON_GetNumberValue(cJSON_GetObjectItem(cjson_body, "grade"));
    char *key = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "key"));
    char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));

    if (grade >= 90) {
        printf("Grade Level A\n");
        test_hthread();
        test_ifconfig();
        test_hdir();
        test_objectpool();
        test_parse_confile();
        test_websocket_client();
    }
    else if (grade >= 80) {
        printf("Grade Level B\n");
        test_socketpair();
        test_hstring();
        test_sync();
        test_base64(data);
    }
    else if (grade >= 70) {
        printf("Grade Level C\n");
    }
    else if (grade >= 60) {
        printf("Grade Level D\n");
    }
    else {
        printf("Grade Level F\n");
    }
    cJSON_Delete(cjson_body);
    response_status(resp, 0, "OK");
    return 200;
}

static int handle_json(HttpRequest *req, HttpResponse *resp) {
    if (req->content_type != APPLICATION_JSON) {
        return response_status(resp, HTTP_STATUS_BAD_REQUEST);
    }
    hv::Json jroot = hv::Json::parse(req->body.c_str(), NULL, false);

    resp->content_type = APPLICATION_JSON;
    resp->json = req->json;
    resp->json["int"] = 123;
    resp->json["float"] = 3.14;
    resp->json["string"] = "hello";
    return 200;
}

int main(int argc, char **argv) {
    HV_MEMCHECK;

    HttpService router;

    reg_sigterm_handler(sigterm_handler);
    initial();
    port = 9090;

    if (argc >= 2) {
        port = atoi(argv[1]);
    }
    printf("Listening at port: [%d]\n", port);
    if (argc >= 3) {
        ::i_data_len = atoi(argv[2]);
    }
    router.POST("/echo", [](HttpRequest *req, HttpResponse *resp) {
        printf("##################################################\t/echo\t##################################################\n");
        response_status(resp, 0, "OK");
        return 200;
    });

    router.POST("/User", [&router](HttpRequest *req, HttpResponse *resp) {
        printf("##################################################\t/User\t##################################################\n");
        string code = req->GetString("user");
        printf("***\tUser, user:%s\n", code.c_str());

        for (auto &param : req->query_params) {
            printf("%s : %s\n", param.first.c_str(), param.second.c_str());
        }
        response_status(resp, 0, "OK");
        return 200;
    });

    router.GET("/WithdrawOrder", [&router](HttpRequest *req, HttpResponse *resp) {
        printf("##################################################\t/WithdrawOrder\t##################################################\n");
        printf("Request url:%s\n", req->url.c_str());
        long order_id = atoi(req->GetParam("order_id").c_str());
        printf("***\torder_id:%ld\n", order_id);

        withdrawOrder(order_id);
        response_status(resp, 0, "OK");
        return 200;
    });

    router.POST("/CheckCourse", handle_check_course);

    router.POST("/InsertOrder", handle_insert_order);

    router.POST("/QueryStockbyCode", handle_query_stock_bycode);

    router.PATCH("/Grade", handle_grade);

    router.PUT("/Json", handle_json);

    http_server_t server;
    server.port = port;
    // uncomment to test multi-processes
    server.worker_processes = 4;
    // uncomment to test multi-threads
    // server.worker_threads = 4;
    server.service = &router;

    // 创建子进程
    child_pid = fork();

    if (child_pid < 0) {
        // fork失败
        perror("fork failed");
        exit(1);
    }
    else if (child_pid == 0) {
        // 子进程
        printf("启动子进程...\n");
        // start_tcp_echo_server();
        test_websocket_server();
    }
    else {
        // 父进程
        printf("父进程PID: %d, 子进程PID: %d\n", getpid(), child_pid);
        http_server_run(&server);
    }

    return 0;
}
