#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <zlib.h>

#define SERVER_PIPE_PATH "/mnt/mta/server_pipe"  
#define MINER_PATH "/mnt/mta/miner_pipe_%d"  
#define LOG_FILE "/var/log/mtacoin.log"  


typedef struct {
    int height;
    int timestamp;
    unsigned int hash;
    unsigned int prev_hash;
    int difficulty;
    int nonce;
    int relayed_by;
} BLOCK_T;
BLOCK_T prev_block;

unsigned int calculateChecksum(const BLOCK_T *block) {
    unsigned char data[sizeof(int)*5];
    int offset = 0;
    memcpy(data + offset, &block->height, sizeof(int));
    offset+=sizeof(int);
    memcpy(data + offset, &block->timestamp, sizeof(int));
    offset+=sizeof(int);
    memcpy(data + offset, &block->prev_hash, sizeof(unsigned int));
    offset+=sizeof(unsigned int);
    memcpy(data + offset, &block->nonce, sizeof(int));
    offset+=sizeof(int);
    memcpy(data + offset, &block->relayed_by, sizeof(int));
    offset+=sizeof(int);
    return crc32(0,data,offset);
}


void log_message(const char *message) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        fprintf(log, "%s\n", message);
        fclose(log);
    }
    
}


int main() {
    char messege[256];
    prev_block.hash=0;
    prev_block.height=-1;
    char miner_pipe[256];
    char buffer[256];
    int miner_id=1;
    while(1){
        snprintf(miner_pipe, sizeof(miner_pipe), MINER_PATH, miner_id);
        if(access(miner_pipe,F_OK)==-1)//check the the next unused pipe number
            break;
        miner_id++;
    }
    //create new pipe with miner id
    mkfifo(miner_pipe, 0666);
    int msg[2];
    msg[0]=1;
    msg[1]=miner_id;

    // Send subscription request to server
    int server_fd = open(SERVER_PIPE_PATH, O_WRONLY);
    if (server_fd == -1) {
        perror("Error opening server pipe");
        exit(EXIT_FAILURE);
    }
    write(server_fd,msg,sizeof(int)*2);
    BLOCK_T temp={0,0,0,0,0,0,0};
    write(server_fd,&temp,sizeof(BLOCK_T));

    sprintf(messege,"miner %d send request to server\n",miner_id);
    log_message(messege);
    close(server_fd);

    // Open miner pipe to read the first block
    int miner_fd = open(miner_pipe, O_RDONLY);
    if (miner_fd == -1) {
        perror("Error opening miner pipe");
        exit(EXIT_FAILURE);
    }

    BLOCK_T current_block;
    read(miner_fd, &current_block, sizeof(BLOCK_T));
    close(miner_fd);

    int miner_fd1 = open(miner_pipe, O_RDONLY | O_NONBLOCK);
        if (miner_fd1 == -1) {
            perror("Error opening miner pipe");
            exit(EXIT_FAILURE);
        }
    while (1) {
        current_block.nonce++;
        current_block.timestamp = time(NULL);
        current_block.relayed_by = miner_id;
        unsigned int hash = calculateChecksum(&current_block);

        // Check the required difficulty
        int validHash = 1;
        for (int i = 0; i < current_block.difficulty; ++i) {
            if ((hash & (1 << i)) != 0) {
                validHash = 0;
                break;
            }
        }
        
        BLOCK_T check;
        int check_hash=current_block.hash;
         int flag=read(miner_fd1, &check, sizeof(BLOCK_T));
        if(flag>0){
             memcpy(&current_block, &check, sizeof(BLOCK_T));
            continue;
        }
        if(current_block.hash!=check_hash){
            memcpy(&current_block, &check, sizeof(BLOCK_T));
            continue;
        }
      
        if (validHash ) {
          if(prev_block.hash==current_block.hash || prev_block.height==current_block.height){
            continue;
            }
            int server_fd2 = open(SERVER_PIPE_PATH, O_WRONLY);
            if (server_fd2 == -1) {
                perror("Error opening server pipe      4");
                exit(EXIT_FAILURE);
            }
            memcpy(&prev_block, &current_block, sizeof(BLOCK_T));
            msg[0] = 2; // Indicating new block mined
            memcpy(buffer, msg ,sizeof(int)*2);
            memcpy(buffer+sizeof(int)*2, &current_block, sizeof(BLOCK_T));
            write(server_fd2, buffer, sizeof(buffer));


            close(server_fd2);

            sprintf(messege,"Miner #%d: Mined a new block #%d with hash 0x%x\n", miner_id, current_block.height +1 , hash);
            log_message(messege);
        }
        
    }
     close(miner_fd1);
    return 0;
}

