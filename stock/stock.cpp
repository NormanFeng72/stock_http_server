#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include "stock.h"

StockClass::StockClass()
{
}
StockClass::~StockClass()
{
}

void StockClass::copy(StockClass stock)
{
    trade_date = stock.trade_date;
    stock_code = stock.stock_code;
    stock_name = stock.stock_name;
    mkt = stock.mkt;
    yesterday_price = stock.yesterday_price;
    open_price = stock.open_price;
    high_price = stock.high_price;
    low_price = stock.low_price;
    close_price = stock.close_price;
    increase_rate = stock.increase_rate;
    qty = stock.qty;
    amount = stock.amount;
    per = stock.per;
}

void StockClass::print()
{
    if (this->stock_code.length() != 6)
        return;
    std::cout << this->trade_date << "\t" << this->stock_code << "\t" << this->stock_name << "\t" << this->mkt << "\t" << this->yesterday_price << "\t" << this->open_price << "\t" << this->high_price << "\t" << this->low_price << "\t" << this->close_price << "\t" << this->increase_rate << "\t" << this->qty << "\t" << this->amount << "\t" << this->per << '\n';
}
