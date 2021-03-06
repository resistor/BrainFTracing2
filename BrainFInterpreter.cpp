//===-- BrainFInterpreter.cpp - BrainF trace compiler interpreter -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===--------------------------------------------------------------------===//

#include "BrainFVM.h"
#include "BrainFTraceRecorder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdio>
using namespace llvm;

//Command line options

static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input brainf>"));

void **BytecodeArray = 0;
void **AltBytecodeArray = 0;
size_t *JumpMap = 0;

int main(int argc, char **argv) {
  cl::ParseCommandLineOptions(argc, argv, " BrainF compiler\n");

  if (InputFilename == "") {
    errs() << "Error: You must specify the filename of the program to "
    "be compiled.  Use --help to see the options.\n";
    abort();
  }

  // Read the input file.
  MemoryBuffer *Code = MemoryBuffer::getFileOrSTDIN(InputFilename);
  const uint8_t *CodeBegin = (const uint8_t*)(Code->getBufferStart());
  
  // Create a new buffer to hold the preprocessed code.
  MemoryBuffer *ParsedCode =
    MemoryBuffer::getNewMemBuffer(sizeof(void*) * 
                                  (Code->getBufferSize()+1));
  MemoryBuffer *AltParsedCode =
    MemoryBuffer::getNewMemBuffer(sizeof(void*) * 
                                  (Code->getBufferSize()+1));
  BytecodeArray = (void**)(ParsedCode->getBufferStart());
  AltBytecodeArray = (void**)(AltParsedCode->getBufferStart());
  size_t BytecodeOffset = 0;
  
  // Create JumpMap, a special on-the-side data array used to implement
  // efficient jumps in the interpreter.
  JumpMap = new size_t[Code->getBufferSize()];
  memset(JumpMap, 0, sizeof(size_t) * Code->getBufferSize());
  std::vector<size_t> Stack;
  
  // Preprocess the input source code, performing three tasks:
  //  1 - Remove non-instruction characters
  //  2 - Replace character literals with opcode function pointers
  //  3 - Precompute the jump targets for [ and ] instructions in JumpMap
  for (size_t i = 0; i < Code->getBufferSize(); ++i) {
    uint8_t opcode = CodeBegin[i];
    switch (opcode) {
      case '>':
        if (BytecodeArray[BytecodeOffset-1] == &&op_right_prof)
          JumpMap[BytecodeOffset-1] += 1;
        else {
          JumpMap[BytecodeOffset] = 1;
          BytecodeArray[BytecodeOffset] = &&op_right_prof;
          AltBytecodeArray[BytecodeOffset++] = &&op_right_rec;
        }
        break;
      case '<':
        if (BytecodeArray[BytecodeOffset-1] == &&op_left_prof)
          JumpMap[BytecodeOffset-1] += 1;
        else {
          JumpMap[BytecodeOffset] = 1;
          BytecodeArray[BytecodeOffset] = &&op_left_prof;
          AltBytecodeArray[BytecodeOffset++] = &&op_left_rec;
        }
        break;
      case '+':
        if (BytecodeArray[BytecodeOffset-1] == &&op_plus_prof)
          JumpMap[BytecodeOffset-1] += 1;
        else {
          JumpMap[BytecodeOffset] = 1;
          BytecodeArray[BytecodeOffset] = &&op_plus_prof;
          AltBytecodeArray[BytecodeOffset++] = &&op_plus_rec;
        }
        break;
      case '-':
        if (BytecodeArray[BytecodeOffset-1] == &&op_minus_prof)
          JumpMap[BytecodeOffset-1] += 1;
        else {
          JumpMap[BytecodeOffset] = 1;
          BytecodeArray[BytecodeOffset] = &&op_minus_prof;
          AltBytecodeArray[BytecodeOffset++] = &&op_minus_rec;
        }
        break;
      case '.':
        BytecodeArray[BytecodeOffset] = &&op_put_prof;
        AltBytecodeArray[BytecodeOffset++] = &&op_put_rec;
        break;
      case ',':
        BytecodeArray[BytecodeOffset] = &&op_get_prof;
        AltBytecodeArray[BytecodeOffset++] = &&op_get_rec;
        break;
      case '[':
        Stack.push_back(BytecodeOffset);
        BytecodeArray[BytecodeOffset] = &&op_if_prof;
        AltBytecodeArray[BytecodeOffset++] = &&op_if_rec;
        break;
      case ']':
        // Special case: [-] --> 0
        if (BytecodeOffset > 1 && 
            CodeBegin[i-2] == '[' && CodeBegin[i-1] == '-') {
          Stack.pop_back();
          BytecodeOffset -= 2;
          BytecodeArray[BytecodeOffset] = &&op_set_zero_prof;
          AltBytecodeArray[BytecodeOffset++] = &&op_set_zero_rec;
        // Special case: [->+<] --> a
        } else if (BytecodeOffset > 4 &&
                 CodeBegin[i-5] == '[' && CodeBegin[i-4] == '-' &&
                 CodeBegin[i-3] == '>' && CodeBegin[i-2] == '+' &&
                 CodeBegin[i-1] == '<' &&
                 JumpMap[BytecodeOffset-1] == JumpMap[BytecodeOffset-3]) {
          BytecodeOffset -= 5;
          JumpMap[BytecodeOffset] = JumpMap[BytecodeOffset+2];
          for (unsigned j = 1; j < 6; ++j)
            JumpMap[BytecodeOffset+j] = 0;
          Stack.pop_back();
          BytecodeArray[BytecodeOffset] = &&op_bin_add_prof;
          AltBytecodeArray[BytecodeOffset++] = &&op_bin_add_rec;
        } else {
          JumpMap[Stack.back()] = BytecodeOffset;
          JumpMap[BytecodeOffset] = Stack.back();
          Stack.pop_back();
          BytecodeArray[BytecodeOffset] = &&op_back_prof;
          AltBytecodeArray[BytecodeOffset++] = &&op_back_rec;
        }
        break;
      default:
        continue;
    }
  }
  
  // Fill in the suffix of the preprocessed source for op_exit.
  // Thus, if we reach the end of the source, the program will terminate.
  while (BytecodeOffset < Code->getBufferSize()+1) {
    BytecodeArray[BytecodeOffset] = &&op_end;
    AltBytecodeArray[BytecodeOffset++] = &&op_end;
  }
  
  // Setup the array.
  uint8_t *BrainFArray = new uint8_t[32768];
  memset(BrainFArray, 0, 32768);
  
  // Main interpreter loop.
  // Note the lack of a explicit loop: every opcode is a tail-recursive
  // function that calls its own successor by indexing into BytecodeArray.
  uint8_t* data = BrainFArray;
  
  BrainFTraceRecorder Recorder;
  
  size_t pc = 0;
  goto *BytecodeArray[pc];
  
op_end:
  //Clean up
  delete Code;
  delete ParsedCode;
  delete[] BrainFArray;
  delete[] JumpMap;

  return 0;

op_plus_prof:
  *data += JumpMap[pc];
  pc += 1;
  goto *BytecodeArray[pc];

op_minus_prof:
  *data -= JumpMap[pc];
  pc += 1;
  goto *BytecodeArray[pc];

op_left_prof:
  data -= JumpMap[pc];
  pc += 1;
  goto *BytecodeArray[pc];

op_right_prof:
  data += JumpMap[pc];
  pc += 1;
  goto *BytecodeArray[pc];

op_put_prof:
  putchar(*data);
  pc += 1;
  goto *BytecodeArray[pc];

op_get_prof:
  *data = getchar();
  pc += 1;
  goto *BytecodeArray[pc];

op_if_prof:
  Recorder.profile(pc);
  if (!*data) pc = JumpMap[pc]+1;
  else pc += 1;
  goto *BytecodeArray[pc];

op_back_prof:
  Recorder.profile(pc);
  if (*data) pc = JumpMap[pc]+1;
  else pc += 1;
  goto *BytecodeArray[pc];

op_set_zero_prof:
  *data = 0;
  pc += 1;
  goto *BytecodeArray[pc];

op_bin_add_prof:
  *(data + JumpMap[pc]) += *data;
  *data = 0;
  pc += 1;
  goto *BytecodeArray[pc];
  
op_plus_rec:
  Recorder.record(pc);
  *data += JumpMap[pc];
  pc += 1;
  goto *BytecodeArray[pc];

op_minus_rec:
  Recorder.record(pc);
  *data -= JumpMap[pc];
  pc += 1;
  goto *BytecodeArray[pc];

op_left_rec:
  Recorder.record(pc);
  data -= JumpMap[pc];
  pc += 1;
  goto *BytecodeArray[pc];

op_right_rec:
  Recorder.record(pc);
  data += JumpMap[pc];
  pc += 1;
  goto *BytecodeArray[pc];

op_put_rec:
  Recorder.record(pc);
  putchar(*data);
  pc += 1;
  goto *BytecodeArray[pc];

op_get_rec:
  Recorder.record(pc);
  *data = getchar();
  pc += 1;
  goto *BytecodeArray[pc];

op_if_rec:
  Recorder.record(pc);
  if (!*data) pc = JumpMap[pc]+1;
  else pc += 1;
  goto *BytecodeArray[pc];

op_back_rec:
  Recorder.record(pc);
  if (*data) pc = JumpMap[pc]+1;
  else pc += 1;
  goto *BytecodeArray[pc];

op_set_zero_rec:
  Recorder.record(pc);
  *data = 0;
  pc += 1;
  goto *BytecodeArray[pc];

op_bin_add_rec:
  Recorder.record(pc);
  *(data + JumpMap[pc]) += *data;
  *data = 0;
  pc += 1;
  goto *BytecodeArray[pc];
}
