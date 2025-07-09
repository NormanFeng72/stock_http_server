#include <fstream>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include "account.h"

AccountClass::AccountClass()
{
}
AccountClass::~AccountClass()
{
}

void AccountClass::print()
{
    std::cout << this->id << "\t" << this->bank_acct << "\t" << this->cust_name << "\t" << this->cust_id << "\t" << this->branch << "\t" << this->tel << "\t" << this->address << std::boolalpha << "\t" << this->mkt_SHA << "\t" << this->mkt_SHB << "\t" << this->mkt_SZA << "\t" << this->mkt_SZB << "\t" << this->mkt_GEM << "\t" << this->mkt_STAR << "\t" << this->mkt_SME << "\t" << std::fixed << this->balance << '\n';
}

void AccountClass::copy(AccountClass acct)
{
    id = acct.id;
    bank_acct = acct.bank_acct;
    cust_name = acct.cust_name;
    cust_id = acct.cust_id;
    branch = acct.branch;
    tel = acct.tel;
    address = acct.address;
    mkt_SHA = acct.mkt_SHA;
    mkt_SHB = acct.mkt_SHB;
    mkt_SZA = acct.mkt_SZA;
    mkt_SZB = acct.mkt_SZB;
    mkt_GEM = acct.mkt_GEM;
    mkt_STAR = acct.mkt_STAR;
    mkt_SME = acct.mkt_SME;
    balance = acct.balance;
}

bool AccountClass::mktPermission(char *mkt_string)
{
    // printf("mkt_string=%s, mkt_SHA=%d\n", mkt_string, this->mkt_SHA);

    if (strcmp(mkt_string, "上海A股") == 0 && this->mkt_SHA == true)
        return true;
    if (strcmp(mkt_string, "上海B股") == 0 && this->mkt_SHB == true)
        return true;
    if (strcmp(mkt_string, "深圳A股") == 0 && this->mkt_SZA == true)
        return true;
    if (strcmp(mkt_string, "深圳B股") == 0 && this->mkt_SZB == true)
        return true;
    if (strcmp(mkt_string, "创业板") == 0 && this->mkt_GEM == true)
        return true;
    if (strcmp(mkt_string, "科创板") == 0 && this->mkt_STAR == true)
        return true;
    if (strcmp(mkt_string, "中小板") == 0 && this->mkt_SME == true)
        return true;
    return false;
}

// - 跨平台（Linux, Windows, Mac, Solaris）
// - 高性能事件循环（网络IO事件、定时器事件、空闲事件）
// - TCP/UDP服务端/客户端/代理
// - SSL/TLS加密通信（WITH_OPENSSL or WITH_MBEDTLS）
// - HTTP服务端/客户端（https http1/x http2 grpc）
// - HTTP文件服务、目录服务、API服务（支持RESTful）
// - WebSocket服务端/客户端