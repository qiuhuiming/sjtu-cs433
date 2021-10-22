#include <mpich/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_WORD_LENGTH 20
#define TOTAL_STRING_LENGTH 200

struct PassMessage {
    int is_end;
    char pass_string[TOTAL_STRING_LENGTH];
};

struct RecordMessage {
    // 0: word is string
    // 1: word is id, and exit
    // 2: exit
    int code;
    int id;
    char word[MAX_WORD_LENGTH];
};

void SendPassStringMessage(struct PassMessage *msg, int dest) {
    MPI_Send(&msg->is_end, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    MPI_Send(msg->pass_string, 100, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
}

void ReceivePassStringMessage(struct PassMessage *msg, int src,
                              struct MPI_Status *st) {
    MPI_Recv(&msg->is_end, 1, MPI_INT, src, 0, MPI_COMM_WORLD, st);
    MPI_Recv(msg->pass_string, 100, MPI_CHAR, src, 0, MPI_COMM_WORLD, st);
}

void SendRecordMessage(struct RecordMessage *msg, int dest) {
    MPI_Send(&msg->code, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    MPI_Send(&msg->id, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
    MPI_Send(msg->word, MAX_WORD_LENGTH, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
}

void ReceiveRecordMessage(struct RecordMessage *msg, int src,
                          struct MPI_Status *st) {
    MPI_Recv(&msg->code, 1, MPI_INT, src, 0, MPI_COMM_WORLD, st);
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
        (*trimmed_word_length)++;
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
    // char origin_string[TOTAL_STRING_LENGTH] =
    //     "Multicore Architecture and Parallel Programming Assignment on
    //     OpenMPI " "Programming\0";
    char origin_string[TOTAL_STRING_LENGTH] =
        "aaaaa bbbbb ccccc ddddd eeeeee ffffff gggg hhhhh iiiiiii jjjj kkkk "
        "llll mmmmm nnnnn\0";
    char sentense_buf[TOTAL_STRING_LENGTH];
    struct PassMessage pass_buf;
    struct RecordMessage record_buf;
    strcpy(pass_buf.pass_string, "\0");
    strcpy(record_buf.word, "\0");
    struct MPI_Status st;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == 0) {
        strcpy(pass_buf.pass_string, origin_string);
        // printf("copy origin: %s\n", pass_buf[0].pass_string);
        int total_sender = comm_sz - 1;
        pass_buf.is_end = 0;
        MPI_Send(&pass_buf, sizeof(struct PassMessage), MPI_CHAR, 1, 0,
                 MPI_COMM_WORLD);
        int current_sender = 1;
        printf("child_id\trecord\n");
        while (total_sender > 0) {
            // printf("try to receive from %d; remain sender: %d\n",
            //    current_sender, total_sender);
            MPI_Recv(&record_buf, sizeof(struct RecordMessage), MPI_CHAR,
                     current_sender, 0, MPI_COMM_WORLD, &st);
            if (record_buf.code) {
                total_sender--;
                if (record_buf.code == 1) {
                    printf("#%d\t\t%s\n", record_buf.id, record_buf.word);
                    printf("end.\n");
                }
            } else {
                printf("#%d\t\t%s\n", record_buf.id, record_buf.word);
            }

            current_sender = NextSender(current_sender, comm_sz);
        }
        printf("return.\n");
    } else {
        // We use a boolean variable `first_recv` to ensure that child #1 can
        // receive first message correctly.
        int first_recv = 1;
        while (1) {
            // Receive from last sender.
            MPI_Recv(&pass_buf, sizeof(struct PassMessage), MPI_CHAR,
                     first_recv ? my_rank - 1 : LastSender(my_rank, comm_sz), 0,
                     MPI_COMM_WORLD, &st);
            // printf("slave #%d: receive: %s\n", my_rank, pass_buf.pass_string);
            first_recv = 0;

            if (pass_buf.is_end) {
                // Notify the master that the process is end.
                record_buf.code = 2;
                record_buf.id = my_rank;
                MPI_Send(&record_buf, sizeof(struct RecordMessage), MPI_CHAR, 0,
                         0, MPI_COMM_WORLD);
                // Send end signal to next sender.
                MPI_Send(&pass_buf, sizeof(struct PassMessage), MPI_CHAR,
                         NextSender(my_rank, comm_sz), 0, MPI_COMM_WORLD);
                break;
            }

            // Trim current word.
            int trimmed = 0;
            char *next_sentense = NextWordPtr(pass_buf.pass_string, &trimmed);

            if (next_sentense == NULL) {
                // End.
                record_buf.code = 1;
                record_buf.id = my_rank;
                sprintf(record_buf.word, "%d", my_rank);
                MPI_Send(&record_buf, sizeof(struct RecordMessage), MPI_CHAR, 0,
                         0, MPI_COMM_WORLD);

                pass_buf.is_end = 1;
                *pass_buf.pass_string = '\0';
                MPI_Send(&pass_buf, sizeof(struct PassMessage), MPI_CHAR,
                         NextSender(my_rank, comm_sz), 0, MPI_COMM_WORLD);

                break;
            }

            record_buf.code = 0;
            record_buf.id = my_rank;
            strncpy(record_buf.word, pass_buf.pass_string, trimmed);
            record_buf.word[trimmed] = '\0';
            MPI_Send(&record_buf, sizeof(struct RecordMessage), MPI_CHAR, 0, 0,
                     MPI_COMM_WORLD);

            // Keep on passing string.
            strncpy(sentense_buf, next_sentense, strlen(next_sentense));
            strncpy(pass_buf.pass_string, sentense_buf, strlen(next_sentense));
            pass_buf.pass_string[strlen(next_sentense)] = '\0';
            // printf("slave #%d: send %s\n", my_rank,pass_buf.pass_string);
            MPI_Send(&pass_buf, sizeof(struct PassMessage), MPI_CHAR,
                     NextSender(my_rank, comm_sz), 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}