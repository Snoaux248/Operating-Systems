//
//  main.c
//  OperatingSystemsProject
//  Due 05/06/2026
//  Created by Nolan Jones on 5/1/26.
//  Project code to run the Elevator Operating System Scheduler ya know big brain stuff
//

#include "main.h"

char *buildingConfiguration = NULL;

int numElevators = 0;
ElevatorNode *elevators[20] = {NULL};

PersonNode *personHead = NULL;
PersonNode *personTail = NULL;

PersonNode *dispatchHead = NULL;
PersonNode *dispatchTail = NULL;

/** Check for person insertion to determine longer travel distance
 
 */
int PersonNode_LinkedList_insertCheck(PersonNode *A, PersonNode *B){
    int diff_A = abs(A->startFloor - A->endFloor);
    int diff_B = abs(B->startFloor - B->endFloor);
    return diff_A <= diff_B;
}

/** Method to insert a PersonNode into a Queue of type PersonNode
    Longest Travel distance takes priority
*/
void PersonNode_LinkedList_insert(PersonNode **root, PersonNode **tail, PersonNode *to_insert){
    to_insert->next = NULL; //clear just incase
    
    if(*root == NULL){ // head insert when null
        *root = to_insert;
        *tail = to_insert; // didnt end up using tail whatsoever
        return;
    }
    
    PersonNode *temp = *root; // head insert when not null
    if(!PersonNode_LinkedList_insertCheck(to_insert, *root)){
        to_insert->next = *root;
        *root = to_insert;
        return;
    }
    
    // traversal
    while(temp->next != NULL && PersonNode_LinkedList_insertCheck(to_insert, temp->next)){
        temp = temp->next;
    }
    
    to_insert->next = temp->next; // actual insertion
    temp->next = to_insert;
    if(temp->next == NULL){ // update tail // didnt end up using tail whatsoever
        *tail = to_insert;
    }
}

/** Method to pop a PersonNode from a Queue of type PersonNode
 
*/
PersonNode* PersonNode_pop(PersonNode **head, PersonNode **tail){
    if(*head == NULL){ // if is empty
        return NULL;
    }

    PersonNode *temp = *head; //basic removal and updateing of head
    *head = (*head)->next;
    temp->next = NULL;

    if(*head == NULL){
        *tail = NULL; //sets tail to null // didnt end up using tail whatsoever
    }
    
    return temp;
}

/** Method to create a new ElevatorNode
 
*/
ElevatorNode* new_ElevatorNode(char *bayId, int lowestFloor, int highestFloor, int currentFloor, int totalCapacity){
    ElevatorNode *temp = (ElevatorNode*) calloc(1, sizeof(ElevatorNode)); // allocates zeroed out memory
    if(temp == NULL){ // checks for failure
        printf("Failed to allocate space for queue node\n");
        exit(3);
    }
    // transfers correct initialization values to node
    temp->bayId = copymem(bayId, get_string_length(bayId));
    temp->currentFloor = currentFloor;
    temp->lowestFloor = lowestFloor;
    temp->highestFloor = highestFloor;
    temp->direction = 'S';
    temp->passengerCount = 0;
    temp->remainingCapacity = totalCapacity;
    return temp;
}

/** Method to create a new PersonNode
 
*/
PersonNode* new_PersonNode(char *personId, int startFloor, int endFloor){
    PersonNode *temp = (PersonNode*) calloc(1, sizeof(PersonNode)); // allocates zeroed out memory
    if(temp == NULL){ // checks for failure
        printf("Failed to allocate space for queue node\n");
        exit(3);
    }
    // transfers correct initialization values to node
    temp->personId = copymem(personId, get_string_length(personId));
    temp->startFloor = startFloor;
    temp->endFloor = endFloor;
    temp->elevatorCast = -1;
    temp->next = NULL;
    return temp;
}

/** Method to display elevator status
    
 */
void display_ElevatorNode(ElevatorNode *temp){
    //printf("\nBayId\t        LF\tHF\tCF\tD\tPC\tRC\n");
    printf("%s\t%d\t%d\t%d\t%c\t%d\t%d\n", temp->bayId, temp->lowestFloor, temp->highestFloor, temp->currentFloor, temp->direction, temp->passengerCount, temp->remainingCapacity);
}

/** Method to read origional building configuration
    from *argv[1]
 */
void read_buildingConfiguration(char *path){
    FILE *inFS = fopen(path, "r");
    if(inFS == NULL){
        printf("Could Not read file %s: \n", path);
    }
    //temp variables for read
    char bayId[100] = {0};
    int lowestFloor = 0;
    int highestFloor = 0;
    int currentFloor = 0;
    int totalCapacity = 0;
    printf("\nBayId\t        LF\tHF\tCF\tD\tPC\tRC\n"); // column headers w/ formatiing
    // scans file input while available
    while(fscanf(inFS, "%s %d %d %d %d[^\n]", bayId, &lowestFloor, &highestFloor, &currentFloor, &totalCapacity) == 5){
        ElevatorNode *temp = new_ElevatorNode(bayId, lowestFloor, highestFloor, currentFloor, totalCapacity); //makes node
        display_ElevatorNode(temp); // displays node
        elevators[numElevators] = temp; // stores
        numElevators++;
    }
    printf("\n");
}
/** Main Method
 
 */
int main(int argc, const char **argv) {
    if(argc <= 2){
        printf("Please provide building configureation and port as command line argument\n");
    }
    printf("Nolan Jones\n");
    printf("R11789850\n");
    
    // Check if building eleevator configuration file exists
    buildingConfiguration = (char*) argv[1];
    if(buildingConfiguration == NULL){
        printf("Building Configuration is not correct\n");
        exit(1);
    }
    // Read the initial building configuration file
    read_buildingConfiguration(buildingConfiguration);
    
    // Check if the port exists
    port = (char*) argv[2];
    if(port == NULL){
        printf("Port is not correct");
        exit(1);
    }
    // Builds server/port string
    serverPort = concatenate(server, port);
    printf("Server and Port: %s\n", serverPort);
    
    // Starts Simulation
    simulationStart();
    for(int i = 0; i < numElevators; i++){
        elevatorStatus(elevators[i]->bayId);
    }
    // Initializes Mutexes ConditionalVariables and Threads
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    
    pthread_cond_init(&cond1, NULL);
    pthread_cond_init(&cond2, NULL);
    
    pthread_t requestThread;
    pthread_t computeThread;
    pthread_t dispatchThread;
    
    // Creates threads and assigns methods
    pthread_create(&requestThread, NULL, request, NULL);
    pthread_create(&computeThread, NULL, compute, NULL);
    pthread_create(&dispatchThread, NULL, dispatch, NULL);
    
    // Joins threads while it waits for completion // lowkey unesessary cause I just EXIT upon completion
    pthread_join(requestThread, NULL);
    pthread_join(computeThread, NULL);
    pthread_join(dispatchThread, NULL);
    
    return 0;
}
