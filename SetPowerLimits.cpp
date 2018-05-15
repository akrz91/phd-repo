#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#define RAPL_BASE_DIRECTORY "/sys/class/powercap/intel-rapl/intel-rapl:0/"
#define LONG_LIMIT "constraint_0_power_limit_uw"
#define SHORT_LIMIT "constraint_1_power_limit_uw"
#define LONG_WINDOW "constraint_0_time_window_us"
#define SHORT_WINDOW "constraint_1_time_window_us"


void writeLimitToFile (std::string fileName, int limit){

    std::ofstream outfile (fileName.c_str(), std::ios::out | std::ios::trunc);
    if (outfile.is_open()){
        outfile << limit;
    } else {
        std::cerr << "cannot write the limit to file\n";
        std::cerr << "file not open\n";
    }
    outfile.close();
}

int readLimitFromFile (std::string fileName){

    std::ifstream limitFile (fileName.c_str());
    std::string line;
    int limit = -1;
    if (limitFile.is_open()){
        while ( getline (limitFile, line) ){
            limit = atoi(line.c_str());
        }
        limitFile.close();
    } else {
        std::cerr << "cannot read the limit file\n";
        std::cerr << "file not open\n";
    }
    return limit;
}

int main (int argc, char *argv[]) {

    if(argc < 3){
        printf("Usage: ./SetPowerLimit longLimit shortLimit\n");
        exit(EXIT_FAILURE);
    }
    std::string raplDir = RAPL_BASE_DIRECTORY;
    std::string pl0dir = raplDir + LONG_LIMIT;
    std::string pl1dir = raplDir + SHORT_LIMIT;

    int powerLimit0 = atoi(argv[1])*1000000;
    int powerLimit1 = atoi(argv[2])*1000000;

    writeLimitToFile (pl0dir, powerLimit0);
    if (readLimitFromFile(pl0dir) != powerLimit0) {
        std::cerr << "Limit was not overwritten succesfully.\n"
                    << "HINT: Check dmesg if it is not locked by BIOS.\n";
    }
    writeLimitToFile (pl1dir, powerLimit1);
    if (readLimitFromFile(pl1dir) != powerLimit1) {
        std::cerr << "Limit was not overwritten succesfully.\n"
                    << "HINT: Check dmesg if it is not locked by BIOS.\n";
    }

	return 0;
}
