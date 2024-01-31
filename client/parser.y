/*
* Parser for aql
*/
%{
#include "ast.h"
#include "xml.h"
#include <libxml/parser.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
extern int yylex();
extern int yyerror(struct ast** ast, const char *s, ...);
%}

%union {
    int ival;
    double fval;
    char *sval;
    struct ast *ast;
    int subtok;
}
    /* names and literal values */
%token <sval> VARNAME STRINGVAL
%token <ival> INTVAL BOOLVAL
%token <fval> FLOATVAL

    /* operators and precedence levels */
%left ','
%left OR
%left AND
%left <subtok> IN CMP


    /* keywords */
%token FOR RETURN FILTER INSERT UPDATE REMOVE WITH INTO CREATE DROP MERGE

    /* types */
%token<subtok> TYPE

    /* punctuation */


%token EOL YYEOF

%type <ast> for_stmt for_first_stmt filter_list
%type <ast> constant 
%type <ast> filter_stmt conditions filter_expr filter_attr_name
%type <ast> return_stmt attr_name merge
%type <ast> query terminal non_terminal_list 
%type <ast> insert_stmt document pairs pair
%type <ast> update_stmt remove_stmt drop_stmt
%type <ast> create_stmt create_pairs create_pair

%glr-parser

%locations
%parse-param { struct ast **root }
%start query
%define parse.error verbose

%%
terminal: return_stmt
    | insert_stmt
    | update_stmt
    | remove_stmt
    | create_stmt
    | drop_stmt
    ;

query: terminal YYEOF                            { $$ = $1;}
    | for_first_stmt YYEOF                       { $$ = $1;}
    ;

for_first_stmt: FOR VARNAME IN VARNAME terminal             { $$ = newfor($2, $4, NULL, $5); *root= $$;}
    | FOR VARNAME IN VARNAME non_terminal_list terminal     { $$ = newfor($2, $4, $5, $6); *root= $$;}
    ;

for_stmt: FOR VARNAME IN VARNAME                                { $$ = newfor($2, $4, NULL, NULL); *root= $$;}
    | FOR VARNAME IN VARNAME non_terminal_list                  { $$ = newfor($2, $4, $5, NULL); *root= $$; }
    ;

non_terminal_list: for_stmt                                  { $$ = newlist($1, NULL); *root= $$;}
    | filter_list                                            { $$ = $1; }
    | filter_list for_stmt                                   { $$ = newlist($2, $1); *root= $$;}  
    ;

filter_list: filter_stmt                                     { $$ = newlist($1, NULL); *root= $$;}  
    | filter_list filter_stmt                                { $$ = newlist($2, $1); *root= $$;}
    ;

/*---------------filter-----------------*/
filter_stmt: FILTER conditions              { $$ = newfilter($2); *root= $$;}
    ;
conditions: '(' filter_expr ')'             { $$ = newfilter_condition($2, NULL, -1); *root= $$;}
    | filter_expr                           { $$ = newfilter_condition($1, NULL, -1); *root= $$;}
    | filter_expr AND conditions            { $$ = newfilter_condition($1, $3, NT_AND); *root= $$;}
    | filter_expr OR conditions             { $$ = newfilter_condition($1, $3, NT_OR); *root= $$;}
    ;
filter_expr: filter_attr_name CMP constant       { $$ = newfilter_expr($1, $3, $2); *root= $$;}
    | filter_attr_name CMP filter_attr_name      { $$ = newfilter_expr($1, $3, $2); *root= $$;}
    | constant IN filter_attr_name              { $$ = newfilter_expr($3, $1, $2); *root= $$;}
    ;

filter_attr_name: VARNAME '.' VARNAME         { $$ = newattr_name($1, $3); *root= $$;}

/*---------------return-----------------*/
return_stmt: RETURN attr_name               { $$ = newreturn($2); *root= $$;}
    |    RETURN MERGE '(' merge ')'         { $$ = newreturn($4); *root= $$;}

merge: VARNAME ',' VARNAME                { $$ = newmerge($1, $3); *root= $$;}
    ;
attr_name: VARNAME '.' VARNAME         { $$ = newattr_name($1, $3);*root= $$;}
    | VARNAME                          { $$ = newattr_name($1, NULL); *root= $$;}
    ;


constant: INTVAL                         { $$ = newint($1); *root= $$;}
        | FLOATVAL                       { $$ = newfloat($1); *root= $$;}
        | BOOLVAL                        { $$ = newbool($1); *root= $$;}
        | STRINGVAL                      { $$ = newstring($1); *root= $$;}
        ;

/*---------------insert-----------------*/
insert_stmt: INSERT document INTO VARNAME       { $$ = newinsert($4, $2); *root= $$;}
    ;

document : '{' pairs '}'                        { $$ = $2;}
    ;
pairs: pair                                     { $$ = newlist($1, NULL);*root= $$;}
    | pairs ',' pair                            { $$ = newlist($3, $1);*root= $$;}
    ;
pair: VARNAME ':' constant                      { $$ = newpair($1, $3); *root= $$;}

/*---------------update-----------------*/
update_stmt: UPDATE attr_name WITH document IN VARNAME       { $$ = newupdate($6, $2, $4); *root= $$;}
    ;

/*---------------remove-----------------*/
remove_stmt: REMOVE attr_name IN VARNAME       { $$ = newremove($4, $2); *root= $$;}
    ;
/*---------------create-----------------*/
create_stmt: CREATE VARNAME WITH '{' create_pairs '}'       { $$ = newcreate($2, $5); *root= $$;}
    ;
create_pairs: create_pair                           { $$ = newlist($1, NULL); *root= $$;}
    | create_pairs ',' create_pair                  { $$ = newlist($3, $1); *root= $$;}
    ;
create_pair: VARNAME ':' TYPE                      { $$ = newcreate_pair($1, $3); *root= $$;}
    ;

/*---------------drop-----------------*/
drop_stmt: DROP VARNAME       { $$ = newdrop($2); *root= $$;}
    ;

%%

int yyerror(struct ast** ast, const char *s, ...)
{
    va_list ap;
    va_start(ap, s);

    if(yylloc.first_line)
        fprintf(stderr, "%d.%d-%d.%d: error: ", 
        yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
    return 0;
}

struct ast * parse_ast(){
    struct ast *root = NULL;
    yyparse(&root);
    return root;
}



