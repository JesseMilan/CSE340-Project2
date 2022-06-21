/*
 * Copyright (C) Rida Bazzi, 2016
 *
 * Do not share this file with anyone
 * MODIFIED BY JESSE MILAN - 06/10/2022
 * THIS IS WORKING TO PARSE THE LEXER, so far received 87% coverage but just finished comment test cases
 */
#include <iostream>
#include <istream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>
#include <list>

#include "lexer.h"
#include "inputbuf.h"

using namespace std;

bool DEBUG = false;

string reserved[] = { "END_OF_FILE",
    "PUBLIC", "PRIVATE", "DIV", "EQUAL", "COLON", 
    "COMMA", "SEMICOLON", "LBRACE", "RBRACE", "ID", "ERROR"
};

#define KEYWORDS_COUNT 2
string keyword[] = { "public", "private" };
LexicalAnalyzer lexer;
Token token;

struct myScope {
    string scope; // global or dynamic and nested
    string permission; // public or private
    string key; // left hand side
};

struct myAssignment {
    string key; // lefthand side
    string value; // righthand side
};

vector<myScope> scopes; // for lookup
vector<myAssignment> assignments; // for printing
string currentScope;
bool isGlobalScopeRemoved = false;

void print_scope_list() {
    if (DEBUG) {
        cout << "printing scope list" << endl;
        
        int count = scopes.size();
        
        for(int i = 0; i < count; i++) {
            cout << "--------------" << endl;
            cout << "scope with item " + scopes[i].scope + " - " + scopes[i].permission + " - " + scopes[i].key << endl;
            cout << "--------------" << endl;
        };
    }
}

void addAssignment(myAssignment tmp) {
    assignments.push_back(tmp);
}

vector<myScope> addScope(myScope tmp) {

    vector<myScope> tmpList;

    tmpList = scopes;
    tmpList.push_back(tmp);

    return tmpList;

}

string findScope(string currentScope, string key) {

    // traversing array
    // - reverse order
    // - find first
	//      - if scope = current scope, can be private
	//      - if scope is not in current scope, must be public or global
	//      - if assigning key and not defined - syntax_error?

    int count = scopes.size();

    for(int i = count - 1; i >= 0; i--) {
        myScope thisScope = scopes[i];

        if (thisScope.key != key) {
            continue;
        }

        if (thisScope.scope == "global") {
            return "::" + thisScope.key;
        }

        if (thisScope.scope == currentScope) {
            return thisScope.scope + "." + thisScope.key;
        }

        if (thisScope.permission == "private") {
            continue;
        }

        return thisScope.scope + "." + thisScope.key;

    }

    return "?." + key;
      
}

int removeScope(string scope) {

    if (DEBUG) {
        cout << "removing scope " << scope << endl;
    }

    // // has global already been removed? 
    // if(isGlobalScopeRemoved) {
    //     cout << "Syntax Error\n";
    //     exit(1);
    // }

    int count = scopes.size();
    int countRemoved = 0;

    for(int i = 0; i < count; i++) {
        if (scopes[i].scope == scope) {
            //lexer.Log("removing scope with item " + scopes[i].scope + " - " + scopes[i].permission + " - " + scopes[i].key);
            //cout << "removing scope with item " + scopes[i].scope + " - " + scopes[i].permission + " - " + scopes[i].key << endl;
            //scopes.erase(scopes.begin() + i);
            scopes.pop_back();
            countRemoved++;
        }
    }

    // if (scope == "global") {
    //     isGlobalScopeRemoved = true;
    // }

    // set currentScope to the next/last in scopes
    if (scopes.size() > 0) {
        currentScope = scopes.back().scope;    

        if (DEBUG) {
            cout << "scope size > 0, new currentScope = " + currentScope << endl;
        }    
    } else {
        //currentScope = "global";
        if (currentScope == "global") {
            currentScope = "";
        } else {
            currentScope = "global";
        }
    }

    // if (isGlobalScopeRemoved && scopes.size() > 0) {
    //     cout << "Syntax Error\n";
    //     exit(1);
    // }

    if (DEBUG) {
        cout << "new scope = " << currentScope << endl;
        print_scope_list();
    }
    
    return countRemoved;
}

void LexicalAnalyzer::print_list() {

    int count = assignments.size();

    for(int i = 0; i < count; i++) {
        cout << assignments[i].key + " = " + assignments[i].value << "\n";
    }

}

void Token::Print()
{
    cout << "{" << this->lexeme << " , " // this -> lexeme = variable name
         << reserved[(int) this->token_type] << " , " // this-> token_type = ID, COMMA, etc. from the ENUM
         << this->line_no << "}\n";
}

void LexicalAnalyzer::syntax_error() {
    cout << "Syntax Error\n";
    exit(1);
}

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}



