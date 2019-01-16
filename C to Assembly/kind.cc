#include "kind.h"
#include <sstream>
using std::string;
using std::istringstream;

// Use the annonymous namespace to prevent external linking
namespace {
  // Template function to convert from a string to some Type
  // Primarily used for string to integer conversions
  template <typename T>
  T fromString(const string& s, bool hex = false){
    istringstream iss(s);
    T n;
    if(hex)
      iss >> std::hex;
    if(iss >> n)
      return n;
    else
      throw string("ERROR: Type not convertible from string.");
  }
  // Strings representing the Kind of Token for printing
  const string kindStrings[] ={
    "ID",          // Opcode or identifier (e.g. label use without colon)
    "NUM",         // Decimal integer
    "LPAREN",      // (
    "RPAREN",      // )
    "WHITESPACE",  // Whitespace
    "ERR",          // Bad/Invalid Token
    "LBRACE", // "{"
    "RBRACE", // "}"
    "RETURN", // "return" 
    "IF", // "if"
    "ELSE", // "else"
    "WHILE", // "while"
    "PRINTLN", // "println"
    "WAIN", // "wain"
    "BECOMES", // "="
    "EQ", // "=="
    "NE", // "!="
    "LT", // "<"
    "GT", // ">"
    "LE", // "<="
    "GE", // ">="
    "PLUS", // "+"
    "MINUS", // "-"
    "STAR", // "*"
    "SLASH", // "/"
    "PCT", // %
    "COMMA", // ","
    "SEMI", // ";"
    "NEW", // "new"
    "DELETE", // "delete"
    "LBRACK", // "["
    "RBRACK", // "]"
    "AMP", // "&"
    "INT", // the string int
    "NULL", // NULL
    "COMMENT"
  };
}

WLP4::Token::Token(WLP4::Kind kind, string lexeme)
: kind(kind), lexeme(lexeme){}

// Cannot convert Tokens that do not represent numbers
// to an Integer
int WLP4::Token::toInt() const{
  throw "ERROR: attempt to convert non-integer token: " + lexeme;
}

string WLP4::Token::toString() const{
  return ::kindStrings[kind];
}

string WLP4::Token::getLexeme() const{
  return lexeme;
}

WLP4::Kind WLP4::Token::getKind() const{
  return kind;
}

WLP4::NumToken::NumToken(WLP4::Kind kind, string lexeme)
: Token(kind,lexeme){}

// Ensure that the NumToken is a valid Integer for conversion
// Arguably, we could do this check in the constructor
int WLP4::NumToken::toInt() const{
  long long l = ::fromString<long long>(lexeme);
  if('-' == lexeme[0]){
    if(l < -2147483648LL)
      throw "ERROR: constant out of range: "  + lexeme;
  } else{
    unsigned long long ul = static_cast<unsigned long long>(l);
    if(ul > 4294967295LL)
      throw "ERROR: constant out of range: " + lexeme;
  }
  return l;
}

// Print the pertinent information to the ostream
std::ostream& WLP4::operator<<(std::ostream& out, const WLP4::Token& t){
  out << t.toString() << " " << t.getLexeme();
  return out;
}

// Make a new Token depending on the Kind provided
// Integral Kinds correspond to the appropriate Token type
WLP4::Token* WLP4::Token::makeToken(WLP4::Kind kind, string lexeme){
  switch(kind){
    case NUM:
      return new NumToken(kind,lexeme);
    default:
      return new Token(kind,lexeme);
  }
}
