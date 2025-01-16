#ifndef COMMANDS_H
#define COMMANDS_H

#include "common.h"

void executeCommand(char command, char* buffer, int lineSize, 
List* parkingLotList, List* vehicleList, char* lastRegistdate,
 HashTable* hashTableVehicle);

void executeP(char* s, int lineSize, List* parkingLotList);

void listP(List* parkingLotList);

int errorsP(char* name, int capacity, float first15, float next15,
 float maxDay, List* parkingLotList);

ParkingLot* createParkingLot(char* name, int capacity, float first15, float next15,
 float maxDay);

void addParkingLotToList(List* parkingLotList, ParkingLot* parkingLot);

ParkingLot* searchParkingLot(List* parkingLotList, char* name);

List* initializeList();

//CHECKPOINT

int validlicencePlate(char* licencePlate);

void formatDate(int day, int month, int year, int hour, int minute,
 char* date);

int daysInMonth(int month);

int daysUpToMonth(int month);

int dateValidation(char* date, char* lastDate);

void writeDate(char* date);

//CHECKPOINT

void executeE(char* s, List* parkingLotList, List* vehicleList,
 char* lastRegistDate, HashTable* hashTableVehicle);

HashTable* initializeHashTable();

int hash(char* licencePlate, int tableSize);

void insertVehicleInHashTable(HashTable* hashTable, Vehicle* vehicle);

Vehicle* searchVehicle(HashTable* hashTable, char* licencePlate);

int errorsE(char* name, char* licencePlate, char* dateEntry,
char* lastRegistDateTime, ParkingLot* parkingLot, Vehicle* vehicle);

Vehicle* createVehicle(char* licencePlate, ParkingLot* parkingLot,
 char* dateEntry);

Log* createLog(char* dateEntry, char* parkName, char* licencePlate);

void addVehicleToList(List* vehicleList, Vehicle* vehicle);

void freeVehicleList(List* vehicleList);

void freeHashTable(HashTable* hashTable);

void freeParkingLotList(List* parkingLotList);

void freeLogList(List* logList);

// CHECKPOINT

void executeS(char* s, List* parkingLotList, char* lastRegistDate,
 HashTable* hashTableVehicle);

long numberOfMinutes(char* date);

float faturationValue(char* date1, char* date2, float first15, float next15,
 float maxDay);

void addLogToVehicleLogList(List* logList, Log* log);

void addLogToParkingLogList(List* logList, Log* log);

Log* completeLog(Log* currentLog, char* dateLeft, float first15, float next15,
 float maxDay);

int errorsS(char* name, char* licencePlate, char* dateLeft,
char* lastRegistDate, ParkingLot* parkingLot, Vehicle* vehicle);

//CHECKPOINT

void executeV(char* s, HashTable* hashTable);
int errorsV(Vehicle* vehicle, char* licencePlate);
void printV(char* parkName, char* dateEntry, char* dateLeft);
void printVehicleLogs(Vehicle* vehicle);
void printLogsUntilCurrent(Node** current, char* parkName);
void printLogs(Node* current);

//CHECKPOINT

void executeF(char* s, List* parkingLotlist, char* lastRegistDate);
int errorsF(char* name, ParkingLot* parkingLot, char* date,
 char* lastRegistDate);
void printParkFaturation(ParkingLot* parkingLot);
void printDateFaturation(ParkingLot* parkingLot, char* date);
void formatDateOnly(int day, int month, int year, char* date);
void writeDateOnly(char* date);
int dateOnlyValidation(char* date, char* lastDate);
void writeTimeOnly(char* date);

#endif /* COMMANDS_H */




