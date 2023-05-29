#pragma once

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
// #include <map>


// std::map <std::string, std::string> type2IRtype = {
//     {"int", "i32"}
// };


// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    std::stringstream ss;
    std::unique_ptr<BaseAST> func_def(ss);
    
    void Dump() const override {
        func_def->Dump();
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    std::stringstream ss;
    std::unique_ptr<BaseAST> func_type(ss);
    std::string ident;
    std::unique_ptr<BaseAST> block(ss);

    void Dump() const override {
        std::cout << "fun @" << ident << "(): ";
        func_type->Dump();
        ss << "{\n";
        block->Dump();
        ss << "}\n";
    }
    FuncDefAST(std::stringstream ss):ss(ss){}
};

class FuncTypeAST : public BaseAST {
public:
    std::stringstream ss;
    std::string type(ss);

    void Dump() const override {
        ss << type;
    }
    FuncTypeAST(std::stringstream ss):ss(ss) {}
};

class BlockAST : public BaseAST {
public:
    std::stringstream ss;
    std::unique_ptr<BaseAST> stmt(ss);

    void Dump() const override {
        ss << "%" << "entry:\n";
        stmt->Dump();
    }
    BlockAST(std::stringstream ss):ss(ss) {}
};

class StmtAST : public BaseAST {
public:
    std::stringstream ss;
    int number;

    void Dump() const override {
        ss << "ret " << number << std::endl;
    }
    StmtAST(std::stringstream ss):ss(ss) {}
};



