//
// @Author: 杨皓然 23301142
// @E-amil: 23301142@bjtu.edu.cn
// @CreateTime: 2025/5/7 15:51
// @Project: GuoShuBase
//
//


#include <cstdio>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cstdlib>

#include "../guoshubase.h"
#include "pf.h"
#include "rm.h"

using namespace std;

//
// Defines
//
const char * FILENAME =  "testrel";       // test file name
#define STRLEN      29               // length of string in testrec
#define PROG_UNIT   50               // how frequently to give progress
//   reports when adding lots of recs
#define FEW_RECS  20                // number of records added in

//
// Computes the offset of a field in a record (should be in <stddef.h>)
//
#ifndef offsetof
#       define offsetof(type, field)   ((size_t)&(((type *)0) -> field))
#endif

//
// Structure of the records we will be using for the tests
//
struct TestRec {
    char  str[STRLEN];
    int   num;
    float r;
};


//
// Global PF_Manager and RM_Manager variables
//
PF_Manager pfm;
RM_Manager rmm(pfm);

//
// Function declarations
//
RC Test1(void);
RC Test2(void);

void PrintErrorAll(RC rc);
void LsFile(char *fileName);
void PrintRecord(TestRec &recBuf);
RC AddRecs(RM_FileHandle &fh, int numRecs);

RC VerifyFile(RM_FileHandle &fh, int numRecs);
RC PrintFile(RM_FileScan &fh);

RC CreateFile(const char *fileName, int recordSize);
RC DestroyFile(const char *fileName);
RC OpenFile(const char *fileName, RM_FileHandle &fh);
RC CloseFile(const char *fileName, RM_FileHandle &fh);
RC InsertRec(RM_FileHandle &fh, char *record, RID &rid);
RC UpdateRec(RM_FileHandle &fh, RM_Record &rec);
RC DeleteRec(RM_FileHandle &fh, RID &rid);

RC GetNextRecScan(RM_FileScan &fs, RM_Record &rec);


//
// Array of pointers to the test functions
//
#define NUM_TESTS       2               // number of tests
int (*tests[])() =                      // RC doesn't work on some compilers
        {
                Test1,
                Test2
        };

//
// main
//
int main(int argc, char *argv[])
{
    RC   rc;
    char *progName = argv[0];   // since we will be changing argv
    int  testNum;

    // Write out initial starting message
    cerr.flush();
    cout.flush();
    cout << "Starting RM component test.\n";
    cout.flush();

    // Delete files from last time
    unlink(FILENAME);

    // If no argument given, do all tests
    if (argc == 1) {
        for (testNum = 0; testNum < NUM_TESTS; testNum++)
            if ((rc = (tests[testNum])())) {

                // Print the error and exit
                PrintErrorAll(rc);
                return (1);
            }
    }
    else {

        // Otherwise, perform specific tests
        while (*++argv != NULL) {

            // Make sure it's a number
            if (sscanf(*argv, "%d", &testNum) != 1) {
                cerr << progName << ": " << *argv << " is not a number\n";
                continue;
            }

            // Make sure it's in range
            if (testNum < 1 || testNum > NUM_TESTS) {
                cerr << "Valid test numbers are between 1 and " << NUM_TESTS << "\n";
                continue;
            }

            // Perform the test
            if ((rc = (tests[testNum - 1])())) {

                // Print the error and exit
                PrintErrorAll(rc);
                return (1);
            }
        }
    }

    // Write ending message and exit
    cout << "Ending RM component test.\n\n";

    return (0);
}


////////////////////////////////////////////////////////////////////
// The following functions may be useful in tests that you devise //
////////////////////////////////////////////////////////////////////

//
// LsFile
//
// Desc: list the filename's directory entry
//

void LsFile(const char *fileName) {
    // 方式1：直接调用系统命令（简单但可能有安全风险）
    char command[256];
    sprintf(command, "cmd /c dir \"%s\"", fileName);  // 使用引号包裹文件名防止空格问题
    printf("Executing: %s\n", command);
    system(command);

    // 方式2：使用 WinAPI（更安全推荐）
    /*
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(fileName, &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            printf("%s\t%lld bytes\n",
                  findData.cFileName,
                  ((LARGE_INTEGER*)&findData.nFileSizeLow)->QuadPart);
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    } else {
        printf("File not found: %s\n", fileName);
    }
    */
}


