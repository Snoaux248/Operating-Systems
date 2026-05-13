//
//  api.c
//  OperatingSystemsProject
//  Due 05/06/2026
//  Created by Nolan Jones on whenever Asignment 3 was
//

#include "main.h"

char *server = "http://127.0.0.1:\0";
char *serverPort = NULL;
char *port = NULL;

char *paths[] = {"/Simulation/start\0", "/Simulation/stop\0", "/Simulation/check\0", "/ElevatorStatus\0", "/NextInput\0", "/AddPersonToElevator\0"};

/** Method to facilitate callback reading as provided in Asssinment 3
    Define the callback function that will receive the response data and store it in a buffer
 */
static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata){
    struct Response* resp = (struct Response*)userdata;
    size_t new_size = resp->size + size * nmemb;
    resp->buffer = realloc(resp->buffer, new_size + 1); // Add an extra byte for the null terminator
    if (resp->buffer == NULL) {
        printf("Error: could not allocate memory\n");
        return 0;
    }
    memcpy(resp->buffer + resp->size, ptr, size * nmemb);
    resp->buffer[new_size] = '\0'; // Add null terminator
    resp->size = new_size;
    return size * nmemb;
}

/** Method to sent a PUT request to the server API
 
 */
void init_put(char *serverAddress, char *sessionKey, const char *data){
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
    
    struct Response resp = { NULL, 0 };
    if(curl){
        // Set the CURL based HTTP headers pointer.
        struct curl_slist * headers = NULL;

        // Append the text/plain content type to the HTTP header.
        headers = curl_slist_append(headers, "Content-Type: text/plain");
        if(sessionKey != NULL){
            headers = curl_slist_append(headers, sessionKey);
        }
        
        // Set the URL and Headers for the PUT request
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, serverAddress);
        
        // Set the required CURL options for a PUT request.
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if(data != NULL){
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        }
        
        // Set the callback function to receive the response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        // Perform the PUT request
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK){
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }else{
            //printf("Response body: %s", resp.buffer);
            //printf("Response body: %s\n", resp.buffer);
        }
        
        // Cleanup the curl handle
        curl_easy_cleanup(curl);
    }
    
    // Cleanup libcurl
    curl_global_cleanup();

}
/* Method to sent a GET request to the server API
 
 */
void init_get(char *serverAddress, char *sessionKey, char **data){
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
  
    // Create a string buffer to hold the response data
    struct Response resp = { NULL, 0 };
    if(curl){
        struct curl_slist * headers = NULL;
        if(sessionKey != NULL){
            headers = curl_slist_append(headers, sessionKey);
        }
        
        // Set the URL and Headers for the GET request
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, serverAddress);
        
        // Set the callback function to receive the response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
        
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        // Perform the GET request
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK){
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }else{
            *data = copymem(resp.buffer, get_string_length(resp.buffer));
            //printf("Response body: %s\n", *data);
        }
      
        // Print the response body
        free(resp.buffer);
        
        // Cleanup the curl handle
        curl_easy_cleanup(curl);
    }

    // Cleanup libcurl
    curl_global_cleanup();
}

/** Method to get string length
 
 */
size_t get_string_length(const char *string){
    size_t i = 0;
    while(string[i] != '\0'){
        i++;
    }
    return i;
}

/** Method to copy memory from temperory variables to dynamically allocatd memory
 
 */
char* copymem(char *word, unsigned long length){
    char *temp = (char*) calloc(length+1, sizeof(char)); // allocation + 1 for termination character
    for(int i = 0; i < length; i++){ // transfer loop
        temp[i] = word[i];
    }
    temp[length] = '\0'; // termination character
    return temp;
}

/** Method to concatenate two strings
    returned string doesn't modify inputs;
 */