void LexicalAnalyzer::parse_program() {
     
     //parse global_vars and scope
    Token tmpToken;
   
    token = lexer.GetToken();

    while(token.token_type != END_OF_FILE) {

        Log("parse_program for token ", token);
        
        if (token.token_type == ID) {

            // check for comma or semi colon
            tmpToken = peek_token();
            
            Log("peeked token = ", tmpToken);
            
            if (tmpToken.token_type == COMMA || tmpToken.token_type == SEMICOLON) {
                parse_global_vars();                
            } else if (tmpToken.token_type == LBRACE) {
                parse_scope();
            } else {
                syntax_error();
            }
            
        } else {
            syntax_error();
        }

        ////Log("end of program while, getting next token and continuing");
        token = lexer.GetToken();
    }

    if (token.token_type == END_OF_FILE) {
        Log("reached the end of file, removing scope global");
        removeScope("global");
        // if (scopes.size() > 0) {
        //     Log("reached the end of file, but a scope still exists other than global");
        //     syntax_error();
        // }
    }

}

// GOOD, parses global vars at the top
void LexicalAnalyzer::parse_global_vars() {
    // empty set or var_list until SEMICOLON
    Log("parse_global_vars:", token);
    currentScope = "global";
    // token is the active character when we get here, so evaluate and advance to semi-colon
    parse_var_list("");

    // token should = SEMICOLON Here, perhaps syntax error if not?
    Log("parse_global_vars, out of while loop, token = ", token);

    if (token.token_type != SEMICOLON) {
        syntax_error();
    }

}

void LexicalAnalyzer::parse_scope() {

    // ID LBRACE public_vars private_vars stmt_list RBRACE
    Token next;
    Token currentToken;

    currentScope = token.lexeme;
    scopes = addScope({currentScope, "", ""});
    Log("Parse_scope2, scope = " + currentScope);  

    // scope should be ID here first
    if (token.token_type != ID) {
        syntax_error();
    }  
    
    // scope is current token, so lbrace should be next or syntax error    
    token = lexer.GetToken();

    if (token.token_type != LBRACE) {
        syntax_error();
    } 

    // passed LBRACE, so get next
    token = lexer.GetToken();

    if (token.token_type == PUBLIC) {
        parse_public_vars();  // ends in semicolon, so get next

        token = lexer.GetToken();    
    }

    if (token.token_type == PRIVATE) {        
        parse_private_vars(); // ends in semicolon, get next token

        token = lexer.GetToken();
    }

    // after ID, LBRACE, PUBLIC, PRIVATE
    // we must have statement list or SCOPE or error?
    Log("in parse_scope2, after ID, LBRACE, PUBLIC, and PRIVATE", token);
    if (token.token_type != ID) {
        syntax_error();
    }

    parse_stmt_list(); // should end on rbrace, don't get next to make sure
    Log("end of parse_stmt_list in scope " + currentScope, token);

    if (token.token_type != RBRACE) {
        syntax_error();
    }

    Log("parse scope reached the end for " + currentScope + ", token type is RBRACE");

    removeScope(currentScope);
    
}

// starts at the first token in the line on the left hand side and goes until 
// it finds a semi colon
void LexicalAnalyzer::parse_var_list(string permission) {
    
    Log("parse_var_list: ", token);
    while(token.token_type != END_OF_FILE && token.token_type != SEMICOLON) {
        
        // store each variable between comma, until semi-colon
        // also may need to check syntax about commas between vars
        if (token.token_type != ID && token.token_type != COMMA) {
            syntax_error();
        }

        if (token.token_type == ID) {
            scopes = addScope({currentScope, permission, token.lexeme});
            Log("variable - permission = " + permission, token);
        }
        
        token = lexer.GetToken();
    }

    Log("out of while loop parse_var_list", token);

}

void LexicalAnalyzer::parse_public_vars() {
    // empty set or PUBLIC COLON var_list SEMICOLON
    Log("in parse_public_vars, current token = ", token);

    // empty set handled prior to coming into function

    // must be of the order above    
    if (token.token_type != PUBLIC) {
        syntax_error();
    } 

    // next should be colon, then vars
    token = lexer.GetToken();

    if (token.token_type != COLON) {
        syntax_error();
    }

    token = lexer.GetToken();

    // parse_var_list returns with semi-colon
    parse_var_list("public");

    if (token.token_type != SEMICOLON) {
        syntax_error();
    }   
    
}

void LexicalAnalyzer::parse_private_vars() {
    // empty set or PRIVATE COLON var_list SEMICOLON
    Log("in parse_private_vars, current token = ", token);
    //token = lexer.GetToken();

    if (token.token_type != PRIVATE) {
        syntax_error();
    } 

    token = lexer.GetToken();

    // next should be colon, then vars
    if (token.token_type != COLON) {
        syntax_error();
    }

    token = lexer.GetToken();

    // parse_var_list returns with semi-colon
    parse_var_list("private");

    if (token.token_type != SEMICOLON) {
        syntax_error();
    }   
}

