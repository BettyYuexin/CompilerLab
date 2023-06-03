#pragma once

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <sstream>
// #define _DEBUG
using namespace std;

// map <string, string> type2IRtype = {
//     {"int", "i32"}
// };
static int tempID = 0;

// 所有 AST 的基类
class BaseAST {
public:
    string parseType;
    string variableName;

    virtual ~BaseAST() = default;

    virtual void Dump() = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    unique_ptr<BaseAST> func_def;
    
    void Dump() override {
        #ifdef _DEBUG
        cout << "CompUnitAST {\n";
        #endif

        func_def->Dump();

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    unique_ptr<BaseAST> func_type;
    string ident;
    unique_ptr<BaseAST> block;

    void Dump() override {
        #ifdef _DEBUG
        cout << "FuncDefAST {\n";
        #endif

        cout << "fun @" << ident << "(): ";
        func_type->Dump();
        cout << "{\n";
        block->Dump();
        cout << "}\n";

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class FuncTypeAST : public BaseAST {
public:
    string type;

    void Dump() override {
        #ifdef _DEBUG
        cout << "FuncTypeAST {\n";
        #endif

        cout << type;

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class BlockAST : public BaseAST {
public:
    unique_ptr<BaseAST> stmt;

    void Dump() override {
        #ifdef _DEBUG
        cout << "BlockAST {\n";
        #endif

        cout << "%" << "entry:\n";
        stmt->Dump();

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class StmtAST : public BaseAST {
public:
    unique_ptr<BaseAST> exp;

    void Dump() override {
        #ifdef _DEBUG
        cout << "StmtAST {\n";
        #endif

        exp->Dump();
        cout << "\tret " << exp->variableName << "\n";

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class ExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> lOrExp;

    void Dump() override {
        #ifdef _DEBUG
        cout << "ExpAST {\n";
        #endif

        lOrExp->Dump();
        variableName = lOrExp->variableName;

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};


class MulExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> mulExp;
    unique_ptr<BaseAST> unaryExp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "MulExpAST {\n";
        #endif
        
        if(!strcmp(parseType.c_str(), "unaryExp")) {
            unaryExp->Dump();
            variableName = unaryExp->variableName;
        }
        else if (!strcmp(parseType.c_str(), "bin")) {
            mulExp->Dump();
            unaryExp->Dump();
            if(!strcmp(op.c_str(), "*")) {
                cout << "\t%" << tempID << " = mul " << mulExp->variableName << ", " << unaryExp->variableName << "\n";
            }
            else if(!strcmp(op.c_str(), "/")) {
                cout << "\t%" << tempID << " = div " << mulExp->variableName << ", " << unaryExp->variableName << "\n";
            }
            else if(!strcmp(op.c_str(), "%%")) {
                cout << "\t%" << tempID << " = mod " << mulExp->variableName << ", " << unaryExp->variableName << "\n";
            }
            variableName = "%" + to_string(tempID);
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};


class AddExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> addExp;
    unique_ptr<BaseAST> mulExp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "AddExpAST {\n";
        #endif
        
        if(!strcmp(parseType.c_str(), "multExp")) {
            mulExp->Dump();
            variableName = mulExp->variableName;
        }
        else if (!strcmp(parseType.c_str(), "bin")) {
            addExp->Dump();
            mulExp->Dump();
            if(!strcmp(op.c_str(), "+")) {
                cout << "\t%" << tempID << " = add " << addExp->variableName << ", " << mulExp->variableName << "\n";
            }
            else if(!strcmp(op.c_str(), "-")) {
                cout << "\t%" << tempID << " = sub " << addExp->variableName << ", " << mulExp->variableName << "\n";
            }
            variableName = "%" + to_string(tempID);
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};



class PrimaryExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> exp;
    int number;

    void Dump() override {
        #ifdef _DEBUG
        cout << "PrimaryExpAST {\n";
        #endif
        
        if(!strcmp(parseType.c_str(), "exp")) {
            exp->Dump();
            variableName = exp->variableName;
        }
        else if (!strcmp(parseType.c_str(), "number")) {
            variableName = to_string(number);
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class UnaryExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> primaryExp;
    unique_ptr<BaseAST> unaryExp;
    string unaryOp;

    void Dump() override {
        #ifdef _DEBUG
        cout << "UnaryExpAST {\n";
        #endif
        
        if(!strcmp(parseType.c_str(), "primary")) {
            primaryExp->Dump();
            variableName = primaryExp->variableName;
        }
        else if (!strcmp(parseType.c_str(), "uop")) {
            unaryExp->Dump();
            if(!strcmp(unaryOp.c_str(), "+")) {
                // + 不生成IR
                variableName = unaryExp->variableName;
            }
            else {
                if(!strcmp(unaryOp.c_str(), "-")) {
                    cout << "\t%" << tempID << " = sub 0, " << unaryExp->variableName << "\n";
                }
                else if(!strcmp(unaryOp.c_str(), "!")) {
                    cout << "\t%" << tempID << " = eq " << unaryExp->variableName << ", 0\n";
                }
                variableName = "%" + to_string(tempID);
                tempID++;
            }
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};


class RelExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> addExp;
    unique_ptr<BaseAST> relExp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "RelExpAST {\n";
        #endif
        
        if(!strcmp(parseType.c_str(), "addExp")) {
            addExp->Dump();
            variableName = addExp->variableName;
        }
        else if (!strcmp(parseType.c_str(), "bin")) {
            relExp->Dump();
            addExp->Dump();
            if(!strcmp(op.c_str(), "<")) {
                cout << "\t%" << tempID << " = lt " << relExp->variableName << ", " << addExp->variableName << "\n";
            }
            else if(!strcmp(op.c_str(), ">")) {
                cout << "\t%" << tempID << " = gt " << relExp->variableName << ", " << addExp->variableName << "\n";
            }
            else if(!strcmp(op.c_str(), "<=")) {
                cout << "\t%" << tempID << " = le " << relExp->variableName << ", " << addExp->variableName << "\n";
            }
            else if(!strcmp(op.c_str(), ">=")) {
                cout << "\t%" << tempID << " = ge " << relExp->variableName << ", " << addExp->variableName << "\n";
            }
            variableName = "%" + to_string(tempID);
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class EqExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> relExp;
    unique_ptr<BaseAST> eqExp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "EqExpAST {\n";
        #endif
        
        if(!strcmp(parseType.c_str(), "relExp")) {
            relExp->Dump();
            variableName = relExp->variableName;
        }
        else if (!strcmp(parseType.c_str(), "bin")) {
            eqExp->Dump();
            relExp->Dump();
            if(!strcmp(op.c_str(), "==")) {
                cout << "\t%" << tempID << " = eq " << eqExp->variableName << ", " << relExp->variableName << "\n";
            }
            else if(!strcmp(op.c_str(), "!=")) {
                cout << "\t%" << tempID << " = ne " << eqExp->variableName << ", " << relExp->variableName << "\n";
            }
            variableName = "%" + to_string(tempID);
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class LAndExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> eqExp;
    unique_ptr<BaseAST> lAndExp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "LAndExpAST {\n";
        #endif
        
        if(!strcmp(parseType.c_str(), "eqExp")) {
            eqExp->Dump();
            variableName = eqExp->variableName;
        }
        else if (!strcmp(parseType.c_str(), "bin")) {
            lAndExp->Dump();
            eqExp->Dump();
            if(!strcmp(op.c_str(), "&&")) {
                cout << "\t%" << tempID << " = ne " << lAndExp->variableName << ", 0\n";
                tempID++;
                cout << "\t%" << tempID << " = ne " << eqExp->variableName << ", 0\n";
                tempID++;
                cout << "\t%" << tempID << " = and %" << tempID - 2 << ", %" << tempID - 1 << "\n";
            }
            else {
                assert(false);
            }
            variableName = "%" + to_string(tempID);
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};

class LOrExpAST : public BaseAST {
public:
    unique_ptr<BaseAST> lAndExp;
    unique_ptr<BaseAST> lOrExp;
    string op;

    void Dump() override {
        #ifdef _DEBUG
        cout << "LOrExpAST {\n";
        #endif
        
        if(!strcmp(parseType.c_str(), "lAndExp")) {
            lAndExp->Dump();
            variableName = lAndExp->variableName;
        }
        else if (!strcmp(parseType.c_str(), "bin")) {
            lOrExp->Dump();
            lAndExp->Dump();
            if(!strcmp(op.c_str(), "||")) {
                cout << "\t%" << tempID << " = or " << lOrExp->variableName << ", " << lAndExp->variableName << "\n";
                tempID++;
                cout << "\t%" << tempID << " = ne %" << tempID - 1 << ", 0\n";
            }
            else {
                assert(false);
            }
            variableName = "%" + to_string(tempID);
            tempID++;
        }

        #ifdef _DEBUG
        cout << "}\n";
        #endif
    }
};




