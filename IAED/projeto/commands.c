#include "commands.h"
#include "parser.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


// General command functions

void executeCommand(char command, char* buffer, int lineSize, 
List* parkingLotList, List* vehicleList, char* lastRegistDate,
 HashTable* hashTableVehicle){
    switch (command) {
        case COMMAND_P:
            executeP(buffer, lineSize, parkingLotList);
            break;
        case COMMAND_E:
            executeE(buffer, parkingLotList, vehicleList, lastRegistDate,
             hashTableVehicle);
            break;
        case COMMAND_S:
            executeS(buffer, parkingLotList, lastRegistDate,
             hashTableVehicle);
            break;
        case COMMAND_V:
            executeV(buffer, hashTableVehicle);
            break;
        case COMMAND_F:
            executeF(buffer, parkingLotList, lastRegistDate);
            break;
        case COMMAND_R:
            //executeR();
            break;
        case COMMAND_Q:
            freeHashTable(hashTableVehicle);
            freeVehicleList(vehicleList);
            freeParkingLotList(parkingLotList);
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, "Error: Unknown command '%c'\n", command);
            exit(EXIT_FAILURE);
    }
}

// Functions for command p

/**
 * @brief Executes the "p" command to list parking lots or create a new one.
 *
 * This function parses the input string to either list existing parking lots or
 * create a new parking lot with the provided parameters. It handles the
 * validation of input arguments and error checking before creating a new
 * parking lot.
 *
 * @param s The input string containing the parameters.
 * @param lineSize The size of the input string.
 * @param parkingLotList Pointer to the list of parking lots.
 */
void executeP(char* s, int lineSize, List* parkingLotList) {
    if (lineSize == 0) { // List parking lots
        listP(parkingLotList);
    } else { // Verify errors and create parking lot
        // Skip first space
        int currentIndex = 1; 
        char* name;
        int capacity;
        float first15, next15, maxDay;
        
        /* Gets pointer to first argument and changes cursor to space between
          arguments*/
        name = getName(s + currentIndex, &currentIndex);
        // Skip space
        currentIndex++; 
        // Gets values for next arguments
        sscanf(s + currentIndex, "%d %f %f %f", &capacity, &first15, &next15,
               &maxDay);
        
        if (!errorsP(name, capacity, first15, next15, maxDay, parkingLotList)) {
            ParkingLot* newParkingLot = createParkingLot(name, capacity,
             first15, next15, maxDay);
            addParkingLotToList(parkingLotList, newParkingLot);
        } else {
            // Free memory of name ptr and make it null to avoid dangling ptr
            free(name);
        }
    }    
}


/**
 * @brief Check for errors in parking lot parameters before adding a new parking
 * lot.
 *
 * This function checks if the provided parameters for a new parking lot contain
 * any errors. It verifies if the parking lot name already exists, if the
 * capacity is non-positive, and if any of the cost values are negative or zero.
 * Additionally, it checks if the maximum number of parking lots has been
 * reached.
 *
 * @param name The name of the parking lot to be checked.
 * @param capacity The maximum capacity of the parking lot.
 * @param first15 The cost for the first 15 minutes of parking.
 * @param next15 The cost for every 15 minutes of parking after the first hour.
 * @param maxDay The maximum daily cost for parking.
 * @param parkingLotList Pointer to the list of parking lots to check for 
 * existing names and to check if the maximum number of parking lots has been 
 * reached.
 * @return Returns 0 if no errors are found, and 1 if any error is encountered.
 */
int errorsP(char* name, int capacity, float first15, float next15, float maxDay,
 List* parkingLotList) {
    // Verify if Parking Lot exists
    ParkingLot* parkingLot = searchParkingLot(parkingLotList, name);
    if (parkingLot != NULL){
        printf("%s: parking already exists.\n", name);
        return 1; // Return error indicator
    }

    // Verify if capacity is negative or zero
    else if (capacity <= 0) {
        printf("%d: invalid capacity.\n", capacity);
        return 1; // Return error indicator
    }

    // Verify if any of the costs is negative or zero or non crescent
    else if (first15 <= 0 || next15 <= 0 || maxDay <= 0 || maxDay <= next15
     || next15 <= first15) {
        printf("invalid cost.\n");
        return 1; // Return error indicator
    } 
    
    // Verify if the max number of parking lots has been created
    else if (parkingLotList->elements >= MAX_PARKING_LOTS) {
        printf("too many parks.\n");
        return 1; // Return error indicator
    }

    return 0; // Return success indicator
}

/**
 * @brief Creates a new parking lot with the specified parameters and adds it to
 * the list.
 *
 * This function allocates memory for a new parking lot structure and
 * initializes its fields with the provided values. The newly created parking
 * lot is added to the linked list of parking lots.
 *
 * @param name The name of the parking lot.
 * @param capacity The maximum capacity of the parking lot.
 * @param first15 The cost for the first 15 minutes of parking.
 * @param next15 The cost for every 15 minutes of parking after the first hour.
 * @param maxDay The maximum daily cost for parking.
 * @param parkingLotList Pointer to the list of parking lots to eventually add a
 * new parking lot to the list.
 */
