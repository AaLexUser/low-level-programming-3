%option noyywrap nodefault yylineno caseless
%{
#include "ast.h"
#include "parser.tab.h"

/* handle locations */ 
int yycolmn = 1;
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
                        yylloc.first_column = yycolmn; \
                        yylloc.last_column = yycolmn + yyleng - 1; \
                        yycolmn += yyleng;

extern YYSTYPE yylval;
int oldstate;
extern int yyerror(struct ast** ast, const char *s, ...);
%}

%x COMMENT

%%

    /* keywords */
for         { return FOR; }
return      { return RETURN; }
filter      { return FILTER; }
insert      { return INSERT; }
update      { return UPDATE; }
remove      { return REMOVE; }
merge       { return MERGE; }

in          { yylval.subtok = NT_IN; return IN; }
with        { return WITH; }
into        { return INTO; }

create      { return CREATE; }
drop        { return DROP; }

int         { yylval.subtok = NT_INTEGER; return TYPE; }
float       { yylval.subtok = NT_FLOAT; return TYPE; }
string      { yylval.subtok = NT_STRING; return TYPE; }
bool        { yylval.subtok = NT_BOOLEAN; return TYPE; }

    /* numbers */
-?[0-9]+      { yylval.ival = atoi(yytext); return INTVAL; }

-?[0-9]+"."[0-9]*               |
-?"."[0-9]+                     |
-?[0-9]+E[-+]?[0-9]+            |
-?[0-9]+"."[0-9]*E[-+]?[0-9]+   |
-?"."[0-9]+E[-+]?[0-9]+         { yylval.fval = atof(yytext); return FLOATVAL; }

    /* booleans */
true        { yylval.ival = 1; return BOOLVAL; }
false       { yylval.ival = 0; return BOOLVAL; }

    /* strings */
'(\\.|''|[^'\n])*'              |
\"(\\.|\"\"|[^"\n])*\"          { yylval.sval = strdup(yytext); return STRINGVAL; }

'(\\.|[^'\n])*$                 { yyerror(NULL, "unterminated string %s", yytext); }
\"(\\.|[^"\n])*$                { yyerror(NULL, "unterminated string %s", yytext); }

    /* operators */
[(){}:,;\.]        { return yytext[0]; }

"&&"            { return AND; }
"||"            { return OR; }
"AND"           { return AND; }
"OR"            { return OR; }

"<"             { yylval.subtok = NT_LT; return CMP; }
">"             { yylval.subtok = NT_GT; return CMP; }
"!="            { yylval.subtok = NT_NEQ; return CMP; }
"=="            { yylval.subtok = NT_EQ; return CMP; }
"<="            { yylval.subtok = NT_LTE; return CMP; }
">="            { yylval.subtok = NT_GTE; return CMP; }

    /* names */
[a-zA-Z_][a-zA-Z0-9_]*  { yylval.sval = strdup(yytext); return VARNAME; }
`[^`/\\.\n]+`           { 
                            yylval.sval = strdup(yytext + 1);
                            yylval.sval[yyleng-2] = 0;
                            return VARNAME; 
                        }
`[^`\n]*$               { yyerror(NULL, "unterminated quoted name %s", yytext); }

    /* comments */

"/*"                    { oldstate = YY_START; BEGIN(COMMENT); }
<COMMENT>"*/"           { BEGIN(oldstate); }
<COMMENT>.|\n           { yycolmn = 1; }

    /* whitespace */
[ \t\r]+                { }
^\n                     { return EOL; }
\n                      { yycolmn = 1; }

    /* everything else */
.                       { yyerror(NULL, "mystery character '%c'", *yytext); }

%%
