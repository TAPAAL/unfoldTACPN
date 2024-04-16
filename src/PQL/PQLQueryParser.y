%{
#include <stdio.h>
#include <memory>

#include "PQL/PQL.h"
#include "PQL/Expressions.h"

using namespace unfoldtacpn::PQL;

std::shared_ptr<unfoldtacpn::PQL::Condition> result_query;
extern int pqlqlex();
void pqlqerror(const char *s) { printf("ERROR: %s\n", s); std::exit(-1); }
%}

%name-prefix "pqlq"
%expect 2

/* Possible data representation */
%union {
	unfoldtacpn::PQL::Expr* expr;
	unfoldtacpn::PQL::Condition* cond;
	std::string *string;
	int token;
}

/* Terminal type definition */
%token <string> ID INT FLOAT
%token <token> DEADLOCK TRUE FALSE COUNT
%token <token> LPAREN RPAREN LBRACK RBRACK
%token <token> AND OR NOT
%token <token> EQUAL NEQUAL LESS LESSEQUAL GREATER GREATEREQUAL
%token <token> PLUS MINUS MULTIPLY
%token <token> EF AG AF EG PF PG CONTROL COLON

/* Terminal associativity */
%left AND OR
%right NOT

/* Nonterminal type definition */
%type <expr> expr term factor bound
%type <cond> logic compare query proba

/* Operator precedence, more possibly coming */

%start control_query

%%

control_query
        : CONTROL COLON query { result_query = std::make_shared<ControlCondition>(Condition_ptr($3)); }
        | query { result_query = Condition_ptr($1); }
		| error { yyerrok; }
        ;

query	: EF logic					{ $$ = new EFCondition(Condition_ptr($2)); }
        | AG logic                  { $$ = new AGCondition(Condition_ptr($2)); }
        | EG logic                  { $$ = new EGCondition(Condition_ptr($2)); }
        | AF logic                  { $$ = new AFCondition(Condition_ptr($2)); }
/*		| proba_comp				{ $$ = $1; }*/
		| proba						{ $$ = $1; }
		;

logic	: logic AND logic			{ $$ = new AndCondition(Condition_ptr($1), Condition_ptr($3)); }
		| logic OR logic 			{ $$ = new OrCondition(Condition_ptr($1), Condition_ptr($3)); }
		| NOT logic					{ $$ = new NotCondition(Condition_ptr($2)); }
		| LPAREN logic RPAREN		{ $$ = $2; }
		| compare					{ $$ = $1; }
		| TRUE						{ $$ = new BooleanCondition(true);}
		| FALSE						{ $$ = new BooleanCondition(false);}
		| DEADLOCK					{ $$ = new DeadlockCondition();}
		;

compare	: expr EQUAL expr			{ $$ = new EqualCondition(Expr_ptr($1), Expr_ptr($3)); }
		| expr NEQUAL expr			{ $$ = new NotEqualCondition(Expr_ptr($1), Expr_ptr($3)); }
		| expr LESS expr			{ $$ = new LessThanCondition(Expr_ptr($1), Expr_ptr($3)); }
		| expr LESSEQUAL expr 		{ $$ = new LessThanOrEqualCondition(Expr_ptr($1), Expr_ptr($3)); }
		| expr GREATER expr			{ $$ = new LessThanCondition(Expr_ptr($3), Expr_ptr($1)); }
		| expr GREATEREQUAL expr	{ $$ = new LessThanOrEqualCondition(Expr_ptr($3), Expr_ptr($1)); }
		;

expr	: expr PLUS term			{ $$ = new PlusExpr(std::vector<Expr_ptr>({Expr_ptr($1), Expr_ptr($3)})); }
		| expr MINUS term			{ $$ = new SubtractExpr(std::vector<Expr_ptr>({Expr_ptr($1), Expr_ptr($3)})); }
		| MINUS expr				{ $$ = new MinusExpr(Expr_ptr($2)); }
		| term						{ $$ = $1; }
		;

term	: term MULTIPLY factor	{ $$ = new MultiplyExpr(std::vector<Expr_ptr>({Expr_ptr($1), Expr_ptr($3)})); }
		| factor				{ $$ = $1; }
		;

factor	: LPAREN expr RPAREN	{ $$ = $2; }
		| INT			{ $$ = new LiteralExpr(atol($1->c_str())); delete $1; }
		| ID			{ $$ = new IdentifierExpr(*$1); delete $1; }
		;

/*
proba_comp
		: proba GREATEREQUAL FLOAT 		{ $$ = new StaticProbaCompCondition(Condition_ptr($1), atol($3->c_str())); delete $3; }
		| proba GREATEREQUAL proba 		{ $$ = new ProbaCompCondition(Condition_ptr($1), Condition_ptr($3)); }
		| proba LESSEQUAL proba 		{ $$ = new ProbaCompCondition(Condition_ptr($3), Condition_ptr($1)); }
		;
*/

proba	: PF bound logic				{ $$ = new PFCondition(Expr_ptr($2), Condition_ptr($3)); }
		| PG bound logic				{ $$ = new PGCondition(Expr_ptr($2), Condition_ptr($3)); }
		;

bound 	: LBRACK LESSEQUAL INT RBRACK			{ $$ = new TimeBoundExpr(atol($3->c_str())); delete $3; }
		| LBRACK COUNT LESSEQUAL INT RBRACK		{ $$ = new StepBoundExpr(atol($4->c_str())); delete $4; }
		;