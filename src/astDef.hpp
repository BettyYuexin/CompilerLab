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
    unique_ptr<BaseAST> unaryExp;

    void Dump() override {
        #ifdef _DEBUG
        cout << "ExpAST {\n";
        #endif

        unaryExp->Dump();
        variableName = unaryExp->variableName;

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







