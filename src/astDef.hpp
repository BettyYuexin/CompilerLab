#pragma once

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <sstream>
// #define _DEBUG


// std::map <std::string, std::string> type2IRtype = {
//     {"int", "i32"}
// };
static int tempID = 0;

// 所有 AST 的基类
class BaseAST {
public:
    std::string parseType;
    std::string variableName;

    virtual ~BaseAST() = default;

    virtual void Dump() = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;
    
    void Dump() override {
        #ifdef _DEBUG
        std::cout << "CompUnitAST {\n";
        #endif

        func_def->Dump();

        #ifdef _DEBUG
        std::cout << "}\n";
        #endif
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump() override {
        #ifdef _DEBUG
        std::cout << "FuncDefAST {\n";
        #endif

        std::cout << "fun @" << ident << "(): ";
        func_type->Dump();
        std::cout << "{\n";
        block->Dump();
        std::cout << "}\n";

        #ifdef _DEBUG
        std::cout << "}\n";
        #endif
    }
};

class FuncTypeAST : public BaseAST {
public:
    std::string type;

    void Dump() override {
        #ifdef _DEBUG
        std::cout << "FuncTypeAST {\n";
        #endif

        std::cout << type;

        #ifdef _DEBUG
        std::cout << "}\n";
        #endif
    }
};

class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() override {
        #ifdef _DEBUG
        std::cout << "BlockAST {\n";
        #endif

        std::cout << "%" << "entry:\n";
        stmt->Dump();

        #ifdef _DEBUG
        std::cout << "}\n";
        #endif
    }
};

class StmtAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;

    void Dump() override {
        #ifdef _DEBUG
        std::cout << "StmtAST {\n";
        #endif

        exp->Dump();
        std::cout << "\tret " << exp->variableName << "\n";

        #ifdef _DEBUG
        std::cout << "}\n";
        #endif
    }
};

class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> unaryExp;

    void Dump() override {
        #ifdef _DEBUG
        std::cout << "ExpAST {\n";
        #endif

        unaryExp->Dump();
        variableName = unaryExp->variableName;

        #ifdef _DEBUG
        std::cout << "}\n";
        #endif
    }
};


class PrimaryExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;
    int number;

    void Dump() override {
        #ifdef _DEBUG
        std::cout << "PrimaryExpAST {\n";
        #endif
        
        if(!strcmp(parseType.c_str(), "exp")) {
            exp->Dump();
            variableName = exp->variableName;
        }
        else if (!strcmp(parseType.c_str(), "number")) {
            variableName = std::to_string(number);
        }

        #ifdef _DEBUG
        std::cout << "}\n";
        #endif
    }
};

class UnaryExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> primaryExp;
    std::unique_ptr<BaseAST> unaryExp;
    std::string unaryOp;

    void Dump() override {
        #ifdef _DEBUG
        std::cout << "UnaryExpAST {\n";
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
                    std::cout << "\t%" << tempID << " = sub 0, " << unaryExp->variableName << "\n";
                }
                else if(!strcmp(unaryOp.c_str(), "!")) {
                    std::cout << "\t%" << tempID << " = eq " << unaryExp->variableName << ", 0\n";
                }
                variableName = "%" + std::to_string(tempID);
                tempID++;
            }
        }

        #ifdef _DEBUG
        std::cout << "}\n";
        #endif
    }
};







