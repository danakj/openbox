%{
#include "obcl.h"
#include "parse.h"
%}

%option yylineno

DGT   [0-9]
ID    [a-zA-Z_][a-zA-Z_\.\-0-9]*


 /* string bummy */
%x str
%%

       char str_buf[1024];
       char *str_buf_ptr;
       int str_char;

 /* begin a string */
[\"\'] {
           str_buf_ptr = str_buf;
           str_char = yytext[0];
           BEGIN(str);
       }

 /* end a string */
<str>[\"\'] {
           if (yytext[0] == str_char) {
               BEGIN(INITIAL);
               *str_buf_ptr = '\0';
               yylval.string = g_strdup(str_buf);
               return TOK_STRING;
           } else {
               *str_buf_ptr++ = yytext[0];
           }
       }

 /* can't have newlines in strings */
<str>\n {
           printf("Error: Unterminated string constant.\n");
           BEGIN(INITIAL);
       }

 /* handle \" and \' */
<str>"\\"[\"\'] {
           if (yytext[1] == str_char)
               *str_buf_ptr++ = yytext[1];
           else {
               *str_buf_ptr++ = yytext[0];
               *str_buf_ptr++ = yytext[1];
           }
       }

 /* eat valid string contents */
<str>[^\\\n\'\"]+ {
           char *yptr = yytext;
           while (*yptr) {
               *str_buf_ptr++ = *yptr++;
           }
       }

 /* numberz */
{DGT}+ {
           yylval.num = atof(yytext);
           return TOK_NUM;
       }

 /* real numbers */
{DGT}+"."{DGT}* {
           yylval.num = atof(yytext);
           return TOK_NUM;
       }

 /* identifiers -- names without spaces and other crap in them */
{ID}   {
           yylval.string = g_strdup(yytext);
           return TOK_ID;
       }

 /* skip comments */
"#".*\n     ;
 /* skip other whitespace */
[ \n\t]+      ;
.           { return yytext[0]; }
%%
