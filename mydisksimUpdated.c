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

// typedef struct {
//     double arrival_time;
//     int lbn;
//     int request_size;
//     double finish_time;
//     double waiting_time;
//     int psn;
//     int cylinder;
//     int surface;
//     double sector_offset;
//     int seek_distance;
// } Request;

// void read_input_file(const char *filename, struct List *list, int *count);
// void write_output_file(const char *filename, struct List *list, int count);
// void simulate_fcfs(struct List *list, int count);
// void simulate_sstf(Request *requests, int count);
// void simulate_scan(Request *requests, int count);
// void simulate_clook(Request *requests, int count);

struct Node {
    char *name;
    double id;
    double arrival_time;
    int lbn;
    int request_size;
    double finish_time;
    double waiting_time;
    int psn;
    int cylinder;
    int surface;
    double sector_offset;
    int seek_distance;
    struct Node *next;
    struct Node *prev;
};

struct List {
    struct Node *head;
    struct Node *tail;
};

struct Node *create_node(char *name, double id, double arrival_time, int lbn,
                        int request_size, double finish_time, double waiting_time,
                        int psn, int cylinder, int surface, double sector_offset,int seek_distance) {
    struct Node *node = malloc(sizeof(struct Node));
    if (node == NULL) {
        fprintf (stderr, "%s: Couldn't create memory for the node; %s\n", "linkedlist", strerror(errno));
        exit(-1);
    }
    node->name = strdup(name);
    node->id = id;
    node->arrival_time = arrival_time;
    node->lbn = lbn;
    node->request_size = request_size;
    node->finish_time = finish_time;
    node->waiting_time = waiting_time;
    node->psn = psn;
    node->cylinder = cylinder;
    node->surface = surface;
    node->sector_offset = sector_offset;
    node->seek_distance = seek_distance;
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
        tmpPtr = create_node("Node", ptr->id, ptr->arrival_time, ptr->lbn, ptr->request_size, ptr->finish_time,
                                ptr->waiting_time, ptr->psn, ptr->cylinder, ptr->surface, ptr->sector_offset, ptr->seek_distance);
        insert_sorted(tmpPtr, listInOrder);
        ptr = ptr->next;
    }
    list->head = NULL;
    list->tail = NULL;
    ptr = listInOrder->head;
    while (ptr != NULL) {
        tmpPtr = create_node("Node", ptr->id, ptr->arrival_time, ptr->lbn, ptr->request_size, ptr->finish_time,
                                ptr->waiting_time, ptr->psn, ptr->cylinder, ptr->surface, ptr->sector_offset, ptr->seek_distance);
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

    double arrival_time = 0.0;
    int lbn  = 0;
    int request_size = 0;
    int psn = 0;
    struct Node *tmp = NULL;

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%lf %d %d", &arrival_time, &lbn, &request_size);
        psn = lbn * 8 + request_size;
        tmp = create_node("Node",(psn / (TRACKSPERCYLINDER * SECTORSPERTRACK)), arrival_time, lbn, request_size,
                                        0, 0, psn, (psn / (TRACKSPERCYLINDER * SECTORSPERTRACK)),
                                        ((psn / SECTORSPERTRACK) % TRACKSPERCYLINDER),
                                        (psn % SECTORSPERTRACK), 0);
        insert_tail(tmp, list);
        (*count)++;
    }
    // print_list(list);

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
                tmp->arrival_time, tmp->finish_time, tmp->waiting_time, tmp->psn,
                tmp->cylinder, tmp->surface, tmp->sector_offset, tmp->seek_distance);
        tmp = tmp->next;
        flag += 1;
    }
    fclose(file);
}