//
// PrintRecord
//
// Desc: Print the TestRec record components
//
void PrintRecord(TestRec &recBuf)
{
    printf("[%s, %d, %f]\n", recBuf.str, recBuf.num, recBuf.r);
}



//
// AddRecs
//
// Desc: Add a number of records to the file
//
RC AddRecs(RM_FileHandle &fh, int numRecs)
{
    RC      rc;
    int     i;
    TestRec recBuf;
    RID     rid;
    PageNum pageNum;
    SlotNum slotNum;

    // We set all of the TestRec to be 0 initially.  This heads off
    // warnings that Purify will give regarding UMR since sizeof(TestRec)
    // is 40, whereas actual size is 37.
    memset((void *)&recBuf, 0, sizeof(recBuf));

    printf("\nadding %d records\n", numRecs);
    for (i = 0; i < numRecs; i++) {
        memset(recBuf.str, ' ', STRLEN);
        sprintf(recBuf.str, "a%d", i);
        recBuf.num = i;
        recBuf.r = (float)i;
        if ((rc = InsertRec(fh, (char *)&recBuf, rid)) ||
            (rc = rid.GetPageNum(pageNum)) ||
            (rc = rid.GetSlotNum(slotNum)))
            return (rc);

        if ((i + 1) % PROG_UNIT == 0){
            printf("%d  ", i + 1);
            fflush(stdout);
        }
    }
    if (i % PROG_UNIT != 0)
        printf("%d\n", i);
    else
        putchar('\n');

    // Return ok
    return (0);
}

//
// VerifyFile
//
// Desc: verify that a file has records as added by AddRecs
//
RC VerifyFile(RM_FileHandle &fh, int numRecs)
{
    RC        rc;
    int       n;
    TestRec   *pRecBuf;
    RID       rid;
    char      stringBuf[STRLEN];
    char      *found;
    RM_Record rec;

    found = new char[numRecs];
    memset(found, 0, numRecs);

    printf("\nverifying file contents\n");

    RM_FileScan fs;
    if ((rc=fs.OpenScan(fh,INT,sizeof(int),offsetof(TestRec, num),
                        NO_OP, NULL, NO_HINT)))
        //int val = 10;
        // if ((rc=fs.OpenScan(fh,INT,sizeof(int),offsetof(TestRec, num),
        //                     LT_OP, (void*)&val, NO_HINT)))
//     const char * grr = "a15";
//     if ((rc=fs.OpenScan(fh,STRING,3,offsetof(TestRec, str),
//                         GE_OP, (void*)grr, NO_HINT)))
        return (rc);

    // For each record in the file
    for (rc = GetNextRecScan(fs, rec), n = 0;
         rc == 0;
         rc = GetNextRecScan(fs, rec), n++) {

        // Make sure the record is correct
        if ((rc = rec.GetData((char *&)pRecBuf)) ||
            (rc = rec.GetRid(rid)))
            goto err;

        memset(stringBuf,' ', STRLEN);
        sprintf(stringBuf, "a%d", pRecBuf->num);

        if (pRecBuf->num < 0 || pRecBuf->num >= numRecs ||
            strcmp(pRecBuf->str, stringBuf) ||
            pRecBuf->r != (float)pRecBuf->num) {
            printf("VerifyFile: invalid record = [%s, %d, %f]\n",
                   pRecBuf->str, pRecBuf->num, pRecBuf->r);
            printf("Nandu expected RID[%d,%d] = [%s, %d, %f]\n",
                   rid.Page(), rid.Slot(), stringBuf, pRecBuf->num, pRecBuf->r);

            exit(1);
        }

        if (found[pRecBuf->num]) {
            printf("VerifyFile: duplicate record = [%s, %d, %f]\n",
                   pRecBuf->str, pRecBuf->num, pRecBuf->r);
            exit(1);
        }

        printf("Nandu found RID[%d,%d] = [%s, %d, %f]\n",
               rid.Page(), rid.Slot(), stringBuf, pRecBuf->num, pRecBuf->r);

        found[pRecBuf->num] = 1;
    }

    if (rc != RM_EOF)
        goto err;

    if ((rc=fs.CloseScan()))
        goto err;

    // make sure we had the right number of records in the file
    if (n != numRecs) {
        delete[] found;
        printf("%d records in file (supposed to be %d)\n",
               n, numRecs);
        exit(1);
    }

    // Return ok
    rc = 0;

    err:
    fs.CloseScan();
    delete[] found;
    return (rc);
}

