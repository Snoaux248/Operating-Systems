/*
    Student: Nolan Jones
    Assignment: Assigment 5 - Scheduling
    Course: Operating Systems (CS 4352-001)
    Professor: Eric Reece
    Due: April 20th 2026
 
    This code takes input from a command line agument and then reads the argument as a file an executes all seven
    scheduling policies printing the ID of the process which operated in the last 10 quantums to the corresponding file
    
    Note this code uses speedups to skip 9 of the 10 quantums, and to quick travel if the queue is empty but it is known
    that data will arrive in the future.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// QueueNode structre
typedef struct q{
    char *processName;
    int arrivalTime;
    int serviceTime;
    float hrrnVal;
    int subQueue;
    struct q *next;
}QueueNode;

// queues
QueueNode *dataQueue = NULL;
QueueNode *queue = NULL;


/** Method to copy string memory
    used to remember process name
 */
char* copymem(char *word, int length){
    char *temp = (char*) calloc(length+1, sizeof(char));
    for(int i = 0; i < length; i++){
        temp[i] = word[i];
    }
    temp[length] = '\0';
    return temp;
}

/** Method to create a new QueueNode based upon data read by the read_file method

 */
QueueNode* new_QueueNode(char *id, int arrivalTime, int serviceTime){
    QueueNode *temp = (QueueNode*) calloc(1, sizeof(QueueNode));
    if(temp == NULL){
        printf("Failed to allocate space for queue node\n");
        exit(3);
    }
    temp->processName = copymem(id, strlen(id));
    temp->arrivalTime = arrivalTime;
    temp->serviceTime = serviceTime;
    temp->hrrnVal = 0;
    temp->subQueue = 0;
    temp->next = NULL;
    return temp;
}

/** Method to open files to write and perform error checking
    is used when actually running scheduling policies
 */
FILE *fileOpener(const char *path, const char *type){
    FILE *outFS = fopen(path, type);
    if(outFS == NULL){
        printf("failed to open file: %s \n", path);
        exit(4);
    }
    return outFS;
}

/** Method to facilitate input checks for
    DataQueue, First In First Out, and Highest Response Ration Next
 */
int dataQueue_check(QueueNode *newNode, QueueNode *next, int current_time){
    return newNode->arrivalTime >= next->arrivalTime;
}
/** Method to facilitate input checks for
    Round Robin Q10, and Round Robin Q40
 */
int roundRobin_check(QueueNode *newNode, QueueNode *next, int current_time){
    return next->arrivalTime <= current_time;
}
/** Method to facilitate input checks for
    Shortest Process Next, and Shortest Time Remaining
 */
int shortestRemainingTime_check(QueueNode *newNode, QueueNode *next, int current_time){
    return (next->arrivalTime <= current_time && next->serviceTime <= newNode->serviceTime);
}
/** Method to facilitate input checks for
    Feedback
 */
int feedback_check(QueueNode *newNode, QueueNode *next, int current_time){
    return (next->subQueue <= newNode->subQueue);
}

/** Generalized Queue insertion method
    admits the Queue, newNode, current_time, and which check to perform for insertion this is a singly linked list
 */
void QueueNode_general_insert(QueueNode **root, QueueNode *newNode, int currentTime, int (*check)(QueueNode*, QueueNode*, int)){
    newNode->next = NULL; // Double ensures that is free link
    
    if(*root == NULL){ // root is empty
        *root = newNode;
        return;
    }
    
    if(!check(newNode, *root, currentTime)){ // replaces head if applicable
        newNode->next = *root;
        *root = newNode;
        return;
    }
    
    QueueNode *temp = *root; // traverses linked list
    while(temp->next != NULL && check(newNode, temp->next, currentTime)){
        temp = temp->next;
    }
    
    newNode->next = temp->next; // actual insertion
    temp->next = newNode;
}

/** Method to dequeue "POP" from the given queue
    removes first node from the Queue
 */
QueueNode* QueueNode_pop(QueueNode **root){
    if(*root == NULL){ // if is empty
        return NULL;
    }
    QueueNode *temp = *root; //basic removal and updateing of head
    *root = (*root)->next;
    temp->next = NULL;
    return temp;
}

/** Method to dispatch nodes
    compares nodes in the data queue's arrival time to current time to determine if they need ot be dispatched and then dispatches
 */
void newArrivals(int current_time, int (*check)(QueueNode*, QueueNode*, int)){
    QueueNode *temp = dataQueue;
    while(temp != NULL && temp->arrivalTime <= current_time){ // checks head's time then pops and inserts if <= current time
        QueueNode *move = QueueNode_pop(&dataQueue);
        move->next = NULL;
        QueueNode_general_insert(&queue, move, current_time, check);
        temp = dataQueue; // reassigns to new head
    }
}

