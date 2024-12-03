#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <zlib.h>

#define PIPE_PATH "/mnt/mta/server_pipe"   
#define CONFIG_PATH "/mnt/mta/config.txt"   
#define MINER_PATH "/mnt/mta/miner_pipe_%d"  
#define LOG_FILE "/var/log/mtacoin.log" 

// Block structure
typedef struct {
    int height;
    int timestamp;
    unsigned int hash;
    unsigned int prev_hash;
    int difficulty;
    int nonce;
    int relayed_by;
} BLOCK_T;


BLOCK_T curr_block;
BLOCK_T prev_block;
int num_miners=0;
int next_miner_id=1;
int difficulty;


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
//send message to mtacon.log
void log_message(const char *message) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        fprintf(log, "%s\n", message);
        fclose(log);
    }

}
//open config file and read the difficulty
void readDifficulty() {
    char d[10];
    FILE *file = fopen(CONFIG_PATH, "r");
    if (file == NULL) {
        perror("Error opening difficulty configuration file");
        exit(4);// EXIT_FAILURE
    }
    fscanf(file, "difficulty=%s", d);
    difficulty=atoi(d);

    char messege[256];
    sprintf(messege,"reading difficulty %d\n" , difficulty);
    log_message(messege);
    fclose(file);
}


void createGenesisBlock() {
    curr_block.height = 0;
    curr_block.timestamp = time(NULL);
    curr_block.hash = 1;
    curr_block.prev_hash = 0;
    curr_block.difficulty = difficulty;
    curr_block.nonce = 0;
    curr_block.relayed_by = -1; // No miner yet
}

void handleNewMiner(int server_fd,int miner_id) {
    num_miners++;
    char miner_pipe[256];

    char messege[256];
    sprintf(messege,"New miner subscribed with id: %d\n\n", miner_id);
    log_message(messege);

    //printf("New miner subscribed with id: %d\n\n", miner_id);
    snprintf(miner_pipe, sizeof(miner_pipe), MINER_PATH, miner_id);
    int miner_fd = open(miner_pipe, O_WRONLY);
    if (miner_fd == -1) {
        perror("Error opening miner pipe");
        return;
    }
    write(miner_fd, &curr_block, sizeof(BLOCK_T));
    close(miner_fd);
    
}

void handleNewBlock(int server_fd, int miner_id,BLOCK_T *block) {
    char messege[256];
    BLOCK_T newBlock;
    memcpy(&newBlock, block, sizeof(BLOCK_T));    
    // Proof of work
    unsigned int hash = calculateChecksum(&newBlock);
    int validHash = 1;
    for (int i = 0; i < newBlock.difficulty; ++i) {
        if ((hash & (1 << i)) != 0) {
            validHash = 0;
            break;
        }
    }
    if (!validHash) {
        printf("Error: Invalid hash difficulty.\n");
        return;
    }
    // Block is valid, add it to the chain
    newBlock.hash = hash;
    newBlock.height++;
    newBlock.prev_hash=curr_block.hash;
    memcpy(&prev_block, &curr_block, sizeof(BLOCK_T));
    memcpy(&curr_block, &newBlock, sizeof(BLOCK_T));

     if(prev_block.hash==curr_block.hash || prev_block.height==curr_block.height){
            return;
    }

    sprintf(messege,"Server: Block added by miner#%d attribtes: height(%d), timestamp(%d), hash(0x%x), prev_hash(0x%x), difficulty(%d), noce(%d) \n\n", newBlock.relayed_by, newBlock.height, newBlock.timestamp, newBlock.hash, newBlock.prev_hash, newBlock.difficulty, newBlock.nonce);
    log_message(messege);
   

    // Broadcast the new block to all miners
    char miner_pipe[256];
    for (int i = 1; i <= num_miners; ++i) {
        snprintf(miner_pipe, sizeof(miner_pipe), MINER_PATH, i);
        int miner_fd1 = open(miner_pipe, O_WRONLY | O_NONBLOCK);
        if (miner_fd1 == -1) {
            continue;
        }
        write(miner_fd1, &newBlock, sizeof(BLOCK_T));
        close(miner_fd1);
    }
}

int main() {
    char messege[256];
    char miner_pipe[256];
    int miner_id=1;
    while(miner_id<10){//reset all pipes
        snprintf(miner_pipe, sizeof(miner_pipe), MINER_PATH, miner_id);
        if(access(miner_pipe,F_OK)==0){
            unlink(miner_pipe);
        }
        miner_id++;
    }
    // Create server pipe
    mkfifo(PIPE_PATH, 0666);

    sprintf(messege,"listening on %s\n",PIPE_PATH);
    log_message(messege);
    readDifficulty();
    createGenesisBlock();
    char buffer[256];

    while (1) {
        int server_fd = open(PIPE_PATH, O_RDONLY);
        if (server_fd == -1) {
            perror("Error opening server pipe");
            exit(2);//EXIT_FAILURE
        }
    
        BLOCK_T block;
        int msg_type[2];
        read(server_fd, buffer ,sizeof(buffer));
        memcpy(msg_type,buffer, sizeof(int)*2);
        memcpy(&block,buffer+sizeof(int)*2, sizeof(BLOCK_T));
        

         close(server_fd);
        if (msg_type[0] == 1) {
            handleNewMiner(server_fd,msg_type[1]);
        } else if (msg_type[0] == 2) {
            handleNewBlock(server_fd,msg_type[1],&block);
        }
        
    }
    
    unlink(PIPE_PATH);
    return 0;
}

