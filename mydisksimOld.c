#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#define MAX_REQUESTS 10000

// Structure for a disk request
typedef struct {
    double arrivalTime;
    int LBN;
    int requestSize;
    int cylinder;
    int surface;
    int sectorOffset;
    int PSN;
    double finishTime;
    double waitingTime;
    int seekDistance;
} DiskRequest;

// Function declarations
void parseInputFile(const char *filename, DiskRequest *requests, int *numRequests);
void writeOutputFile(const char *filename, DiskRequest *requests, int numRequests);
void simulateFCFS(DiskRequest *requests, int numRequests, int limit);
void simulateSSTF(DiskRequest *requests, int numRequests, int limit);
void simulateSCAN(DiskRequest *requests, int numRequests, int limit);
void simulateCLOOK(DiskRequest *requests, int numRequests, int limit);

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Usage: ./mydisksim <inputfile> <outputfile> <algorithm> [limit]\n");
        return 1;
    }

    // Read command-line arguments
    const char *inputFile = argv[1];
    const char *outputFile = argv[2];
    const char *algorithm = argv[3];
    int limit = (argc == 5) ? atoi(argv[4]) : MAX_REQUESTS;

    // Array to store disk requests
    DiskRequest requests[MAX_REQUESTS];
    int numRequests = 0;

    // Read the input file
    parseInputFile(inputFile, requests, &numRequests);

    // Simulate the chosen algorithm
    if (strcmp(algorithm, "FCFS") == 0) {
        simulateFCFS(requests, numRequests, limit);
    } else if (strcmp(algorithm, "SSTF") == 0) {
        simulateSSTF(requests, numRequests, limit);
    } else if (strcmp(algorithm, "SCAN") == 0) {
        simulateSCAN(requests, numRequests, limit);
    } else if (strcmp(algorithm, "CLOOK") == 0) {
        simulateCLOOK(requests, numRequests, limit);
    } else {
        fprintf(stderr, "Unknown algorithm: %s\n", algorithm);
        return 1;
    }

    // Write the output file
    writeOutputFile(outputFile, requests, numRequests);

    return 0;
}

// Function to parse the input file
void parseInputFile(const char *filename, DiskRequest *requests, int *numRequests) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening input file");
        exit(1);
    }

    int i = 0;
    while (fscanf(file, "%lf %d %d", 
                  &requests[i].arrivalTime, 
                  &requests[i].LBN, 
                  &requests[i].requestSize) == 3) {
        i++;
    }

    *numRequests = i;
    fclose(file);
}

// Function to write the output file
void writeOutputFile(const char *filename, DiskRequest *requests, int numRequests) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening output file");
        exit(1);
    }

    for (int i = 0; i < numRequests; i++) {
        fprintf(file, "%.6f %.6f %.6f %d %d %d %.6f %d\n",
                requests[i].arrivalTime,
                requests[i].finishTime,
                requests[i].waitingTime,
                requests[i].PSN,
                requests[i].cylinder,
                requests[i].surface,
                requests[i].sectorOffset,
                requests[i].seekDistance);
    }

    fclose(file);
}

// FCFS algorithm simulation
void simulateFCFS(DiskRequest *requests, int numRequests, int limit) {
    double currentTime = 0.0;
    for (int i = 0; i < limit && i < numRequests; i++) {
        if (currentTime < requests[i].arrivalTime) {
            currentTime = requests[i].arrivalTime;
        }

        requests[i].waitingTime = currentTime - requests[i].arrivalTime;
        requests[i].finishTime = currentTime + 0.001 * requests[i].requestSize;
        requests[i].seekDistance = 0; // Seek distance placeholder
        currentTime = requests[i].finishTime;
    }
}

