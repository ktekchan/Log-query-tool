/* 
 * ----------------------------------------------------------------------------
 * Author: Khushboo Tekchandani
 * 
 * This is a simulator for generating dummy log files 
 *
 * Creates a directory for the given year.
 * Creates a directory for the given month for the above created directory.
 * Creates a text file with the given date for writing log information.
 * Log information for the given date is then entered into this file
 *
 * This simulator can only support a maximum of 254*254 servers
 * 
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>

#define NUM_SERVERS 1000

using namespace std;

int main(int argc, char **argv){

   if(argc<2){
      cout << "Please provide a data path\n";
      exit(EXIT_FAILURE);
   }

   char *path = argv[1];

   // Taking user input for simulating log files
   int logYear;
   int logMonth;
   int logDay;
   char ch;

   // Assumption - User enters a valid date
   cout << "Enter a date to generate a simulated log file (YYYY-MM-DD)\n";
   cin >> logYear >> ch >> logMonth >> ch >> logDay;

   // Checks for invalid input type
   if(cin.fail()){
      cout << "Invalid date!\n";
      exit(EXIT_FAILURE);
   }

   //To convert the given date into Unix timestamp
   struct tm tmStart;

   // Initializing the seconds, minute and hours to 00 as the simulator will
   // start logging from 00:00:00 of the day
   tmStart.tm_sec = 0;
   tmStart.tm_min = 0;
   tmStart.tm_hour = 0;
   tmStart.tm_mday = logDay;
   tmStart.tm_mon = logMonth - 1;
   tmStart.tm_year = logYear - 1900;

   time_t startTime, endTime;
   startTime = mktime(&tmStart);
   endTime = startTime + (24*60*60);


   // Make a directory for the given year that contains a directory for the 
   // given month
   string yearDir = "mkdir " + string(path) + "/" + to_string(logYear) 
      + " > /dev/null 2>&1";

   system(yearDir.c_str());

   string monthDir = "mkdir " + string(path) + "/" + to_string(logYear) + "/" 
      + to_string(logMonth) + " > /dev/null 2>&1";

   system(monthDir.c_str());


   string outFile = to_string(logDay)+".txt";

   // Open the log file
   fstream fout(outFile.c_str());
   fout.open(outFile, fstream::out| fstream::app);

   if(!fout){
      fout.open(outFile, fstream::trunc | fstream::out);
      fout.close();
      fout.open(outFile, fstream::out | fstream::app);
   }

   fout << "Timestamp\t IP\t CPU_ID\t Usage\n";

   // Random number generator between 0 to 100 to log the CPU usage
   srand((unsigned)time(0));
   int floor = 0, ceiling = 100, range = (ceiling - floor);
   int cpuUsage;

   int i,j;
   int id;
   int count = 0;

   startTime = startTime - 60;

   // Carries out the entire logging task
start:
      count = 0;
      i = 1;
      j = 1;
      startTime = startTime+60;

   while(startTime!=endTime){

      for(i=1; i<=254; i++){
         for(j=1; j<=254; j++){

            for (id=0; id<2; id++){
               cpuUsage = floor + int(rand() % (range + 1));
               fout << startTime << "\t" << "192.168." << i << "." << j << 
                  "\t" << id << " " << cpuUsage << std::endl;
            }

            if(count >= NUM_SERVERS-1)
               goto start;
            count++;
         }
      }
      startTime = startTime+60;   
   }

   // Moves the log file to the appropriate folder
   string move = "mv " + outFile + " " + string(path) + "/"  
      + to_string(logYear) + "/" + to_string(logMonth) + "/";

   system(move.c_str());

   return 0;
}
