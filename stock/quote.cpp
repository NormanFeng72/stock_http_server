

#include "quote.h"
//extern "C" void __gcov_flush();

QuoteClass::QuoteClass()
{
    quote = new std::list<StockClass>();
}

QuoteClass::~QuoteClass()
{
    quote->clear();
    delete quote;
}

void QuoteClass::read()
{
    std::string details;
    std::fstream fs;
    const char *delim = ",";
    std::list<string> elements;
    std::list<string>::iterator iter_string;

    fs.open("data/quote.csv", std::ios::in);
    while (fs)
    {
        StockClass stock = StockClass();
        std::getline(fs, details); // read each line into a string
        elements = UtilsClass::split((char *)details.c_str(), delim);
        if (elements.size() < 10)
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
        stock.open_price = *iter_string;
        iter_string++;
        stock.high_price = *iter_string;
        iter_string++;
        stock.low_price = *iter_string;
        iter_string++;
        stock.close_price = *iter_string;
        iter_string++;
        stock.increase_rate = *iter_string;
        iter_string++;
        stock.qty = *iter_string;
        iter_string++;
        stock.amount = *iter_string;
        iter_string++;
        stock.per = (*iter_string == "") ? " " : *iter_string;
        // std::cout << stock.trade_date << stock.stock_code << stock.stock_name << stock.yesterday_price << stock.open_price << stock.high_price << stock.low_price << stock.close_price << stock.increase_rate << stock.qty << stock.amount << stock.per << '\n';
        quote->push_back(stock);
        elements.clear();
    }

    // std::cout << "read complete" << "\n";
    fs.close();
    //__gcov_flush();
}

void QuoteClass::print()
{
    if (quote->empty())
        return;
    std::list<StockClass>::iterator iter;
    for (iter = quote->begin(); iter != quote->end(); iter++)
    {

        iter->print();
    }
}

int QuoteClass::queryStockbyCode(char *stockcode, StockClass *stock)
{
    StockClass item;
    std::list<StockClass>::iterator iter_stock;
    for (iter_stock = this->quote->begin(); iter_stock != this->quote->end(); iter_stock++)
    {
        item = *iter_stock;
        // std::cout << stock.trade_date << "\n";
        if (item.stock_code == stockcode)
        {
            stock->copy(item);
            //__gcov_flush();
            return 0;
        }
    }
    //__gcov_flush();
    return -1;
}

int QuoteClass::queryStock(char *stockcode, int mktID, StockClass *stock)
{
    StockClass item;
    std::list<StockClass>::iterator iter_stock;
    for (iter_stock = this->quote->begin(); iter_stock != this->quote->end(); iter_stock++)
    {
        item = *iter_stock;
        // std::cout << stock.trade_date << "\n";
        if (item.stock_code == stockcode && item.mkt == MktName[mktID])
        {
            stock->copy(item);
            //__gcov_flush();
            return 0;
        }
    }
    //__gcov_flush();
    return -1;
}