/** Method to read the provided command line argument input file
 
 */
void read_File(const char *path){
    FILE *inFS = fopen(path, "r");
    if(inFS == NULL){
        printf("Failed to open file %s\n", path);
        exit(2);
    }
    // temp variables
    char id[4];
    int arrivalTime;
    int serviceTime;
    
    // reading and generating
    while(fscanf(inFS, "%s %d %d\n", id, &arrivalTime, &serviceTime) == 3){
        QueueNode *newNode = new_QueueNode(id, arrivalTime/10, serviceTime/10); // quantum reduction
        QueueNode_general_insert(&dataQueue, newNode, 0, dataQueue_check);
    }

    fclose(inFS);
}

/** Method to determine the length of the Queue
    only necessary for bubble sorting Highest Response Ratio Next
 */
int QueueNode_length(QueueNode *root){
    int i = 1;
    while(root->next != NULL){
        root = root->next;
        i++;
    }
    return i;
}

/** Method to swap contents of nodes
    only necessary for bubble sorting Highest Response Ratio Next
 */
void QueueNode_swap(QueueNode *A, QueueNode *B){
    char *tempName = A->processName; // swaps processName's
    A->processName = B->processName;
    B->processName = tempName;
    
    int tempArrival = A->arrivalTime; // swaps arrivalTime's
    A->arrivalTime = B->arrivalTime;
    B->arrivalTime = tempArrival;
    
    int tempService = A->serviceTime; // swaps serviceTimes's
    A->serviceTime = B->serviceTime;
    B->serviceTime = tempService;

    float tempHrrn = A->hrrnVal; // swaps hrrnVal's
    A->hrrnVal = B->hrrnVal;
    B->hrrnVal = tempHrrn;
    
    int tempSubQueue = A->subQueue; // swaps subQueue's
    A->subQueue = B->subQueue;
    B->subQueue = tempSubQueue;
}

/** Method to bubbleSort the current Queue
    used after each recompute of Highest Response Ratio Next
 */
void QueueNode_HighestResponseRatioNext_resort(QueueNode **root){
    if(*root == NULL || (*root)->next == NULL){
        return;
    }
    int length = QueueNode_length(*root);
    
    // this is bubble sort so efficient
    for(int i = length - 1; i > -1; i--){
        QueueNode *current = *root;
        for(int j = 0; j < i && current->next != NULL; j++){
            if(current->hrrnVal < current->next->hrrnVal){
                QueueNode_swap(current, current->next);
            }
            current = current->next;
        }
    }
}

/** Method to recalculate the hrrnVal for each node
 
 */
void QueueNode_HighestResponseRatioNext_recompute(QueueNode *root, int current_time){
    QueueNode *temp = root;
    while(temp != NULL){
        temp->hrrnVal = (float)(current_time - temp->arrivalTime + temp->serviceTime) / temp->serviceTime; // compute ratio
        temp = temp->next;
    }
}

/** Method to perform the First Come First Serve Scheduler
    
 */
void firstComeFirstServe(){
    FILE *outFS = fileOpener("output/fcfs.out", "w");

    int current_time = 0;
    newArrivals(current_time, dataQueue_check);

    while(dataQueue != NULL || queue != NULL){
    
        if(queue == NULL){ // fast forward
            current_time = dataQueue->arrivalTime;
            newArrivals(current_time, dataQueue_check);
        }

        QueueNode *current = QueueNode_pop(&queue);
        
        for(int i = 0; i < current->serviceTime; i++){
            fprintf(outFS, "%s\n", current->processName);
            current_time++;
        }
        
        free(current->processName);
        free(current);
        newArrivals(current_time, dataQueue_check);
    }
}

/** Method to perform the Round Robin Scheduler
    
 */
void roundRobin(int intervel){
    FILE *outFS = fileOpener(intervel == 1 ? "output/rr_10.out": "output/rr_40.out", "w");

    int current_time = 0;
    newArrivals(current_time, roundRobin_check);

    while(dataQueue != NULL || queue != NULL){
        if(queue == NULL){ // fast forward
            current_time = dataQueue->arrivalTime;
            newArrivals(current_time, roundRobin_check);
        }

        QueueNode *current = QueueNode_pop(&queue);
        int inc = intervel > current->serviceTime ? current->serviceTime : intervel; //short circuit if serviceTime < quantum

        for(int i = 0; i < inc; i++){
            current->serviceTime--;
            fprintf(outFS, "%s\n", current->processName);
            current_time++;
        }

        newArrivals(current_time, roundRobin_check);
        
        // reinsert if applicable
        if(current->serviceTime != 0){
            QueueNode_general_insert(&queue, current, current_time, roundRobin_check);
        }else{
            free(current->processName);
            free(current);
        }
    }
}

