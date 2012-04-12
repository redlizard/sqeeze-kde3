%{
#include <stdlib.h>
#include <stdio.h>
#include "ktraderparse.h"

void yyerror(const char *s);
int yylex();
void KTraderParse_initFlex( const char *s );

%}

%union
{
     char valb;
     int vali;
     double vald;
     char *name;
     void *ptr;
}

%token NOT
%token EQ
%token NEQ
%token LEQ
%token GEQ
%token LE
%token GR
%token OR
%token AND
%token TOKEN_IN
%token EXIST
%token MAX
%token MIN

%token <valb> VAL_BOOL
%token <name> VAL_STRING
%token <name> VAL_ID
%token <vali> VAL_NUM
%token <vald> VAL_FLOAT

%type <ptr> bool
%type <ptr> bool_or
%type <ptr> bool_and
%type <ptr> bool_compare
%type <ptr> expr_in
%type <ptr> expr_twiddle
%type <ptr> expr
%type <ptr> term
%type <ptr> factor_non
%type <ptr> factor

/* Grammar follows */

%%

constraint: /* empty */ { KTraderParse_setParseTree( 0L ); }
          | bool { KTraderParse_setParseTree( $<ptr>1 ); }
;

bool: bool_or { $$ = $<ptr>1; }
;

bool_or: bool_and OR bool_or { $$ = KTraderParse_newOR( $<ptr>1, $<ptr>3 ); }
       | bool_and { $$ = $<ptr>1; }
;

bool_and: bool_compare AND bool_and { $$ = KTraderParse_newAND( $<ptr>1, $<ptr>3 ); }
        | bool_compare { $$ = $<ptr>1; }
;

bool_compare: expr_in EQ expr_in { $$ = KTraderParse_newCMP( $<ptr>1, $<ptr>3, 1 ); }
            | expr_in NEQ expr_in { $$ = KTraderParse_newCMP( $<ptr>1, $<ptr>3, 2 ); }
            | expr_in GEQ expr_in { $$ = KTraderParse_newCMP( $<ptr>1, $<ptr>3, 3 ); }
            | expr_in LEQ expr_in { $$ = KTraderParse_newCMP( $<ptr>1, $<ptr>3, 4 ); }
            | expr_in LE expr_in { $$ = KTraderParse_newCMP( $<ptr>1, $<ptr>3, 5 ); }
            | expr_in GR expr_in { $$ = KTraderParse_newCMP( $<ptr>1, $<ptr>3, 6 ); }
            | expr_in { $$ = $<ptr>1; }
;

expr_in: expr_twiddle TOKEN_IN VAL_ID { $$ = KTraderParse_newIN( $<ptr>1, KTraderParse_newID( $<name>3 ) ); }
       | expr_twiddle { $$ = $<ptr>1; }
;

expr_twiddle: expr '~' expr { $$ = KTraderParse_newMATCH( $<ptr>1, $<ptr>3 ); }
            | expr { $$ = $<ptr>1; }
;

expr: expr '+' term { $$ = KTraderParse_newCALC( $<ptr>1, $<ptr>3, 1 ); }
    | expr '-' term { $$ = KTraderParse_newCALC( $<ptr>1, $<ptr>3, 2 ); }
    | term { $$ = $<ptr>1; }
;

term: term '*' factor_non { $$ = KTraderParse_newCALC( $<ptr>1, $<ptr>3, 3 ); }
    | term '/' factor_non { $$ = KTraderParse_newCALC( $<ptr>1, $<ptr>3, 4 ); }
    | factor_non { $$ = $<ptr>1; }
;

factor_non: NOT factor { $$ = KTraderParse_newNOT( $<ptr>2 ); }
          | factor { $$ = $<ptr>1; }
;

factor: '(' bool_or ')' { $$ = KTraderParse_newBRACKETS( $<ptr>2 ); }
      | EXIST VAL_ID { $$ = KTraderParse_newEXIST( $<name>2 ); }
      | VAL_ID { $$ = KTraderParse_newID( $<name>1 ); }
      | VAL_NUM { $$ = KTraderParse_newNUM( $<vali>1 ); }
      | VAL_FLOAT { $$ = KTraderParse_newFLOAT( $<vald>1 ); }
      | VAL_STRING { $$ = KTraderParse_newSTRING( $<name>1 ); }
      | VAL_BOOL { $$ = KTraderParse_newBOOL( $<valb>1 ); }
      | MAX VAL_ID { $$ = KTraderParse_newMAX2( $<name>2 ); }
      | MIN VAL_ID { $$ = KTraderParse_newMIN2( $<name>2 ); }
;

/* End of grammar */

%%

void yyerror ( const char *s )  /* Called by yyparse on error */
{
    KTraderParse_error( s );
}

void KTraderParse_mainParse( const char *_code )
{
  KTraderParse_initFlex( _code );
  yyparse();
}
