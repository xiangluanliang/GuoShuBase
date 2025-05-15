//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/5/15 23:01
// @Project: guoshuqt
//
//

#ifndef GUOSHUBASE_INTERFACE_H
#define GUOSHUBASE_INTERFACE_H

#pragma once
#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include "guoshubase.h"
#include "rm.h"
#include "sm.h"
#include "ql.h"
#include "parser.h"

static PF_Manager pfm;
static RM_Manager rmm(pfm);
static IX_Manager ixm(pfm);
static SM_Manager smm(ixm, rmm);
static QL_Manager qlm(smm, ixm, rmm);
static void PrintErrorExit(RC rc) {
    PrintErrorAll(rc);
#ifndef _GUI
    exit(rc);
#endif
}
int OpenDb(const char *dbname);
int CreateDb(const char *dbname);
int CloseDb();

int inputSQL(std::ostream& os, const char *sql);
#endif //GUOSHUBASE_INTERFACE_H
