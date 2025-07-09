#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>

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