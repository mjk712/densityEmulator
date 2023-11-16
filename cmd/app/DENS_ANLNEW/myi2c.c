#include "myi2c.h"

static xdata BYTE xfer_out[2];
static xdata BYTE xfer_in;

BYTE EEPROMReadOneByte(WORD addr)
{
  xfer_out[0] = (addr >> 8) & 0xFF;
  xfer_out[1] =  addr & 0xFF;
  EZUSB_WriteI2C(TAG_24LC256, 2, &xfer_out[0]);
  EZUSB_ReadI2C(TAG_24LC256, 1, &xfer_in);  
  return xfer_in;
}
