//
//  threads.c
//  OperatingSystemsProject
//  Due 05/06/2026
//  Mostly Created by Nolan Jones on 5/5/26
//

#include "main.h"

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

pthread_cond_t cond1;
pthread_cond_t cond2;

/** Thread to request people from API
    places user into personHead as a Queue
 */
void* request(void* arg){
    
    while(simulationCheck() == true){ // allows simulation to complete

        pthread_mutex_lock(&mutex1);

        if(nextInput() == 1){
            pthread_cond_signal(&cond1); // unlocks cond for compute
            pthread_mutex_unlock(&mutex1);
        }else{
            pthread_mutex_unlock(&mutex1);
            usleep(200000); // pervents sserver disconnect from spam
        }
    }
    simulationStop();
    exit(0);
    return NULL;
}

/** Thread to compute persons elevator placemnt and update elevator status's
    pops user from personHead Queue
    places user into dispatchHead as a Queue
 */
void* compute(void* arg){
    
    int i = 0;
    while(true){
        
        pthread_mutex_lock(&mutex1);
        while(personHead == NULL){ // conditional wait to ensure person exists
            pthread_cond_wait(&cond1, &mutex1);
        }
        
        PersonNode *temp = PersonNode_pop(&personHead, &personTail); // pop person head
        pthread_mutex_unlock(&mutex1);
        ElevatorNode *e = elevators[i];
        
        int check; // holds number of failed elevator insertions
        while(temp != NULL){
            check = 0;
            while((temp->endFloor > e->highestFloor ||
                 temp->endFloor < e->lowestFloor ||
                 temp->startFloor > e->highestFloor ||
                 temp->startFloor < e->lowestFloor ||
                 e->remainingCapacity < 1 ||
                 e->direction == 'E')
                 && check < numElevators){
                
                i = (i + 1) % numElevators;
                e = elevators[i];
                check++;
            }
            
            if(check >= numElevators){ // retrieves all elevator info only if person couldnt be placed
                for(int i = 0; i < numElevators; i++){
                    elevatorStatus(elevators[i]->bayId);
                }
                usleep(200000); // pervents sserver disconnect from spam
                continue; // prevents placement by jumpoing back up and trying to place the person again
            }

            temp->elevatorCast = i; // add elevator index for dispatcher
            elevators[temp->elevatorCast]->passengerCount++; // increment passengerCount local variable
            elevators[temp->elevatorCast]->remainingCapacity--; // decrement remainingCapacity local variable
            
            pthread_mutex_lock(&mutex2);
            PersonNode_LinkedList_insert(&dispatchHead, &dispatchTail, temp); //inserts to dispatchQueue
            pthread_cond_signal(&cond2); // unlocks cond dispatcher
            pthread_mutex_unlock(&mutex2);
            
            i = (i + 1) % numElevators;

            break;
        }
    }

    return NULL;
}
/** Thread to dispatch persons to the elevator calculated in compute
  pops user from dispatchHead Queue
  free data
*/

void* dispatch(void* arg){

    while(true){
        
        pthread_mutex_lock(&mutex2);
        while(dispatchHead == NULL){ // conditional wait to ensure person exists
            pthread_cond_wait(&cond2, &mutex2);
        }

        PersonNode *temp = PersonNode_pop(&dispatchHead, &dispatchTail); // pop dispatch head
        
        pthread_mutex_unlock(&mutex2);

        addPersonToElevator(temp->personId, elevators[temp->elevatorCast]->bayId); //dispatch user to correct elevator

        free(temp->personId); //free data
        free(temp); //free data
    }

    return NULL;
}

