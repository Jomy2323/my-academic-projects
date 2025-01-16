#include "commands.h"
#include "parser.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


int main(){
    // Variable command to know which case and buffer to read the command lines
    char command, buffer[BUFSIZ];
    // Initialize lists
    List* parkingLotList = initializeList();
    List* vehiclesList = initializeList();
    HashTable* hashTableVehicle = initializeHashTable();
    char lastRegistDate[LOG_FORMAT_SIZE] = "";

    while (1){
        command = getchar();
        int lineSize = readLine(buffer);
        executeCommand(command, buffer, lineSize, parkingLotList, vehiclesList,
         lastRegistDate, hashTableVehicle);
    }
    // This part should never be reached
    exit(EXIT_FAILURE);
}




