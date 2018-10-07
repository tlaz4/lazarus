#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

typedef struct{
    char *buffer;
    int segSize;
    int curPointer;
    int maxPointer;
}bufferC_t;

typedef struct{
    int size;
    int clients[5];
    bufferC_t *joinBuffer;
}list_t;


long getFileLen(FILE *file);
int unpackSizeTag(char *buffer);
long extractMp3(FILE *formatFile, FILE *file);
void streamMedia(FILE *formatFile, long fileLen, list_t *clientList);
void parseSchedule(char *str[4]);
void * handleStreaming(void *arg);
void initBuffer(bufferC_t *buffer);
void addToBuffer(bufferC_t *buffer, char *buff);
void broadcastMedia(list_t *clientList, char *streamBuffer, int length);

/*
        program to stream mp3 audio, tcp server not implemented yet
*/

int main(int argc, char* argv[]){

        int sock, snew;

        struct sockaddr_in server, client;

        char *reply =
                "HTTP/1.1 200 OK\n"
                "Content-Type: audio/mpeg\n"
                "Date: Wed, 17 April 2018 12:27:04 GMT\n"
                "Cache-Control: no-cache, no-store\n"
                "Access-Control-Allow-Origin: *\n"
                "Access-Control-Allow-Headers: Origin, Accept, X-Requested-With, Content-Type\n"
                "Access-Control-Allow-Methods: GET, OPTIONS, HEAD\n"
                "Connection: Close\n"
                "Expires: Mon, 26 Jul 1997 05:00:00 GMT\n"
                "\n";


        bufferC_t joinBuffer;
        initBuffer(&joinBuffer);

        list_t clientList;
        clientList.size = 0;
        clientList.joinBuffer = &joinBuffer;

        sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock < 0){
                perror("Server: cannot open master socket");
                exit(1);
        }

        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr("0.0.0.0");
        server.sin_port = htons(2224);

        if (bind(sock, (struct sockaddr*) &server, sizeof(server))) {
                perror ("Server: cannot bind master socket");
                exit (1);
        }

        listen(sock, 5);

        int clientLength = sizeof(client);

        pthread_t threadId;
        pthread_create(&threadId, NULL, handleStreaming, (void *) &clientList);

        while(1){
            snew = accept(sock, (struct sockaddr*) &client, &clientLength);
            if (snew < 0) {
                    perror ("Server: accept failed");
                    exit (1);
            }
            printf("Client connected!\n");
            send(snew, reply, strlen(reply), 0);
            send(snew, joinBuffer.buffer, joinBuffer.curPointer * 1400, 0);
            clientList.clients[clientList.size] = snew;
            clientList.size++;
        }

}

// get the length in bytes of the mp3 file
long getFileLen(FILE *file){
        fseek(file, 0, SEEK_END);
        long fileLen = ftell(file);
        rewind(file);

        return fileLen;
}

// unpack the size of the id tag for extracting the mp3 data
int unpackSizeTag(char *buffer){
        int a, b, c, d;

        a = buffer[6] & 0x0000007f;
        b = buffer[7] & 0x0000007f;
        c = buffer[8] & 0x0000007f;
        d = buffer[9] & 0x0000007f;

        return (a << 24) | (b << 14 ) | (c << 7) | (d);
}

// extract the mp3 data from the tag, return the new size in bytes of the file
long extractMp3(FILE *formatFile, FILE *file){
        char *buffer;
        int fileLen, tagSize;

        fileLen = getFileLen(file);

        buffer = (char *)malloc(fileLen * sizeof(char));
        fread(buffer, fileLen, 1, file);
        fclose(file);

        if(buffer[0] == 'I' && buffer[1] == 'D' && 1 == 0){
                tagSize = unpackSizeTag(buffer);
                memmove(buffer, buffer + tagSize, fileLen - tagSize);
                fwrite(buffer, 1, fileLen - tagSize, formatFile);
                fclose(formatFile);
        }else{
                fwrite(buffer, 1, fileLen, formatFile);
                tagSize = 0;
        }

        return (fileLen - tagSize);
}

// stream the file in packets of 1400 bytes and output to stdout
void streamMedia(FILE *formatFile, long fileLen, list_t *clientList){
        int i;
        char c;
        char *streamBuffer;
        FILE *stream;

        streamBuffer = (char *)malloc(1400 * sizeof(char));

        //stream = popen("mpg123 -", "w");

        while(1){
                i = 0;
                c = fgetc(formatFile);
                while (i < 1400){
                        streamBuffer[i] = c;
                        c = fgetc(formatFile);
                        if(feof(formatFile)){
                            broadcastMedia(clientList, streamBuffer, i);
                            printf("done\n");
                            return;
                        }
                        i++;
                }
                fseek(formatFile, -1, SEEK_CUR);

                addToBuffer(clientList->joinBuffer, streamBuffer);
                broadcastMedia(clientList, streamBuffer, 1400);

                usleep(100000);
        }
}

// parse schedule.txt to get todays n show playlist
void parseSchedule(char *str[4]){
         FILE * schedule, *show;

        schedule = fopen("schedule.txt", "r");

        for(int i = 0; i < 4; i++){
                fgets(str[i], 200, schedule);

                char* nl = strchr(str[i], '\n');
                if (nl != NULL){
                        *nl = '\0';
                }

                printf("This Show is: %s\n", str[i]);
        }
        fclose(schedule);
}

void * handleStreaming(void * arg){
        FILE *file;
        FILE *formatFile;
        long fileLen;
        char *str[4];
        char buffer[400];

        list_t *clientList = (list_t*)arg;

    // allocate room for 4 200 byte strings for file titles
        for(int i = 0; i < 4; i++){
                str[i] = malloc(sizeof(char) * 200);
        }

        // create a string of file names to be opened throughout program
        parseSchedule(str);

        for(int i = 0; i < 4; i++){
                printf("New Show to stream\n");

                file = fopen(str[i], "rb");

                formatFile = fopen("formattedMp3.mp3", "wb");

                fileLen = extractMp3(formatFile, file);

                formatFile = fopen("formattedMp3.mp3", "rb");
                sleep(5);
                // stream media for each show scheduled

                streamMedia(formatFile, fileLen, clientList);
        }

        return 0;
}

void initBuffer(bufferC_t *buffer){
    buffer->buffer = malloc(35000 * sizeof(char));
    buffer->segSize = 1400;
    buffer->curPointer = 0;
    buffer->maxPointer = 25;
}

void addToBuffer(bufferC_t *buffer, char *buff){
    if(buffer->curPointer < buffer->maxPointer){
         memmove(buffer->buffer + (buffer->curPointer * buffer->segSize), buff, buffer->segSize);
         buffer->curPointer++;
    }else{
        memmove(buffer->buffer,  buffer->buffer + buffer->segSize, (buffer->curPointer - 1) * buffer->segSize);
        memmove(buffer->buffer + (buffer->curPointer - 1) * buffer->segSize, buff, buffer->segSize);
    }
}

void broadcastMedia(list_t *clientList, char *streamBuffer, int length){
    for(int i = 0; i < clientList->size; i++){
        send(clientList->clients[i], streamBuffer, length, 0);
    }
}
