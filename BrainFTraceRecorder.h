#ifndef BRAINF_TRACE_RECORDER_H
#define BRAINF_TRACE_RECORDER_H

#include "BrainFVM.h"
#include <algorithm>
#include <stdint.h>
#include <cstring>

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

class BrainFTraceRecorder {
  uint8_t *ProfilingTable;
  size_t *TraceBuffer, *TraceCursor;
  
  static const size_t PROFILING_TABLE_SIZE = 512;
  static const size_t HOT_THRESHOLD        =  20;
  static const size_t TRACE_BUFFER_SIZE    = 100;
  
public:
  BrainFTraceRecorder();
  ~BrainFTraceRecorder();
  
  void profile(size_t pc) {
    size_t idx = pc % PROFILING_TABLE_SIZE;
    uint8_t &val = ProfilingTable[idx];
    ++val;
    if (unlikely(val > HOT_THRESHOLD)) {
      val = 0;
      *TraceCursor++ = pc;
      std::swap(BytecodeArray, AltBytecodeArray);
    }
  }
  
  void record(size_t pc) {
    *TraceCursor++ = pc;
    if (unlikely(TraceCursor == TraceBuffer + TRACE_BUFFER_SIZE)) {
      TraceCursor = TraceBuffer;
      std::swap(BytecodeArray, AltBytecodeArray);
    }
  }
  
};

#endif
