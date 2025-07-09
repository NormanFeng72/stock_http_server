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
#include "cwe_samples.h"
#include <sanitizer/asan_interface.h>

/******************************************************************************\
 *
 *      Register sigacation
 *
\******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
void __gcov_flush();
#ifdef __cplusplus
}
#endif

static void sigterm_handler(int signum) {
    printf("received signal\n");
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

int buyStockbyComboConditions(char *stock_code, int mktID, float price, int qty) {
    StockClass *stock = new StockClass();
    int ret = queryStock(stock_code, mktID, stock);
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

int sellStockbyComboConditions(char *stock_code, int mktID, float price, int qty) {
    StockClass *stock = new StockClass();
    int ret = queryStock(stock_code, mktID, stock);
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

int main(int argc, char **argv) {
    HV_MEMCHECK;

    HttpService router;

    reg_sigterm_handler(sigterm_handler);

    initial();
    int port = 8080;

    if (argc >= 2) {
        port = atoi(argv[1]);
    }
    printf("Listening at port: [%d]\n", port);
    if (argc >= 3) {
        ::i_data_len = atoi(argv[2]);
    }
    printf("Call CWE code while data len exceeds: [%d]\n", ::i_data_len);

    router.GET("/ping", [](HttpRequest *req, HttpResponse *resp) { return resp->String("pong"); });

    router.GET("/data", [](HttpRequest *req, HttpResponse *resp) {
        static char data[] = "0123456789";
        return resp->Data(data, 10);
    });

    router.GET("/paths", [&router](HttpRequest *req, HttpResponse *resp) { return resp->Json(router.Paths()); });

    router.POST("/echo", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        resp->body = req->body;
        return 200;
    });

    router.POST("/sync", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        resp->body = req->body;
        printf("Request body: %s\n", req->body);

        return 200;
    });

    router.GET("/User", [&router](HttpRequest *req, HttpResponse *resp) {
        string code = req->GetString("userId");
        printf("***\tUser, userId:%s\n", code.c_str());

        for (auto &param : req->query_params) {
            printf("%s : %s\n", param.first.c_str(), param.second.c_str());
        }
        return 200;
    });

    router.GET("/PrintParameters", [&router](HttpRequest *req, HttpResponse *resp) {
        printf("***\tPrintParameters:%s\n");
        for (auto &param : req->query_params) {
            printf("%s : %s\n", param.first.c_str(), param.second.c_str());
        }
        return 200;
    });

    router.POST("/CheckCourse", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        resp->body = req->body;
        printf("Request Body:%s\n", req->body.c_str());

        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *weekday = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "weekday"));
        float hours = cJSON_GetNumberValue(cJSON_GetObjectItem(cjson_body, "hours"));
        int qty = static_cast<int>(std::round(cJSON_GetNumberValue(cJSON_GetObjectItem(cjson_body, "qty"))));

        printf("***\tCheckCourse, weekday:%s, hours:%.3f, qty:%d\n", weekday, hours, qty);
        if (strcmp(weekday, "MONDAY") == 0) {
            printf("MONDAY...\n");
        }
        else if (strcmp(weekday, "TUESDAY") == 0) {
            printf("TUESDAY...\n");
        }
        else if (strcmp(weekday, "WEDNESDAY") == 0) {
            printf("WEDNESDAY...\n");
        }
        else if (strcmp(weekday, "THURSDAY") == 0) {
            printf("THURSDAY...\n");
        }
        else if (strcmp(weekday, "FRIDAY") == 0) {
            printf("FRIDAY...\n");
        }
        else if (strcmp(weekday, "SATDAY") == 0) {
            printf("SATDAY...\n");
        }
        else if (strcmp(weekday, "SUNDAY") == 0) {
            printf("SUNDAY...\n");
        }
        else {
            printf("Others...\n");
        }

        if (hours >= 8.30 && hours <= 18.30) {
            printf("Working hours...\n");
        }
        else {
            printf("Off hours...\n");
        }

        switch (qty) {
        case 1: printf("a\n");
        case 2: printf("b\n");
        case 3: printf("c\n");
        default: printf("others\n");
        }

        if (cjson_body != NULL) {
            cJSON_Delete(cjson_body);
        }
        cjson_body = cJSON_CreateObject();
        cJSON_AddStringToObject(cjson_body, "return_code", "0");
        cJSON_AddStringToObject(cjson_body, "message", "成功！");
        resp->body = cJSON_Print(cjson_body);
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/InsertOrder", [](HttpRequest *req, HttpResponse *resp) {
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
        return 200;
    });

    router.POST("/QueryStockbyCode", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        resp->body = req->body;
        // printf("Request Body:%s\n", req->body.c_str());
        int ret_code;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        cJSON *code = cJSON_GetObjectItem(cjson_body, "stock_code");
        printf("***\tQueryStockbyCode, stock_code:%s\n", cJSON_GetStringValue(code));
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
            return 400;
        }
        delete stock;
        cJSON_Delete(cjson_body);
        return 200;
    });

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    router.POST("/CWE_561", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_561\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_561(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_788", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_788\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_788(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_476", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_476\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_476(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_129", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_129\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_129(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_252", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_252\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_252(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_401", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_401\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_401(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_805", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_805\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_805(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_131", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_131\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_131(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_667", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_667\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_667(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_366", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_366\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_366(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_820", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_820\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_820(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_833", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_833\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_833(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_835", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_835\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_835(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_606", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_606\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_606(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_562", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_562\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_562(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_786", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_786\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_786(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_194", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_194\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_194(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_789", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_789\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_789(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_783", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_783\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_783(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_805", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_805\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_805(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_390", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_390\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_390(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_404", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_404\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_404(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_477", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_477\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_477(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_478", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_478\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_478(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_480", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_480\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) % 2 == 0) {
            if (strlen(data) > ::i_data_len) test_CWE_480(data, strlen(data));
        }
        else {
            if (strlen(data) > ::i_data_len) test_CWE_480_case2(data, strlen(data));
        }
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_484", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_484\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_484(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_123", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_123\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_123(data);
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_570", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_570\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_570(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_571", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_571\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_571(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_665", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_665\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_665(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_456", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_456\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_456(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_457", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_457\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_457(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_197", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_197\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_197(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_732", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_732\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_732(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_170", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_170\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_170(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_798", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_798\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_798(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_908", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_908\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_908(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_1041", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_1041\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_1041(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_571", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_571\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_571(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_787", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_787\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_787();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_125", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_125\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_125();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_191", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_191\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_191();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_416", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_416\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_416();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_190", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_190\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_190();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_120", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_120\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_120();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_369", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_369\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_369();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_468", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_468\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_468();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_681", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_681\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_681();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_415", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_415\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_415();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_590", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_590\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_590();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_193", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_193\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_193();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_464", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_464\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_464();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_88", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_88\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_88(data, strlen(data));
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_482", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_482\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_482();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/StackBufferUnderflow", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>StackBufferUnderflow\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) stackBufferUnderflow();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/MemLeak", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>MemLeak\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) memLeak();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/ArrayIndexUnderflow", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>ArrayIndexUnderflow\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) arrayIndexUnderflow();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CWE_467", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CWE_467\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) test_CWE_467();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/ModifyStringLiteral", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>ModifyStringLiteral\t[len=%d],\t[data : %s]\n", strlen(data), data);
        // int a=0;
        // int b = 10/a;
        if (strlen(data) > ::i_data_len) modifyStringLiteral();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/GlobalBufferOverflow", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>GlobalBufferOverflow\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) globalBufferOverflow();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/UseAfterScope", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>UseAfterScope\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) useAfterScope();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/Pointer2ObjectWithEndedLifetime", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>Pointer2ObjectWithEndedLifetime\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) pointer2ObjectWithEndedLifetime();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/UseAfterReturn", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>UseAfterReturn\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) useAfterReturn();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/ConversionBetweenTwoPointerTtypes", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>ConversionBetweenTwoPointerTtypes\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) conversionBetweenTwoPointerTtypes();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/FunctionCallThroughtPointer", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>FunctionCallThroughtPointer\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) functionCallThroughtPointer();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/PointerSubtraction", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>PointerSubtraction\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) pointerSubtraction();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/PointerAddition", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>PointerAddition\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) pointerAddition();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/ArrayOutofBoundRead", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>ArrayOutofBoundRead\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) arrayOutofBoundRead();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/SubstractingTwoPointers", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>SubstractingTwoPointers\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) substractingTwoPointers();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/CopyingOverlappingMemory", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>CopyingOverlappingMemory\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) copyingOverlappingMemory();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/PointUsedAfterFree", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>PointUsedAfterFree\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) pointUsedAfterFree();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/LibFunctionWithInvalidValue", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>LibFunctionWithInvalidValue\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) libFunctionWithInvalidValue();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/BitShiftingWithNegativeNumber", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>BitShiftingWithNegativeNumber\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) bitShiftingWithNegativeNumber();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/SizeExpression", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>SizeExpression\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) sizeExpression();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/DataRace", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>DataRace\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) dataRace();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/DereferNullPointer", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>DereferNullPointer\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) dereferNullPointer();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    router.POST("/ModifyObjTwice", [](HttpRequest *req, HttpResponse *resp) {
        resp->content_type = req->content_type;
        cJSON *cjson_body = cJSON_Parse(req->body.c_str());
        char *data = cJSON_GetStringValue(cJSON_GetObjectItem(cjson_body, "data"));
        printf("=>ModifyObjTwice\t[len=%d],\t[data : %s]\n", strlen(data), data);
        if (strlen(data) > ::i_data_len) modifyObjTwice();
        resp->json["data"] = data;
        cJSON_Delete(cjson_body);
        return 200;
    });

    http_server_t server;
    server.port = port;
    // uncomment to test multi-processes
    // server.worker_processes = 4;
    // uncomment to test multi-threads
    // server.worker_threads = 4;
    server.service = &router;

#if 1
    http_server_run(&server);
#else
    // test http_server_stop
    http_server_run(&server, 0);
    sleep(30);
    http_server_stop(&server);
#endif

    return 0;
}
