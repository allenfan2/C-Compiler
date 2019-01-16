#ifndef __LEXER_H__
#define __LEXER_H__
#include <string>
#include <vector>

namespace WLP4{
  // The different states the the MIPS recognizer uses
  // Judicious use of the pimpl idiom or the Bridge pattern
  // would allow us to place this in the implementation file
  // However, that's more complexity than is necessary
  enum State {
    ST_ERR,
    ST_START,
    ST_NUM,
    ST_ID,
    ST_LPAREN,
    ST_RPAREN,
    ST_ZERO,
    ST_COMMENT,
    ST_WHITESPACE,
    ST_LBRACE, // "{"
    ST_RBRACE, // "}"
    ST_RETURN, // "return" 
    ST_IF, // "if"
    ST_ELSE, // "else"
    ST_WHILE, // "while"
    ST_PRINTLN, // "println"
    ST_WAIN, // "wain"
    ST_BECOMES, // "="
    ST_EXCLAM, // "!"
    ST_EQ, // "=="
    ST_NE, // "!="
    ST_LT, // "<"
    ST_GT, // ">"
    ST_LE, // "<="
    ST_GE, // ">="
    ST_PLUS, // "+"
    ST_MINUS, // "-"
    ST_STAR, // "*"
    ST_SLASH, // "/"
    ST_PCT, // %
    ST_COMMA, // ","
    ST_SEMI, // ";"
    ST_NEW, // "new"
    ST_DELETE, // "delete"
    ST_LBRACK, // "["
    ST_RBRACK, // "]"
    ST_AMP, // "&"
    ST_INT, // the string int
    ST_NULLCHAR, // NULL
    ST_TOOMANYEQUAL,
    ST_ZEROERROR,
    ST_NUMERROR,
    ST_COMPERR
  };
  // Forward declare the Token class to reduce compilation dependencies
  class Token;

  // Class representing a MIPS recognizer
  class Lexer {
    // At most 21 states and 256 transitions (max number of characters in ASCII)
    static const int maxStates = 50;
    static const int maxTrans = 256;
    // Transition function
    State delta[maxStates][maxTrans];
    // Private method to set the transitions based upon characters in the
    // given string
    void setTrans(State from, const std::string& chars, State to);
  public:
    Lexer();
    // Output a vector of Tokens representing the Tokens present in the
    // given line
    std::vector<Token*> scan(const std::string& line);
  };
}

#endif