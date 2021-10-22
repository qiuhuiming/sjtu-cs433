#include <mpich/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_LENGTH 20

struct PassMessage {
    int is_end;
    char pass_string[100];

};

struct RecordMessage {
    int is_last;
    int id;
    char word[MAX_WORD_LENGTH];
};

void SendPassStringMessage(struct PassMessage *msg, int dest) {
    MPI_Send(&msg->is_end, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    MPI_Send(msg->pass_string, 100, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
}

void ReceivePassStringMessage(struct PassMessage *msg, int src, struct MPI_Status *st) {
    MPI_Recv(&msg->is_end, 1, MPI_INT, src, 0, MPI_COMM_WORLD, st);
    MPI_Recv(msg->pass_string, 100, MPI_CHAR, src, 0, MPI_COMM_WORLD, st);
}

void SendRecordMessage(struct RecordMessage *msg, int dest) {
    MPI_Send(&msg->is_last, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    MPI_Send(&msg->id, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    MPI_Send(msg->word, MAX_WORD_LENGTH, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
}

void ReceiveRecordMessage(struct RecordMessage *msg, int src, struct MPI_Status *st) {
    MPI_Recv(&msg->is_last, 1, MPI_INT, src, 0, MPI_COMM_WORLD, st);
    MPI_Recv(&msg->id, 1, MPI_INT, src, 0, MPI_COMM_WORLD, st);
    MPI_Recv(msg->word, MAX_WORD_LENGTH, MPI_CHAR, src, 0, MPI_COMM_WORLD, st);
}

int NextSender(int current_sender, int total) {
    current_sender++;
    if (current_sender >= total) {
        current_sender = 1;
    }
    return current_sender;
}

int LastSender(int current_sender, int total) {
    current_sender--;
    if (current_sender == 0) {
        current_sender = total - 1;
    }
    return current_sender;
}

char *NextWordPtr(char *sentense, int *trimmed_word_length) {
    char *p = sentense;
    *trimmed_word_length = 0;
    while (*p != '\0' && *p != ' ') {
        p++;
        *trimmed_word_length++;
    }
    if (*p == '\0') {
        return NULL;
    }

    while (*p != '\0' && *p == ' ') {
        p++;
    }
    if (*p == '\0') {
        return NULL;
    }
    return p;
}

int main() {
    int comm_sz;
    int my_rank;
    char origin_string[100] =
        "Multicore Architecture and Parallel Programming Assignment on OpenMPI "
        "Programming";
    struct PassMessage pass_buf;
    struct RecordMessage record_buf;
    struct MPI_Status st;
    strcpy(pass_buf.pass_string, "");
    strcpy(record_buf.word, "");

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == 0) {
        strcpy(pass_buf.pass_string, origin_string);
        printf("copy origin: %s\n", pass_buf.pass_string);
        pass_buf.is_end = 0;
        MPI_Send(&pass_buf, sizeof(struct PassMessage), MPI_CHAR, 1, 0,
                 MPI_COMM_WORLD);
        printf("after send\n");
        MPI_Recv(&record_buf, sizeof(struct RecordMessage), MPI_CHAR, 1, 0, MPI_COMM_WORLD, &st);
        printf("is_last:%d, id:%d, word:%s\n", record_buf.is_last, record_buf.id, record_buf.word);
    } else {
        struct MPI_Status st;
        MPI_Recv(&pass_buf, sizeof(struct PassMessage), MPI_CHAR, 0, 0, MPI_COMM_WORLD, &st);
        int trim = 0;
        char* nextSentense = NextWordPtr(pass_buf.pass_string, &trim);
        strncpy(record_buf.word, "absdshaljkff\0", 20);
        record_buf.is_last = 1;
        record_buf.id = my_rank;
        MPI_Send(&record_buf, sizeof(struct RecordMessage), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}