char* concatenate(char *s1, char *s2){
    size_t s1_length = get_string_length(s1);
    size_t s2_length = get_string_length(s2);
    char *s3 = (char*) calloc(s1_length + s2_length + 1, sizeof(char));
    if(s3 == NULL){
        return NULL;
    }
    for(size_t i = 0; i < s1_length; i++){
        s3[i] = s1[i];
    }
    for(size_t i = 0; i < s2_length; i++){
        s3[i+s1_length] = s2[i];
    }
    s3[s1_length+s2_length] = '\0';
    return s3;
}
/** Method to find elevator based upon bayId
 
 */
int find_elevator(char *bayId){
    for(int i = 0; i < numElevators; i++){
        if(strcmp(elevators[i]->bayId, bayId) == 0){
            //printf("%s, %s\n", elevators[i]->bayId, bayId);
            return i;
        }
    }
    return -1;
}
/* Method to parse the recieved Elevator information from the server and update values;
 
 */
void parse_elevator(char *result, char *elevatorId){
    int index = find_elevator(elevatorId);
    if(index == -1){
        return;
    }
    
    char bayId[100] = {0};
    sscanf(result, "%[^|]|%d|%c|%d|%d", bayId, &(elevators[index]->currentFloor), &(elevators[index]->direction), &(elevators[index]->passengerCount), &(elevators[index]->remainingCapacity));
}

/** Method to parse received person information from the server and add them to the people queue
 
 */
void parse_person(char *result){
    char personId[100] = {0};
    int startFloor = 0;
    int endFloor = 0;
    sscanf(result, "%[^|]|%d|%d", personId, &startFloor, &endFloor);
    printf("%s %d %d\n", personId, startFloor, endFloor);
    PersonNode *temp = new_PersonNode(personId, startFloor, endFloor);
    PersonNode_LinkedList_insert(&personHead, &personTail, temp);
}

/** Method to start the simulation from the API
 
 */
void simulationStart(void){
    printf("Starting Simulation\n");
    printf("\n");
    char *URL = concatenate(serverPort, "/Simulation/start\0");
    printf("URL: %s\n", URL);
    init_put(URL, NULL, NULL);
    
    free(URL);
}

/** Method to stop the simulation from the API
 
 */
void simulationStop(void){
    printf("Stoping Simulation\n");
    printf("\n");
    char *URL = concatenate(serverPort, paths[1]);
    printf("URL: %s\n", URL);
    init_put(URL, NULL, NULL);
    
    free(URL);
}

/** Method to check the simulation status from the API
 
 */
int simulationCheck(void){
    char *URL = concatenate(serverPort, paths[2]);
    char *response = NULL;
    init_get(URL, NULL, &response);
    
    free(URL);
    if(strcmp(response, "Simulation is complete.") == 0){
        printf("Simulation is complete.\n");
        return 0;
    }
    
    return 1;
}

/** Method to check elevator status's from the API
 
 */
void elevatorStatus(char *elevatorID){
    char *URL  = concatenate(serverPort, paths[3]);
    char *URL2 = concatenate(URL, "/\0");
    char *URL3 = concatenate(URL2, elevatorID);
    char *response = NULL;
    init_get(URL3, NULL, &response);
    parse_elevator(response, elevatorID);
    
    free(URL);
    free(URL2);
    free(URL3);
}

/** Method to get the next person from the API
 
 */
int nextInput(void){
    char *URL = concatenate(serverPort, paths[4]);
    char *response = NULL;
    init_get(URL, NULL, &response);
    if(!(strcmp(response, "NONE\0") == 0)){
        parse_person(response);
        free(URL);
        return 1;
    }
    free(URL);
    return 0;
}

/** Method to asssign the next person an elevator from the API
 
 */
void addPersonToElevator(char *personId, char *elevatorId){
    printf("Adding %s to Elevator %s\n", personId, elevatorId);
    printf("\n");
    char *URL  = concatenate(serverPort, paths[5]);
    char *URL2 = concatenate(URL, "/\0");
    char *URL3 = concatenate(URL2, personId);
    char *URL4 = concatenate(URL3, "/\0");
    char *URL5 = concatenate(URL4, elevatorId);
    char *response = NULL;
    init_put(URL5, NULL, response);
    
    free(URL);
    free(URL2);
    free(URL3);
    free(URL4);
    free(URL5);
}
