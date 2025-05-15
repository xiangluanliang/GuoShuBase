#include "guoshubase_interface.h"
using namespace std;
std::ostream* gBaseOut = &std::cout;


int OpenDb(const char *dbname){
    return smm.OpenDb(dbname);
}

int CreateDb(const char *dbname){
    int rc;
    if (chdir(dbname) < 0) {
        cerr << " chdir error to " << dbname << "\n";
        exit(1);
    }
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

    return rc;
}

int CloseDb(){
    return smm.CloseDb();
}

int inputSQL(std::ostream& os, const char *sql){
    gBaseOut = &os;
//    *gBaseOut << sql;
    return GBparseSQL(pfm, smm, qlm, sql);
}