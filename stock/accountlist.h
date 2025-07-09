#include <fstream>
#include <iostream>
#include <string>
#include <list>
#include "utils.h"
#include "account.h"

using namespace std;

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