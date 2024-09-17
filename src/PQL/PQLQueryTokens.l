%{
#include <string>
#include <memory>
#include "PQL/PQL.h"
#include "PQLQueryParser.parser.hpp"

#define SAVE_TOKEN pqlqlval.string = new std::string(pqlqtext, pqlqleng)
#define SAVE_QUOTED_TOKEN pqlqlval.string = new std::string(pqlqtext+1, pqlqleng-2)
#define TOKEN(t) (pqlqlval.token = t)

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#endif

#define register      // Deprecated in C++11.
extern "C" int pqlqwrap(){return 1;}
extern std::shared_ptr<unfoldtacpn::PQL::Condition> result_query;
extern int pqlqparse();
%}
%option prefix="pqlq"
%option nounput

digit         [0-9]
letter        [a-zA-Z_]
%%

[ \t\n\r]         			;
"true"            			{return TOKEN(TRUE);}
"false"           			{return TOKEN(FALSE);}
"deadlock"          		{return TOKEN(DEADLOCK);}
"and"           			{return TOKEN(AND);}
"AND"           			{return TOKEN(AND);}
"or"            			{return TOKEN(OR);}
"OR"            			{return TOKEN(OR);}
"not"           			{return TOKEN(NOT);}
"NOT"           			{return TOKEN(NOT);}
"AG"						{return TOKEN(AG);}
"EG"						{return TOKEN(EG);}
"AF"						{return TOKEN(AF);}
"EF"						{return TOKEN(EF);}
"PG"						{return TOKEN(PG);}
"PF"						{return TOKEN(PF);}
"control"                   {return TOKEN(CONTROL);}
{letter}({letter}|{digit})* {SAVE_TOKEN; return ID;}
{digit}+          			{SAVE_TOKEN; return INT;}
{digit}+\.{digit}+			{SAVE_TOKEN; return FLOAT;}
(\"(\\.|[^"])*\") 			{SAVE_QUOTED_TOKEN; return ID;}
"&&"						{return TOKEN(AND);}
"||"						{return TOKEN(OR);}
"!"							{return TOKEN(NOT);}
"=="						{return TOKEN(EQUAL);}
"!="						{return TOKEN(NEQUAL);}
"<"							{return TOKEN(LESS);}
"<="						{return TOKEN(LESSEQUAL);}
">"							{return TOKEN(GREATER);}
">="						{return TOKEN(GREATEREQUAL);}
"("							{return TOKEN(LPAREN);}
")"							{return TOKEN(RPAREN);}
"["							{return TOKEN(LBRACK);}
"]"							{return TOKEN(RBRACK);}
"+"							{return TOKEN(PLUS);}
"-"							{return TOKEN(MINUS);}
"*"							{return TOKEN(MULTIPLY);}
"="							{return TOKEN(EQUAL);}
":"							{return TOKEN(COLON);}
"#"							{return TOKEN(COUNT);}
.							{printf("Unknown token %s!\n", pqlqtext); yyterminate();}

%%
namespace unfoldtacpn{ namespace PQL {
std::shared_ptr<Condition> parseQuery(const std::string& queryString) {
	//Load up input buffer in Flex
	YY_BUFFER_STATE buf = pqlq_scan_string(queryString.c_str());

	if(pqlqparse() != 0)
		return NULL;

	//Delete the buffer
	pqlq_delete_buffer(buf);
	return result_query;
}
}}
#ifdef __clang__
#pragma clang diagnostic pop
#endif
