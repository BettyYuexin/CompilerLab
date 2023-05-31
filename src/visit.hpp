#pragma once

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <sstream>
#include "koopa.h"
// #define _DEBUG


std::string tempRegName[15] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};

// 函数声明
void Visit(const koopa_raw_program_t&);
void Visit(const koopa_raw_slice_t&);
void Visit(const koopa_raw_function_t&);
void Visit(const koopa_raw_basic_block_t&);
void Visit(const koopa_raw_value_t&);
void Visit(const koopa_raw_return_t&);
void Visit(const koopa_raw_integer_t&);
void Visit(const koopa_raw_binary_t&);
void Visit(const koopa_raw_binary_op_t&);



// 访问 raw program
void Visit(const koopa_raw_program_t &program) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  #ifdef _DEBUG
  std::cout << "visit program { \n";
  #endif
  std::cout << "\t.text\n";
  std::cout << "\t.globl main\n";
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);

  #ifdef _DEBUG
  std::cout << " }\n";
  #endif
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  #ifdef _DEBUG
  std::cout << "visit slice { \n";
  #endif

  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // std::cout << "false slice val!!!" << slice.kind << std::endl;
        assert(false);
    }

    #ifdef _DEBUG
    std::cout << " }\n";
    #endif
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有基本块
  #ifdef _DEBUG
  std::cout << "visit raw function { \n";
  #endif
  const char * funcName = func->name;
  std::string nameOut(funcName);
  std::cout << nameOut.substr(1) << ":\n";
  Visit(func->bbs);

  #ifdef _DEBUG
  std::cout << " }\n";
  #endif
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  #ifdef _DEBUG
  std::cout << "visit basic block { \n";
  #endif

  Visit(bb->insts);

  #ifdef _DEBUG
  std::cout << " }\n";
  #endif
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  #ifdef _DEBUG
  std::cout << "visit value { \n";
  #endif

  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      Visit(kind.data.integer);
      break;
    case KOOPA_RVT_BINARY:
      Visit(kind.data.binary);
      break;
    default:
      // std::cout << "false val!!!" << kind.tag << std::endl;
      assert(false);
  }

  #ifdef _DEBUG
  std::cout << " }\n";
  #endif
}

void Visit(const koopa_raw_return_t &ret) {
  #ifdef _DEBUG
  std::cout << "visit ret { \n";
  #endif

  std::cout << "\tli a0, ";
  Visit(ret.value);
  std::cout << "\n";
  std::cout << "\tret\n";

  #ifdef _DEBUG
  std::cout << " }\n";
  #endif
}

void Visit(const koopa_raw_integer_t &int_val) {
  #ifdef _DEBUG
  std::cout << "visit int_val:" << int_val.value << std::endl;
  #endif

  std::cout << int_val.value;
}

void Visit(const koopa_raw_binary_t &bin_val) {
  #ifdef _DEBUG
  std::cout << "visit bin_val:" << std::endl;
  #endif
  std::cout << "\t";
  Visit(bin_val.op);
  std::cout << "\t";
  Visit(bin_val.lhs);
  std::cout << ", ";
  Visit(bin_val.rhs);
  std::cout << "\n";
  // std::cout << int_val.value;
}


void Visit(const koopa_raw_binary_op_t &bin_op) {
  #ifdef _DEBUG
  std::cout << "visit bin_op:" << bin_op << std::endl;
  #endif
  switch(bin_op) {
    case KOOPA_RBO_NOT_EQ:
      std::cout << "ne"; break;
    case KOOPA_RBO_EQ:
      std::cout << "eq"; break;
    case KOOPA_RBO_GT:
      std::cout << "gt"; break;
    case KOOPA_RBO_LT:
      std::cout << "lt"; break;
    case KOOPA_RBO_GE:
      std::cout << "ge"; break;
    case KOOPA_RBO_LE:
      std::cout << "le"; break;
    case KOOPA_RBO_ADD:
      std::cout << "add"; break;
    case KOOPA_RBO_SUB:
      std::cout << "sub"; break;
    case KOOPA_RBO_MUL:
      std::cout << "mul"; break;
    case KOOPA_RBO_DIV:
      std::cout << "div"; break;
    case KOOPA_RBO_MOD:
      std::cout << "mod"; break;
    case KOOPA_RBO_AND:
      std::cout << "and"; break;
    case KOOPA_RBO_OR:
      std::cout << "or"; break;
    case KOOPA_RBO_XOR:
      std::cout << "xor"; break;
    case KOOPA_RBO_SHL:
      std::cout << "shl"; break;
    case KOOPA_RBO_SHR:
      std::cout << "shr"; break;
    case KOOPA_RBO_SAR:
      std::cout << "sar"; break;
    default:
      // std::cout << "false op!!!" << bin_op << std::endl;
      assert(false);
  }
}