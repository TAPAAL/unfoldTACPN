%{
#include <stdio.h>
#include <memory>

#include "PQL/PQL.h"
#include "PQL/Expressions.h"

using namespace unfoldtacpn::PQL;

std::shared_ptr<unfoldtacpn::PQL::Condition> query;
extern int pqlqlex();
void pqlqerror(const char *s) {printf("ERROR: %s\n", s);}
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
%token <string> ID INT
%token <token> DEADLOCK TRUE FALSE
%token <token> LPAREN RPAREN
%token <token> AND OR NOT
%token <token> EQUAL NEQUAL LESS LESSEQUAL GREATER GREATEREQUAL
%token <token> PLUS MINUS MULTIPLY
%token <token> EF AG AF EG

/* Terminal associativity */
%left AND OR
%right NOT

/* Nonterminal type definition */
%type <expr> expr term factor
%type <cond> logic compare

/* Operator precedence, more possibly coming */

%start query

%%

query	: EF logic					{ query = std::make_shared<EFCondition>(Condition_ptr($2)); }
        | AG logic                  { query = std::make_shared<AGCondition>(Condition_ptr($2)); }
        | EG logic                  { query = std::make_shared<EGCondition>(Condition_ptr($2)); }
        | AF logic                  { query = std::make_shared<AFCondition>(Condition_ptr($2)); }
		| error						{ yyerrok; }
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
		| expr GREATER expr			{ $$ = new GreaterThanCondition(Expr_ptr($1), Expr_ptr($3)); }
		| expr GREATEREQUAL expr	{ $$ = new GreaterThanOrEqualCondition(Expr_ptr($1), Expr_ptr($3)); }
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
