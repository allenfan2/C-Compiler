#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>

using namespace std;

 struct Node {
    string rule;
    string termval;   // for terminals TERMVAL is the value of the terminal token  |  for non terminals its the beginning of the rule.
    string type;
    vector <string> tokens;
    vector <Node *> children;
    int size = 0;
    Node(string rule): rule{rule}{
        istringstream iss(rule);
        string token;
        iss >> token;
        if (token[0] <= 'z' && token[0] >= 'a'){
            termval = token;
            while(iss >> token){
                tokens.emplace_back(token);
                ++size;
            }
        } else {
            iss >> token;
            termval = token;
        }
        
    }
};


Node *createPTree(){
    string rule;
    getline(cin,rule);
    Node *t = new Node(rule);
    for (int i = 0; i< t->size; ++i){
        Node *t2 = createPTree();
        t->children.emplace_back(t2);
    }
    return t;
}

map<string,string> stable;

map<string,int> atable;
int offset = 0;

bool errorstatus = false;


void scanPTree(Node *tree){
    int len = tree->size;
    if (tree->termval == "dcl"){
        Node * type = tree->children[0];
        Node * ID = tree->children[1];
        int typesize = type->size;
        string idname = ID->termval;
        if (stable.find(idname) == stable.end()){
            if (typesize == 2){
                stable[idname] = "int*";
                tree->type = "int*";
            } else {
                stable[idname] = "int";
                tree->type = "int";
             }
        } else {
            errorstatus = true;
            return;
        }
    } else if (tree->rule == "lvalue ID") {
        Node * ID = tree->children[0];
        string idname = ID->termval;
        if (stable.find(ID->termval) == stable.end()){
            errorstatus = true;
            return;
        }
        ID->type = stable[ID->termval];
    } else if (tree->rule == "factor ID") {
        Node * ID = tree->children[0];
        string idname = ID->termval;
        if (stable.find(ID->termval) == stable.end()){
            errorstatus = true;
            return;
        }
        ID->type = stable[ID->termval];
    } else {
        for (int i = 0 ; i < len ; ++i){
            scanPTree(tree->children[i]);
        }
    }
}

