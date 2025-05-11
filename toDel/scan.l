%{
#define YYSTYPE RB_STYPE
%}
%option c++
%option prefix="RB_"
%option noyywrap
%option yylineno
%{
/*
 * scan.l: lex spec for RQL
 *
 * Authors: Dallan Quass
 *          Jan Jannink
 *
 * originally by: Mark McAuliffe, University of Wisconsin - Madison, 1991
 */
#define YYSTYPE RB_STYPE
#include <string.h>
#include "guoshubase.h"          /* parse.h needs the definition of real */
#include "parser_internal.h"  /* y.tab.h needs the definition of NODE */
#include "y.tab.hpp"

static int get_id(char *s);   /* defined in scanhelp.c */
static char *get_qstring(char *qstring, int len);


/*
 * scanhelp.c: help functions for lexer
 *
 * Authors: Jan Jannink
 *          Jason McHugh
 *
 * originally by: Mark McAuliffe, University of Wisconsin - Madison, 1991
 *
 * 1997 Changes: "print", "buffer", "reset" and "io" added.
 * 1998 Changes: "resize", "queryplans", "on" and "off" added.
 *
 *
 * This file is not compiled separately; it is #included into lex.yy.c .
 */

/*
 * size of buffer of strings
 */
#define MAXCHAR		5000

/*
 * buffer for string allocation
 */
static char charpool[MAXCHAR];
static int charptr = 0;

static int lower(char *dst, char *src, int max);
static char *mk_string(char *s, int len);

/*
 * string_alloc: returns a pointer to a string of length len if possible
 */
static char *string_alloc(int len)
{
   char *s;

   if(charptr + len > MAXCHAR){
      fprintf(stderr, "out of memory\n");
      exit(1);
   }

   s = charpool + charptr;
   charptr += len;

   return s;
}

/*
 * reset_charptr: releases all memory allocated in preparation for the
 * next query.
 *
 * No return value.
 */
void reset_charptr(void)
{
    charptr = 0;
}

/*
 * reset_scanner: resets the scanner after a syntax error
 *
 * No return value.
 */
void reset_scanner(void)
{
    charptr = 0;
    yyrestart(yyin);
}

/*
 * get_id: determines whether s is a reserved word, and returns the
 * appropriate token value if it is.  Otherwise, it returns the token
 * value corresponding to a string.  If s is longer than the maximum token
 * length (MAXSTRINGLEN) then it returns NOTOKEN, so that the parser will
 * flag an error (this is a stupid kludge).
 */
static int get_id(char *s)
{
   static char string[MAXSTRINGLEN];
   int len;

   if((len = lower(string, s, MAXSTRINGLEN)) == MAXSTRINGLEN)
      return NOTOKEN;

   /*  SM layer lexemes */

   if(!strcmp(string, "create"))
      return yylval.ival = RW_CREATE;
   if(!strcmp(string, "drop"))
      return yylval.ival = RW_DROP;
   if(!strcmp(string, "table"))
      return yylval.ival = RW_TABLE;
   if(!strcmp(string, "index"))
      return yylval.ival = RW_INDEX;
   if(!strcmp(string, "load"))
      return yylval.ival = RW_LOAD;
   if(!strcmp(string, "help"))
      return yylval.ival = RW_HELP;
   if(!strcmp(string, "exit"))
      return yylval.ival = RW_EXIT;
   if(!strcmp(string, "print"))
      return yylval.ival = RW_PRINT;
   if(!strcmp(string, "set"))
      return yylval.ival = RW_SET;

   if(!strcmp(string, "and"))
      return yylval.ival = RW_AND;

   if(!strcmp(string, "into"))
      return yylval.ival = RW_INTO;
   if(!strcmp(string, "values"))
      return yylval.ival = RW_VALUES;


   /*  QL layer lexemes */
   if(!strcmp(string, "select"))
      return yylval.ival = RW_SELECT;
   if(!strcmp(string, "from"))
      return yylval.ival = RW_FROM;
   if(!strcmp(string, "where"))
      return yylval.ival = RW_WHERE;
   if(!strcmp(string, "order"))
      return yylval.ival = RW_ORDER;
   if(!strcmp(string, "group"))
      return yylval.ival = RW_GROUP;
   if(!strcmp(string, "by"))
      return yylval.ival = RW_BY;
   if(!strcmp(string, "desc"))
      return yylval.ival = RW_DESC;
   if(!strcmp(string, "asc"))
      return yylval.ival = RW_ASC;
   if(!strcmp(string, "insert"))
      return yylval.ival = RW_INSERT;
   if(!strcmp(string, "delete"))
      return yylval.ival = RW_DELETE;
   if(!strcmp(string, "update"))
      return yylval.ival = RW_UPDATE;
   if(!strcmp(string, "max"))
     return yylval.ival = RW_MAX;
   if(!strcmp(string, "min"))
      return yylval.ival = RW_MIN;
   if(!strcmp(string, "avg"))
      return yylval.ival = RW_AVG;
   if(!strcmp(string, "sum"))
      return yylval.ival = RW_SUM;
   if(!strcmp(string, "count"))
      return yylval.ival = RW_COUNT;

   /* IO Statistics lexemes */
   if(!strcmp(string, "reset"))
      return yylval.ival = RW_RESET;
   if(!strcmp(string, "io"))
      return yylval.ival = RW_IO;

   if(!strcmp(string, "resize"))
      return yylval.ival = RW_RESIZE;
   if(!strcmp(string, "buffer"))
      return yylval.ival = RW_BUFFER;

   if(!strcmp(string, "queryplans"))
      return yylval.ival = RW_QUERY_PLAN;
   if(!strcmp(string, "on"))
      return yylval.ival = RW_ON;
   if(!strcmp(string, "off"))
      return yylval.ival = RW_OFF;

   /*  unresolved lexemes are strings */

   yylval.sval = mk_string(s, len);
   return T_STRING;
}

/*
 * lower: copies src to dst, converting it to lowercase, stopping at the
 * end of src or after max characters.
 *
 * Returns:
 * 	the length of dst (which may be less than the length of src, if
 * 	    src is too long).
 */
static int lower(char *dst, char *src, int max)
{
   int len;

   for(len = 0; len < max && src[len] != '\0'; ++len){
      dst[len] = src[len];
      if(src[len] >= 'A' && src[len] <= 'Z')
         dst[len] += 'a' - 'A';
   }
   dst[len] = '\0';

   return len;
}

/*
 * get_qstring: removes the quotes from a quoted string, allocates
 * space for the resulting string.
 *
 * Returns:
 * 	a pointer to the new string
 */
static char *get_qstring(char *qstring, int len)
{
   /* replace ending quote with \0 */
   qstring[len - 1] = '\0';

   /* copy everything following beginning quote */
   return mk_string(qstring + 1, len - 2);
}

/*
 * mk_string: allocates space for a string of length len and copies s into
 * it.
 *
 * Returns:
 * 	a pointer to the new string
 */
static char *mk_string(char *s, int len)
{
   char *copy;

   /* allocate space for new string */
   if((copy = string_alloc(len + 1)) == NULL){
      printf("out of string space\n");
      exit(1);
   }

   /* copy the string */
   strncpy(copy, s, len + 1);
   return copy;
}


%}
letter               [A-Za-z]
digit                [0-9]
num                  {digit}+
s_num                [+\-]?{num}
%x comment
%x shell_cmd
%%
"/*"                 {BEGIN(comment);}
<comment>[^*]        {/* ignore the text of the comment */}
<comment>"*/"        {BEGIN(INITIAL);}
<comment>\*          {/* ignore *'s that aren't part of */}
[ \n\t]              {/* ignore spaces, tabs, and newlines */}
{s_num}              {sscanf(yytext, "%d", &yylval.ival);
                      return T_INT;}
{s_num}\.{num}       {sscanf(yytext, "%f", &yylval.rval);
                      return T_REAL;}
{s_num}\.{num}[Ee]{s_num}   {sscanf(yytext, "%f", &yylval.rval);
                             return T_REAL;}
\"([^\"\n]|(\"\"))*\"       {yylval.sval = get_qstring(yytext, yyleng);
                             return T_QSTRING;}
\"([^\"\n]|(\"\"))*\n       {printf("newline in string constant\n");}
{letter}({letter}|{digit}|_)*   {return get_id(yylval.sval = yytext);}
"<"                  {return T_LT;}
"<="                 {return T_LE;}
">"                  {return T_GT;}
">="                 {return T_GE;}
"="                  {return T_EQ;}
"!="                 {return T_NE;}
"<>"                 {return T_NE;}
!                    {BEGIN(shell_cmd);}
<shell_cmd>[^\n]*    {yylval.sval = yytext; return T_SHELL_CMD;}
<shell_cmd>\n        {BEGIN(INITIAL);}
[*/+\-=<>':;,.|&()]  {return yytext[0];}
<<EOF>>              {return T_EOF;}
.                    {printf("illegal character [%c]\n", yytext[0]);}
%%