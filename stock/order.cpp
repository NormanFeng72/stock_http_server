#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include "order.h"

OrderClass::OrderClass()
{
}
OrderClass::~OrderClass()
{
}

void OrderClass::print()
{
    std::cout << this->trade_date << "\t" << this->stock_code << "\t" << this->stock_name << "\t" << this->mkt << "\t" << this->order_type << "\t" << this->price << "\t" << this->qty << "\t" << this->status << "\t" << this->order_id << '\n';
}
