#include "kind.h"
#include "lexer.h"
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <map>

// Use only the neeeded aspects of each namespace
using std::string;
using std::stringstream;
using std::vector;
using std::map;
using std::endl;
using std::cerr;
using std::cin;
using std::cout;
using std::getline;
using ASM::Token;
using ASM::Lexer;
using ASM::Kind;

int PC = 0xc;
bool nulladd = false;


void ocommand(unsigned int value){
  putchar(value>>24);
  putchar(value>>16);
  putchar(value>>8);
  putchar(value);
}

int main(int argc, char* argv[]){
  // Nested vector representing lines of Tokens
  // Needs to be used here to cleanup in the case
  // of an exception
  vector< vector<Token*> > tokLines;
  map< string, int> ltable;
  vector <int> reloctable;
  try{
    // Create a MIPS recognizer to tokenize
    // the input lines
    Lexer lexer;
    // Tokenize each line of the input
    string line;
    //Error checking and saving labels.
    bool error = false;

    while(getline(cin,line)){
      vector <Token*> inst = lexer.scan(line);
      vector <string> submitted;
      int tokenLen = inst.size();
      int index = 0;
      if (tokenLen == 0){
        continue;
      }
      for (;index < tokenLen; ++index){
        string op = inst[index]->toString();
        if (op != "LABEL"){
          break;
        }
        string key = inst[index]->getLexeme();
        key.pop_back();
        bool fail = (key[0] >= 48 && key[0] <= 57)||(!(ltable.find(key) == ltable.end()))||(std::find(submitted.begin(), submitted.end(), key) != submitted.end());
        if (fail){
          error = true;
          break;
        }
        submitted.emplace_back(key);
        ltable[key] = PC;
      }

      if (error == true){
        cerr << "ERROR" << endl;
        vector<vector<Token*> >::iterator it;
          for(it = tokLines.begin(); it != tokLines.end(); ++it){
            vector<Token*>::iterator it2;
            for(it2 = it->begin(); it2 != it->end(); ++it2){
              delete *it2;
          }
        }
        return -1;
      }

      int currlen = tokenLen - index;
      //all the forwarded labels are removed.
      if (index == tokenLen){  // line only had labels!
        nulladd = false;
        error = false;
      } else if (currlen == 2){
        string k1 = inst[index]->toString();
        string k1s = inst[index]->getLexeme();
        string k2 = inst[index+1]->toString();
        string k2s = inst[index+1]->getLexeme();
        if (k1 == "DOTWORD" && (k2 == "HEXINT" || k2 == "INT" || k2 == "ID")){
          nulladd = true;
          if (k2 == "ID"){
            reloctable.emplace_back(PC);
          }
        } else if (k1 == "ID" && k2 == "REGISTER"){
          string regStr = k2s;
          regStr = regStr.substr(1).append(regStr.substr(0,1));
          int regNum = stoi(regStr); 
          if ((k1s == "jr" || k1s == "jalr" || k1s == "mflo"|| k1s == "mfhi"|| k1s == "lis")&&(regNum >= 0 && regNum <= 31)){
            nulladd = true;
            error = false;
          } else {
            error = true;
          }
        } else {
          error = true;
        }
      } else if (currlen == 6) {
        string k1 = inst[index]->toString();
        string k1s = inst[index]->getLexeme();
        string k2 = inst[index+1]->toString();
        string k2s = inst[index+1]->getLexeme();
        string k3 = inst[index+3]->toString();
        string k3s = inst[index+3]->getLexeme();
        string k4 = inst[index+5]->toString();
        string k4s = inst[index+5]->getLexeme();
        string c1 = inst[index+2]->toString();
        string c2 = inst[index+4]->toString();
        if (k1 == "ID" && k2 == "REGISTER" && k3 == "REGISTER" && k4 == "REGISTER"  && c1 == "COMMA" && c2 == "COMMA"){
          string regStr1 = k2s; //$d
          regStr1 = regStr1.substr(1).append(regStr1.substr(0,1));
          int regNum1 = stoi(regStr1);
          string regStr2 = k3s; //$s
          regStr2 = regStr2.substr(1).append(regStr2.substr(0,1));
          int regNum2 = stoi(regStr2);
          string regStr3 = k4s; //$t
          regStr3 = regStr3.substr(1).append(regStr3.substr(0,1));
          int regNum3 = stoi(regStr3);
          if((k1s == "add" || k1s == "sub" || k1s == "slt" || k1s == "sltu") && (regNum1 <= 31  && regNum1 >= 0  && regNum2 <= 31  && regNum2 >= 0 && regNum3 <= 31  && regNum3 >= 0)){
            error = false;
            nulladd = true;
          } else {
            error = true;
          }
        } else if (k1 == "ID" && k2 == "REGISTER" && k3 == "REGISTER" && (k4 == "INT"||k4 == "HEXINT"||k4 =="ID")  && c1 == "COMMA" && c2 == "COMMA"){ 
          string regStr1 = k2s; //$d
          regStr1 = regStr1.substr(1).append(regStr1.substr(0,1));
          int regNum1 = stoi(regStr1);
          string regStr2 = k3s; //$s
          regStr2 = regStr2.substr(1).append(regStr2.substr(0,1));
          int regNum2 = stoi(regStr2);
          if ((k1s == "beq" || k1s == "bne")&&(regNum1 <= 31  && regNum1 >= 0  && regNum2 <= 31  && regNum2 >= 0)){
            int test;
            if(k4== "INT"||k4=="HEXTINT"){
              test = inst[index+5]->toInt();
            } 
            if (k4 == "INT" && (test < -32768 || test > 32767)){
              error = true;
            } else if (k4 == "HEXINT"){
              unsigned int test = inst[index+5]->toInt();
              if (test > 0xffff || test < 0){
                error = true;
              } else {
                nulladd = true;
                error = false;
              }
            } else {
              nulladd = true;
              error = false;
            }
          } else {
            error = true;
          }
        }
      } else if (currlen == 4){
        string k1 = inst[index]->toString();
        string k1s = inst[index]->getLexeme();
        string k2 = inst[index+1]->toString();
        string k2s = inst[index+1]->getLexeme();
        string k3 = inst[index+3]->toString();
        string k3s = inst[index+3]->getLexeme();
        string c1 = inst[index+2]->toString();
        if (k1 == "ID" && k2 == "REGISTER" && k3 == "REGISTER"  && c1 == "COMMA"){
          string regStr1 = k2s; //$s
          regStr1 = regStr1.substr(1).append(regStr1.substr(0,1));
          int regNum1 = stoi(regStr1);
          string regStr2 = k3s; //$t
          regStr2 = regStr2.substr(1).append(regStr2.substr(0,1));
          int regNum2 = stoi(regStr2);
          if ((k1s == "multu" ||k1s == "mult" ||k1s == "divu" ||k1s == "div")&&(regNum1 >= 0 && regNum1 <= 31 && regNum2 >= 0 && regNum2 <= 31)){
            error = false;
            nulladd = true;
          } else {
            error = true;
          }
        } else {
          error = true;
        }
      } else if (currlen == 7) {
        string k1 = inst[index]->toString();
        string k1s = inst[index]->getLexeme();
        string k2 = inst[index+1]->toString();
        string k2s = inst[index+1]->getLexeme();
        string k3 = inst[index+2]->toString();
        string k3s = inst[index+2]->getLexeme();
        string k4 = inst[index+3]->toString();
        string k4s = inst[index+3]->getLexeme();
        string k5 = inst[index+4]->toString();
        string k5s = inst[index+4]->getLexeme();
        string k6 = inst[index+5]->toString();
        string k6s = inst[index+5]->getLexeme();
        string k7 = inst[index+6]->toString();
        string k7s = inst[index+6]->getLexeme();
        if (k1 == "ID" && (k1s == "lw" || k1s =="sw") && k2 == "REGISTER" && k3 == "COMMA"  && (k4 == "INT"||k4 == "HEXINT") && k5 == "LPAREN" && k6 == "REGISTER" && k7 == "RPAREN"){
          if (k4 == "INT"){
            int offset = inst[index+3]->toInt();
            if (offset < -32768 || offset > 32767){
              error = true;
            } else {
              nulladd = true;
            }
          } else if (k4 == "HEXINT"){
            unsigned int offset = inst[index+3]->toInt();
            if (offset > 0xffff || offset  < 0){
              error = true;
            } else {
              nulladd = true;
            }
          }
        }
      } else {
        error = true;
      }

      if (error == true){
        cerr << "ERROR" << endl;
        break;
      } else {
        tokLines.push_back(inst);
        if (nulladd == true){
          PC += 4;
        }
      }
    }
    
    //for(map<string, int>::const_iterator it = ltable.begin();it != ltable.end(); ++it){
    // cerr << it->first << " " << it->second << endl;
    //}

    if (error == true){
      vector<vector<Token*> >::iterator it;
      for(it = tokLines.begin(); it != tokLines.end(); ++it){
        vector<Token*>::iterator it2;
        for(it2 = it->begin(); it2 != it->end(); ++it2){
          delete *it2;
        }
      }
      return -1;
    }

    int cookie = 0x10000002;
    ocommand(cookie);
    int relocnumber = reloctable.size();
    int endcode = PC;
    int endfile = PC + relocnumber*2*4;
    ocommand(endfile);
    ocommand(endcode);

    PC = 0xc;

    vector<vector<Token*> >::iterator it;
    for(it = tokLines.begin(); it != tokLines.end(); ++it){
      bool error = false;
      nulladd = false;
      //cerr << PC << endl; // PC printer
      vector <Token*> inst = *it;
      int tokenLen = inst.size();
      //label parsing
      int index = 0;
      if (tokenLen == 0){
        continue;
      }
      for (;index < tokenLen; ++index){
        string op = inst[index]->toString();
        if (op != "LABEL"){
          break;
        }
      }
      int currlen = tokenLen - index;
      //all the forwarded labels are removed.
      if (index == tokenLen){  // line only had labels!
        error = false;
      } else if (currlen == 2){
        string k1 = inst[index]->toString();
        string k1s = inst[index]->getLexeme();
        string k2 = inst[index+1]->toString();
        string k2s = inst[index+1]->getLexeme();
        if (k1 == "DOTWORD" && (k2 == "HEXINT" || k2 == "INT" || k2 == "ID")){
          unsigned int value;
          if (k2 == "ID"){
            string label = inst[index+1]->getLexeme();
            if (ltable.find(label) == ltable.end()){
              error = true;
            }else{
            value = ltable[label];
            }
          } else {
            value = inst[index+1]->toInt();
          }
          if (error == false){
            ocommand(value);
            nulladd = true;
          }
        } else if (k1 == "ID" && k2 == "REGISTER"){ // JR
          nulladd = true;
          string regStr = k2s;
          regStr = regStr.substr(1).append(regStr.substr(0,1));
          int regNum = stoi(regStr);
          if (k1s == "jr"){
            unsigned int value = (0 << 26)| (regNum << 21) | (0 << 4) | (0x0008);
            ocommand(value);
          } else if (k1s == "jalr"){
            unsigned int value = (0 << 26)| (regNum << 21) | (0 << 4) | (0x0009);
            ocommand(value);
          } else if (k1s == "mflo"){
            unsigned int value = (0 << 16)|(regNum << 11)|(0x12);
            ocommand(value);
          } else if (k1s == "mfhi"){
            unsigned int value = (0 << 16)|(regNum << 11)|(0x10);
            ocommand(value);
          } else if (k1s == "lis"){
            unsigned int value = (0 << 16)|(regNum << 11)|(0x14);
            ocommand(value);
          } else {
            nulladd = false;
            error = true;
          }
        } else {
          error = true;
        }
      } else if (currlen == 6) {
        string k1 = inst[index]->toString();
        string k1s = inst[index]->getLexeme();
        string k2 = inst[index+1]->toString();
        string k2s = inst[index+1]->getLexeme();
        string k3 = inst[index+3]->toString();
        string k3s = inst[index+3]->getLexeme();
        string k4 = inst[index+5]->toString();
        string k4s = inst[index+5]->getLexeme();
        if (k1 == "ID" && k2 == "REGISTER" && k3 == "REGISTER" && k4 == "REGISTER" ){
          unsigned int value;
          string regStr1 = k2s;
          regStr1 = regStr1.substr(1).append(regStr1.substr(0,1));
          unsigned int regNum1 = stoi(regStr1);
          string regStr2 = k3s;
          regStr2 = regStr2.substr(1).append(regStr2.substr(0,1));
          unsigned int regNum2 = stoi(regStr2);
          string regStr3 = k4s;
          regStr3 = regStr3.substr(1).append(regStr3.substr(0,1));
          unsigned int regNum3 = stoi(regStr3);
          if(k1s == "add"){
            value = (0<<26)|(regNum2<<21)|(regNum3<<16)|(regNum1<<11)|(0x0020);
          } else if (k1s == "sub"){
            value = (0<<26)|(regNum2<<21)|(regNum3<<16)|(regNum1<<11)|(0x0022);
          } else if (k1s == "slt"){
            value = (0<<26)|(regNum2<<21)|(regNum3<<16)|(regNum1<<11)|(0x002a);
          } else if (k1s == "sltu"){
            value = (0<<26)|(regNum2<<21)|(regNum3<<16)|(regNum1<<11)|(0x002b);
          }
          nulladd = true;
          ocommand(value);
        } else if (k1 == "ID" && k2 == "REGISTER" && k3 == "REGISTER" && (k4 == "INT"||k4 == "HEXINT"||k4 == "ID")){ 
          unsigned int value;
          int offset;
          string regStr1 = k2s; //$d
          regStr1 = regStr1.substr(1).append(regStr1.substr(0,1));
          int regNum1 = stoi(regStr1);
          string regStr2 = k3s; //$s
          regStr2 = regStr2.substr(1).append(regStr2.substr(0,1));
          int regNum2 = stoi(regStr2);
          if (k4 == "ID"){
            string label = k4s;
            if(ltable.find(label) == ltable.end()){
              error = true;
            }else {
              int val = ltable[label];
              offset = (val-PC-4)/4;
              if (offset < -32768 || offset > 32767){
                error = true;
              }
            }
          }else{
            offset = inst[index+5]->toInt();
          }
          if (error==true){
            cerr << "ERROR" << endl;
            break;
          }
          if (k1s == "beq"){
            value = (4<<26)|(regNum1<<21)|(regNum2<<16)|(offset & 0xffff);
          } else if (k1s == "bne"){
            value = (5<<26)|(regNum1<<21)|(regNum2<<16)|(offset & 0xffff);
          }
          nulladd = true;
          ocommand(value);
        } else {
          error = true;
        }
      } else if (currlen == 4){
        unsigned int value;
        string k1 = inst[index]->toString();
        string k1s = inst[index]->getLexeme();
        string k2 = inst[index+1]->toString();
        string k2s = inst[index+1]->getLexeme();
        string k3 = inst[index+3]->toString();
        string k3s = inst[index+3]->getLexeme();
        string c1 = inst[index+2]->toString();
        if (k1 == "ID" && k2 == "REGISTER" && k3 == "REGISTER" && c1 == "COMMA"){
          nulladd = true;
          string regStr1 = k2s; //$s
          regStr1 = regStr1.substr(1).append(regStr1.substr(0,1));
          int regNum1 = stoi(regStr1);
          string regStr2 = k3s; //$t
          regStr2 = regStr2.substr(1).append(regStr2.substr(0,1));
          int regNum2 = stoi(regStr2);
          if (k1s == "mult"){
            value = (0<<26)|(regNum1<<21)|(regNum2<<16)|(1<<4)|(0x8);
            ocommand(value);
          } else if (k1s == "multu"){
            value = (0<<26)|(regNum1<<21)|(regNum2<<16)|(1<<4)|(0x9);
            ocommand(value);
          } else if (k1s == "div"){
            value = (0<<26)|(regNum1<<21)|(regNum2<<16)|(1<<4)|(0xa);
            ocommand(value);
          } else if (k1s == "divu"){
            value = (0<<26)|(regNum1<<21)|(regNum2<<16)|(1<<4)|(0xb);
            ocommand(value);
          } else {
            error = true;
            nulladd = false;
          }
        } else {
          error = true;
        }
      } else if (currlen == 7) {
        string k1 = inst[index]->toString();
        string k1s = inst[index]->getLexeme();
        string k2 = inst[index+1]->toString();
        string k2s = inst[index+1]->getLexeme();
        string k3 = inst[index+2]->toString();
        string k3s = inst[index+2]->getLexeme();
        string k4 = inst[index+3]->toString();
        string k4s = inst[index+3]->getLexeme();
        string k5 = inst[index+4]->toString();
        string k5s = inst[index+4]->getLexeme();
        string k6 = inst[index+5]->toString();
        string k6s = inst[index+5]->getLexeme();
        string k7 = inst[index+6]->toString();
        string k7s = inst[index+6]->getLexeme();
        if (k1 == "ID" && k2 == "REGISTER" && k3 == "COMMA"  && (k4 == "INT"||k4 == "HEXINT") && k5 == "LPAREN" && k6 == "REGISTER" && k7 == "RPAREN"){
          int offset = inst[index+3]->toInt();
          unsigned int value;
          string regStr1 = k2s; //$t
          regStr1 = regStr1.substr(1).append(regStr1.substr(0,1));
          int regNum1 = stoi(regStr1);
          string regStr2 = k6s; //$s
          regStr2 = regStr2.substr(1).append(regStr2.substr(0,1));
          int regNum2 = stoi(regStr2);
          bool validReg = (regNum1 <= 31 && regNum1 >= 0 && regNum2 <= 31 && regNum2 >= 0);
          if (k1s == "lw" && validReg == true){
            value = (35<<26)|(regNum2<<21)|(regNum1<<16)|(offset & 0xffff);
            ocommand(value);
          } else if (k1s == "sw" && validReg == true){
            value = (43<<26)|(regNum2<<21)|(regNum1<<16)|(offset & 0xffff);
            ocommand(value);
          } else {
            error = true;
          }
        } else {
          error = true;
        }
      } else {
        error = true;
      }
      if (error == true){
        cerr << "ERROR" << endl;
        break;
      } else {
        if (nulladd == true){
          PC += 4;
        }
      }
    }

    for (int i = 0; i < relocnumber; ++i){
      int formatcode = 0x00000001;
      ocommand(formatcode);
      int address = reloctable[i];
      ocommand(address);
    }

    reloctable.clear();

    // Iterate over the lines of tokens and print them
    // to standard error
    //vector<vector<Token*> >::iterator it;
    //for(it = tokLines.begin(); it != tokLines.end(); ++it){
      //vector<Token*>::iterator it2;
      //for(it2 = it->begin(); it2 != it->end(); ++it2){
        //cerr << "  Token: "  << *(*it2) << endl;
      //}
    //}

  } catch(const string& msg){
    // If an exception occurs print the message and end the program
    cerr << msg << endl;
  }

  // Delete the Tokens that have been made
  vector<vector<Token*> >::iterator it;
  for(it = tokLines.begin(); it != tokLines.end(); ++it){
    vector<Token*>::iterator it2;
    for(it2 = it->begin(); it2 != it->end(); ++it2){
      delete *it2;
    }
  }
}
