/*
 * Copyright (C) Rida Bazzi, 2016
 *
 * Do not share this file with anyone
 */
#ifndef __LEXER__H__
#define __LEXER__H__

#include <vector>
#include <string>

#include "inputbuf.h"

// ------- token types -------------------

typedef enum { END_OF_FILE = 0,
    PUBLIC, PRIVATE, DIV, EQUAL, COLON, COMMA, SEMICOLON, LBRACE, RBRACE, ID, ERROR
} TokenType;

class Token {
  public:
    void Print();

    std::string lexeme;
    TokenType token_type;
    int line_no;
};

class LexicalAnalyzer {
  public:
    Token GetToken();
    TokenType UngetToken(Token);
    LexicalAnalyzer();
    void Log(std::string);
    void Log(std::string, Token);
    void syntax_error();
    void parse_program();
    void parse_global_vars();    
    void parse_var_list(std::string);
    void parse_scope();
    void parse_public_vars();
    void parse_private_vars();
    void parse_stmt_list();
    void parse_stmt();  
    char peek_char();
    Token peek_token();

    void print_list();
    void print_scope_list();

  private:
    std::vector<Token> tokens;
    int line_no;
    Token tmp;
    InputBuffer input;
    
    bool SkipSpace();
    void SkipComment();
    bool IsKeyword(std::string);
    TokenType FindKeywordIndex(std::string);
    Token ScanIdOrKeyword();
    //Token ScanNumber();	
};

#endif  //__LEXER__H__
