/*
 * ----------------------------------------------------------------------------
 * Author: Khushboo Tekchandani
 *
 * This file contains the header information for a query tool for the log monitoring
 * system. The query tool allows a user to query the system for system usage for
 * a given time-slot on a given date.
 *
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <list>
#include <utility>

using namespace std;


// Structure to store date
struct date{
   int day;
   int month;
   int year;

   date(int yyyy, int mm, int dd){
      day = dd;
      month = mm;
      year = yyyy;
   }
};

// Structure to store time
struct utime{
   int hour;
   int minute;
   int second;

   utime(int hh, int mm, int ss){
      hour = hh; 
      minute = mm; 
      second = ss;
   }
};

// Converts given date and time to Unix time
time_t normalToUnix (date*, utime*);

// Converts give unix time to normal time
struct tm* unixToNormal (time_t unixTime);

// forward declarations
class initialMapping;
class query;

// Class to save all the mappings
class initialMapping{
   private:
     string filename;
     map< string, list < pair<time_t, int> > > *info;
   public:
     initialMapping();
     list<pair <time_t, int> > getValue(string);
     void addKeyValue(string, pair< time_t, int>);
     void createIndex();
     void printIndex();
     void setFilename(string);

     friend void iterateDirectory(initialMapping*, char*);
     friend void iterateFiles(const char*, initialMapping*);
     friend void executeQuery(query*, initialMapping*);
};

// query class
class query{
   private:
      string serverIp;
      int cpuId;
      utime *startTime;
      utime *endTime;
      date *startDate;
      date *endDate; 
  public:
      query(string ip, int id, utime*, utime*, date*, date*);

      // Getters
      string getServerIp(){ return serverIp;}
      int getCpuId() {return cpuId;}
      utime* getStartTime(){ return startTime;}
      utime* getEndTime(){ return endTime;}
      date* getStartDate(){ return startDate;}
      date* getEndDate(){ return endDate;}

      // Executes the query
      friend void executeQuery(query*, initialMapping*);
};