ParkingLot* createParkingLot(char* name, int capacity, float first15,
 float next15, float maxDay) {
    
    ParkingLot* newParkingLot = malloc(sizeof(ParkingLot));

    if (newParkingLot == NULL) {
        // Deal with memory allocation failure and exit with status 1
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    newParkingLot->name = name;
    name = NULL;
    newParkingLot->capacity = capacity;
    newParkingLot->spotsAvailable = capacity;
    newParkingLot->first15 = first15;
    newParkingLot->next15 = next15;
    newParkingLot->maxDay = maxDay;
    newParkingLot->selfLogList = initializeList();
    
    return newParkingLot;
}

/**
 * @brief Adds a parking lot to the end of the list.
 *
 * This function creates a new node containing the provided parking lot
 * information and adds it to the end of the linked list. If the list is
 * empty, the new node becomes the head of the list.
 *
 * @param parkingLotList Pointer to the list to which the parking lot will be
 * added.
 * @param parkingLot Pointer to the parking lot structure to be stored in the
 * new node.
 */
void addParkingLotToList(List* parkingLotList, ParkingLot* parkingLot) {
    // Allocate memory for new node, verify its allocation and initialize node
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    
    // Set the key and content fields of the new node
    newNode->key = parkingLot->name; // Assuming name is used as the key
    newNode->content = parkingLot;
    newNode->next = NULL;

    // Place the new node at the end of the list
    if (parkingLotList->head == NULL) {
        // If the list is empty, set the new node as the head
        parkingLotList->head = newNode;
    } else {
        // If the list is not empty, find the last node
        Node* current = parkingLotList->head;
        while (current->next != NULL) {
            current = current->next;
        }
        // Link the new node to the last node
        current->next = newNode;
    }

    parkingLotList->elements++;
}


/**
 * @brief Frees the memory allocated for the parking lot list and its contents.
 *
 * This function deallocates the memory allocated for each parking lot node
 * in the linked list, as well as the memory allocated for the parking lot
 * structures and their names.
 *
 * @param parkingLotList A pointer to the head of the parking lot list.
 */
void freeParkingLotList(List* parkingLotList) {
    Node* current = parkingLotList->head;
    Node* temp;

    while (current != NULL) {
        // Free the content of the node
        ParkingLot* parkingLot = (ParkingLot*)current->content;
        Node* logNode = parkingLot->selfLogList->head;
        Node* tempNode;
        while (logNode != NULL){
            tempNode = logNode;
            logNode = logNode->next;
            free(tempNode);
        }
        free(parkingLot->selfLogList);
        free(parkingLot->name);
        free(parkingLot);

        // Move to the next node and free the current node
        temp = current;
        current = current->next;
        free(temp);
    }
    
    // Finally, free the list itself
    free(parkingLotList);
}



/**
 * @brief Prints information about each parking lot in the provided list.
 *
 * This function iterates over each node in the provided list of parking lots,
 * printing information about each parking lot, including its name, capacity,
 * and spots available.
 *
 * @param parkingLotList A pointer to the list of parking lots to be printed.
 */
void listP(List* parkingLotList) {
    Node* current = parkingLotList->head;
    while (current != NULL) {
        ParkingLot* parkingLot = (ParkingLot*)current->content;
        printf("%s %d %d\n", current->key, parkingLot->capacity,
         parkingLot->spotsAvailable);
        current = current->next;
    }
}

ParkingLot* searchParkingLot(List* parkingLotList, char* name){
    Node* current = parkingLotList->head;
    while (current != NULL) {
        ParkingLot* parkingLot = (ParkingLot*)current->content;
        if (strcmp(current->key, name) == 0) {
            return parkingLot;
        }
        current = current->next; // Move to the next node
    }
    return NULL;
}

/**
 * @brief Initializes a new linked list.
 *
 * This function allocates memory for a new linked list, initializes its
 * properties and returns a pointer to the newly created list.
 *
 * @return A pointer to the newly created linked list.
 */
List* initializeList() {
    List* newList = (List*)malloc(sizeof(List));
    if (newList == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE); // Terminate the program if memory allocation fails
    }
    newList->head = NULL;
    newList->elements = 0;
    return newList; // Return a pointer to the newly created list
}

//CHECKPOINT

/**
 * @brief Validates a license plate string.
 *
 * This function checks whether the provided license plate string conforms to
 * the format consisting of letter-digit pairs separated by a dash.
 *
 * @param licencePlate The license plate string to validate.
 * @return 1 if the license plate is valid, 0 otherwise.
 */
int validlicencePlate(char* licencePlate) {
    int letterPairCounter = 0, digitPairCounter = 0;

    // Iterate through the license plate string
    // In loop 6 = "AA-00-X" where X is the first character in the third pair
    for (int i = 0; i <= 6; i += PAIR_SIZE) {
        // Check if the current pair consists of uppercase letters
        if (isalpha(licencePlate[i]) && isupper(licencePlate[i]) &&
            isalpha(licencePlate[i + 1]) && isupper(licencePlate[i + 1])) {
            letterPairCounter++;
        }
        // Check if the current pair consists of digits
        else if (isdigit(licencePlate[i]) && isdigit(licencePlate[i + 1])) {
            digitPairCounter++;
        }
        else {
            return 0; // Pair made of letter and digit
        }
    }

    // Check if there is at least one pair of uppercase letters and one pair of
    // digits
    if (letterPairCounter != 0 && digitPairCounter != 0) {
        return 1; // Valid license plate
    }

    return 0; // License plate is either missing a digit pair or a letter pair
}

/**
 * @brief Formats date and time components into a string.
 *
 * This function formats the given date and time components (day, month, year,
 * hour, minute) into a string in the format "YYYYMMDDHHMM" with leading zeros
 * if necessary.
 *
 * @param day The day component of the date.
 * @param month The month component of the date.
 * @param year The year component of the date.
 * @param hour The hour component of the time.
 * @param minute The minute component of the time.
 * @param date A character array to store the formatted date and time string.
 * It must have sufficient memory to accommodate the formatted string (at least
 * 13 characters including the null-terminator).
 */
void formatDate(int day, int month, int year, int hour, int minute,
 char* date) {
    // Format the date components with leading zeros if necessary
    sprintf(date, "%04d%02d%02d%02d%02d", year, month, day, hour, minute);
}


