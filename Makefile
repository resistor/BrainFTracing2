CXX = g++
CXXFLAGS = $(shell llvm-config --cxxflags)
LDFLAGS = $(shell llvm-config --ldflags)
LIBS = $(shell llvm-config --libs jit bitwriter nativecodegen)

OBJS = BrainFInterpreter.o

BrainFTracing2: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -O3 -c -o $@ $<

clean:
	rm -rf *.o BrainFTracing2