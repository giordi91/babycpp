#include "parser.h"

namespace babycpp {
namespace parser {

using lexer::Token;

const std::unordered_map<char, int> Parser::BIN_OP_PRECEDENCE = {
    {'<', 10}, {'+', 20}, {'-', 20}, {'*', 40}, {'/', 50}};


NumberExprAST* Parser::parseNumber()
{
    if(lex->currtok != Token::tok_number){
        return nullptr;}
    auto* node = new NumberExprAST(lex->value);
    lex->gettok(); //eating the number;
    return  node;
}

}//namespace parser
}//namespace babycpp
