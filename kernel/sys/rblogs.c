/*  
*   File: rblogs.c
*
*   Author: Garnek
*   
*   Description: Ringbuffer Logs. This is Just an Experiment. 
*                RBLogs will probably be removed in the near future.
*/
// SPDX-License-Identifier: BSD-2-Clause

#include "rblogs.h"
#include <mem/memutil/memutil.h>

rb_entry_t RBEntries[RINGBUFFER_ENTRIES];
rb_entry_t* currentEntry;

//initialise the ringbuffer that will store the init logs
void rb_init(){
    for(size_t i = 0; i < RINGBUFFER_ENTRIES; i++){
        if(i == RINGBUFFER_ENTRIES-1){
            RBEntries[RINGBUFFER_ENTRIES-1].next = &RBEntries[0];
            break;
        }
        RBEntries[i].next = &RBEntries[i+1]; 
    }
    currentEntry = &RBEntries[0];
}

void rb_log(char* log, uint8_t status){
    if(strlen(log) > RB_LOG_MAX_STRLEN) return;
    memcpy(&currentEntry->log, log, strlen(log));
    currentEntry->status = status;
    currentEntry = currentEntry->next;
}