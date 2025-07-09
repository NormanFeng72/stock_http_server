

#include <fstream>
#include <iostream>
#include <string.h>
#include <list>
#include <stdio.h>
#include <vector>
#include "utils.h"
#include "stock.h"

using namespace std;

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