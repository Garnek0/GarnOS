#ifndef RBLOGS_H
#define RBLOGS_H

#include <types.h>

#define RINGBUFFER_ENTRIES 32
#define RB_LOG_MAX_STRLEN 128

typedef struct _rb_entry {
    char log[RB_LOG_MAX_STRLEN];
    uint8_t status; //KLOG_OK, KLOG_FAILED etc.

    struct _rb_entry* next;
} rb_entry_t;
extern rb_entry_t RBEntries[32];

void rb_init();
void rb_log(char* log, uint8_t status);

#endif //RBLOGS_H