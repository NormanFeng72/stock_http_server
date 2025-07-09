#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>

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

const char MktName[][64] = {"", "上海A股", "上海B股", "深圳A股", "深圳B股", "创业板", "科创板", "中小板"};

class StockClass
{
public:
    std::string trade_date;
    std::string stock_code;
    std::string stock_name;
    std::string mkt;
    std::string yesterday_price; // 作为持仓时，字段用作买入价格
    std::string open_price;
    std::string high_price;
    std::string low_price;
    std::string close_price;
    std::string increase_rate;
    std::string qty;    // 作为持仓时，字段用作持股数量
    std::string amount; // 作为持仓时，字段用作持股市值
    std::string per;
    StockClass();
    ~StockClass();
    void copy(StockClass stock);
    void print();
};