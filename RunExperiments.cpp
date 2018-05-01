#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>
#include <vector>
#include <iomanip>

#include "Rapl.h"

#define RAPL_BASE_DIRECTORY "/sys/class/powercap/intel-rapl/intel-rapl:0/"
#define LONG_LIMIT "constraint_0_power_limit_uw"
#define SHORT_LIMIT "constraint_1_power_limit_uw"
#define LONG_WINDOW "constraint_0_time_window_us"
#define SHORT_WINDOW "constraint_1_time_window_us"
#define START_POWER_LIMIT 15
#define INCREMENT_POWER_LIMIT 1
#define STOP_POWER_LIMIT 3
#define NO_OF_REPEATS 5

struct ResultsContainer {
    double energy, power, time;
};

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

void saveSampleToPowerFile (Rapl *raplPtr, std::ofstream &outfile){

    outfile << raplPtr->pkg_current_power() << "\t"
            << raplPtr->pp0_current_power() << "\t"
            << raplPtr->pp1_current_power() << "\t"
            << raplPtr->dram_current_power() << "\t"
            << raplPtr->total_time() << std::endl;
}

void saveRecordToAvResultFile (ResultsContainer &container, std::ofstream &outfile, int powerLimit){

    outfile << powerLimit / 1000000 << "\t\t" //convert uW to W
            << std::fixed << std::setprecision(4)
            << container.energy << "\t" 
            << container.power << "\t"
            << container.time << std::endl;
}

void saveRecordToResultFile (Rapl *raplPtr, std::ofstream &outfile, int powerLimit){

    outfile << powerLimit / 1000000 << "\t\t" //convert uW to W
            << std::fixed << std::setprecision(4)
            << raplPtr->pkg_total_energy() << "\t" 
            << raplPtr->pkg_average_power() << "\t"
            << raplPtr->total_time() << std::endl;
}

int main (int argc, char *argv[]) {

    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << "CZAS: " << std::ctime(&t) << "\n";

    std::stringstream ss;
    ss << "experiment_result_" << t << ".csv";
    std::string outResultFileName =  ss.str();
	std::ofstream outResultFile (outResultFileName, std::ios::out | std::ios::trunc);
    ss.str("");
    ss << "experiment_current_power_" << t << ".csv";
    std::string outPowerFileName =  ss.str();
	std::ofstream outPowerFile (outPowerFileName, std::ios::out | std::ios::trunc);
    ss.str("");
    ss << "experiment_average_result_" << t << ".csv";
    std::string outAvResultFileName =  ss.str();
    std::ofstream outAvResultFile (outAvResultFileName, std::ios::out | std::ios::trunc);
    ss.str("");
    
	Rapl *globalRaplPtr, *localRaplPtr;
    globalRaplPtr = new Rapl();
	int ms_pause = 100;       // sample every 100ms

    std::string raplDir = RAPL_BASE_DIRECTORY;
    std::string pl0dir = raplDir + LONG_LIMIT;
    std::string pl1dir = raplDir + SHORT_LIMIT;

    int defaultPowerLimit0 = readLimitFromFile(pl0dir);
    int defaultPowerLimit1 = readLimitFromFile(pl1dir);

    std::vector<int> powerLimitsVec;
    for (int li = START_POWER_LIMIT; li >= STOP_POWER_LIMIT; li -= INCREMENT_POWER_LIMIT){
        powerLimitsVec.push_back(li*1000000);
    }

    // write header to result files
    outResultFile << "#lim P [W]\ttotal E [J]\tav. P [W]\ttotal t [s]\n";
    outAvResultFile << "#lim P [W]\ttotal E [J]\tav. P [W]\ttotal t [s]\n";

    for(auto currentLimit : powerLimitsVec){

        std::cout << "Run for power limit " << currentLimit/1000000 << "W\n";

        writeLimitToFile (pl0dir, currentLimit);
        if (readLimitFromFile(pl0dir) != currentLimit) {
            std::cerr << "Limit was not overwritten succesfully.\n"
                      << "HINT: Check dmesg if it is not locked by BIOS.\n";
        }
        writeLimitToFile (pl1dir, currentLimit);
        if (readLimitFromFile(pl1dir) != currentLimit) {
            std::cerr << "Limit was not overwritten succesfully.\n"
                      << "HINT: Check dmesg if it is not locked by BIOS.\n";
        }

        ResultsContainer tmpResultsContainer { 0.0, 0.0, 0.0 };

        for (int i = 0; i < NO_OF_REPEATS; i++){

            std::cout << "Iteration " << i + 1 << ".\n";

            localRaplPtr = new Rapl();

            pid_t child_pid = fork();
            if (child_pid >= 0) { //fork successful
                if (child_pid == 0) { // child process
                    //printf("CHILD: child pid=%d\n", getpid());
                    int exec_status = execvp(argv[1], argv+1);
                    if (exec_status) {
                        std::cerr << "execv failed with error " 
                            << errno << " "
                            << strerror(errno) << std::endl;
                    }
                } else {              // parent process
                    int status = 1;
                    waitpid(child_pid, &status, WNOHANG);

                    while (status) {
                        
                        usleep(ms_pause * 1000);

                        // rapl sample
                        globalRaplPtr -> sample();
                        localRaplPtr -> sample();
                        saveSampleToPowerFile(globalRaplPtr, outPowerFile);

                        waitpid(child_pid, &status, WNOHANG);	
                    }
                    wait(&status); /* wait for child to exit, and store its status */

                    if (WEXITSTATUS(status) !=0){
                        std::cerr << "ERROR: Child's exit code is: " << WEXITSTATUS(status) << std::endl;
                    }
                
                saveRecordToResultFile (localRaplPtr, outResultFile, currentLimit);
                
                }
            } else {
                std::cerr << "fork failed" << std::endl;
                return 1;
            }
            tmpResultsContainer.energy += localRaplPtr->pkg_total_energy();
            tmpResultsContainer.power += localRaplPtr->pkg_average_power();
            tmpResultsContainer.time += localRaplPtr->total_time();

            delete localRaplPtr;
        }
        tmpResultsContainer.energy /= NO_OF_REPEATS;
        tmpResultsContainer.power /= NO_OF_REPEATS;
        tmpResultsContainer.time /= NO_OF_REPEATS;
        saveRecordToAvResultFile (tmpResultsContainer, outAvResultFile, currentLimit);
    }
    //restore default ilmits
    writeLimitToFile (pl0dir, defaultPowerLimit0);
    writeLimitToFile (pl1dir, defaultPowerLimit1);
    std::cout << "Default limits restored.\n";

    delete globalRaplPtr;

	return 0;
}
