#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include <list>
#include "stock.h"

// 客户号	银行账号	客户姓名	身份证号	开户网点	电话	地址	上海A股	上海B股	深圳A股	深圳B股	创业板	科创板	中小板	资金余额
//  SHA = 1,
//  SHB,
//  SZA,
//  SZB,
//  GEM,  // 创业版
//  STAR, // 科创版
//  SME   // 中小板

class AccountClass
{
public:
    std::string id;
    std::string bank_acct;
    std::string cust_name;
    std::string cust_id;
    std::string branch;
    std::string tel;
    std::string address;
    bool mkt_SHA;
    bool mkt_SHB;
    bool mkt_SZA;
    bool mkt_SZB;
    bool mkt_GEM;
    bool mkt_STAR;
    bool mkt_SME;
    double balance;

    AccountClass();
    ~AccountClass();
    void print();
    void copy(AccountClass acct);
    bool mktPermission(char *mkt_string);
};