void simulate_fcfs(struct List *list, int count) {
    double previousFinishTime = 0;
    int current_cylinder = 0;
    double sectorOffSetAddition = 0;
    int flag = 0;
    printf("COUNT:%d", count);
    struct Node *tmp = list->head;
    while(tmp != NULL && flag < count){
        if (tmp == list->head){ //first one
            tmp->waiting_time = 0;
        }else{
            tmp->waiting_time = previousFinishTime - tmp->arrival_time;
        }
        tmp->seek_distance = abs(tmp->cylinder - current_cylinder);

        double seekTime = (0.000028 * (abs(tmp->cylinder - current_cylinder))) + 2;
        seekTime /= 1000;
        current_cylinder = tmp->cylinder;

        double transferTime = (8.0 * tmp->request_size * PHYSICALSECTORSIZE) / (1024.0*1024*1024);

        double rotationalLatency = 0.0;
        double updatedCurrentSectorOffset = ((seekTime*1000.0) / ((60.0*1000.0)/(SECTORSPERTRACK*RPM))) + sectorOffSetAddition;
        double targetSectorOffset = tmp->sector_offset - tmp->request_size;
        if (updatedCurrentSectorOffset > SECTORSPERTRACK){
            updatedCurrentSectorOffset -= SECTORSPERTRACK;
        }
        if (updatedCurrentSectorOffset < targetSectorOffset){
            rotationalLatency = targetSectorOffset - updatedCurrentSectorOffset;
        }else{
            rotationalLatency = SECTORSPERTRACK - updatedCurrentSectorOffset + targetSectorOffset;
        }
        rotationalLatency *= (60.0*1000.0)/(SECTORSPERTRACK*RPM);
        rotationalLatency /= 1000;
        sectorOffSetAddition = tmp->sector_offset;

        double serviceTime = seekTime + transferTime + rotationalLatency;
        tmp->finish_time = serviceTime + tmp->arrival_time + tmp->waiting_time;
        previousFinishTime = tmp->finish_time;
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
    struct List *listTmp = create_list();
    struct Node *tmp = list->head;
    struct Node *tmp2 = NULL;
    struct Node *tmp3 = list->head;
    int flag = 0; int flag2 = 0;
    int current_cylinder = 0;
    while(tmp != NULL && flag < count){
        flag2 = 0;
        tmp3 = list->head;
        while(tmp3 != NULL && flag2 < (count - flag)){
            double seekTime = (0.000028 * (abs(tmp->cylinder - current_cylinder))) + 2;
            seekTime /= 1000;
            printf("seek time: %f & arrival time:%f\n", seekTime, tmp->arrival_time);
            tmp2 = create_node("Node", seekTime, tmp->arrival_time, tmp->lbn, tmp->request_size, tmp->finish_time,
                                    tmp->waiting_time, tmp->psn, tmp->cylinder, tmp->surface, tmp->sector_offset, tmp->seek_distance);
            insert_sorted(tmp2, listTmp);
            flag2 += 1;
            tmp3 = tmp3->next;
        }
        insert_tail(listTmp->head, listSSTF);
        current_cylinder = listTmp->head->cylinder;
        remove_by_id(listTmp->head->id, list);
        tmp = tmp->next;
        flag += 1;
    }

    print_list(listSSTF);
    simulate_fcfs(listSSTF, count);
    // insertion_sort_by_ID_increasing(listSSTF);
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

    // Request *requests = NULL;
    struct List *list = create_list();
    int total_requests = 0;

    read_input_file(input_file, list, &total_requests);

    if (limit > 0 && limit < total_requests) {
        total_requests = limit;
    }
    if (strcmp(algorithm, "FCFS") == 0) {
        simulate_fcfs(list, total_requests);
        write_output_file(output_file, list, total_requests);
    } else if (strcmp(algorithm, "SSTF") == 0) {
        simulate_sstf(list, total_requests, output_file);
    } 
    //else if (strcmp(algorithm, "SCAN") == 0) {
    //     simulate_scan(requests, total_requests);
    // } else if (strcmp(algorithm, "CLOOK") == 0) {
    //     simulate_clook(requests, total_requests);
    // } 
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
