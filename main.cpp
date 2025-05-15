#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include "../guoshubase.h"
#include "rm.h"
#include "sm.h"
#include "ql.h"
#include "parser.h"
#include <random>   // For mt19937, uniform_int_distribution
#include <algorithm> // For generate_n
#include <iterator>  // For ostream_iterator
using namespace std;


// 函数：生成指定长度的随机字符串（大小写字母）
string generateRandomString(mt19937& rng, int maxLength) {
    const string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    uniform_int_distribution<int> length_dist(1, maxLength); // 字符串长度在 1 到 maxLength 之间
    uniform_int_distribution<int> char_dist(0, characters.length() - 1);

    int length = length_dist(rng);
    string random_string;
    random_string.reserve(length);

    for (int i = 0; i < length; ++i) {
        random_string += characters[char_dist(rng)];
    }
    return random_string;
}

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

    RC parseRC;
    random_device rd;
    mt19937 rng(rd()); // Mersenne Twister 随机数引擎
    // 分布器：用于生成 1 到 100 之间的随机整数
    uniform_int_distribution<int> int_dist(1, 100);

    // 打开数据库
//    for (int j = 31; j <= 500 ; ++j) {
//        if ((rc = smm.OpenDb(dbname)))
//            PrintErrorExit(rc);
//        for (int i = (j-1)*2000 + 1; i <= j*2000; ++i) {
////        for (int i = 57041+1; i <= 60000; ++i) {
//            vector<Value> values;
//            Value v;
//            v.type = INT;
//            v.data = new int(i);
//            values.push_back(v);
//
//            string random_str = generateRandomString(rng, 8);
//            v.type = STRING;
//            v.data = strdup(random_str.c_str());
//            values.push_back(v);
//
//            // 3. 生成并输出 1~100 的随机整数
//            int random_int = int_dist(rng);
//            v.type = INT;
//            v.data = new int(random_int);
//            values.push_back(v);
//
//            parseRC = qlm.Insert("STUDENT",values.size(),values.data());
//
//            for (Value &val : values) {
//                if (val.type == INT) {
//                    delete static_cast<int*>(val.data);
//                } else if (val.type == STRING) {
//                    free(val.data);  // strdup 用 free 释放
//                }
//            }
//        }
////     关闭数据库
//
//        if ((rc = pfm.ClearBuffer())||
////            (rc = pfm.PrintBuffer())||
//        (rc = smm.CloseDb()))
//            PrintErrorExit(rc);
//    }



    if ((rc = smm.OpenDb(dbname)))
        PrintErrorExit(rc);
    parseRC = GBparse(pfm, smm, qlm);
    if ((rc = smm.CloseDb())) {
        PrintErrorExit(rc);
    }

    if(parseRC == 42){
        cout << "Bye.\n";
        return parseRC;
    }
    if(parseRC != 0) {
        PrintErrorExit(parseRC);
    }
    return parseRC;
}
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
