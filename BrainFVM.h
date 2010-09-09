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

/// BytecodeArray - An array of function pointers representing the
/// source program.  Indexed by PC address.
extern void **BytecodeArray;

/// JumpMap - An array of on-the-side data used by the interpreter.
/// Indexed by PC address.
extern size_t *JumpMap;

#endif