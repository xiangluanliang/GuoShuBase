//
// ql_error.h
//


#ifndef QL_ERROR_H
#define QL_ERROR_H

#include "../guoshubase.h"

//
// Print-error function
//
void QL_PrintError(RC rc);

#define QL_KEYNOTFOUND    (START_QL_WARN + 0)  // cannot find key
#define QL_INVALIDSIZE    (START_QL_WARN + 1)  // invalid number of attributes
#define QL_ENTRYEXISTS    (START_QL_WARN + 2)  // key,rid already
// exists in index
#define QL_NOSUCHENTRY    (START_QL_WARN + 3)  // key,rid combination
// does not exist in index

#define QL_LASTWARN QL_ENTRYEXISTS


#define QL_BADJOINKEY      (START_QL_ERR - 0)
#define QL_ALREADYOPEN     (START_QL_ERR - 1)
#define QL_BADATTR         (START_QL_ERR - 2)
#define QL_DUPREL          (START_QL_ERR - 3)
#define QL_RELMISSINGFROMFROM (START_QL_ERR - 4)
#define QL_FNOTOPEN        (START_QL_ERR - 5)
#define QL_JOINKEYTYPEMISMATCH (START_QL_ERR - 6)
#define QL_BADOPEN         (START_QL_ERR - 7)
#define QL_EOF             (START_QL_ERR - 8)
#define QL_INVALIDQUERY    (START_QL_ERR - 9)
#define QL_ATTRNOTFOUND    (QL_INVALIDQUERY - 1) //Attribute not found
#define QL_AMBIGUOUSATTR   (QL_INVALIDQUERY - 2) //Ambiguous attribute
#define QL_SELECTERR       (QL_INVALIDQUERY - 3)
#define QL_INSERTERR       (QL_INVALIDQUERY - 4)
#define QL_DELETEERR       (QL_INVALIDQUERY - 5)
#define QL_UPDATEERR       (QL_INVALIDQUERY - 6)
#define QL_CREATETABLEERR  (QL_INVALIDQUERY - 7)
#define QL_DROPTABLEERR    (QL_INVALIDQUERY - 8)
#define QL_CREATEINDEXERR  (QL_INVALIDQUERY - 9)
#define QL_DROPINDEXERR    (QL_INVALIDQUERY - 10)
#define QL_SHOWINDEXERR    (QL_INVALIDQUERY - 11)
#define QL_LASTERROR QL_SHOWINDEXERR

#endif // QL_ERROR_H
