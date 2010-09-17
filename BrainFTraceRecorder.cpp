#include "BrainFTraceRecorder.h"

BrainFTraceRecorder::BrainFTraceRecorder() {
  ProfilingTable = new uint8_t[PROFILING_TABLE_SIZE];
  memset(ProfilingTable, 0, PROFILING_TABLE_SIZE);
  
  TraceBuffer = TraceCursor = new size_t[TRACE_BUFFER_SIZE];
  memset(TraceBuffer, 0, TRACE_BUFFER_SIZE);
}

BrainFTraceRecorder::~BrainFTraceRecorder() {
  delete[] ProfilingTable;
  delete [] TraceBuffer;
}