void typePropPTree(Node* tree){
    int len = tree->size;
    for (int i = 0 ; i < len ; ++i){
            typePropPTree(tree->children[i]);
    }
    string rule = tree->rule;
    string termval = tree->termval;
    //cerr << tree->rule << endl;
    if (rule == "factor ID" || rule == "term factor" || rule == "expr term" || rule == "lvalue ID"){
        //cerr << "reached 1" << endl;
        tree->type = (tree->children[0])->type;
    } else if (rule == "factor NUM" || rule == "factor ID LPAREN arglist RPAREN" || rule =="factor ID LPAREN RPAREN"){
        //cerr << "reached 2" << endl;
        tree->type = "int";
    } else if (rule == "factor NULL"){
        //cerr << "reached 3" << endl;
        tree->type = "int*";
    } else if (rule == "factor LPAREN expr RPAREN" || rule == "lvalue LPAREN lvalue RPAREN"){
        //cerr << "reached 4" << endl;
        tree->type = (tree->children[1])->type;
    } else if (rule == "term term STAR factor" || rule == "term term SLASH factor" || rule == "term term PCT factor"){
        //cerr << "reached 5" << endl;
        string type1 = (tree->children[0])->type;
        string type2 = (tree->children[2])->type;
        if (type1 == "int" && type2 == "int"){
            tree->type = "int";
        }else{
            errorstatus = true;
            cerr << "14" << endl;
            return;
        }
    } else if (rule == "expr expr PLUS term" || rule == "expr expr MINUS term"){
        //cerr << "reached 6" << endl;
        string type1 = (tree->children[0])->type;
        string type2 = (tree->children[2])->type;
        if (type1 == "int" && type2 == "int"){
            tree->type = "int";
        } else if (type1 == "int*" && type2 == "int"){
            tree->type = "int*";
        } else if (rule == "expr expr PLUS term" && type1 == "int" && type2 == "int*"){
            tree->type = "int*";
        } else if (rule == "expr expr MINUS term" && type1 == "int*" && type2 == "int*"){
            tree->type = "int";
        } else {
            errorstatus = true;
            //cerr << "14" << endl;
            return;
        }
    } else if (rule == "factor AMP lvalue"){
        //cerr << "reached 7" << endl;
        string type = (tree->children[1])->type;
        if (type == "int"){
            tree->type = "int*";
        } else {
            errorstatus = true;
            //cerr << "13" << endl;
            return;
        }
    } else if (rule == "factor STAR factor" || rule == "lvalue STAR factor"){
        //cerr << "reached 8" << endl;
        string type = (tree->children[1])->type;
        if (type == "int*"){
            tree->type = "int";
        } else {
            errorstatus = true;
            //cerr << "12" << endl;
            return;
        }
    } else if (rule == "factor NEW INT LBRACK expr RBRACK"){
        //cerr << "reached 9" << endl;
        string type = (tree->children[3])->type;
        if (type == "int"){
            tree->type = "int*";
        } else {
            errorstatus = true;
            //cerr << "11" << endl;
            return;
        }
    } else if (rule == "test expr EQ expr" || rule == "test expr NE expr" || rule == "test expr LT expr" || rule == "test expr LE expr" || rule == "test expr GE expr"|| rule == "test expr GT expr"){
        string type1 = (tree->children[0])->type;
        string type2 = (tree->children[2])->type;
        if (type1 == type2){
            tree->type = "good";
        } else {
            tree->type = "bad";
            errorstatus = true;
            //cerr << "10" << endl;
            return;
        }
    } else if (rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE"){
        string type1 = (tree->children[2])->type;
        string type2 = (tree->children[5])->type;
        string type3 = (tree->children[9])->type;
        if (type1 == "good" && type2 == "good" && type3 == "good"){
            tree->type = "good";
        } else {
            tree->type = "bad";
            errorstatus = true;
            //cerr << "9" << endl;
            return;
        }
    } else if (rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE"){
        string type1 = (tree->children[2])->type;
        string type2 = (tree->children[5])->type;
        if (type1 == "good" && type2 == "good"){
            tree->type = "good";
        } else {
            tree->type = "bad";
            errorstatus = true;
            //cerr << "8" << endl;
            return;
        }
    } else if (rule == "statement DELETE LBRACK RBRACK expr SEMI"){
        string type = (tree->children[3])->type;
        if (type == "int*"){
            tree->type = "good";
        } else {
            tree->type = "bad";
            errorstatus = true;
            //cerr << "7" << endl;
            return;
        }
    } else if (rule == "statement PRINTLN LPAREN expr RPAREN SEMI"){
        string type = (tree->children[2])->type;
        if (type == "int"){
            tree->type = "good";
        } else {
            tree->type = "bad";
            errorstatus = true;
            //cerr << "6" << endl;
            return;
        }
    } else if (rule == "statements"){
        tree->type = "good";
    } else if (rule == "statements statements statement"){
        string type1 = (tree->children[0])->type;
        string type2 = (tree->children[1])->type;
        if (type1 == "good"  && type2 == "good"){
            tree->type = "good";
        } else {
            tree->type = "bad";
            errorstatus = true;
            //cerr << "5" << endl;
            return;
        }
    } else if ( rule == "statement lvalue BECOMES expr SEMI"){
        string type1 = (tree->children[0])->type;
        string type2 = (tree->children[2])->type;
        if (type1 == type2){
            tree->type = "good";
        } else {
            tree->type = "bad";
            errorstatus = true;
            //cerr << "4" << endl;
            return;
        }
    } else if (rule == "dcls"){
        tree->type = "good";
    } else if (rule == "dcls dcls dcl BECOMES NUM SEMI"){
        string type1 = (tree->children[0])->type;
        string type2 = (tree->children[1])->type;
        if (type1 == "good"&& type2 == "int"){
            tree->type = "good";
        } else {
            tree->type = "bad";
            errorstatus = true;
            //cerr << "3" << type1 << "_" << type2  << endl;
            return;
        }
    } else if (rule == "dcls dcls dcl BECOMES NULL SEMI"){
        string type1 = (tree->children[0])->type;
        string type2 = (tree->children[1])->type;
        if (type1 == "good" && type2 == "int*"){
            tree->type = "good";
        } else {
            tree->type = "bad";
            errorstatus = true;
            //cerr << "2" << type1 << "_" << type2 << endl;
            return;
        }
    } else if (rule == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE"){
        string dcl2 = (tree->children[5])->type;
        string dclstype = (tree->children[8])->type;
        string stype = (tree->children[9])->type;
        string exprtype = (tree->children[11])->type;
        if (dcl2 == "int" && dclstype == "good" && stype == "good" && exprtype == "int"){
            tree->type = "good";
        } else {
            tree->type = "bad";
            errorstatus = true;
            //cerr << "1" << endl;
            return;
        }
    }
}

