/*
 * ----------------------------------------------------------------------------
 * Author: Khushboo Tekchandani
 *
 * This file contains the implementation for the query tool for the log
 * monitoring system. The query tool allows a user to query the system for 
 * system usage for a given time-slot on a given date.
 * ----------------------------------------------------------------------------
 */

#include <cstdio>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <dirent.h>
#include "query_tool.h"

using namespace std;

/* 
 * Converts human readable time to unix time 
 */
time_t  normalToUnix (date *givenDate, utime *givenTime){

   struct tm tmNormal;
   tmNormal.tm_sec = givenTime->second;
   tmNormal.tm_min = givenTime->minute;
   tmNormal.tm_hour = givenTime->hour;
   tmNormal.tm_mday = givenDate->day;
   tmNormal.tm_mon = givenDate->month - 1;
   tmNormal.tm_year = givenDate->year - 1900;

   time_t unixTime = mktime(&tmNormal);

   return unixTime;
}

/* 
 * Converts time_t to struct tm which can be used to reconstruct human readable 
 * time from unix time
 */
struct tm* unixToNormal (time_t unixTime){
   
   struct tm *normalTime;
   normalTime = localtime(&unixTime);
   return normalTime;
}

/* 
 * Constructor for initial mapping
 */
initialMapping::initialMapping(){
   filename = "";
   info = new map< string, list < pair<time_t, int> > > ();
}

/*
 * Sets the filename in the initial mapping
 */
void 
initialMapping::setFilename(string fname){
   filename = fname;
}

/* 
 * Adds a new key value pair to the hashmap in our mapping
 */
void 
initialMapping::addKeyValue(string key, 
      pair<time_t, int> val){

   list <pair <time_t, int> > curr;

   if(info->find(key) != info->end()){
      info->find(key)->second.push_back(val);
   }

   else{
      curr.clear();
      curr.push_back(val);
      info->insert(pair < string, list< pair<time_t, int> > >(key, curr));
   }
}

/*
 * Helper function to print the index of the log files
 */
void 
initialMapping::printIndex(){

   map< string, list< pair<time_t, int> > >::iterator it;
   list< pair<time_t, int> >::iterator listit;
   for(it = info->begin(); it!= info->end(); it++){
      for(listit = it->second.begin(); listit!=it->second.end(); listit++){
         cout << it->first << " " << listit->first << " " << listit->second << endl;
      }
   }
}

/* 
 * Creates the initial indexing of all log files
 */
void 
initialMapping::createIndex(){

   int count = 0;
   string line;
   char *info;

   // Open the file for reading
   ifstream infile(filename);

   if(infile.is_open()){

      while(!getline(infile,line).eof()){

         char *cstr = new char[line.length()+1];
         cstr[line.length()] = '\0';
         strcpy(cstr, line.c_str());
         info = strtok(cstr, " \t\n");
         vector<char *> in;

         if(strcasecmp(info, "Timestamp") == 0 || line.length() == 0){
            // ignore first line
         }

         else{

            // Read each line
            while(info != NULL){
               in.push_back(info);
               info = strtok(NULL, " \t\n");
            }

            // Create key using IP address and CPUID
            string key = string(in[1])+string(in[2]);

            // Process the timestamp and convert it to time_t
            char* temp = in[0];
            istringstream ss(temp);
            time_t timeTemp;
            ss >> timeTemp;

            // Create value pair using timestamp and usage
            pair<time_t, int> value = make_pair(timeTemp, atoi(in[3]));

            // Insert into the map
            addKeyValue(key,value);
         }
      }

   }
}

/*
 * Constructor for query class
 */
query::query(string ip, int id, utime* stime, utime* etime, date* sdate, 
      date* edate){
   serverIp = ip;
   cpuId = id;
   startTime = stime;
   endTime = etime;
   startDate = sdate;
   endDate = edate;
}

/* 
 * Constructs a query from the input arguments
 */
query* makeQuery(vector<char *> input){

   string ip = input[1];
   int id = atoi(input[2]);

   char *in;

   // Save the start date
   int *inDate = new int[3];
   in = strtok(input[3], "-");
   for(int i=0; i<3; i++){
      inDate[i] = atoi(in);
      in = strtok(NULL, "-");
   }

   date *sdate = new date(inDate[0], inDate[1], inDate[2]);

   // Save the end date
   in = strtok(input[5], "-");
   for(int i=0; i<3; i++){
      inDate[i] = atoi(in);
      in = strtok(NULL, "-");
   }

   date *edate = new date(inDate[0], inDate[1], inDate[2]);

   // Save the start time
   int *inTime = new int[2];
   in = strtok(input[4], ":");
   inTime[0] = atoi(in);
   in = strtok (NULL, ":");
   inTime[1] = atoi(in);

   utime *stime = new utime(inTime[0], inTime[1], 0);

   // Save the end time
   in = strtok(input[6], ":");
   inTime[0] = atoi(in);
   in = strtok(NULL, ":");
   inTime[1] = atoi(in);

   utime *etime = new utime(inTime[0], inTime[1], 0);

   query *q = new query(ip, id, stime, etime, sdate, edate);

   return q;
}