/**
 * @brief Calculates the number of days up to the start of a given month.
 *
 * This function calculates the total number of days up to the start of the
 * given month in a non-leap year. The month is specified using a symbolic
 * constant representing its position in the year (e.g., JAN for January, etc.).
 *
 * @param month An integer representing the month (using symbolic constants).
 * @return The number of days up to the start of the given month in a non-leap
 * year.
 */
int daysUpToMonth(int month) {
    switch(month) {
        case JAN:   return 0;   // January: 0 days
        case FEB:   return 31;  // February: 31 days
        case MAR:   return 59;  // March: 59 days
        case APR:   return 90;  // April: 90 days
        case MAY:   return 120; // May: 120 days
        case JUN:   return 151; // June: 151 days
        case JUL:   return 181; // July: 181 days
        case AUG:   return 212; // August: 212 days
        case SEP:   return 243; // September: 243 days
        case OCT:   return 273; // October: 273 days
        case NOV:   return 304; // November: 304 days
        case DEC:   return 334; // December: 334 days
        default:    exit(EXIT_FAILURE); // Invalid month
    }
}

/**
 * @brief Calculates the number of days in a given month.
 *
 * This function calculates the total number of days in the given month.
 * The month is specified using a symbolic constant representing its position
 * in the year (e.g., JAN for January, FEB for February, etc.).
 *
 * @param month An integer representing the month (using symbolic constants).
 * @return The number of days in the given month.
 */
int daysInMonth(int month) {
    switch(month) {
        case JAN:   return 31; // January: 31 days
        case FEB:   return 28; // February: 28 days (no leap years)
        case MAR:   return 31; // March: 31 days
        case APR:   return 30; // April: 30 days
        case MAY:   return 31; // May: 31 days
        case JUN:   return 30; // June: 30 days
        case JUL:   return 31; // July: 31 days
        case AUG:   return 31; // August: 31 days
        case SEP:   return 30; // September: 30 days
        case OCT:   return 31; // October: 31 days
        case NOV:   return 30; // November: 30 days
        case DEC:   return 31; // December: 31 days
        default:    exit(EXIT_FAILURE); // Invalid month
    }
}

/**
 * @brief Validates a date string.
 *
 * This function validates the given date string by checking if it falls within
 * valid ranges and is chronologically after a specified last date.
 *
 * @param date A string representing the date to validate.
 * @param lastDate A string representing the last date that the given date
 * should be after.
 * @return 1 if the date is invalid, 0 if the date is valid.
 */
int dateValidation(char* date, char* lastDate) {
    int year, month, day, hour, minute;

    // Extract year, month, day, hour, and minute from the date string
    sscanf(date, "%4d%2d%2d%2d%2d", &year, &month, &day, &hour, &minute);
    
    // Check if minute or hour is out of valid range
    // or if month is out of range or day is out of range for that month
    // or if year is negative
    if (minute > 59 || minute < 0 || hour > 23 || hour < 0 || month < JAN ||
        month > DEC || day < 1 || day > daysInMonth(month) || year < 0) {
        return 1; // Date is invalid
    }
    // Check if the date is before or equal to the lastDate
    else if (strcmp(date, lastDate) < 0) {
        return 1; // Date is invalid
    }

    return 0; // Date is valid
}

/**
 * @brief Parses a date string and prints it in the format "DD-MM-YYYY HH:MM".
 *
 * This function takes a date string in the format "YYYYMMDDHHMM" and parses it
 * to extract the individual components (day, month, year, hour, and minute).
 * It then prints these components in the format "DD-MM-YYYY HH:MM".
 *
 * @param date A string representing the date in the format "YYYYMMDDHHMM".
 */
void writeDate(char* date) {
    int day, month, year, hour, minute;

    // Parse the date string to extract day, month, year, hour, and minute
    sscanf(date, "%4d%2d%2d%2d%2d", &year, &month, &day, &hour, &minute);

    // Print the parsed date components in the desired format
    printf("%02d-%02d-%04d %02d:%02d", day, month, year, hour, minute);
}

/**
 * @brief Executes the "e" command to regist a new vehicle entry to a parking
 * lot.
 *
 * This function parses the input string to make a new entry regist in the
 * system with the provided parameters. It handles the validation of input
 * arguments and error checking before creating a new regist.
 *
 * @param s Input string containing the command and parameters.
 * @param parkingLotList Pointer to the list of parking lots.
 * @param vehicleList Pointer to the list of vehicles.
 * @param lastRegistDate Pointer to the last registration date.
 * @param hashTableVehicle Pointer to the hash table containing vehicles.
 */
void executeE(char* s, List* parkingLotList, List* vehicleList,
              char* lastRegistDate, HashTable* hashTableVehicle) {
    // Skip first space
    int currentIndex = 1;
    int day, month, year, hour, minute;
    char* name;
    char licencePlate[PLATE_SIZE];
    char dateEntry[LOG_FORMAT_SIZE];
    name = getName(s + currentIndex, &currentIndex);
    // Skip space
    currentIndex++;
    sscanf(s + currentIndex, "%s", licencePlate);
    // Skip licence plate size plus space
    currentIndex += PLATE_SIZE;
    sscanf(s + currentIndex, "%d-%d-%d %d:%d", &day, &month, &year, &hour,
           &minute);
    formatDate(day, month, year, hour, minute, dateEntry);
    ParkingLot* parkingLot = searchParkingLot(parkingLotList, name);
    Vehicle* vehicle = searchVehicle(hashTableVehicle, licencePlate);
    if (errorsE(name, licencePlate, dateEntry, lastRegistDate, parkingLot,
                 vehicle)) {
        free(name);
        return;
    }
    if (vehicle == NULL) {
        vehicle = createVehicle(licencePlate, parkingLot, dateEntry);
        addVehicleToList(vehicleList, vehicle);
        insertVehicleInHashTable(hashTableVehicle, vehicle);
    } else {
        vehicle->currentLog = createLog(dateEntry, parkingLot->name,
        vehicle->licencePlate);
        vehicle->currentParkingLot = parkingLot;
    }

    parkingLot->spotsAvailable--;
    strcpy(lastRegistDate, dateEntry);
    printf("%s %d\n", name, parkingLot->spotsAvailable);
    free(name);
}


