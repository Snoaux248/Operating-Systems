//
//  main.h
//  OperatingSystemsProject
//  Due 05/06/2026
//  Created by Nolan Jones on 5/1/26.
//

#ifndef main_h
#define main_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <stdbool.h>

#include <curl/curl.h>
#include <ctype.h>

// stores API response
struct Response {
    char* buffer;
    size_t size;
};

// Stores Elevator Info;
typedef struct elevator{
    char *bayId;
    int currentFloor;
    int lowestFloor;
    int highestFloor;
    char direction;
    int passengerCount;
    int remainingCapacity;
}ElevatorNode;

// Elevator Queue Node
typedef struct person{
    char *personId;
    int startFloor;
    int endFloor;
    int elevatorCast;
    struct person *next;
}PersonNode;


// resides in API;
extern char *server;
extern char *serverPort;
extern char *port;
extern char *paths[];

static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
void init_put(char *serverAddress, char *sessionKey, const char *data);
void init_get(char *serverAddress, char *sessionKey, char **data);
char* copymem(char *word, unsigned long length);
char* concatenate(char *s1, char *s2);
size_t get_string_length(const char *string);

int find_elevator(char *bayId);
void parse_elevator(char *result, char *elevatorId);
void parse_person(char *result);
void simulationStart(void);
void simulationStop(void);
int simulationCheck(void);
void elevatorStatus(char *elevatorID);
int nextInput(void);
void addPersonToElevator(char *personId, char *elevatorId);


// resides in threads;

extern pthread_mutex_t mutex1;
extern pthread_mutex_t mutex2;

extern pthread_cond_t cond1;
extern pthread_cond_t cond2;

void* request(void* arg);
void* compute(void* arg);
void* dispatch(void* arg);


//resides in main
extern char *buildingConfiguration;

extern int numElevators;
extern ElevatorNode *elevators[20];

extern PersonNode *personHead;
extern PersonNode *personTail;

extern PersonNode *dispatchHead;
extern PersonNode *dispatchTail;


void PersonNode_LinkedList_insert(PersonNode **root, PersonNode **tail, PersonNode *to_insert);
PersonNode* PersonNode_pop(PersonNode **head, PersonNode **tail);
ElevatorNode* new_ElevatorNode(char *bayId, int lowestFloor, int highestFloor, int currentFloor, int totalCapacity);
PersonNode* new_PersonNode(char *personId, int startFloor, int endFloor);
void display_ElevatorNode(ElevatorNode *temp);
void read_buildingConfiguration(char *path);




#endif /* main_h */
