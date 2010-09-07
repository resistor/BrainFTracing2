//===-- BrainFVM.h - BrainF interpreter header ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===--------------------------------------------------------------------===//

#ifndef BRAINF_VM_H
#define BRAINF_VM_H

#include "stdint.h"
#include <cstring>

/// opcode_func_t - A function pointer signature for all opcode functions.
typedef size_t(*opcode_func_t)(size_t pc, uint8_t** data);

/// BytecodeArray - An array of function pointers representing the
/// source program.  Indexed by PC address.
extern opcode_func_t *BytecodeArray;

/// JumpMap - An array of on-the-side data used by the interpreter.
/// Indexed by PC address.
extern size_t *JumpMap;

/// op_plus - Implements the '+' instruction.
size_t op_plus(size_t, uint8_t**);

/// op_minus - Implements the '-' instruction.
size_t op_minus(size_t, uint8_t**);

// op_left - Implements the '<' instruction.
size_t op_left(size_t, uint8_t**);

// op_right - Implements the '>' instruction.
size_t op_right(size_t, uint8_t**);

// op_put - Implements the '.' instruction.
size_t op_put(size_t, uint8_t**);

// op_get - Implements the ',' instruction.
size_t op_get(size_t, uint8_t**);

// op_if - Implements the '[' instruction.
size_t op_if(size_t, uint8_t**);

// op_back - Implements the ']' instruction.
size_t op_back(size_t, uint8_t**);

// op_set_zero - Implements the '0' synthetic instruction.
size_t op_set_zero(size_t, uint8_t**);

// op_bin_add - Implements the '0' synthetic instruction.
size_t op_bin_add(size_t, uint8_t**);

// op_end - Terminates an execution.
size_t op_end(size_t, uint8_t**);


#endif