void prologue(){
    cout << "lis $4" << endl;
    cout << ".word 4" << endl;
    cout << "lis $11" << endl;
    cout << ".word 1" << endl;
    cout << "sw $31, -4($30)" << endl;
    cout << "sub $30,$30,$4" << endl;
    cout << ".import print" << endl;
    cout << ".import init" << endl;
    cout << ".import new" << endl;
    cout << ".import delete" << endl;
    cout << "sub $29,$30,$4" << endl;
    cout << "sw $1, -4($30)" << endl;
    cout << "sub $30,$30,$4" << endl;
    cout << "sw $2, -4($30)" << endl;
    cout << "sub $30,$30,$4" << endl;
}

void epilog(){
    cout << "add $30,$29,$4" << endl;
    cout << "lw $31, 0($30)" << endl;
    cout << "add $30,$30,$4" << endl;
    cout << "jr $31" << endl;
}

void push3(){
    cout << "sw $3, -4($30)" << endl;
    cout << "sub $30,$30,$4" << endl;
}

void pop5(){
	cout << "lw $5, 0($30)" << endl;
    cout << "add $30,$30,$4" << endl;
}

string lvalName(Node *tree){
    string rule = tree->rule;
    if (rule == "lvalue ID"){
    	string idname = tree->children[0]->termval;
    	return idname;
	} else if (rule == "lvalue LPAREN lvalue RPAREN"){
    	lvalName(tree->children[1]);
    }
}






int labelCounter = 1;
int ifLabelCounter = 1;