/** Method to perform the Shortest Process Next Scheduler
    
 */
void shortestProcessNext(){
    FILE *outFS = fileOpener("output/spn.out", "w");

    int current_time = 0;
    newArrivals(current_time, shortestRemainingTime_check);

    while(dataQueue != NULL || queue != NULL){
        if(queue == NULL){ // fast forward
            current_time = dataQueue->arrivalTime;
            newArrivals(current_time, shortestRemainingTime_check);
        }

        QueueNode *current = QueueNode_pop(&queue);

        for(int i = 0; i < current->serviceTime; i++){
            fprintf(outFS, "%s\n", current->processName);
            current_time++;
        }
        
        free(current->processName);
        free(current);
        newArrivals(current_time, shortestRemainingTime_check);
    }
}

/** Method to perform the Shortest Remaining Time Scheduler
    
 */
void shortestRemainingTime(){
    FILE *outFS = fileOpener("output/srt.out", "w");

    int current_time = 0;
    newArrivals(current_time, shortestRemainingTime_check);

    while(dataQueue != NULL || queue != NULL){
        if(queue == NULL){ // fast forward
            current_time = dataQueue->arrivalTime;
            newArrivals(current_time, shortestRemainingTime_check);
        }

        QueueNode *current = QueueNode_pop(&queue);

        current->serviceTime--;
        fprintf(outFS, "%s\n", current->processName);
        current_time++;
        
        // reinsert if applicable
        if(current->serviceTime != 0){
            QueueNode_general_insert(&queue, current, current_time, shortestRemainingTime_check);
        }else{
            free(current->processName);
            free(current);
        }

        newArrivals(current_time, shortestRemainingTime_check);
    }
}

/** Method to perform the Highest Response Ratio Next Scheduler
    
 */
void highestResponseRatioNext(){
    FILE *outFS = fileOpener("output/hrrn.out", "w");

    int current_time = 0;
    newArrivals(current_time, dataQueue_check);
    QueueNode_HighestResponseRatioNext_recompute(queue, current_time); // recompute
    QueueNode_HighestResponseRatioNext_resort(&queue); // resort

    while(dataQueue != NULL || queue != NULL){
        if(queue == NULL){ // fast forward
            current_time = dataQueue->arrivalTime;
            newArrivals(current_time, dataQueue_check);
            QueueNode_HighestResponseRatioNext_recompute(queue, current_time); // recompute
            QueueNode_HighestResponseRatioNext_resort(&queue); // resort
        }

        QueueNode *current = QueueNode_pop(&queue);

        for(int i = 0; i < current->serviceTime; i++){
            fprintf(outFS, "%s\n", current->processName);
        }

        current_time += current->serviceTime;
        
        free(current->processName);
        free(current);
        newArrivals(current_time, dataQueue_check);
        QueueNode_HighestResponseRatioNext_recompute(queue, current_time); // recompute
        QueueNode_HighestResponseRatioNext_resort(&queue); // resort
    }
}

/** Method to perform the Feedback Scheduler
    
 */
void feedback(){
    FILE *outFS = fileOpener("output/feedback.out", "w");
    int current_time = 0;
    
    newArrivals(current_time, feedback_check);
    
    while(dataQueue != NULL || queue != NULL){
        if(queue == NULL){ // fast forward
            current_time = dataQueue->arrivalTime;
            newArrivals(current_time, feedback_check);
        }
        
        QueueNode *current = QueueNode_pop(&queue);
        fprintf(outFS, "%s\n", current->processName);
        current_time++;
        current->serviceTime--;
        
        // reinsert if applicable
        if(current->serviceTime != 0){
            current->subQueue++;
            QueueNode_general_insert(&queue, current, current_time, feedback_check);
        }else{
            free(current->processName);
            free(current);
        }

        newArrivals(current_time, feedback_check);
    }
}

/** Method to check command line arguments, prep queues and start schedulers
 
 */
int main(int argc, char **argv){
    if(argc < 2){
        printf("Please provide input file as a command line argument\n");
        exit(1);
    }
    // runs First Come First Serve
    read_File(argv[1]);
    firstComeFirstServe();
    // runs Round Robin Quantium 10
    read_File(argv[1]);
    roundRobin(1);
    // runs Round Robin Quantium 40
    read_File(argv[1]);
    roundRobin(4);
    // runs Shortest Process Next
    read_File(argv[1]);
    shortestProcessNext();
    // runs Shortest Remaining Time
    read_File(argv[1]);
    shortestRemainingTime();
    // runs Highest Response Ratio Next
    read_File(argv[1]);
    highestResponseRatioNext();
    // run Feedback
    read_File(argv[1]);
    feedback();

    return 0;
}
