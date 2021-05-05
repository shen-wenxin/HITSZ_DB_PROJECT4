#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"


int main(int argc, char **argv)
{
    Buffer buf; /* A buffer */
    unsigned char *blk; /* A pointer to a block */
    int i = 0;

    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    // for(int blk_id = 17;blk_id <= 48;blk_id ++){
    //     if ((blk = readBlockFromDisk(blk_id, &buf)) == NULL){
    //         perror("Reading Block Failed!\n");
    //         return -1;
    //     }
    //     int C = -1,D = -1,addr = -1;
    //     char str[5];
    //     for (i = 0; i < 7; i++){
    //         for (int k = 0; k < 4; k++){
    //             str[k] = *(blk + i*8 + k);
    //         }
    //         C = atoi(str);
    //         for (int k = 0; k < 4; k++){
    //             str[k] = *(blk + i*8 + 4 + k);
    //         }
    //         D = atoi(str);
    //         if(C == 50){
    //             printf("(%d %d)\n",C,D);
    //         }
    //     }
    //     for (int k = 0; k < 4; k++)
    //     {
    //         str[k] = *(blk + i*8 + k);
    //     }
    //     addr = atoi(str);
    //     freeBlockInBuffer(blk, &buf);
        
    // }
    for(int tt = 600;tt <=602;tt ++ ){
        printf("\nblock: %d\t",tt);
        if ((blk = readBlockFromDisk(tt, &buf)) == NULL){
            perror("Reading Block Failed!\n");
            return -1;
        }
        int C = -1,D = -1,addr = -1;
        char str[5];
        for (i = 0; i < 7; i++){
            for (int k = 0; k < 4; k++){
                str[k] = *(blk + i*8 + k);
            }
            C = atoi(str);
            for (int k = 0; k < 4; k++){
                str[k] = *(blk + i*8 + 4 + k);
            }
            D = atoi(str);
            printf("(%d %d)",C,D);
        }
        for (int k = 0; k < 4; k++)
        {
            str[k] = *(blk + i*8 + k);
        }
        addr = atoi(str);
        freeBlockInBuffer(blk,&buf);

    }
     
    return 0;
}

