//
// dbcreate.cc
//
// Author: Jason McHugh (mchughj@cs.stanford.edu)
//
// This shell is provided for the student.

#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include <unistd.h>
#include "rm.h"
#include "sm.h"
#include "ix.h"
#include "../guoshubase.h"
#include "catalog.h"

using namespace std;

static void PrintErrorExit(RC rc) {
  PrintErrorAll(rc);
  exit(rc);
}

//
// main
//
int main(int argc, char *argv[])
{
  RC rc;

  // Look for 2 arguments. The first is always the name of the program
  // that was executed, and the second should be the name of the
  // database.
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " <dbname> \n";
    exit(1);
  }

    // The database name is the second argument
  string dbname(argv[1]);

  // Create a subdirectory for the database
  stringstream command;
  command << "mkdir " << dbname;
  rc = system (command.str().c_str());
  if(rc != 0) {
    cerr << argv[0] << " mkdir error for " << dbname << "\n";
    exit(rc);
  }

  if (chdir(dbname.c_str()) < 0) {
        cerr << argv[0] << " chdir error to " << dbname << "\n";
        exit(1);
  }

  // Create the system catalogs...
  PF_Manager pfm;
  RM_Manager rmm(pfm);
  RM_FileHandle relfh, attrfh;

  if(
    (rc = rmm.CreateFile("relcat", DataRelInfo::size()))
    || (rc =  rmm.OpenFile("relcat", relfh))
    )
    PrintErrorExit(rc);

  if(
    (rc = rmm.CreateFile("attrcat", DataAttrInfo::size()))
    || (rc =  rmm.OpenFile("attrcat", attrfh))
    )
    PrintErrorExit(rc);

  DataRelInfo relcat_rel;
  strcpy(relcat_rel.relName, "relcat");
  relcat_rel.attrCount = DataRelInfo::members();
  relcat_rel.recordSize = DataRelInfo::size();
  relcat_rel.numPages = 1; // initially
  relcat_rel.numRecords = 2; // relcat & attrcat


  DataRelInfo attrcat_rel;
  strcpy(attrcat_rel.relName, "attrcat");
  attrcat_rel.attrCount = DataAttrInfo::members();
  attrcat_rel.recordSize = DataAttrInfo::size();
  attrcat_rel.numPages = 1; // initially
  attrcat_rel.numRecords = DataAttrInfo::members() + DataRelInfo::members();

  RID rid;
  if ((rc = relfh.InsertRec((char*) &relcat_rel, rid)) < 0
      ||   (rc = relfh.InsertRec((char*) &attrcat_rel, rid)) < 0
    )
    PrintErrorExit(rc);

  // relcat attrs
  DataAttrInfo a;
  strcpy(a.relName, "relcat");
  strcpy(a.attrName, "relName");
  a.offset = offsetof(DataRelInfo, relName);
  a.attrType = STRING;
  a.attrLength = MAXNAME+1;
  a.indexNo = -1;
  if ((rc = attrfh.InsertRec((char*) &a, rid)) < 0)
    PrintErrorExit(rc);

  strcpy(a.relName, "relcat");
  strcpy(a.attrName, "recordSize");
  a.offset = offsetof(DataRelInfo, recordSize);
  a.attrType = INT;
  a.attrLength = sizeof(int);
  a.indexNo = -1;
  if ((rc = attrfh.InsertRec((char*) &a, rid)) < 0)
    PrintErrorExit(rc);

  strcpy(a.relName, "relcat");
  strcpy(a.attrName, "attrCount");
  a.offset = offsetof(DataRelInfo, attrCount);
  a.attrType = INT;
  a.attrLength = sizeof(int);
  a.indexNo = -1;
  if ((rc = attrfh.InsertRec((char*) &a, rid)) < 0)
    PrintErrorExit(rc);

  strcpy(a.relName, "relcat");
  strcpy(a.attrName, "numPages");
  a.offset = offsetof(DataRelInfo, numPages);
  a.attrType = INT;
  a.attrLength = sizeof(int);
  a.indexNo = -1;
  if ((rc = attrfh.InsertRec((char*) &a, rid)) < 0)
    PrintErrorExit(rc);

  strcpy(a.relName, "relcat");
  strcpy(a.attrName, "numRecords");
  a.offset = offsetof(DataRelInfo, numRecords);
  a.attrType = INT;
  a.attrLength = sizeof(int);
  a.indexNo = -1;
  if ((rc = attrfh.InsertRec((char*) &a, rid)) < 0)
    PrintErrorExit(rc);


  // attrcat attrs
  strcpy(a.relName, "attrcat");
  strcpy(a.attrName, "relName");
  a.offset = offsetof(DataAttrInfo, relName);
  a.attrType = STRING;
  a.attrLength = MAXNAME+1;
  a.indexNo = -1;
  if ((rc = attrfh.InsertRec((char*) &a, rid)) < 0)
    PrintErrorExit(rc);

  strcpy(a.relName, "attrcat");
  strcpy(a.attrName, "attrName");
  a.offset = offsetof(DataAttrInfo, relName) + MAXNAME+1;
  a.attrType = STRING;
  a.attrLength = MAXNAME+1;
  a.indexNo = -1;
  if ((rc = attrfh.InsertRec((char*) &a, rid)) < 0)
    PrintErrorExit(rc);

  strcpy(a.relName, "attrcat");
  strcpy(a.attrName, "offset");
  a.offset = offsetof(DataAttrInfo, offset);
  a.attrType = INT;
  a.attrLength = sizeof(int);
  a.indexNo = -1;
  if ((rc = attrfh.InsertRec((char*) &a, rid)) < 0)
    PrintErrorExit(rc);

  strcpy(a.relName, "attrcat");
  strcpy(a.attrName, "attrType");
  a.offset = offsetof(DataAttrInfo, attrType);
  a.attrType = INT;
  a.attrLength = sizeof(AttrType);
  a.indexNo = -1;
  if ((rc = attrfh.InsertRec((char*) &a, rid)) < 0)
    PrintErrorExit(rc);

  strcpy(a.relName, "attrcat");
  strcpy(a.attrName, "attrLength");
  a.offset = offsetof(DataAttrInfo, attrLength);
  a.attrType = INT;
  a.attrLength = sizeof(int);
  a.indexNo = -1;
  if ((rc = attrfh.InsertRec((char*) &a, rid)) < 0)
    PrintErrorExit(rc);

  strcpy(a.relName, "attrcat");
  strcpy(a.attrName, "indexNo");
  a.offset = offsetof(DataAttrInfo, indexNo);
  a.attrType = INT;
  a.attrLength = sizeof(int);
  a.indexNo = -1;
  if ((rc = attrfh.InsertRec((char*) &a, rid)) < 0)
    PrintErrorExit(rc);

  strcpy(a.relName, "attrcat");
  strcpy(a.attrName, "func");
  a.offset = offsetof(DataAttrInfo, func);
  a.attrType = INT;
  a.attrLength = sizeof(AggFun);
  a.indexNo = -1;
       if ((rc = attrfh.InsertRec((char*) &a, rid)) < 0)
    PrintErrorExit(rc);
    if ((rc =  rmm.CloseFile(attrfh)) < 0
        || (rc =  rmm.CloseFile(relfh)) < 0
        )
        PrintErrorExit(rc);

   AttrInfo attrs[3];

// 属性 1: id
    attrs[0].attrName = new char[strlen("id") + 1];
    strcpy(attrs[0].attrName, "id");
    attrs[0].attrType = INT;
    attrs[0].attrLength = sizeof(int);

// 属性 2: name
    attrs[1].attrName = new char[strlen("name") + 1];
    strcpy(attrs[1].attrName, "name");
    attrs[1].attrType = STRING;
    attrs[1].attrLength = 20;  // 字符串长度（可变）

    // 属性 3: age
    attrs[2].attrName = new char[strlen("age") + 1];
    strcpy(attrs[2].attrName, "age");
    attrs[2].attrType = INT;
    attrs[2].attrLength = sizeof(int);

    IX_Manager ixm(pfm);
    SM_Manager smm(ixm,rmm);

    RC rc1;
    if ((rc1 = smm.OpenDb("."))||
    (rc1 = smm.CreateTable("STUDENT", 3, attrs)))
        PrintErrorExit(rc1);

    if((rc1 = smm.Print("STUDENT")))
        PrintErrorExit(rc1);

    for (int i = 0; i < 3; ++i) {
        delete[] attrs[i].attrName;
    }
    if((rc1 = smm.CloseDb()))
        PrintErrorExit(rc1);



  return(0);
}

