#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#define MAX_REQUEST_LIMIT 10000

// Structure representing a disk request
typedef struct {
    double arrivalTime;
    int logicalBlockNumber;
    int sizeInBlocks;
    int targetCylinder;
    int targetSurface;
    int sectorOffset;
    int physicalSectorNumber;
    double completionTime;
    double waitDuration;
    int travelDistance;
} DiskRequest;

// Function prototypes
void loadRequestsFromFile(const char *inputFilename, DiskRequest *requests, int *requestCount);
void saveResultsToFile(const char *outputFilename, DiskRequest *requests, int requestCount);
void runFirstComeFirstServe(DiskRequest *requests, int requestCount, int maxRequests);
void runShortestSeekTimeFirst(DiskRequest *requests, int requestCount, int maxRequests);
void runElevatorScan(DiskRequest *requests, int requestCount, int maxRequests);
void runCircularLook(DiskRequest *requests, int requestCount, int maxRequests);

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Usage: %s <inputfile> <outputfile> <algorithm> [limit]\n", argv[0]);
        return 1;
    }

    // Parse command-line arguments
    const char *inputFilename = argv[1];
    const char *outputFilename = argv[2];
    const char *selectedAlgorithm = argv[3];
    int requestLimit = (argc == 5) ? atoi(argv[4]) : MAX_REQUEST_LIMIT;

    // Array for disk requests
    DiskRequest requests[MAX_REQUEST_LIMIT];
    int totalRequests = 0;

    // Load disk requests from the input file
    loadRequestsFromFile(inputFilename, requests, &totalRequests);

    // Execute the chosen scheduling algorithm
    if (strcmp(selectedAlgorithm, "FCFS") == 0) {
        runFirstComeFirstServe(requests, totalRequests, requestLimit);
    } else if (strcmp(selectedAlgorithm, "SSTF") == 0) {
        runShortestSeekTimeFirst(requests, totalRequests, requestLimit);
    } else if (strcmp(selectedAlgorithm, "SCAN") == 0) {
        runElevatorScan(requests, totalRequests, requestLimit);
    } else if (strcmp(selectedAlgorithm, "CLOOK") == 0) {
        runCircularLook(requests, totalRequests, requestLimit);
    } else {
        fprintf(stderr, "Error: Unsupported algorithm '%s'\n", selectedAlgorithm);
        return 1;
    }

    // Save the simulation results
    saveResultsToFile(outputFilename, requests, totalRequests);

    return 0;
}

// Function to read requests from an input file
void loadRequestsFromFile(const char *inputFilename, DiskRequest *requests, int *requestCount) {
    FILE *inputFile = fopen(inputFilename, "r");
    if (!inputFile) {
        perror("Unable to open input file");
        exit(EXIT_FAILURE);
    }

    int index = 0;
    while (fscanf(inputFile, "%lf %d %d", 
                  &requests[index].arrivalTime, 
                  &requests[index].logicalBlockNumber, 
                  &requests[index].sizeInBlocks) == 3) {
        index++;
    }
    *requestCount = index;

    fclose(inputFile);
}

