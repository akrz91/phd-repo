CXX = g++
CXXFLAGS = -std=c++11 -g
LD = $(CXX)
LDFLAGS = -lm -lrt
#MY_LDFLAGS = -lboost_filesystem -lboost_system

all : PowerMonitor AppPowerMeter RunExperiments SetPowerLimits

run : PowerMonitor AppPowerMeter RunExperiments SetPowerLimits
	#./PowerMonitor
	./AppPowerMeter sleep 5

SetPowerLimits : SetPowerLimits.o
	$(LD) $(LDFLAGS) -o $@ $^

SetPowerLimits.o : SetPowerLimits.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

RunExperiments : RunExperiments.o Rapl.o
	$(LD) $(LDFLAGS) -o $@ $^ $(MY_LDFLAGS)

RunExperiments.o : RunExperiments.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

AppPowerMeter : AppPowerMeter.o Rapl.o
	$(LD) $(LDFLAGS) -o $@ $^

AppPowerMeter.o : AppPowerMeter.cpp Rapl.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

PowerMonitor : PowerMonitor.o Rapl.o
	$(LD) $(LDFLAGS) -o $@ $^

PowerMonitor.o : PowerMonitor.cpp Rapl.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Rapl.o : Rapl.cpp Rapl.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<



clean :
	rm -f *.o
	rm -f rapl.csv
	rm -f AppPowerMeter
	rm -f PowerMonitor
	rm -f RunExperiments
	rm -f SetPowerLimits
