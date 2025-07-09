
#include "accountlist.h"
//extern "C" void __gcov_flush();

AcctListClass::AcctListClass()
{
    acct_list = new std::list<AccountClass>();
}

AcctListClass::~AcctListClass()
{
    acct_list->clear();
    delete acct_list;
}

void AcctListClass::read()
{
    std::string details;
    std::fstream fs;
    const char *delim = ",";
    std::list<string> elements;
    std::list<string>::iterator iter_string;

    fs.open("data/acct.csv", std::ios::in);
    while (fs)
    {
        AccountClass acct = AccountClass();
        std::getline(fs, details); // read each line into a string
        elements = UtilsClass::split((char *)details.c_str(), delim);

        // printf("elements size = %d\n", elements.size());

        if (elements.size() < 5)
            break;
        iter_string = elements.begin();
        acct.id = *iter_string;
        iter_string++;
        acct.bank_acct = *iter_string;
        iter_string++;
        acct.cust_name = *iter_string;
        iter_string++;
        acct.cust_id = *iter_string;
        iter_string++;
        acct.branch = *iter_string;
        iter_string++;
        acct.tel = *iter_string;
        iter_string++;
        acct.address = *iter_string;
        iter_string++;

        if (*iter_string == "yes")
        {
            acct.mkt_SHA = true;
        }
        else
        {
            acct.mkt_SHA = false;
        }
        iter_string++;

        if (*iter_string == "yes")
        {
            acct.mkt_SHB = true;
        }
        else
        {
            acct.mkt_SHB = false;
        }
        iter_string++;

        if (*iter_string == "yes")
        {
            acct.mkt_SZA = true;
        }
        else
        {
            acct.mkt_SZA = false;
        }
        iter_string++;

        if (*iter_string == "yes")
        {
            acct.mkt_SZB = true;
        }
        else
        {
            acct.mkt_SZB = false;
        }
        iter_string++;

        if (*iter_string == "yes")
        {
            acct.mkt_GEM = true;
        }
        else
        {
            acct.mkt_GEM = false;
        }
        iter_string++;

        if (*iter_string == "yes")
        {
            acct.mkt_STAR = true;
        }
        else
        {
            acct.mkt_STAR = false;
        }
        iter_string++;
        // printf("nameiter_string=[%s]\n", iter_string->c_str());
        if (*iter_string == "yes")
        {
            acct.mkt_SME = true;
        }
        else
        {
            acct.mkt_SME = false;
        }
        iter_string++;
        // printf("nameiter_string=[%s]\n", iter_string->c_str());
        acct.balance = (double)std::atof(iter_string->c_str());

        // printf("===============================================================================\n");
        // printf("name=%s, id = %s, 资金=%s\n", acct.cust_name.c_str(), acct.cust_id.c_str(), iter_string->c_str());
        cout.precision(4);
        // std::cout << acct.id << acct.bank_acct << acct.cust_name << acct.cust_id << acct.branch << acct.tel << acct.address << boolalpha << acct.mkt_SHA << acct.mkt_SHB << acct.mkt_SZA << acct.mkt_SZB << acct.mkt_GEM << acct.mkt_STAR << acct.mkt_SME << std::fixed << acct.balance << '\n';
        acct_list->push_back(acct);
        elements.clear();
    }

    // std::cout << "read complete" << "\n";
    fs.close();
    //__gcov_flush();
}

void AcctListClass::print()
{
    if (this->acct_list->empty())
        return;
    std::list<AccountClass>::iterator iter;
    for (iter = this->acct_list->begin(); iter != this->acct_list->end(); iter++)
    {
        iter->print();
    }
}

int AcctListClass::getAcct(char *id, AccountClass *acct)
{
    AccountClass item;
    std::list<AccountClass>::iterator iter_acct;
    for (iter_acct = this->acct_list->begin(); iter_acct != this->acct_list->end(); iter_acct++)
    {
        item = *iter_acct;
        // std::cout << stock.trade_date << "\n";
        if (item.id == id)
        {
            acct->copy(item);
            //__gcov_flush();
            return 0;
        }
    }
    return -1;
}