

#include <fstream>
#include <iostream>
#include <string.h>
#include <list>
#include <stdio.h>
#include <vector>
#include "utils.h"
#include "stock.h"

using namespace std;

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