#ifndef _MYI2C_H_
#define _MYI2C_H_

#include <stddef.h>
#include "fx2.h"
#include "fx2regs.h"

#define TAG_24LC256 0x51

BYTE EEPROMReadOneByte(WORD addr);

#endif
