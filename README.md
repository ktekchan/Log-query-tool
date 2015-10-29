# Log-query-tool
A command line to query cpu usage logs

##User Manual.

###Simulator

####To compile:
g++ -std=c++11 log_generator.cpp -o generate

####To run:
./generate <data path>

###Query tool

####To compile:
g++ std=c++11 query_tool.cpp -o query

####To run
./query <data path>

####To query
query <server ip> <cpu id> <start date> <start time> <end date> <end time> 