/**
 * @brief Executes the "s" command to register a vehicle leaving the parking
 * lot.
 *
 * This function parses the input string to register the departure of a vehicle
 * from the parking lot with the provided parameters. It handles the validation
 * of input arguments and error checking before updating the vehicle's logs and
 * the parking lot's logs.
 *
 * @param s Input string containing the command and parameters.
 * @param parkingLotList Pointer to the list of parking lots.
 * @param lastRegistDate Pointer to the last registration date.
 * @param hashTableVehicle Pointer to the hash table containing vehicles.
 */
void executeS(char* s, List* parkingLotList, char* lastRegistDate,
              HashTable* hashTableVehicle) {
    // Skip first space
    int currentIndex = 1;
    int day, month, year, hour, minute;
    char* name;
    char licencePlate[PLATE_SIZE];
    char dateLeft[LOG_FORMAT_SIZE];
    name = getName(s + currentIndex, &currentIndex);
    // Skip space
    currentIndex++;
    sscanf(s + currentIndex, "%s", licencePlate);
    // Skip licence plate size plus space
    currentIndex += PLATE_SIZE;
    sscanf(s + currentIndex, "%d-%d-%d %d:%d", &day, &month, &year, &hour,
           &minute);
    formatDate(day, month, year, hour, minute, dateLeft);
    ParkingLot* parkingLot = searchParkingLot(parkingLotList, name);
    Vehicle* vehicle = searchVehicle(hashTableVehicle, licencePlate);
    if (errorsS(name, licencePlate, dateLeft, lastRegistDate, parkingLot,
                vehicle)) {
        free(name);
        return;
    }
    Log* log = completeLog(vehicle->currentLog, dateLeft, parkingLot->first15,
                          parkingLot->next15, parkingLot->maxDay);
    vehicle->currentLog = NULL;
    vehicle->currentParkingLot = NULL;
    addLogToVehicleLogList(vehicle->selfLogList, log);
    addLogToParkingLogList(parkingLot->selfLogList, log);
    parkingLot->spotsAvailable++;

    printf("%s ", licencePlate);
    writeDate(log->dateEntry);
    printf(" ");
    writeDate(dateLeft);
    printf(" %.2f\n", log->value);
    free(name);
}


/**
 * @brief Completes the log entry for a vehicle's departure from the parking lot.
 *
 * This function updates the departure date and calculates the fee for the
 * vehicle's parking based on the entry and exit times and the parking rates.
 *
 * @param currentLog Pointer to the log entry to be completed.
 * @param dateLeft The date and time when the vehicle left the parking lot.
 * @param first15 The cost for the first 15 minutes of parking.
 * @param next15 The cost for every 15 minutes of parking after the first hour.
 * @param maxDay The maximum fee for a day of parking.
 * @return A pointer to the completed log entry.
 */
Log* completeLog(Log* currentLog, char* dateLeft, float first15, float next15,
                 float maxDay) {
    // Update departure date
    strcpy(currentLog->dateLeft, dateLeft);
    
    // Calculate parking fee
    currentLog->value = faturationValue(currentLog->dateEntry, dateLeft,
     first15, next15, maxDay);

    return currentLog;
}


/**
 * @brief Checks for errors in registering a new vehicle entry.
 *
 * This function checks for various error conditions that may occur when
 * attempting to register a new vehicle entry into a parking lot. It verifies
 * the existence of the parking lot, availability of parking spots, validity of
 * the license plate, whether the vehicle is already parked, and if the date
 * format is valid and after the last system registration.
 *
 * @param name Name of the parking lot.
 * @param licencePlate License plate of the vehicle.
 * @param dateEntry Date and time of the vehicle entry.
 * @param lastRegistDate Date and time of the last system registration.
 * @param parkingLot Pointer to the parking lot where the vehicle is to be
 * parked.
 * @param vehicle Pointer to the vehicle.
 * @return An integer indicating whether errors were found (1) or not (0).
 */
int errorsE(char* name, char* licencePlate, char* dateEntry,
            char* lastRegistDate, ParkingLot* parkingLot, Vehicle* vehicle) {
    // Verification to see if parking lot exists
    if (parkingLot == NULL) {
        printf("%s: no such parking.\n", name);
        return 1; // Return error indicator
    }

    // Verify if there are spots available inside the parking lot
    else if (parkingLot->spotsAvailable <= 0) {
        printf("%s: parking is full.\n", name);
        return 1; // Return error indicator
    }

    // Verify if license plate is valid
    else if (!validlicencePlate(licencePlate)) {
        printf("%s: invalid licence plate.\n", licencePlate);
        return 1; // Return error indicator
    }

    // Verify if vehicle is already inside a parking lot
    else if (vehicle != NULL && vehicle->currentParkingLot != NULL) {
        printf("%s: invalid vehicle entry.\n", licencePlate);
        return 1; // Return error indicator
    }

    // Verify if date format is valid and if it's after the last system
    // registration
    else if (dateValidation(dateEntry, lastRegistDate)) {
        printf("invalid date.\n");
        return 1; // Return error indicator
    }
    
    return 0; // No errors found
}


