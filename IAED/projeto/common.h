#ifndef COMMON_H
#define COMMON_H

#define OUT 0
#define IN 1
#define ON 0
#define OFF 1
#define PLATE_SIZE 9
#define DATE_SIZE 11
#define TIME_SIZE 6
#define LOG_FORMAT_SIZE 13
#define DATE_ONLY_SIZE 9
#define DATE_TIME_JUMP 8
#define PAIR_SIZE 3
#define MAX_PARKING_LOTS 20
#define BUFSIZ 8192
#define BLOCKS_IN_HOUR 4
#define MINUTES_IN_BLOCK 15
#define MINUTES_IN_HOUR 60
#define MINUTES_IN_DAY 1440
#define MINUTES_IN_YEAR 525600    // Assuming years have 365 days
#define HOURS_IN_DAY 24
#define HASHTABLE_SIZE 5001


enum commands {
    COMMAND_Q = 'q',
    COMMAND_P = 'p',
    COMMAND_E = 'e',
    COMMAND_S = 's',
    COMMAND_V = 'v',
    COMMAND_F = 'f',
    COMMAND_R = 'r'
};

enum months{
    JAN = 1,
    FEB = 2,
    MAR = 3,
    APR = 4,
    MAY = 5,
    JUN = 6,
    JUL = 7,
    AUG = 8,
    SEP = 9,
    OCT = 10,
    NOV = 11,
    DEC = 12,
};

typedef struct{
    int day, month, year;
} Date;

typedef struct{
    int hour;
    int minutes;
} Time;

typedef struct{
    char dateEntry[LOG_FORMAT_SIZE];
    char dateLeft[LOG_FORMAT_SIZE];
    char* parkName;
    char* licencePlate;
    float value;
} Log;


typedef struct Node Node;

struct Node{
    Node* next;
    void* content;
    char* key;
};

typedef struct{
    Node* head;
    int elements;
} List;

typedef struct {
    char* name;
    int capacity, spotsAvailable;
    float first15, next15, maxDay;
    List* selfLogList;
} ParkingLot;

typedef struct{
    char licencePlate[PLATE_SIZE];
    List* selfLogList;
    Log* currentLog;
    ParkingLot* currentParkingLot; 
} Vehicle;

typedef struct {
    Node* table[HASHTABLE_SIZE];
} HashTable;



#endif  /* COMMON_H */
