#include <fstream>
#include <iostream>
#include <string>
#include <list>
using namespace std;

typedef enum enumMktID
{
    SHA = 1,
    SHB,
    SZA,
    SZB,
    GEM,  // 创业版
    STAR, // 科创版
    SME   // 中小板
} MktID;

class StockClass
{
public:
    std::string trade_date;
    std::string stock_code;
    std::string stock_name;
    std::string mkt;
    std::string yesterday_price;
    std::string open_price;
    std::string high_price;
    std::string low_price;
    std::string close_price;
    std::string increase_rate;
    std::string qty;
    std::string amount;
    std::string per;
    StockClass();
    ~StockClass();
    void copy(StockClass stock);
    void print();
};

class QuoteClass
{
public:
    std::list<StockClass> *quote;

public:
    QuoteClass();
    ~QuoteClass();
    void read();
    void print();
    int queryStockbyCode(char *stockcode, StockClass *stock);
    int queryStock(char *stockcode, int mktID, StockClass *stock);
};

class AccountClass
{
public:
    std::string id;
    std::string bank_acct;
    std::string cust_name;
    std::string cust_id;
    std::string branch;
    std::string tel;
    std::string address;
    bool mkt_SHA;
    bool mkt_SHB;
    bool mkt_SZA;
    bool mkt_SZB;
    bool mkt_GEM;
    bool mkt_STAR;
    bool mkt_SME;
    double balance;
    std::list<StockClass> *stock_holdings;

    AccountClass();
    ~AccountClass();
    void print();
    void copy(AccountClass acct);
    bool mktPermission(char *mkt_string);
};

class AcctListClass
{
public:
    std::list<AccountClass> *acct_list;

    AcctListClass();
    ~AcctListClass();

    void read();
    void print();
    int getAcct(char *id, AccountClass *acct);
};

class HoldingsClass
{

public:
    std::list<StockClass> *stk_list;

public:
    HoldingsClass();
    ~HoldingsClass();
    void read();
    void print();
    int query_holdings(StockClass stock);
    void update_holdings(std::string order_type, int qty, StockClass stock);
};

class OrderClass
{
public:
    std::string trade_date;
    std::string stock_code;
    std::string stock_name;
    std::string mkt;
    std::string order_type;
    std::string price;
    std::string qty;
    std::string status;
    long order_id;
    OrderClass();
    ~OrderClass();
    void print();
};