/**
 * @brief Checks for errors in registering a vehicle exit from a parking lot.
 *
 * This function checks for various error conditions that may occur when
 * attempting to register a vehicle exit from a parking lot. It verifies
 * the existence of the parking lot, the validity of the license plate,
 * whether the vehicle is already outside the parking lot or if it's in
 * the wrong one, and if the date format is valid and after the last system
 * registration.
 *
 * @param name Name of the parking lot.
 * @param licencePlate License plate of the vehicle.
 * @param dateLeft Date and time when the vehicle left the parking lot.
 * @param lastRegistDate Date and time of the last system registration.
 * @param parkingLot Pointer to the parking lot from which the vehicle is
 * exiting.
 * @param vehicle Pointer to the vehicle.
 * @return An integer indicating whether errors were found (1) or not (0).
 */
int errorsS(char* name, char* licencePlate, char* dateLeft,
            char* lastRegistDate, ParkingLot* parkingLot, Vehicle* vehicle) {
    // Verification to see if parking lot exists
    if (parkingLot == NULL) {
        printf("%s: no such parking.\n", name);
        return 1; // Return error indicator
    }

    // Verify if license plate is valid
    else if (!validlicencePlate(licencePlate)) {
        printf("%s: invalid licence plate.\n", licencePlate);
        return 1; // Return error indicator
    }

    // Verify if vehicle is already outside a parking lot or if it's in the 
    // wrong one
    else if (vehicle == NULL || vehicle->currentParkingLot == NULL ||
             strcmp(vehicle->currentParkingLot->name, name) != 0) {
        printf("%s: invalid vehicle exit.\n", licencePlate);
        return 1; // Return error indicator
    }

    // Verify if date format is valid and if it's after the last system
    // registration
    else if (dateValidation(dateLeft, lastRegistDate)) {
        printf("invalid date.\n");
        return 1; // Return error indicator
    }
    
    return 0; // No errors found
}

/**
 * @brief Initializes a hash table for storing vehicle records.
 *
 * This function initializes a hash table for storing vehicle records.
 *
 * @return A pointer to the initialized hash table.
 */
HashTable* initializeHashTable() {
    HashTable* newHashTable = (HashTable*)malloc(sizeof(HashTable));
    if (newHashTable == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE); // Terminate the program if memory allocation fails
    }

    // Initialize each bucket to NULL
    for (int i = 0; i < HASHTABLE_SIZE; i++) {
        newHashTable->table[i] = NULL;
    }

    return newHashTable;
}




/**
 * @brief Hashes a license plate to an index in the hash table.
 *
 * This function calculates the hash value for a given license plate
 * and maps it to an index in the hash table based on the table size.
 *
 * @param licencePlate The license plate to be hashed.
 * @param tableSize The size of the hash table.
 * @return The hashed index for the license plate.
 */
int hash(char* licencePlate, int tableSize) {
    int i = 0, sum = 0;
    
    // Calculate the sum of ASCII values of characters in the license plate
    while (licencePlate[i] != '\0') {
        sum += licencePlate[i];
        i++;
    }

    // Return the hashed index by taking the remainder when divided by the table
    // size
    return sum % tableSize;
}


/**
 * @brief Inserts a vehicle into the hash table.
 *
 * This function calculates the hash value for the vehicle's license plate,
 * creates a new node for the vehicle, and inserts it into the appropriate
 * bucket of the hash table.
 *
 * @param hashTable Pointer to the hash table.
 * @param vehicle Pointer to the vehicle to be inserted.
 */
void insertVehicleInHashTable(HashTable* hashTable, Vehicle* vehicle) {
    // Calculate the hash value for the license plate
    int hashValue = hash(vehicle->licencePlate, HASHTABLE_SIZE);

    // Create a new node for the vehicle
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE); // Terminate the program if memory allocation fails
    }

    newNode->content = (Vehicle*)vehicle;
    newNode->next = NULL;
    newNode->key = vehicle->licencePlate;

    if (hashTable->table[hashValue] == NULL) {
        // If the bucket is empty, just assign the new node
        hashTable->table[hashValue] = newNode;
    } else {
        // If the bucket is not empty, append the new node to the end of the
        // list
        Node* current = hashTable->table[hashValue];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}


/**
 * @brief Searches for a vehicle in the hash table.
 *
 * This function calculates the hash value for the given license plate,
 * traverses the linked list in the corresponding bucket of the hash table,
 * and searches for a vehicle with the matching license plate.
 *
 * @param hashTable Pointer to the hash table.
 * @param licencePlate The license plate to search for.
 * @return A pointer to the vehicle with the matching license plate if found, otherwise NULL.
 */
Vehicle* searchVehicle(HashTable* hashTable, char* licencePlate) {
    // Calculate the hash value for the license plate
    int hashValue = hash(licencePlate, HASHTABLE_SIZE);

    // Traverse the linked list in the corresponding bucket
    Node* current = hashTable->table[hashValue];
    while (current != NULL) {
        Vehicle* currentVehicle = (Vehicle*)current->content;
        if (strcmp(current->key, licencePlate) == 0) {
            // Found the vehicle with the matching license plate
            return currentVehicle;
        }
        current = current->next;
    }

    // License plate not found in the hash table
    return NULL;
}

/**
 * @brief Creates a new vehicle.
 *
 * This function allocates memory for a new vehicle, initializes its fields
 * with the provided parameters, and returns a pointer to the newly created
 * vehicle.
 *
 * @param licencePlate The license plate of the new vehicle.
 * @param parkingLot Pointer to the parking lot where the vehicle is parked.
 * @param dateEntry The date and time when the vehicle entered the parking lot.
 * @return A pointer to the newly created vehicle.
 */