//
// PrintFile
//
// Desc: Print the contents of the file
//
RC PrintFile(RM_FileScan &fs)
{
    RC        rc;
    int       n;
    TestRec   *pRecBuf;
    RID       rid;
    RM_Record rec;

    printf("\nprinting file contents\n");

    // for each record in the file
    for (rc = GetNextRecScan(fs, rec), n = 0;
         rc == 0;
         rc = GetNextRecScan(fs, rec), n++) {

        // Get the record data and record id
        if ((rc = rec.GetData((char *&)pRecBuf)) ||
            (rc = rec.GetRid(rid)))
            return (rc);

        // Print the record contents
        PrintRecord(*pRecBuf);
    }

    if (rc != RM_EOF)
        return (rc);

    printf("%d records found\n", n);

    // Return ok
    return (0);
}

////////////////////////////////////////////////////////////////////////
// The following functions are wrappers for some of the RM component  //
// methods.  They give you an opportunity to add debugging statements //
// and/or set breakpoints when testing these methods.                 //
////////////////////////////////////////////////////////////////////////


//
// CreateFile
//
// Desc: call RM_Manager::CreateFile
//
RC CreateFile(const char *fileName, int recordSize)
{
    printf("\ncreating %s\n", fileName);
    return (rmm.CreateFile(fileName, recordSize));
}

//
// DestroyFile
//
// Desc: call RM_Manager::DestroyFile
//
RC DestroyFile(const char *fileName)
{
    printf("\ndestroying %s\n", fileName);
    return (rmm.DestroyFile(fileName));
}


//
// OpenFile
//
// Desc: call RM_Manager::OpenFile
//
RC OpenFile(const char *fileName, RM_FileHandle &fh)
{
    printf("\nopening %s\n", fileName);
    return (rmm.OpenFile(fileName, fh));
}

//
// CloseFile
//
// Desc: call RM_Manager::CloseFile
//
RC CloseFile(const char *fileName, RM_FileHandle &fh)
{
    if (fileName != NULL)
        printf("\nClosing %s\n", fileName);
    return (rmm.CloseFile(fh));
}


//
// InsertRec
//
// Desc: call RM_FileHandle::InsertRec
//
RC InsertRec(RM_FileHandle &fh, char *record, RID &rid)
{
    return (fh.InsertRec(record, rid));
}

//
// DeleteRec
//
// Desc: call RM_FileHandle::DeleteRec
//
RC DeleteRec(RM_FileHandle &fh, RID &rid)
{
    return (fh.DeleteRec(rid));
}

//
// UpdateRec
//
// Desc: call RM_FileHandle::UpdateRec
//
RC UpdateRec(RM_FileHandle &fh, RM_Record &rec)
{
    return (fh.UpdateRec(rec));
}

//
// GetNextRecScan
//
// Desc: call RM_FileScan::GetNextRec
//
RC GetNextRecScan(RM_FileScan &fs, RM_Record &rec)
{
    return (fs.GetNextRec(rec));
}


/////////////////////////////////////////////////////////////////////
// Sample test functions follow.                                   //
/////////////////////////////////////////////////////////////////////

//
// Test1 tests simple creation, opening, closing, and deletion of files
//
RC Test1(void)
{
    RC            rc;
    RM_FileHandle fh;

    printf("test1 starting ****************\n");

    if (
            (rc = CreateFile(FILENAME, sizeof(TestRec)))
            || (rc = OpenFile(FILENAME, fh))
            || (rc = CloseFile(FILENAME, fh))
            )
        return (rc);

    LsFile(FILENAME);

    if ((rc = DestroyFile(FILENAME)))
        return (rc);

    printf("\ntest1 done ********************\n");
    return (0);
}

//
// Test2 tests adding a few records to a file.
//
RC Test2(void)
{
    RC            rc;
    RM_FileHandle fh;

    printf("test2 starting ****************\n");

    if ((rc = CreateFile(FILENAME, sizeof(TestRec))) ||
        (rc = OpenFile(FILENAME, fh)) ||
        (rc = AddRecs(fh, FEW_RECS)) ||
        (rc = VerifyFile(fh, FEW_RECS)) ||
        (rc = CloseFile(FILENAME, fh)))
        return (rc);

    LsFile(FILENAME);
    // PrintFile(fh);

    if ((rc = DestroyFile(FILENAME)))
        return (rc);

    printf("\ntest2 done ********************\n");
    return (0);
}
