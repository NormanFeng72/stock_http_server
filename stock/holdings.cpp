

#include "holdings.h"
//extern "C" void __gcov_flush();

HoldingsClass::HoldingsClass()
{
    stk_list = new std::list<StockClass>();
}

HoldingsClass::~HoldingsClass()
{
    stk_list->clear();
    delete stk_list;
}

void HoldingsClass::read()
{
    std::string details;
    std::fstream fs;
    const char *delim = ",";
    std::list<string> elements;
    std::list<string>::iterator iter_string;

    fs.open("data/holdings.csv", std::ios::in);
    while (fs)
    {
        StockClass stock = StockClass();
        std::getline(fs, details); // read each line into a string
        elements = UtilsClass::split((char *)details.c_str(), delim);
        if (elements.size() < 5)
            break;
        iter_string = elements.begin();
        stock.trade_date = *iter_string;
        iter_string++;
        stock.stock_code = *iter_string;
        iter_string++;
        stock.stock_name = *iter_string;
        iter_string++;
        stock.mkt = *iter_string;
        iter_string++;
        stock.yesterday_price = *iter_string;
        iter_string++;
        stock.qty = *iter_string;
        iter_string++;
        stock.amount = *iter_string;
        // 其余都显示空白
        stock.open_price = " ";
        stock.high_price = " ";
        stock.low_price = " ";
        stock.close_price = " ";
        stock.increase_rate = " ";
        stock.per = " ";

        // std::cout << stock.trade_date << stock.stock_code << stock.stock_name << stock.yesterday_price << stock.open_price << stock.high_price << stock.low_price << stock.close_price << stock.increase_rate << stock.qty << stock.amount << stock.per << '\n';
        stk_list->push_back(stock);
        elements.clear();
    }

    // std::cout << "read complete" << "\n";
    fs.close();
    //__gcov_flush();
}

void HoldingsClass::print()
{
    std::list<StockClass>::iterator iter;

    if (stk_list->empty())
        return;
    for (iter = stk_list->begin(); iter != stk_list->end(); iter++)
    {
        iter->print();
    }
}

int HoldingsClass::query_holdings(StockClass stock)
{
    StockClass item;
    std::list<StockClass>::iterator iter_stock;
    for (iter_stock = this->stk_list->begin(); iter_stock != this->stk_list->end(); iter_stock++)
    {
        item = *iter_stock;
        // std::cout << stock.trade_date << "\n";
        if (item.stock_code == stock.stock_code)
        {
            //__gcov_flush();
            return std::atoi(item.qty.c_str());
        }
    }
    //__gcov_flush();
    return -404;
}

void HoldingsClass::update_holdings(std::string order_type, int qty, StockClass stock)
{
    StockClass item;
    std::list<StockClass>::iterator iter_stock;
    // 查询是否有此股票
    for (iter_stock = this->stk_list->begin(); iter_stock != this->stk_list->end(); iter_stock++)
    {
        item = *iter_stock;
        if (item.stock_code == stock.stock_code) // 查到该股票有持仓
        {
            if (order_type == "买入")
            {
                iter_stock->qty = std::to_string(std::atoi(item.qty.c_str()) + qty);
            }
            else
            {

                if ((std::atoi(item.qty.c_str()) - qty) < 1)
                {
                    stk_list->erase(iter_stock--);
                }
                else
                {
                    iter_stock->qty = std::to_string(std::atoi(item.qty.c_str()) - qty);
                }
            }
            //__gcov_flush();
            return;
        }
    }
    // 没有查到该股票，就是没有该股票持仓，增加该股票
    item.trade_date = UtilsClass::getDate();
    item.stock_code = stock.stock_code;
    item.stock_name = stock.stock_name;
    item.mkt = stock.mkt;
    // item.yesterday_price;  持仓价格不修改
    item.qty = std::to_string(qty);
    item.amount = std::to_string(qty * std::atof(stock.yesterday_price.c_str()));
    stk_list->push_back(item);
    //__gcov_flush();
}