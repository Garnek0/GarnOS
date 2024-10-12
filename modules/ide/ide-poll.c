#include "ide.h"

uint8_t ide_poll(ide_channel_t* channel, uint8_t reg, uint8_t bit, bool checkErrors){
    while(ide_read_reg(channel, reg) & bit);

    uint8_t err = ide_error(ide_read_reg(channel, ATA_REG_ERROR));

    if(checkErrors){
        if(ide_read_reg(channel, ATA_REG_STATUS) & ATA_SR_ERR) err;
    }

    return err;
}
