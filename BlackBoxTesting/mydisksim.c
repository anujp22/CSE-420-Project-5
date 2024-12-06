#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>

#define SECTORSPERTRACK     200
#define TRACKSPERCYLINDER   8
#define CYLINDERS           500000
#define RPM                 10000
#define PHYSICALSECTORSIZE  512     //(Bytes)
#define LOGICALBLOCKSIZE    4096    //(Bytes)
#define TRACKTOTRACKSEEK    2       //(milliseconds)
#define FULLSEEK            16      //(milliseconds)
#define TRANSFERRATE        1       //(Gb/s)

struct Node {
    char *name;
    double id;
    double arrivalTime;
    int lbn;
    int requestSize;
    double finishTime;
    double waitingTime;
    int psn;
    int cylinder;
    int surface;
    double sectorOffset;
    int seekDistance;
    int extraSeekDistance;
    struct Node *next;
    struct Node *prev;
};

struct List {
    struct Node *head;
    struct Node *tail;
};

struct Node *create_node(char *name, double id, double arrivalTime, int lbn,
                        int requestSize, double finishTime, double waitingTime,
                        int psn, int cylinder, int surface, double sectorOffset,
                        int seekDistance, int extraSeekDistance) {
    struct Node *node = malloc(sizeof(struct Node));
    if (node == NULL) {
        fprintf (stderr, "%s: Couldn't create memory for the node; %s\n", "linkedlist", strerror(errno));
        exit(-1);
    }
    node->name = strdup(name);
    node->id = id;
    node->arrivalTime = arrivalTime;
    node->lbn = lbn;
    node->requestSize = requestSize;
    node->finishTime = finishTime;
    node->waitingTime = waitingTime;
    node->psn = psn;
    node->cylinder = cylinder;
    node->surface = surface;
    node->sectorOffset = sectorOffset;
    node->seekDistance = seekDistance;
    node->extraSeekDistance = extraSeekDistance;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

struct List *create_list() {
    struct List *list = malloc(sizeof(struct List));
    if (list == NULL) {
        fprintf (stderr, "%s: Couldn't create memory for the list; %s\n", "linkedlist", strerror (errno));
        exit(-1);
    }
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void insert_tail(struct Node *node, struct List *list) {
    if (list->head == NULL && list->tail == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        node->prev = list->tail;
        list->tail = node;
    }
}

void print_list(struct List *list) {
    struct Node *ptr = list->head;  
    while (ptr != NULL) {
        if (ptr != list->head) {
        printf("->");
        }
        printf("(%s,%f)", ptr->name, ptr->id);
        ptr = ptr->next;
    }
    printf("\n");
}

void destroy_list(struct List *list) {
    struct Node *ptr = list->head;
    struct Node *tmp;  
    while (ptr != NULL) {
        free(ptr->name);
        tmp = ptr;
        ptr = ptr->next;
        free(tmp);
    }
    free(list);
}

struct Node *find_by_id(double id, struct List *list) {
    struct Node *ptr = list->head;  
    while (ptr != NULL) {
        if (ptr->id == id) {
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;
}

void remove_by_id(double id, struct List *list) {
    struct Node *ptr = list->head, *nodeToBeRemoved = find_by_id(id, list), *tmpPtr = NULL;
    if (nodeToBeRemoved != NULL) {
        while (ptr != NULL){
            if (ptr == nodeToBeRemoved && ptr != list->head && ptr != list->tail){
                free(ptr->name);
                tmpPtr = ptr;
                ptr->prev->next = nodeToBeRemoved->next;
                nodeToBeRemoved->next->prev = ptr->prev;
                free(tmpPtr);
                return;
            }
            else if (ptr == nodeToBeRemoved && ptr == list->head){
                free(ptr->name);
                tmpPtr = ptr;
                list->head = ptr->next;
                free(tmpPtr);
                return;
            }
            else if (ptr == nodeToBeRemoved && ptr == list->tail){
                free(ptr->name);
                tmpPtr = ptr;
                ptr->prev->next = NULL;
                list->tail = ptr->prev;
                free(tmpPtr);
                return;
            }
            ptr = ptr->next;
        }
    }
}

void insert_sorted(struct Node *node, struct List *list) {
    if (list->head == NULL && list->tail == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        struct Node *ptr = list->head, *ptrPrev = NULL;
        while (ptr != NULL){
            if (node->id < ptr->id && ptr == list->head){
                list->head = node;
                node->next = ptr;
                ptr->prev = node;
                return;
            }else if(node->id < ptr->id && ptr == list->tail){
                ptrPrev->next = node;
                node->next = ptr;
                ptr->prev = node;
                return;
            }else if(node->id < ptr->id){
                node->next = ptr;
                ptrPrev->next = node;
                return;
            }
            ptrPrev = ptr;
            ptr = ptr->next;
        }
        insert_tail(node, list);
    }
}

void insertion_sort_by_ID_increasing(struct List *list) {
    struct List *listInOrder = create_list();
    struct Node *ptr = list->head, *tmpPtr = NULL;
    while (ptr != NULL) {
        tmpPtr = create_node("Node", ptr->id, ptr->arrivalTime, ptr->lbn, ptr->requestSize, ptr->finishTime,
                                ptr->waitingTime, ptr->psn, ptr->cylinder, ptr->surface, ptr->sectorOffset, ptr->seekDistance, 0);
        insert_sorted(tmpPtr, listInOrder);
        ptr = ptr->next;
    }
    list->head = NULL;
    list->tail = NULL;
    ptr = listInOrder->head;
    while (ptr != NULL) {
        tmpPtr = create_node("Node", ptr->id, ptr->arrivalTime, ptr->lbn, ptr->requestSize, ptr->finishTime,
                                ptr->waitingTime, ptr->psn, ptr->cylinder, ptr->surface, ptr->sectorOffset, ptr->seekDistance, 0);
        insert_tail(tmpPtr,list);
        ptr = ptr->next;
    }
    destroy_list(listInOrder);
}

void read_input_file(const char *filename, struct List *list, int *count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    char line[128];
    *count = 0;

    double arrivalTime = 0.0;
    int lbn  = 0;
    int requestSize = 0;
    int psn = 0;
    struct Node *tmp = NULL;

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%lf %d %d", &arrivalTime, &lbn, &requestSize);
        psn = lbn * 8 + requestSize;
        tmp = create_node("Node",arrivalTime, arrivalTime, lbn, requestSize,
                                        0, 0, psn, (psn / (TRACKSPERCYLINDER * SECTORSPERTRACK)),
                                        ((psn / SECTORSPERTRACK) % TRACKSPERCYLINDER),
                                        (psn % SECTORSPERTRACK), 0, 0);
        insert_tail(tmp, list);
        (*count)++;
    }

    fclose(file);
}

void write_output_file(const char *filename, struct List *list, int count) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    struct Node *tmp = list->head;
    int flag = 0;
    while(tmp != NULL && flag < count){
        fprintf(file, "%.6f %.6f %.6f %d %d %d %.6f %d\n",
                tmp->arrivalTime, tmp->finishTime, tmp->waitingTime, tmp->psn,
                tmp->cylinder, tmp->surface, tmp->sectorOffset, tmp->seekDistance);
        tmp = tmp->next;
        flag += 1;
    }
    fclose(file);
}

void simulate_fcfs(struct List *list, int count) {
    double previousFinishTime = 0;
    int currentCylinder = 0;
    double sectorOffSetAddition = 0;
    int flag = 0;
    struct Node *tmp = list->head;
    while(tmp != NULL && flag < count){
        if (tmp == list->head){ //first one
            tmp->waitingTime = 0;
        }else{
            tmp->waitingTime = previousFinishTime - tmp->arrivalTime;
        }
        double seekTime = 0.0;
        if (tmp->extraSeekDistance == 0){
            tmp->seekDistance = abs(tmp->cylinder - currentCylinder);
            if (tmp->seekDistance != 0){
                seekTime = (0.000028 * (abs(tmp->cylinder - currentCylinder))) + 2;
            }else{
                seekTime = 0;
            }
        }else if(tmp->extraSeekDistance > 0){
            tmp->seekDistance = tmp->extraSeekDistance;
            seekTime = (0.000028 * ((abs(tmp->cylinder - currentCylinder)) + tmp->extraSeekDistance)) + 2;
        }else{
            tmp->seekDistance = (tmp->extraSeekDistance*-1);
            if (tmp->seekDistance != 0){
                seekTime = (0.000028 * (abs(tmp->cylinder - currentCylinder))) + 2;
            }else{
                seekTime = 0;
            }
        }
        seekTime /= 1000;
        currentCylinder = tmp->cylinder;

        double transferTime = (8.0 * tmp->requestSize * PHYSICALSECTORSIZE) / (1024.0*1024*1024);

        double rotationalLatency = 0.0;
        double updatedCurrentSectorOffset = ((seekTime*1000.0) / ((60.0*1000.0)/(SECTORSPERTRACK*RPM))) + sectorOffSetAddition;
        double targetSectorOffset = tmp->sectorOffset - tmp->requestSize;
        while (updatedCurrentSectorOffset > SECTORSPERTRACK){
            updatedCurrentSectorOffset -= SECTORSPERTRACK;
        }
        if (updatedCurrentSectorOffset < targetSectorOffset){
            rotationalLatency = targetSectorOffset - updatedCurrentSectorOffset;
        }else{
            rotationalLatency = SECTORSPERTRACK - updatedCurrentSectorOffset + targetSectorOffset;
        }
        rotationalLatency *= (60.0*1000.0)/(SECTORSPERTRACK*RPM);
        rotationalLatency /= 1000;
        sectorOffSetAddition = tmp->sectorOffset;

        double serviceTime = seekTime + transferTime + rotationalLatency;
        tmp->finishTime = serviceTime + tmp->arrivalTime + tmp->waitingTime;
        previousFinishTime = tmp->finishTime;
        tmp = tmp->next;
        flag += 1;
        // printf("seek time(seconds): %.10f\n", seekTime);
        // printf("transfer time(seconds): %.15f\n", transferTime);
        // printf("updatedCurrentSectorOffset: %.10f\n", updatedCurrentSectorOffset);
        // printf("targetSectorOffset: %.10f\n", targetSectorOffset);
        // printf("rotational latency(seconds): %.10f\n", rotationalLatency);
        // printf("service time(seconds): %.10f\n", serviceTime);
    }
}

void simulate_sstf(struct List *list, int count, const char *outFilename){
    struct List *listSSTF = create_list();
    struct Node *tmp = list->head;
    struct Node *tmp2 = NULL;
    struct Node *tmp3 = NULL;
    int flag = 0; int flag2 = 0;
    int currentCylinder = 0; double holdBest = 0.0; double holdCurrentTime = -1.0;
    while(tmp != NULL && flag < count){
        flag2 = flag;
        holdBest = -1.0;
        tmp2 = list->head;
        while(tmp2 != NULL && flag2 < count){
            if(holdCurrentTime < 0){
                tmp3 = create_node("Node", tmp->arrivalTime, tmp->arrivalTime, tmp->lbn, tmp->requestSize, tmp->finishTime,
                                        tmp->waitingTime, tmp->psn, tmp->cylinder, tmp->surface, tmp->sectorOffset, tmp->seekDistance, 0);
                flag2 = count;
            }
            else if (tmp2->arrivalTime < holdCurrentTime){
                double seekTime = (0.000028 * (abs(tmp2->cylinder - currentCylinder))) + 2;
                seekTime /= 1000;
                if (holdBest < 0 || holdBest > seekTime){
                    holdBest = seekTime;
                    tmp3 = create_node("Node", tmp2->arrivalTime, tmp2->arrivalTime, tmp2->lbn, tmp2->requestSize, tmp2->finishTime,
                                        tmp2->waitingTime, tmp2->psn, tmp2->cylinder, tmp2->surface, tmp2->sectorOffset, tmp2->seekDistance, 0);
                }
            }
            tmp2 = tmp2->next;
            flag2 += 1;
        }
        currentCylinder = tmp3->cylinder;
        insert_tail(tmp3, listSSTF);
        simulate_fcfs(listSSTF, (flag+1));
        holdCurrentTime = listSSTF->tail->finishTime;
        remove_by_id(tmp3->arrivalTime, list);
        tmp = list->head;
        flag += 1;
    }

    write_output_file(outFilename, listSSTF, count);
}

void simulate_clook(struct List *list, int count, const char *outFilename){
    struct List *listSSTF = create_list();
    struct Node *tmp = list->head;
    struct Node *tmp2 = NULL;
    struct Node *tmp3 = NULL;
    int flag = 0; int flag2 = 0;
    int currentCylinder = 0; double holdCurrentTime = -1.0; int tmpCurrentCylinder = 0;
    while(tmp != NULL && flag < count){
        flag2 = flag;
        tmp2 = list->head;
        tmpCurrentCylinder = -1;
        while(tmp2 != NULL && flag2 < count){
            if(holdCurrentTime < 0){
                tmp3 = create_node("Node", tmp->arrivalTime, tmp->arrivalTime, tmp->lbn, tmp->requestSize, tmp->finishTime,
                                        tmp->waitingTime, tmp->psn, tmp->cylinder, tmp->surface, tmp->sectorOffset, tmp->seekDistance, 0);
                flag2 = count;
            }
            else if (tmp2->arrivalTime < holdCurrentTime){
                if (currentCylinder <= tmp2->cylinder && (tmpCurrentCylinder < 0 || tmpCurrentCylinder > tmp2->cylinder)){
                    tmpCurrentCylinder = tmp2->cylinder;
                    tmp3 = create_node("Node", tmp2->arrivalTime, tmp2->arrivalTime, tmp2->lbn, tmp2->requestSize, tmp2->finishTime,
                                        tmp2->waitingTime, tmp2->psn, tmp2->cylinder, tmp2->surface, tmp2->sectorOffset, tmp2->seekDistance, 0);
                }
            }
            tmp2 = tmp2->next;
            flag2 += 1;
        }
        if(tmpCurrentCylinder < 0 && holdCurrentTime > 0){
            flag2 = flag;
            tmp2 = list->head;
            tmpCurrentCylinder = -1;
            while(tmp2 != NULL && flag2 < count){
                if (tmp2->arrivalTime < holdCurrentTime){
                    if (tmpCurrentCylinder < 0 || tmpCurrentCylinder > tmp2->cylinder){
                        tmpCurrentCylinder = tmp2->cylinder;
                        tmp3 = create_node("Node", tmp2->arrivalTime, tmp2->arrivalTime, tmp2->lbn, tmp2->requestSize, tmp2->finishTime,
                                            tmp2->waitingTime, tmp2->psn, tmp2->cylinder, tmp2->surface, tmp2->sectorOffset, tmp2->seekDistance, 0);
                    }
                }
                tmp2 = tmp2->next;
                flag2 += 1;
            }
        }
        currentCylinder = tmp3->cylinder;
        insert_tail(tmp3, listSSTF);
        simulate_fcfs(listSSTF, (flag+1));
        holdCurrentTime = listSSTF->tail->finishTime;
        remove_by_id(tmp3->arrivalTime, list);
        tmp = list->head;
        flag += 1;
    }
    write_output_file(outFilename, listSSTF, count);
}

void simulate_scan(struct List *list, int count, const char *outFilename){
    struct List *listSSTF = create_list();
    struct Node *tmp = list->head;
    struct Node *tmp2 = NULL;
    struct Node *tmp3 = NULL;
    int flag = 0; int flag2 = 0; int flag3 = 0; int extraDistance = 0;
    int currentCylinder = 0; double holdCurrentTime = -1.0; int tmpCurrentCylinder = 0;
    int direction = 0; //0 = right, 1 = left
    while(tmp != NULL && flag < count){
        flag2 = flag;
        tmp2 = list->head;
        tmpCurrentCylinder = -1;
        flag3 = 0;
        extraDistance = 0;
        while(tmp2 != NULL && flag2 < count){
            if(holdCurrentTime < 0){
                tmp3 = create_node("Node", tmp->arrivalTime, tmp->arrivalTime, tmp->lbn, tmp->requestSize, tmp->finishTime,
                                        tmp->waitingTime, tmp->psn, tmp->cylinder, tmp->surface, tmp->sectorOffset, tmp->seekDistance, 0);
                flag2 = count + 4;
            }else if (tmp2->arrivalTime < holdCurrentTime && direction == 0){
                if (currentCylinder <= tmp2->cylinder && (tmpCurrentCylinder < 0 || tmpCurrentCylinder > tmp2->cylinder)){
                    tmpCurrentCylinder = tmp2->cylinder;
                    tmp3 = create_node("Node", tmp2->arrivalTime, tmp2->arrivalTime, tmp2->lbn, tmp2->requestSize, tmp2->finishTime,
                                        tmp2->waitingTime, tmp2->psn, tmp2->cylinder, tmp2->surface, tmp2->sectorOffset, tmp2->seekDistance, 0);
                }
            }else if (tmp2->arrivalTime < holdCurrentTime && direction == 1){
                if (currentCylinder >= tmp2->cylinder && (tmpCurrentCylinder < 0 || tmpCurrentCylinder < tmp2->cylinder)){
                    tmpCurrentCylinder = tmp2->cylinder;
                    tmp3 = create_node("Node", tmp2->arrivalTime, tmp2->arrivalTime, tmp2->lbn, tmp2->requestSize, tmp2->finishTime,
                                        tmp2->waitingTime, tmp2->psn, tmp2->cylinder, tmp2->surface, tmp2->sectorOffset, tmp2->seekDistance, 0);
                }
            }
            tmp2 = tmp2->next;
            flag2 += 1;
        }
        if(tmpCurrentCylinder < 0 && holdCurrentTime > 0){
            flag3 = 1;
            if (direction == 1){
                direction = 0;
            }else{
                direction = 1;
            }
            flag2 = flag;
            tmp2 = list->head;
            tmpCurrentCylinder = -1;
            while(tmp2 != NULL && flag2 < count){
                if (tmp2->arrivalTime < holdCurrentTime && direction == 0){
                    if (currentCylinder <= tmp2->cylinder && (tmpCurrentCylinder < 0 || tmpCurrentCylinder > tmp2->cylinder)){
                        tmpCurrentCylinder = tmp2->cylinder;
                        tmp3 = create_node("Node", tmp2->arrivalTime, tmp2->arrivalTime, tmp2->lbn, tmp2->requestSize, tmp2->finishTime,
                                            tmp2->waitingTime, tmp2->psn, tmp2->cylinder, tmp2->surface, tmp2->sectorOffset, tmp2->seekDistance, 0);
                    }
                }else if (tmp2->arrivalTime < holdCurrentTime && direction == 1){
                    if (currentCylinder >= tmp2->cylinder && (tmpCurrentCylinder < 0 || tmpCurrentCylinder < tmp2->cylinder)){
                        tmpCurrentCylinder = tmp2->cylinder;
                        tmp3 = create_node("Node", tmp2->arrivalTime, tmp2->arrivalTime, tmp2->lbn, tmp2->requestSize, tmp2->finishTime,
                                            tmp2->waitingTime, tmp2->psn, tmp2->cylinder, tmp2->surface, tmp2->sectorOffset, tmp2->seekDistance, 0);
                    }
                }
                tmp2 = tmp2->next;
                flag2 += 1;
            }
        }
        currentCylinder = tmp3->cylinder;
        insert_tail(tmp3, listSSTF);
        if (flag3 == 1){
            if(direction == 1){
                extraDistance = (499999 - tmp3->cylinder) + (499999 - listSSTF->tail->prev->cylinder);
            }else{
                extraDistance = tmp3->cylinder + listSSTF->tail->prev->cylinder;
                extraDistance *= -1;
            }
            tmp3->extraSeekDistance = extraDistance;
        }
        simulate_fcfs(listSSTF, (flag+1));
        holdCurrentTime = listSSTF->tail->finishTime;
        remove_by_id(tmp3->arrivalTime, list);
        tmp = list->head;
        flag += 1;
    }
    write_output_file(outFilename, listSSTF, count);
}

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Usage: %s <inputfile> <outputfile> <algorithm> [limit]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];
    const char *algorithm = argv[3];
    int limit = -1;

    if (argc == 5) {
        limit = atoi(argv[4]);
    }

    struct List *list = create_list();
    int total_requests = 0;

    read_input_file(input_file, list, &total_requests);

    if (limit > 0 && limit < total_requests) {
        total_requests = limit;
    }
    if (strcmp(algorithm, "FCFS") == 0 || strcmp(algorithm, "fcfs") == 0) {
        simulate_fcfs(list, total_requests);
        write_output_file(output_file, list, total_requests);
    } else if (strcmp(algorithm, "SSTF") == 0 || strcmp(algorithm, "sstf") == 0) {
        simulate_sstf(list, total_requests, output_file);
    } else if (strcmp(algorithm, "CLOOK") == 0 || strcmp(algorithm, "clook") == 0) {
        simulate_clook(list, total_requests, output_file);
    } else if (strcmp(algorithm, "SCAN") == 0 || strcmp(algorithm, "scan") == 0) {
        simulate_scan(list, total_requests, output_file);
    } 
    else {
        fprintf(stderr, "Unknown algorithm: %s\n", algorithm);
        destroy_list(list);
        // free(list);
        return EXIT_FAILURE;
    }

    destroy_list(list);
    // free(list);
    return EXIT_SUCCESS;
}
