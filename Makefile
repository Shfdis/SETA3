CXX = g++
CXXFLAGS = -std=c++17 -Wall

OBJS = RandomStreamGen.o HashFuncGen.o HyperLogLog.o
LIBS = libRandomStreamGen.a libHashFuncGen.a libHyperLogLog.a

.PHONY: all clean experiment plot

all: $(LIBS) experiment

plot: experiment
	./experiment && python3 plot_results.py

libRandomStreamGen.a: RandomStreamGen.o
	ar rcs $@ $^

libHashFuncGen.a: HashFuncGen.o
	ar rcs $@ $^

libHyperLogLog.a: HyperLogLog.o
	ar rcs $@ $^

experiment: main.cpp $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ main.cpp -L. -lHyperLogLog -lHashFuncGen -lRandomStreamGen

RandomStreamGen.o: RandomStreamGen.cpp RandomStreamGen.h
	$(CXX) $(CXXFLAGS) -c -o $@ RandomStreamGen.cpp

HashFuncGen.o: HashFuncGen.cpp HashFuncGen.h
	$(CXX) $(CXXFLAGS) -c -o $@ HashFuncGen.cpp

HyperLogLog.o: HyperLogLog.cpp HyperLogLog.h HashFuncGen.h
	$(CXX) $(CXXFLAGS) -c -o $@ HyperLogLog.cpp

clean:
	rm -f $(OBJS) $(LIBS) experiment graph1_data.csv graph2_data.csv graph1_ft0_vs_nt.png graph2_E_Nt_sigma.png