// Function to write simulation results to an output file
void saveResultsToFile(const char *outputFilename, DiskRequest *requests, int requestCount) {
    FILE *outputFile = fopen(outputFilename, "w");
    if (!outputFile) {
        perror("Unable to open output file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < requestCount; i++) {
        fprintf(outputFile, "%.6f %.6f %.6f %d %d %d %.6f %d\n",
                requests[i].arrivalTime,
                requests[i].completionTime,
                requests[i].waitDuration,
                requests[i].physicalSectorNumber,
                requests[i].targetCylinder,
                requests[i].targetSurface,
                requests[i].sectorOffset,
                requests[i].travelDistance);
    }

    fclose(outputFile);
}

// FCFS Algorithm
void runFirstComeFirstServe(DiskRequest *requests, int requestCount, int maxRequests) {
    double currentTime = 0.0;

    for (int i = 0; i < maxRequests && i < requestCount; i++) {
        if (currentTime < requests[i].arrivalTime) {
            currentTime = requests[i].arrivalTime;
        }

        requests[i].waitDuration = currentTime - requests[i].arrivalTime;
        requests[i].completionTime = currentTime + 0.001 * requests[i].sizeInBlocks;
        requests[i].travelDistance = 0; // Assume no seek for simplicity
        currentTime = requests[i].completionTime;
    }
}

// SSTF Algorithm
void runShortestSeekTimeFirst(DiskRequest *requests, int requestCount, int maxRequests) {
    int processedRequests[MAX_REQUEST_LIMIT] = {0};
    double currentTime = 0.0;
    int currentPosition = 0;

    for (int count = 0; count < maxRequests && count < requestCount; count++) {
        int closestRequest = -1;
        int minimalSeek = INT_MAX;

        for (int i = 0; i < requestCount; i++) {
            if (!processedRequests[i] && requests[i].arrivalTime <= currentTime) {
                int seekDistance = abs(requests[i].logicalBlockNumber - currentPosition);
                if (seekDistance < minimalSeek) {
                    minimalSeek = seekDistance;
                    closestRequest = i;
                }
            }
        }

        if (closestRequest == -1) {
            currentTime = requests[count].arrivalTime;
            count--;
            continue;
        }

        processedRequests[closestRequest] = 1;
        requests[closestRequest].waitDuration = currentTime - requests[closestRequest].arrivalTime;
        requests[closestRequest].completionTime = currentTime + 0.001 * requests[closestRequest].sizeInBlocks;
        requests[closestRequest].travelDistance = abs(requests[closestRequest].logicalBlockNumber - currentPosition);
        currentPosition = requests[closestRequest].logicalBlockNumber;
        currentTime = requests[closestRequest].completionTime;
    }
}

// SCAN Algorithm
void runElevatorScan(DiskRequest *requests, int requestCount, int maxRequests) {
    int processedRequests[MAX_REQUEST_LIMIT] = {0};
    double currentTime = 0.0;
    int currentPosition = 0;
    int direction = 1; // 1 for upward, -1 for downward

    for (int count = 0; count < maxRequests; count++) {
        int closestRequest = -1;
        int minimalSeek = INT_MAX;

        for (int i = 0; i < requestCount; i++) {
            if (!processedRequests[i] && requests[i].arrivalTime <= currentTime) {
                int seekDistance = requests[i].targetCylinder - currentPosition;
                if ((direction == 1 && seekDistance >= 0) || (direction == -1 && seekDistance <= 0)) {
                    seekDistance = abs(seekDistance);
                    if (seekDistance < minimalSeek) {
                        minimalSeek = seekDistance;
                        closestRequest = i;
                    }
                }
            }
        }

        if (closestRequest == -1) {
            direction *= -1;
            count--;
            continue;
        }

        processedRequests[closestRequest] = 1;
        requests[closestRequest].waitDuration = currentTime - requests[closestRequest].arrivalTime;
        requests[closestRequest].completionTime = currentTime + 0.001 * requests[closestRequest].sizeInBlocks;
        requests[closestRequest].travelDistance = abs(requests[closestRequest].targetCylinder - currentPosition);
        currentPosition = requests[closestRequest].targetCylinder;
        currentTime = requests[closestRequest].completionTime;
    }
}

// CLOOK Algorithm
void runCircularLook(DiskRequest *requests, int requestCount, int maxRequests) {
    int processedRequests[MAX_REQUEST_LIMIT] = {0};
    double currentTime = 0.0;
    int currentPosition = 0;

    for (int count = 0; count < maxRequests; count++) {
        int closestRequest = -1;
        int minimalSeek = INT_MAX;

        for (int i = 0; i < requestCount; i++) {
            if (!processedRequests[i] && requests[i].arrivalTime <= currentTime) {
                int seekDistance = requests[i].targetCylinder - currentPosition;
                if (seekDistance >= 0 && seekDistance < minimalSeek) {
                    minimalSeek = seekDistance;
                    closestRequest = i;
                }
            }
        }

        if (closestRequest == -1) {
            for (int i = 0; i < requestCount; i++) {
                if (!processedRequests[i] && requests[i].arrivalTime <= currentTime) {
                    int seekDistance = requests[i].targetCylinder;
                    if (seekDistance < minimalSeek) {
                        minimalSeek = seekDistance;
                        closestRequest = i;
                    }
                }
            }
        }

        if (closestRequest != -1) {
            processedRequests[closestRequest] = 1;
            requests[closestRequest].waitDuration = currentTime - requests[closestRequest].arrivalTime;
            requests[closestRequest].completionTime = currentTime + 0.001 * requests[closestRequest].sizeInBlocks;
            requests[closestRequest].travelDistance = abs(requests[closestRequest].targetCylinder - currentPosition);
            currentPosition = requests[closestRequest].targetCylinder;
            currentTime = requests[closestRequest].completionTime;
        }
    }
}