// SSTF algorithm simulation
void simulateSSTF(DiskRequest *requests, int numRequests, int limit) {
    int processed[MAX_REQUESTS] = {0};
    double currentTime = 0.0;
    int currentCylinder = 0;

    for (int count = 0; count < limit && count < numRequests; count++) {
        int closestIndex = -1;
        int minSeek = INT_MAX;

        for (int i = 0; i < numRequests; i++) {
            if (!processed[i] && requests[i].arrivalTime <= currentTime) {
                int seekDistance = abs(requests[i].LBN - currentCylinder);
                if (seekDistance < minSeek) {
                    minSeek = seekDistance;
                    closestIndex = i;
                }
            }
        }

        if (closestIndex == -1) {
            currentTime = requests[count].arrivalTime;
            count--;
            continue;
        }

        processed[closestIndex] = 1;
        requests[closestIndex].waitingTime = currentTime - requests[closestIndex].arrivalTime;
        requests[closestIndex].finishTime = currentTime + 0.001 * requests[closestIndex].requestSize;
        requests[closestIndex].seekDistance = abs(requests[closestIndex].LBN - currentCylinder);
        currentCylinder = requests[closestIndex].LBN;
        currentTime = requests[closestIndex].finishTime;
    }
}

// SCAN algorithm simulation
void simulateSCAN(DiskRequest *requests, int numRequests, int limit) {
    int processed[MAX_REQUESTS] = {0};
    double currentTime = 0.0;
    int currentCylinder = 0;
    int direction = 1; // 1 for upward, -1 for downward

    for (int count = 0; count < limit; count++) {
        int closestIndex = -1;
        int minSeek = INT_MAX;

        // Find the closest request in the current direction
        for (int i = 0; i < numRequests; i++) {
            if (!processed[i] && requests[i].arrivalTime <= currentTime) {
                int seekDistance = requests[i].cylinder - currentCylinder;
                if ((direction == 1 && seekDistance >= 0) || (direction == -1 && seekDistance <= 0)) {
                    seekDistance = abs(seekDistance);
                    if (seekDistance < minSeek) {
                        minSeek = seekDistance;
                        closestIndex = i;
                    }
                }
            }
        }

        // If no request in the current direction, reverse direction
        if (closestIndex == -1) {
            direction *= -1;
            count--;
            continue;
        }

        // Process the request
        processed[closestIndex] = 1;
        requests[closestIndex].waitingTime = currentTime - requests[closestIndex].arrivalTime;
        requests[closestIndex].finishTime = currentTime + 0.001 * requests[closestIndex].requestSize;
        requests[closestIndex].seekDistance = abs(requests[closestIndex].cylinder - currentCylinder);
        currentCylinder = requests[closestIndex].cylinder;
        currentTime = requests[closestIndex].finishTime;
    }
}

// CLOOK algorithm simulation
void simulateCLOOK(DiskRequest *requests, int numRequests, int limit) {
    int processed[MAX_REQUESTS] = {0};
    double currentTime = 0.0;
    int currentCylinder = 0;

    for (int count = 0; count < limit; count++) {
        int closestIndex = -1;
        int minSeek = INT_MAX;

        // Find the closest request in the current upward direction
        for (int i = 0; i < numRequests; i++) {
            if (!processed[i] && requests[i].arrivalTime <= currentTime) {
                int seekDistance = requests[i].cylinder - currentCylinder;
                if (seekDistance >= 0 && seekDistance < minSeek) {
                    minSeek = seekDistance;
                    closestIndex = i;
                }
            }
        }

        // If no request upward, jump to the lowest cylinder with a pending request
        if (closestIndex == -1) {
            for (int i = 0; i < numRequests; i++) {
                if (!processed[i] && requests[i].arrivalTime <= currentTime) {
                    int seekDistance = requests[i].cylinder;
                    if (seekDistance < minSeek) {
                        minSeek = seekDistance;
                        closestIndex = i;
                    }
                }
            }
        }

        // Process the request
        if (closestIndex != -1) {
            processed[closestIndex] = 1;
            requests[closestIndex].waitingTime = currentTime - requests[closestIndex].arrivalTime;
            requests[closestIndex].finishTime = currentTime + 0.001 * requests[closestIndex].requestSize;
            requests[closestIndex].seekDistance = abs(requests[closestIndex].cylinder - currentCylinder);
            currentCylinder = requests[closestIndex].cylinder;
            currentTime = requests[closestIndex].finishTime;
        }
    }
}
