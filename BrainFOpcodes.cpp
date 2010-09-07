//===-- BrainFOpcodes.cpp - BrainF interpreter opcodes ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===--------------------------------------------------------------------===//

#include "BrainFVM.h"
#include <cstdio>

opcode_func_t *BytecodeArray = 0;
size_t *JumpMap = 0;
uint8_t executed = 0;

size_t op_plus(size_t pc, uint8_t **data) {
  **data += JumpMap[pc];
  return pc+1;
}

size_t op_minus(size_t pc, uint8_t **data) {
  **data -= JumpMap[pc];
  return pc+1;
}

size_t op_left(size_t pc, uint8_t **data) {
  *data -= JumpMap[pc];
  return pc+1;
}

size_t op_right(size_t pc, uint8_t **data) {
  *data += JumpMap[pc];
  return pc+1;
}

size_t op_put(size_t pc, uint8_t **data) {
  putchar(**data);
  return pc+1;
}

size_t op_get(size_t pc, uint8_t **data) {
  **data = getchar();
  return pc+1;
}

size_t op_if(size_t pc, uint8_t **data) {
  size_t new_pc = pc+1;
  if (!**data) new_pc = JumpMap[pc]+1;
  return new_pc;
}

size_t op_back(size_t pc, uint8_t **data) {
  size_t new_pc = JumpMap[pc];
  return new_pc;
}

size_t op_set_zero(size_t pc, uint8_t **data) {
  **data = 0;
  return pc+1;
}

size_t op_end(size_t, uint8_t **) {
  return ~0ULL;
}