/*
 * Executes query by looking up the mapping and printing information for the
 * requests
 *
 * Limitations:
 *
 * The tool assumes that the user inputs the start date and end
 * date in the correct order. This is start date < end date and start time <
 * endtime.
 * 
 * The order of output of the tool depends on the initial mapping.
 */
void executeQuery(query *q, initialMapping* index){

   cout << "CPU" << to_string(q->cpuId) << " usage on " << q->serverIp << ":\n"; 
   string key = q->serverIp + to_string(q->cpuId);

   if(index->info->find(key) == index->info->end()){
      cout << "No logs found.\n";
   }

   else{

      map <string, list< pair<time_t, int> > >::iterator it;
      list< pair<time_t, int> >::iterator listit;
      time_t stime = normalToUnix(q->startDate, q->startTime);
      time_t etime = normalToUnix(q->endDate, q->endTime);

      struct tm *curr;
      char buff[80];

      it = index->info->find(key);
      for(listit = it->second.begin(); listit!=it->second.end(); listit++){

         if(listit->first < stime){
            // Information is not of use for our query
         }

         else if(listit->first >= stime && listit->first <= etime){
            curr = unixToNormal(listit->first);
            strftime(buff, 20, "%Y-%m-%d %H:%M", curr);
            cout << "(" << buff <<", " << listit->second << "%),";
         }

         else if(listit->first > etime) {
            // Information is not of use for our query
         }
      }

      cout << endl;
   }
}

/*
 * Iterates over all files in a directory
 */
void iterateFiles(const char* path, initialMapping* index){

   DIR* dir = opendir(path);
   char *fname;

   if (dir)
   {
      struct dirent* file;
      while ((file = readdir(dir)) != NULL )
      {
         if (!strcmp( file->d_name, "."  )) continue;
         if (!strcmp( file->d_name, ".." )) continue;
         if (( file->d_name[0] == '.' )) continue;

         if ( strstr( file->d_name, ".txt" )){
            string fname = string(path) + "/" + string(file->d_name);
            index->setFilename(fname);
            index->createIndex();
         }
      }  

      closedir(dir);
   }
}

/* 
 * Iterates over two levels of directories, all years and all months
 */
void iterateDirectory(initialMapping* index, char* path){

 //  const char* path = ".";
   DIR *dir = opendir(path);
   struct dirent *entry = readdir(dir);

   while (entry != NULL)
   {
      if (entry->d_type == DT_DIR ){

         // Skip hidden directories
         if(entry->d_name[0] == '.');

         else{
            char pathLevel1[80];
            strcpy(pathLevel1, path);
            strcat(pathLevel1, "/");
            strcat(pathLevel1, entry->d_name);

            DIR *dir2 = opendir(pathLevel1);
            struct dirent *entry2 = readdir(dir2);

            // Iterate over the next level of directories
            while(entry2 != NULL){

               if(entry2->d_type == DT_DIR){

                  if(entry2->d_name[0] == '.');
                  else{
                     char pathLevel2 [100];
                     strcpy(pathLevel2, pathLevel1);
                     strcat(pathLevel2, "/");
                     strcat(pathLevel2, entry2->d_name);

                     DIR *dir3 = opendir(pathLevel2);
                     iterateFiles(pathLevel2, index);
                  }
               }
               entry2 = readdir(dir2);
            }
            closedir(dir2);
         }
      }
      entry = readdir(dir);
   }
   closedir(dir);
}


/*
 * Main driver function
 */
int main(int argc, char **argv){

   cout << "Initializing query tool...\n";

   char *path = argv[1];
   // Initialize tool
   initialMapping *index = new initialMapping();
   iterateDirectory(index, path);

   while(true){

      // Prompt
      cout << "> " ;
      cout.flush();
      string line;
      getline(cin, line);
      cout.flush();

      if(line == ""){
         continue;
      }
      // Convert string to char* for strtok
      char *command;
      char *cstr = new char[line.length() + 1];
      cstr[line.length()] = '\0';
      strcpy(cstr, line.c_str());

      // Read the command given to the tool
      command = strtok(cstr, " ");

      int count;
      // Process query
      if(strcasecmp(command, "QUERY") == 0){

         count = 0; // to keep track of the number arguments to the query
         vector<char *> input;
         while(command != NULL){
            count++;
            input.push_back(command);
            command = strtok (NULL, " ");
         }

         if(count != 7){
            cout << "Usage: QUERY <serverIp> <cpuID> <startDate> "
               "<startTime> <endDate> <endTime>\n";

            continue;
         }

         // Construct a query from the given input
         query* q = makeQuery(input);

         // execute query
         executeQuery(q, index);

      }

      // Process exit command
      else if(strcasecmp(command, "EXIT") == 0){
         cout << "Exiting tool..\n";
         exit(EXIT_SUCCESS);
      }

      // Invalid Command
      else{
         cout << "Invalid Command!\n";
      }

   }

   return 0;
}