void LexicalAnalyzer::parse_stmt_list() {
    // stmt or stmt and stmt_list
    Log("in parse_stmt_list", token);
    // stmt_list goes until RBRACE?
    
    // list of statements, continue until we hit RBRACE
    while(token.token_type != RBRACE) {
        
        // if we hit EOF before RBRACE, error because the statement can't be outside of global scope?
        if (token.token_type == END_OF_FILE) {
            syntax_error();
        }

        // ID EQUAL ID SEMICOLON
        if (token.token_type != ID) {
            syntax_error();
        }

        parse_stmt();

        token = lexer.GetToken();
    }

    Log("out of while loop in parse_stmt_list", token);
    
}

void LexicalAnalyzer::parse_stmt() {
    //ID EQUAL ID SEMICOLON
    // or SCOPE
    Token next;
    string myKey, myValue;
    myAssignment tmp;

    if (token.token_type != ID) {
        syntax_error();
    }

    next = peek_token();

    if (next.token_type == LBRACE) {
        parse_scope();
    } else if (next.token_type == EQUAL) {

        myKey = token.lexeme;
        tmp.key = findScope(currentScope, myKey);

        token = lexer.GetToken();

        if (token.token_type != EQUAL) {
            syntax_error();
        }

        token = lexer.GetToken();

        if (token.token_type != ID) {
            syntax_error();
        }

        myValue = token.lexeme;
        tmp.value = findScope(currentScope, myValue);

        token = lexer.GetToken();

        if (token.token_type != SEMICOLON) {
            syntax_error();
        }
        
        ////Log("my value = " + myValue + " for scope " + currentScope);
        addAssignment(tmp);
        Log("ASSIGNMENT = " + tmp.key + " = " + tmp.value, token);

    } else {
        syntax_error();
    }
    
}



// HELPERS
void LexicalAnalyzer::Log(string message) {
    if (DEBUG) {
        cout << "LOG: " << message << endl;
    }
}

void LexicalAnalyzer::Log(string message, Token myToken) {
    if (DEBUG) {
        cout << "LOG: " << message << endl;
        myToken.Print();
    }
}

char LexicalAnalyzer::peek_char() {
    char c, peek;

    input.GetChar(c);

    peek = c;

    input.UngetChar(c);

    return peek;
}

Token LexicalAnalyzer::peek_token() {
    Token t, peek;

    t = lexer.GetToken();

    peek = t;

    lexer.UngetToken(t);

    return peek;
}

// SKIPS
void LexicalAnalyzer::SkipComment() {
    
    Log("IN Skip comment");

    SkipSpace();
    
    char c;

    // if end of input or not DIV, return
    if(input.EndOfInput()) {
        Log("input is EOF");
        input.UngetChar(c);
        return;
    }

    input.GetChar(c);     

    if (c != '/') {
        Log("input is != DIV, return ");
        input.UngetChar(c);
        return;
    }

    // legal char is //
    if (c == '/'){
        Log("first char is a div, getting next");

        input.GetChar(c);

        if (c != '/') {
            Log("second c is not a div, continue or throw error?");
            input.UngetChar(c);
            // SYNTAX ERROR cause there's only one DIV and that's illegal? 
            return;
        }

        Log("second c is a DIV, skipping all chars until end of line ");

        while (!input.EndOfInput() && c != '\n'){
            input.GetChar(c);
            if (c == '\n') {
                Log("c is end of line");
            } else {
                Log("c is not end of line, continue");
            }
        }
        
        if (c == '\n') {
            Log("c is line break");
        }   

        Log("out of while, reached end of line, running skip_comment again");
        line_no += (c == '\n');
        SkipComment();
        
    }

    //input.UngetChar(c);
}

bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    input.GetChar(c);    
    line_no += (c == '\n');

    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }

    if (!input.EndOfInput()) {
        
        input.UngetChar(c);
    }

    return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}

Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    input.GetChar(c);

    if (isalpha(c)) {
        tmp.lexeme = "";
        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
        }
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;
        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }
    
    SkipSpace();
    SkipComment();

    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);

    switch (c) {
        case '/':
            tmp.token_type = DIV;
            return tmp;
        case '=':
            tmp.token_type = EQUAL;
            return tmp;
        case ':':
            tmp.token_type = COLON;
            return tmp;
        case ',':
            tmp.token_type = COMMA;
            return tmp;
        case ';':
            tmp.token_type = SEMICOLON;
            return tmp;
        case '{':
            tmp.token_type = LBRACE;
            return tmp;
        case '}':
            tmp.token_type = RBRACE;
            return tmp;           
        
        default:
             if (isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrKeyword();
            } else if (input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else {
                tmp.token_type = ERROR;
                if (c == '\n') {
                    Log("c is line break in gettoken");
                } else {
                    Log("GetToken, token_type = ERROR, c = " + std::to_string(c));
                }
                
            }
            return tmp;
    }
}

int main()
{
    lexer.parse_program();
    lexer.print_list();
}
 