void codeGenerate(Node * tree){
    string rule = tree->rule;
	//cerr << rule << endl;
	string rulehead = rule.substr(0, rule.find(' '));
    if (rule == "start BOF procedures EOF"){
            codeGenerate(tree->children[1]);   // procedures one 
    } else if (rule == "procedures main") {
            codeGenerate(tree->children[0]);
    } else if (rule == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
            codeGenerate(tree->children[3]);
            codeGenerate(tree->children[5]);
            codeGenerate(tree->children[8]);
            codeGenerate(tree->children[9]);
            codeGenerate(tree->children[11]);
    } else if (rule == "dcl type ID"){
        string idname = (tree->children[1])->termval;
        if (offset == 0){
        	if ((tree->children[0])->size ==1){
        			cout << "add $2, $0, $0" << endl;
        	}
    		cout << "lis $10" << endl;
    		cout << ".word init" << endl;
    		cout << "jalr $10" << endl;

        }
        atable[idname] = offset;
        offset -= 4;
    } else if (rule == "factor ID"){
        string idname = (tree->children[0])->termval;
        int loc = atable[idname];
        cout << "lw $3, " << loc << "($29)" << endl;
    } else if (rule == "factor NUM"){
    	string num = (tree->children[0])->termval;
    	cout << "lis $5" << endl;
    	cout << ".word " << num << endl;
    	cout << "add $3, $5, $0" << endl;
    } else if (rule == "factor LPAREN expr RPAREN"){
    	codeGenerate(tree->children[1]);
    } else if (rule == "expr term" || rule == "term factor"){
    	codeGenerate(tree->children[0]);
    } else if (rule == "expr expr PLUS term"){
    	string type1 = tree->children[0]->type;
    	string type2 = tree->children[2]->type;
    	if (type1 == "int" && type2 == "int"){
    		codeGenerate(tree->children[0]);
    		push3();
    		codeGenerate(tree->children[2]);
    		pop5();
    		cout << "add $3, $5, $3" << endl;
    	} else if (type1 == "int*" && type2 == "int"){
    		codeGenerate(tree->children[0]);
    		push3();
    		codeGenerate(tree->children[2]);
    		cout << "mult $3, $4" << endl;
    		cout << "mflo $3" << endl;
    		pop5();
    		cout << "add $3, $5, $3" << endl;
    	} else if (type1 == "int" && type2 == "int*"){
    		codeGenerate(tree->children[2]);
    		push3();
    		codeGenerate(tree->children[0]);
    		cout << "mult $3, $4" << endl;
    		cout << "mflo $3" << endl;
    		pop5();
    		cout << "add $3, $5, $3" << endl;
    	}
    } else if (rule == "expr expr MINUS term"){
    	string type1 = tree->children[0]->type;
    	string type2 = tree->children[2]->type;
    	if (type1 == "int" && type2 == "int"){
    		string type1 = tree->children[0]->type;
    		string type2 = tree->children[2]->type;
    		codeGenerate(tree->children[0]);
    		push3();
    		codeGenerate(tree->children[2]);
    		pop5();
    		cout << "sub $3, $5, $3" << endl;
    	} else if (type1 == "int*" && type2 == "int"){
    		codeGenerate(tree->children[0]);
    		push3();
    		codeGenerate(tree->children[2]);
    		cout << "mult $3, $4" << endl;
    		cout << "mflo $3" << endl;
    		pop5();
    		cout << "sub $3, $5, $3" << endl; 
    	} else if (type1 == "int*" && type2 == "int*"){
    		codeGenerate(tree->children[0]);
    		push3();
    		codeGenerate(tree->children[2]);
    		pop5();
    		cout << "sub $3, $5, $3" << endl;
    		cout << "div $3, $4" << endl;  
    		cout << "mflo $3" << endl;
    	}
    } else if (rule == "term term STAR factor"){
    	codeGenerate(tree->children[0]);
    	push3();
    	codeGenerate(tree->children[2]);
    	pop5();
    	cout << "mult $5, $3" << endl;
    	cout << "mflo $3" << endl;
    } else if (rule == "term term SLASH factor"){
    	codeGenerate(tree->children[0]);
    	push3();
    	codeGenerate(tree->children[2]);
    	pop5();
    	cout << "div $5, $3" << endl;
    	cout << "mflo $3" << endl;
    } else if (rule == "term term PCT factor"){
    	codeGenerate(tree->children[0]);
    	push3();
    	codeGenerate(tree->children[2]);
    	pop5();
    	cout << "div $5, $3" << endl;
    	cout << "mfhi $3"<< endl;
    } else if (rule == "statements statements statement"){
    	codeGenerate(tree->children[0]);
    	codeGenerate(tree->children[1]);
    } else if (rule == "statement PRINTLN LPAREN expr RPAREN SEMI" ){
    	codeGenerate(tree->children[2]);
    	cout << "add $1, $3, $0" << endl;
    	cout << "lis $10" << endl;
    	cout << ".word print" << endl;
    	cout << "jalr $10" << endl;
    } else if (rule == "dcls dcls dcl BECOMES NUM SEMI"){
    	codeGenerate(tree->children[0]);
    	codeGenerate(tree->children[1]);
    	codeGenerate(tree->children[3]);
    	string idname = ((tree->children[1])->children[1])->termval;
    	int loc = atable[idname];
        cout << "sw $3, " << loc << "($29)" << endl;
        cout << "sub $30, $30, $4" << endl;
    } else if (rule == "dcls dcls dcl BECOMES NULL SEMI"){
    	codeGenerate(tree->children[0]);
    	codeGenerate(tree->children[1]);
    	string idname = ((tree->children[1])->children[1])->termval;
    	int loc = atable[idname];
        cout << "sw $11, " << loc << "($29)" << endl;
        cout << "sub $30, $30, $4" << endl;
    } else if (rule == "statement lvalue BECOMES expr SEMI"){ // OVER HERE
    	string type1 = tree->children[0]->type;
    	string type2 = tree->children[2]->type;
    	//cerr << type1 << " " << type2 << endl;
    	if (type1 == "int" && type2 == "int"){
    		string rule = tree->children[0]->rule;
    		if (rule == "lvalue STAR factor"){
    			codeGenerate(tree->children[2]);
    			push3();
    			codeGenerate(tree->children[0]);
    			pop5();
    			cout << "sw $5, 0($3)" << endl;
    		} else {
    			codeGenerate(tree->children[2]);
    			string idname = lvalName(tree->children[0]);
    			int loc = atable[idname];
        		cout << "sw $3, " << loc << "($29)" << endl;
        	}
        } else if (type1 == "int*" && type2 == "int*"){
    		codeGenerate(tree->children[2]);
    		push3();
    		codeGenerate(tree->children[0]);
    		pop5();
    		cout << "sw $5, 0($3)" << endl;
        }
    } else if (rulehead == "NUM"){
    	string num = tree->termval;
    	cout << "lis $5" << endl;
    	cout << ".word " << num << endl;
    	cout << "add $3, $5, $0" << endl;
    } else if (rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE"){
    	int i = labelCounter;
    	++labelCounter;
    	cout << "startwl" << i << ":"<< endl;
    	codeGenerate(tree->children[2]);
    	cout << "beq $3, $0, endwl" << i << endl;
    	codeGenerate(tree->children[5]);
    	cout << "beq $0, $0, startwl" << i << endl;
    	cout << "endwl" << i << ":"<< endl;
    } else if (rule == "test expr LT expr"){
    	string type = tree->children[0]->type;
    	if (type == "int"){
    		codeGenerate(tree->children[0]);
    		push3();
    		codeGenerate(tree->children[2]);
    		pop5();
    		cout << "slt $3, $5, $3" << endl;
    	} else if (type == "int*"){
    		codeGenerate(tree->children[0]);
    		push3();
    		codeGenerate(tree->children[2]);
    		pop5();
    		cout << "sltu $3, $5, $3" << endl;
    	}
    } else if (rule == "test expr GT expr"){
    	string type = tree->children[0]->type;
    	if (type == "int"){
    		codeGenerate(tree->children[0]);
    		push3();
    		codeGenerate(tree->children[2]);
    		pop5();
    		cout << "slt $3, $3, $5" << endl;
    	} else if (type == "int*"){
    		codeGenerate(tree->children[0]);
    		push3();
    		codeGenerate(tree->children[2]);
    		pop5();
    		cout << "sltu $3, $3, $5" << endl;
    	}
    } else if (rule == "test expr LE expr"){
    	string type = tree->children[0]->type;
    	if (type == "int"){
    		codeGenerate(tree->children[0]);
    		push3();
    		codeGenerate(tree->children[2]);
    		pop5();
    		cout << "slt $3, $3, $5" << endl;
    		cout << "sub $3, $11, $3" << endl;
    	} else if (type == "int*"){
    		codeGenerate(tree->children[0]);
    		push3();
    		codeGenerate(tree->children[2]);
    		pop5();
    		cout << "sltu $3, $3, $5" << endl;
    		cout << "sub $3, $11, $3" << endl;
    	}
    } else if (rule == "test expr GE expr"){
    	string type = tree->children[0]->type;
    	if (type == "int"){
    		codeGenerate(tree->children[0]);
    		push3();
    		codeGenerate(tree->children[2]);
    		pop5();
    		cout << "slt $3, $5, $3" << endl;
    		cout << "sub $3, $11, $3" << endl;
    	} else if (type == "int*"){
    		codeGenerate(tree->children[0]);
    		push3();
    		codeGenerate(tree->children[2]);
    		pop5();
    		cout << "sltu $3, $5, $3" << endl;
    		cout << "sub $3, $11, $3" << endl;
    	}
    } else if (rule == "test expr NE expr"){
    	string type = tree->children[0]->type;
    	codeGenerate(tree->children[0]);
    	push3();
    	codeGenerate(tree->children[2]);
    	pop5();
    	if (type == "int"){
    		cout << "slt $6, $3, $5" << endl;
    		cout << "slt $7, $5, $3" << endl;
    	} else if (type == "int*"){
    		cout << "sltu $6, $3, $5" << endl;
    		cout << "sltu $7, $5, $3" << endl;
    	}
    	cout << "add $3, $6, $7" << endl;
    } else if (rule == "test expr EQ expr"){
    	string type = tree->children[0]->type;
    	codeGenerate(tree->children[0]);
    	push3();
    	codeGenerate(tree->children[2]);
    	pop5();
    	if (type == "int"){
    		cout << "slt $6, $3, $5" << endl;
    		cout << "slt $7, $5, $3" << endl;
    	} else if (type == "int*"){
    		cout << "sltu $6, $3, $5" << endl;
    		cout << "sltu $7, $5, $3" << endl;
    	}
    	cout << "add $3, $6, $7" << endl;
    	cout << "sub $3, $11, $3" << endl;
    } else if (rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE"){
    	int i = ifLabelCounter;
    	++ifLabelCounter;
    	codeGenerate(tree->children[2]);
    	cout << "beq $3, $0, startelse" << i << endl;
    	codeGenerate(tree->children[5]);
    	cout << "beq $0, $0, endelse" << i << endl;
    	cout << "startelse" << i << ":" << endl;
    	codeGenerate(tree->children[9]);
    	cout << "endelse" << i << ":" << endl;
    } else if (rule == "factor STAR factor"){
    	codeGenerate(tree->children[1]);
    	cout << "lw $3, 0($3)" << " ; reached 1" << endl;
    } else if (rule == "factor NULL"){
        cout << "add $3, $0, $11 "  << endl;
    } else if (rule == "factor AMP lvalue"){
    	string crule = tree->children[1]->rule;
    	if (crule == "lvalue STAR factor"){
    		codeGenerate(tree->children[1]);
    	} else if (crule == "lvalue ID"){
    		codeGenerate(tree->children[1]);
    	} else if (crule == "lvalue LPAREN lvalue RPAREN"){
    		codeGenerate((tree->children[1])->children[1]);
    	}
    } else if (rule == "lvalue STAR factor"){
    	codeGenerate(tree->children[1]);
    } else if (rule == "lvalue ID"){
    	string idname = tree->children[0]->termval;
    	int offset = atable[idname];
    	cout << "lis $3" << " ; reached 3" << endl;
    	cout << ".word " << offset << endl;
    	cout << "add $3, $3, $29" << endl;
    } else if (rule == "lvalue LPAREN lvalue RPAREN"){
    	codeGenerate(tree->children[1]);
    } else if (rule == "factor NEW INT LBRACK expr RBRACK"){
    	codeGenerate(tree->children[3]);
    	cout << "add $1, $3, $0" << endl;
    	cout << "lis $10" << endl;
    	cout << ".word new" << endl;
    	cout << "jalr $10" << endl;
    	cout << "bne $3, $0, 1" << endl;
    	cout << "add $3, $11, $0" << endl;
    } else if (rule == "statement DELETE LBRACK RBRACK expr SEMI"){
    	codeGenerate(tree->children[3]);
    	cout << "beq $3, $11, 4" << endl;
    	cout << "add $1, $3, $0" << endl;
    	cout << "lis $10" << endl;
    	cout << ".word delete" << endl;
    	cout << "jalr $10" << endl;
    }
}

int lvalCheck(Node * tree){
	string subrule = tree->children[1]->rule;
    if (subrule == "lvalue ID"){
    	string idname = lvalName(tree->children[1]);
    	int offset = atable [idname];
    	cout << "lis $3" << endl;
    	cout << ".word " << offset << endl;
    	cout << "add $3, $3, $29" << endl;
    } else if (subrule == "lvalue STAR factor"){
    	codeGenerate(tree->children[1]);
    } else if (subrule == "lvalue LPAREN lvalue RPAREN"){
    	lvalCheck(tree->children[1]);
    }
}



int main(){
    Node * parseTree = createPTree();
    scanPTree(parseTree);
    if (errorstatus == true){
    	cerr << "error1" << endl;
    	return 0;
    }
    typePropPTree(parseTree);
    if (errorstatus == true){
    	cerr << "error2" << endl;
    	return 0;
    }
    prologue();
    codeGenerate(parseTree);
    epilog();
}