Vehicle* createVehicle(char* licencePlate, ParkingLot* parkingLot,
                        char* dateEntry) {
    Vehicle* newVehicle = malloc(sizeof(Vehicle));

    if (newVehicle == NULL) {
        // Deal with memory allocation failure and exit with status 1
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Initialize vehicle fields
    strcpy(newVehicle->licencePlate, licencePlate);
    newVehicle->currentParkingLot = parkingLot;
    newVehicle->selfLogList = initializeList();
    newVehicle->currentLog = createLog(dateEntry, parkingLot->name,
     newVehicle->licencePlate);
    
    return newVehicle;
}

/**
 * @brief Creates a new log entry.
 *
 * This function allocates memory for a new log entry, initializes its fields
 * with the provided parameters, and returns a pointer to the newly created log
 * entry.
 *
 * @param dateEntry The date and time when the vehicle entered the parking lot.
 * @param parkName The name of the parking lot where the vehicle is parked.
 * @return A pointer to the newly created log entry.
 */
Log* createLog(char* dateEntry, char* parkName, char* licensePlate) {
    Log* newLog = malloc(sizeof(Log));

    if (newLog == NULL) {
        // Deal with memory allocation failure and exit with status 1
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Initialize log entry fields
    strcpy(newLog->dateEntry, dateEntry);
    strcpy(newLog->dateLeft,"");
    newLog->parkName = parkName;
    newLog->licencePlate = licensePlate;
    newLog->value = 0;

    return newLog;
}

/**
 * @brief Adds a vehicle to the list of vehicles.
 *
 * This function creates a new node to hold the vehicle, adds it to the end
 * of the vehicle list, and updates the list's head if necessary.
 *
 * @param vehicleList Pointer to the list of vehicles.
 * @param vehicle Pointer to the vehicle to be added to the list.
 */
void addVehicleToList(List* vehicleList, Vehicle* vehicle) {
    // Allocate memory for new node, verify its allocation, and initialize node
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Set the key and content fields of the new node
    newNode->key = vehicle->licencePlate; 
    newNode->content = vehicle;
    newNode->next = NULL;
    
    // Place the new node at the end of the list
    if (vehicleList->head == NULL) {
        // If the list is empty, set the new node as the head
        vehicleList->head = newNode;
    } else {
        // If the list is not empty, find the last node
        Node* current = vehicleList->head;
        while (current->next != NULL) {
            current = current->next;
        }
        // Link the new node to the last node
        current->next = newNode;
    }
}

/**
 * @brief Frees the memory allocated for a list of vehicles.
 *
 * This function traverses the list of vehicles, frees the memory allocated for
 * each node, and finally frees the memory allocated for the list itself.
 *
 * @param vehicleList Pointer to the list of vehicles to be freed.
 */
void freeVehicleList(List* vehicleList) {
    Node* current = vehicleList->head;
    Node* temp;

    // Traverse the list and free memory for each node
    while (current != NULL) {
        // Move to the next node and free the current node
        temp = current;
        current = current->next;
        free(temp);
    }

    // Finally, free the memory allocated for the list itself
    free(vehicleList);
}


/**
 * @brief Frees the memory allocated for a hash table of vehicles.
 *
 * This function traverses the hash table, frees the memory allocated for
 * each node and vehicle, including their logs, and finally frees the memory
 * allocated for the hash table itself.
 *
 * @param hashTable Pointer to the hash table of vehicles to be freed.
 */
void freeHashTable(HashTable* hashTable) {
    // Traverse the hash table
    for (int i = 0; i < HASHTABLE_SIZE; i++) {
        Node* current = hashTable->table[i];
        while (current != NULL) {
            Node* temp = current; // Store current node to free it later
            Vehicle* vehicle = (Vehicle*)current->content;
            current = current->next; // Move to next node

            // Free the vehicle's log list
            freeLogList(vehicle->selfLogList);
            
            // Free the current log if it exists
            if (vehicle->currentLog != NULL) {
                free(vehicle->currentLog);
            }

            // Free the vehicle itself
            free(vehicle);

            // Free the node
            free(temp);
        }
    }
    // Free the hash table itself
    free(hashTable);
}

/**
 * @brief Frees the memory allocated for a list of log entries.
 *
 * This function traverses the list of log entries, frees the memory allocated
 * for each log entry, and finally frees the memory allocated for the list
 * itself.
 *
 * @param logList Pointer to the list of log entries to be freed.
 */
void freeLogList(List* logList) {
    Node* current = logList->head;
    Node* temp;

    // Traverse the list and free memory for each log entry
    while (current != NULL) {
        // Get the current log entry and free its memory
        Log* log = (Log*)current->content;
        free(log);

        // Move to the next node and free the current node
        temp = current;
        current = current->next;
        free(temp);
    }

    // Finally, free the memory allocated for the list itself
    free(logList);
}

/**
 * @brief Calculates the fee for parking based on the duration of stay.
 *
 * This function calculates the fee for parking based on the duration of stay
 * between two given dates. It takes into account the pricing structure for
 * the first 15 minutes, the subsequent 15-minute blocks, and the maximum
 * daily fee.
 *
 * @param date1 The earlier date and time.
 * @param date2 The later date and time.
 * @param first15 The fee for the first 15 minutes.
 * @param next15 The fee for each subsequent 15-minute block.
 * @param maxDay The maximum daily fee.
 * @return The calculated fee for parking.
 */
float faturationValue(char* date1, char* date2, float first15, float next15,
 float maxDay) {
    float value = 0;
    long minutesPassed = 0;

    // Calculate the total number of minutes passed between the two dates
    minutesPassed = numberOfMinutes(date1) - numberOfMinutes(date2);
    minutesPassed *= ((minutesPassed < 0) ? -1 : 1); // Ensure a positive value

    // Calculate the number of days and remaining minutes
    int days = minutesPassed / MINUTES_IN_DAY;
    int minutesLeft = minutesPassed % MINUTES_IN_DAY;
    int blocksLeft = minutesLeft / MINUTES_IN_BLOCK;

    // Adjust the number of blocks left if there are remaining minutes
    if (minutesLeft % MINUTES_IN_BLOCK != 0) {
        blocksLeft++;
    }

    // Calculate the fee based on the pricing structure
    float comparisonValue = 0;
    if (blocksLeft <= BLOCKS_IN_HOUR) {
        comparisonValue += blocksLeft * first15;
    } else {
        comparisonValue += BLOCKS_IN_HOUR * first15 +
                            (blocksLeft - BLOCKS_IN_HOUR) * next15;
    }

    // Compare with the maximum daily fee and add to the total fee
    if (comparisonValue > maxDay) {
        value += maxDay;
    } else {
        value += comparisonValue;
    }

    // Add the fee for each day and return the total fee
    value += days * maxDay;
    return value;
}

/**
 * @brief Calculates the total number of minutes from a given date.
 *
 * This function calculates the total number of minutes elapsed from the year 0
 * up to the given date.
 *
 * @param date The date and time in the format YYYYMMDDHHMM.
 * @return The total number of minutes elapsed from the year 0 up to the given
 * date.
 */
long numberOfMinutes(char* date) {
    int year, month, day, hour, minute;
    long totalMinutes = 0;

    // Parse the date string to extract year, month, day, hour, and minute
    sscanf(date, "%4d%2d%2d%2d%2d", &year, &month, &day, &hour, &minute);

    // Calculate the total number of minutes
    totalMinutes += year * MINUTES_IN_YEAR + (daysUpToMonth(month) + day)
     * MINUTES_IN_DAY + hour * MINUTES_IN_HOUR + minute;

    return totalMinutes;
}

/**
 * @brief Adds a log entry to the list of vehicle logs.
 *
 * This function adds a new log entry to the list of vehicle logs, maintaining
 * the order of logs based on the park name.
 *
 * @param logList Pointer to the list of vehicle logs.
 * @param log Pointer to the log entry to be added to the list.
 */
void addLogToVehicleLogList(List* logList, Log* log) {
    // Allocate memory for a new node
    Node* newNode = (Node*)malloc(sizeof(Node));

    // Check for memory allocation failure
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the new node with log information
    newNode->content = (Log*)log;
    newNode->key = log->parkName;
    newNode->next = NULL;

    // If the list is empty, insert the new node as the head
    if (logList->head == NULL) {
        logList->head = newNode;
        return;
    }

    // Traverse the list to find the right position to insert the node
    Node* current = logList->head;
    Node* prev = NULL;
    while (current != NULL && strcmp(current->key, newNode->key) <= 0) {
        prev = current;
        current = current->next;
    }

    // Insert newNode before the first node with a different park name
    if (prev == NULL) {
        newNode->next = logList->head;
        logList->head = newNode;
    } else {
        prev->next = newNode;
        newNode->next = current;
    } 
}

/**
 * @brief Adds a log entry to the list of parking logs.
 *
 * This function adds a new log entry to the list of parking logs, inserting
 * it at the end of the list.
 *
 * @param logList Pointer to the list of parking logs.
 * @param log Pointer to the log entry to be added to the list.
 */
void addLogToParkingLogList(List* logList, Log* log) {
    // Allocate memory for the new node
    Node* newNode = (Node*)malloc(sizeof(Node));

    // Check for memory allocation failure
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the new node
    newNode->content = (Log*)log;
    newNode->key = log->dateLeft;
    newNode->next = NULL;

    // If the list is empty, insert the new node as the head
    if (logList->head == NULL) {
        logList->head = newNode;
        return;
    }

    // Traverse to the end of the list
    Node* current = logList->head;
    while (current->next != NULL) {
        current = current->next;
    }

    // Insert the new node at the end of the list
    current->next = newNode;
}

/**
 * @brief Executes the "v" command to display vehicle logs.
 *
 * This function searches for the vehicle with the given license plate in the
 * hash table and prints its log entries. If a vehicle is found, its log
 * entries are printed in chronological order, with the current log printed
 * first if it exists.
 *
 * @param s Input string containing the command and parameters.
 * @param hashTable Pointer to the hash table containing vehicles.
 */
void executeV(char* s, HashTable* hashTable) {
    char licencePlate[PLATE_SIZE];
    sscanf(s, " %s", licencePlate);

    Vehicle* vehicle = searchVehicle(hashTable, licencePlate);

    if (errorsV(vehicle, licencePlate)) {
        return;
    }

    printVehicleLogs(vehicle);
}

/**
 * @brief Prints the log entries of a vehicle in chronological order.
 *
 * This function prints the log entries of a vehicle in chronological order,
 * starting with the current log entry (if it exists) followed by the remaining
 * log entries.
 *
 * @param vehicle Pointer to the vehicle whose log entries are to be printed.
 */
void printVehicleLogs(Vehicle* vehicle) {
    if (vehicle->currentLog == NULL) {
        printLogs(vehicle->selfLogList->head);
    } else {
        Node* current = vehicle->selfLogList->head;
        printLogsUntilCurrent(&current, vehicle->currentLog->parkName);

        printf("%s ", vehicle->currentLog->parkName);
        writeDate(vehicle->currentLog->dateEntry);
        printf("\n");

        printLogs(current);
    }
}


/**
 * @brief Prints log entries until the current log entry is reached.
 *
 * This function prints log entries from the list until the current log entry
 * is reached, based on the park name comparison.
 *
 * @param current Pointer to the pointer to the current node in the log list.
 * @param parkName Park name of the current log entry.
 */
void printLogsUntilCurrent(Node** current, char* parkName) {
    while (*current != NULL && strcmp((*current)->key, parkName) <= 0) {
        Log* log = (Log*)(*current)->content;
        printV(log->parkName, log->dateEntry, log->dateLeft);
        *current = (*current)->next;
    }
}


/**
 * @brief Prints log entries after the current log entry is reached.
 *
 * This function prints log entries from the list after the current log entry
 * is reached, until the end of the list.
 *
 * @param current Pointer to the current node in the log list.
 */
void printLogs(Node* current) {
    while (current != NULL) {
        Log* log = (Log*)current->content;
        printV(log->parkName, log->dateEntry, log->dateLeft);
        current = current->next;
    }
}


void printV(char* parkName, char* dateEntry, char* dateLeft){
    printf("%s ", parkName); writeDate(dateEntry); printf(" ");
    writeDate(dateLeft);printf("\n");
}

int errorsV(Vehicle* vehicle, char* licencePlate){

    if (!validlicencePlate(licencePlate)){
        printf("%s: invalid licence plate.\n", licencePlate);
        return 1; // Return error indicator
    }

    else if (vehicle == NULL || (vehicle->selfLogList->head == NULL &&
     vehicle->currentLog == NULL)){
        printf("%s: no entries found in any parking.\n", licencePlate);
        return 1; // Return error indicator
    }

    return 0; // No errors found
}

void executeF(char* s, List* parkingLotlist, char* lastRegistDate){
    // Skip first space
    int currentIndex = 1; 
    char* name;
    char date[DATE_ONLY_SIZE] = "";
    name = getName(s + currentIndex, &currentIndex);

    if (s[currentIndex] != '\0'){
        currentIndex++;
        int day, month, year;
        sscanf(s + currentIndex, "%d-%d-%d", &day, &month, &year);
        formatDateOnly(day, month, year, date);
    }
    ParkingLot* parkingLot = searchParkingLot(parkingLotlist, name);

    if (errorsF(name, parkingLot, date, lastRegistDate)){
        free(name);
        return;
    }
    if (strcmp(date, "") == 0){
        printParkFaturation(parkingLot);
    }
    else{
        printDateFaturation(parkingLot, date);
    }

    free(name);
}


int errorsF(char* name, ParkingLot* parkingLot, char* date,
 char* lastRegistDate){
    if (parkingLot == NULL){
        printf("%s: no such parking.\n", name);
        return 1; // Return error indicator
    }
    // Verify if date format is valid and if it's after the last system
    // registration
    if (strcmp(date, "") != 0 && dateOnlyValidation(date, lastRegistDate)) {
        printf("invalid date.\n");
        return 1; // Return error indicator
    }

    return 0; // No errors found
}

void printParkFaturation(ParkingLot* parkingLot){
    Node* current = parkingLot->selfLogList->head;
    if (current == NULL){
        return;
    }
    char* currentDate = current->key;  // Get the current date
    while (current != NULL){
        float dateValue = 0;
        while (current != NULL && strncmp(current->key, currentDate,
         DATE_ONLY_SIZE - 1) == 0){
            Log* log = (Log*)current->content;
            dateValue += log->value;
            current = current->next;
        }
        if (dateValue > 0){
            writeDateOnly(currentDate);
            printf(" %.2f\n", dateValue);
        }
        if (current != NULL) {
            currentDate = current->key; // Update currentDate only if current is not NULL
        }
    }
}



void printDateFaturation(ParkingLot* parkingLot, char* date){
    Node* current = parkingLot->selfLogList->head;
    if (current == NULL){
        return;
    }
    while (current != NULL && strncmp(current->key, date,
     DATE_ONLY_SIZE - 1) != 0){
        current = current->next;
    }
    while(current != NULL && strncmp(current->key,
     date, DATE_ONLY_SIZE - 1) == 0){
        Log* log = (Log*)current->content;
        printf("%s ", log->licencePlate);
        writeTimeOnly(log->dateLeft);
        printf(" %.2f\n", log->value);
        current = current->next;
    }
}

void formatDateOnly(int day, int month, int year, char* date){
    // Format the date components with leading zeros if necessary
    sprintf(date, "%04d%02d%02d", year, month, day);

}

void writeDateOnly(char* date){
    int day, month, year;

    // Parse the date string to extract day, month and year
    sscanf(date, "%4d%2d%2d", &year, &month, &day);

    // Print the parsed date components in the desired format
    printf("%02d-%02d-%04d", day, month, year);
}

int dateOnlyValidation(char* date, char* lastDate){
    int year, month, day;

    // Extract year, month, day, hour, and minute from the date string
    sscanf(date, "%4d%2d%2d", &year, &month, &day);
    
    // Check if month is out of range or day is out of range for that month
    // or if year is negative
    if (month < JAN || month > DEC || day < 1 || day > daysInMonth(month) ||
     year < 0) {
        return 1; // Date is invalid
    }
    // Check if the date is before or equal to the 
    if (strncmp(date, lastDate, DATE_ONLY_SIZE - 1) > 0) {
        return 1; // Date is invalid
    }

    return 0; // Date is valid
}

void writeTimeOnly(char* date){
        int hour, minute;

    // Parse the date string to extract day, month and year
    sscanf(date + DATE_TIME_JUMP, "%2d%2d", &hour, &minute);

    // Print the parsed date components in the desired format
    printf("%02d:%02d", hour, minute);

}


