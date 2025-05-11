#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include "../guoshubase.h"
#include "rm.h"
#include "sm.h"
#include "ql.h"
#include "parser.h"

using namespace std;

static void PrintErrorExit(RC rc) {
    PrintErrorAll(rc);
    exit(rc);
}

int main(int argc, char* argv[]) {
    RC rc;

    // 数据库名参数检查
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " dbname\n";
        return 1;
    }
    char* dbname = argv[1];

    // 初始化RedBase组件
    PF_Manager pfm;
    RM_Manager rmm(pfm);
    IX_Manager ixm(pfm);
    SM_Manager smm(ixm, rmm);
    QL_Manager qlm(smm, ixm, rmm);

    // 打开数据库
    if ((rc = smm.OpenDb(dbname)))
        PrintErrorExit(rc);

    RC parseRC;
//    if(//(rc = smm.Load("student","student"))||
//            (rc = smm.Print("STUDENT")))
//        PrintErrorExit(rc);

//    AggRelAttr ss = {
//            NO_F,"STUDENT","id"
//    };
//
//    char* rs[] = {"STUDENT"};
//    Condition cs[] = {};
//    RelAttr orderAttr;
//    RelAttr groupAttr;
//    parseRC
//    = qlm.Select(1, &ss, 1, rs,
//                      0, cs,
//                      0, orderAttr,
//                 false, groupAttr);

    rc = GBparse(pfm, smm, qlm);
    // 关闭数据库
    if ((rc = smm.CloseDb()))
        PrintErrorExit(rc);

    if(parseRC != 0)
        PrintErrorExit(parseRC);

    cout << "Bye.\n";
    return parseRC;
}
