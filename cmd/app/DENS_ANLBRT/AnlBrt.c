/**
 * @authors
 * � �������   �. �., 2004-2006
 * � ��������� �. �., 2011
 * @brief ������������� ���������� �������� (���)
 * @file AnlBrt.c
 * @details
 * ������: ���-3 
 * ���������������: CY7C64713
 * �����: ��� 103
 */

//*****************************************************************************
//                            ������������ �����
//****************************************************************************/

#include "fx2.h"
#include "fx2regs.h"
#include "syncdly.h"            // ������ SYNCDELAY
#include "AnlBrt.h"
#include <string.h>
#include <intrins.h>

//*****************************************************************************
//                            ���������� ����������
//****************************************************************************/

//! �������� ���������� ��������
xdata AnlSignals valSignals;
//! ������ �������� ���������� ��������
xdata AnlSignals oldSignals;

xdata volatile DENSITY_TEST_DATA dtd;

/** 
 * ��� ��������� � ����������, ����������� �� ������ 0x8000 ���������� ������
 * �� ����� ������ "D" �� ������� ������. �� ������ � ������ �����-������ IOD.
 * � ���� ������ �� ����� "D" ���������� �� ��� ���� ������, ��. �����. 
 */
xdata volatile BYTE empty_data _at_ 0x8000;

//*****************************************************************************
//                            ���������� �������
//****************************************************************************/

void UpdateAnlSignal(PAnlSignals newVal, PAnlSignals oldVal);
void SetDCVal(unsigned short val, unsigned char addr);
unsigned short UpdateBinary();

//*****************************************************************************
//                            �������
//****************************************************************************/

//! ���� D - ������� ����� ������ ����������
#define ADDR_LO IOD
//! ���� E - ������� ����� ������ ����������
#define ADDR_HI IOE

//! �������� ��� ���������� �������
#define FRQ_ON  ADDR_LO |=  0x10
//! ��������� ��� ���������� �������
#define FRQ_OFF ADDR_LO &= ~0x10

//! ����������� ��� �������� �������
#define FRQ_COEFF 4000

//*****************************************************************************
//                            ���������� �������
//****************************************************************************/

/** @defgroup maingroup �������� �������
 *  ��� ������� ��������� �������� ���������������� ����������
 *  @{
 */

/**
 * @brief  ������������� ������������� ���������� ��������
 *
 * ����� ���������������� ���������, ����� �����-������. ���������� ���������
 * ������������� ���������� ��������.
 */
void InitAnlBrt(void)
{
  CPUCS |= bmCLKSPD1; // ��������������� ������������� �� ������� 48 ���.
                      // ������ �������� ������ �� ��������� ������.
  
  // ����������� ������� ������ � ������ - MOVX �� ��� ����� (�� ��������� 3)
  CKCON &= 0xF8; 

  AUTOPTRSETUP |= 7;

  //=======================================================

  PORTACFG = 0;  
  IFCONFIG &= ~0x03;
  PORTCCFG = 0xC0;  
 
  OEA = 0xFF;
  OEB = 0x0F;
  OEC = 0xFF;
  OED = 0xFF;
  OEE = 0xFF;

  IOC = 0x3C;
  IOD = 0x00;
  IOE = 0x00;

  IP = 0x08;
  TMOD = 0x21; //48 mhz timer 0 16 bit, timer 1 8 bit w/auto reload
  //timer 48 mhz/12 == 4 mhz
  TH0 = 0x3C; 
  TL0 = 0xBF;
  TH1 = 0x06;
  TL1 = 0x06;
  TR0 = 1;
  ET0 = 1;
  //TR1 = 1;
  //ET1 = 1; ������ 1 �� ��������

  // ��������� ������������� �������� ��� ������ ��������
  memset((void*)&valSignals, 0x00, sizeof(valSignals)); 
  
  memset((void*)&valSignals.anlgSgnls, 0xFF, sizeof(valSignals.anlgSgnls)); 
   
  memset((void*)&oldSignals, 0x00, sizeof(oldSignals));
  
  memset((void*)&dtd, 0x00, sizeof(dtd)); 
  dtd.base_pressure = 3850;
  
  empty_data = 0xFF;
  FRQ_ON;
  FRQ_OFF;

  //=======================================================
  // USB:

  // Endpoint 2

  EP2CFG = 0xA2;
  SYNCDELAY;                    

  EP2BCL = 0;
  SYNCDELAY;
  EP2BCL = 0;
  SYNCDELAY;
}

/**
* @brief  ������ ������.
*
* ���������� ���������� ��������. ������������� ������
* ���������������� ��� �������������� ��� ������.
*/
void int_timer_0(void) interrupt 1
{
  static BOOL ready;
  static BYTE counter;
  static short counter_sec = 0;
  static BYTE dtd_state = DTD_BASE;
  
  //(0xFFFF-0x3CBF) = 49984
  //4000000 / (0xFFFF-0x3CBF) == 80
  
  // dac = 20 == 4095
  
  TH0 = 0x3C; // 80 ��� � �������
  TL0 = 0xBF;

  if (++counter > 20) // 4 ���� � �������
  {
    counter = 0;
    ready = !ready;
  }
  if (ready)
  {    
    IOC |= 0x20;
  }
  else
  {
    IOC &= ~0x20;
  }

  ///if (dtd.enable)
  ///{
    ///valSignals.anlgSgnls[8] = 3000; // ��
    if (dtd.start)
    {
      dtd.start = 0;
      dtd_state = DTD_HIGH;
      counter_sec = 2*80;
    }
    if (dtd.reset)
    {
      dtd.reset = 0;
      dtd_state = DTD_BASE;
    }
    switch (dtd_state)
    {
      default:
      case DTD_BASE:
        valSignals.anlgSgnls[7] = dtd.base_pressure;
      break;
      
      case DTD_HIGH:
        valSignals.anlgSgnls[7] = dtd.top_pressure;
        if (0 == counter_sec)
        {
          dtd_state = DTD_MID;
          counter_sec = dtd.num_sec * 80;
        }
      break;
        
      case DTD_MID:
        valSignals.anlgSgnls[7] = dtd.mid_pressure;
        if (0 == counter_sec)
        {
          dtd_state = DTD_LOW;
        }
      break;
      
      case DTD_LOW:
        valSignals.anlgSgnls[7] = dtd.low_pressure;
      break;
    }
    valSignals.anlgSgnls[8] = 3500;
    
    if (counter_sec > 0) 
    {
      counter_sec--; //80 ��� � �������
    }
  ///}

  UpdateAnlSignal(&valSignals, &oldSignals);
}

/**
 * @brief  ������ ������.
 * ����� �������������� ��������� � �������� �������.
 */
#if 0
void int_timer_1(void) interrupt 3
{
  static USHORT counter[4] = { 0, 0, 0, 0 };
  static USHORT bits[4] = { 0x01, 0x02, 0x04, 0x08 };
  static BYTE state_frq = 0xFF;
  static BYTE half[4] = { 0, 0, 0, 0 }; // �������� ������� �� 2.
  int i;
  
  valSignals.binarySgnls = UpdateBinary();

  for (i = 0; i < 4; i++)
  {
    if (valSignals.freqSgnls[i])
    {
      if (valSignals.freqSgnls[i] >= counter[i])
      {
        if (half[i])
        {
          if (state_frq & bits[i])
          {
            state_frq &= ~bits[i];
          }
          else
          {
            state_frq |= bits[i];
          }
  
          empty_data = state_frq; // �� D ���������� �������� state_frq                
          FRQ_ON;                 // ������� �������      
          FRQ_OFF;                // ��������� �������

          half[i] = 0;
        } else
        { 
          half[i] = 1;
        }
        counter[i] = FRQ_COEFF;  
      }
      else
      {
        counter[i]--;
      }
    }
  }
}
#endif
/**
 * @brief  ���������� ���������� ��������
 * @param[in] newVal ����� �������� ���������� ��������
 * @param[in] oldVal ������ �������� ���������� ��������
 *
 * ������ ���� ������� - �������� �������� �
 * ���������� ����� �������� ��� ������� ���������� ��������
 */
void UpdateAnlSignal(PAnlSignals newVal, PAnlSignals oldVal)
{
  int i;

  for (i = 0; i < ANLG_SGNLS_COUNT; i++)  //14
  {
    if (newVal->anlgSgnls[i] != oldVal->anlgSgnls[i])
    {
      SetDCVal(newVal->anlgSgnls[i], i);
      oldVal->anlgSgnls[i] = newVal->anlgSgnls[i];
    }
  }  
}

/**
 * @brief  ������� ���������� � ��������� ��� �� ��������
 * @param[in] val ��������
 * @param[in] addr ����� ����������
 */
void SetDCVal(unsigned short val, unsigned char addr)
{ 
  // 4-� ��� (������ �� 0) - �������� �� �������.
  // ��������� �� ������� (����������) ����� ���� ����������

  ADDR_LO = 0x00;
  ADDR_HI = 0x00;

  switch (addr)
  {
    case  0: ADDR_LO = 0x01; break;
    case  1: ADDR_LO = 0x02; break;
    case  2: ADDR_LO = 0x04; break;
    case  3: ADDR_LO = 0x08; break;
    // ��� 0x10 �������� �� �������
    case  4: ADDR_LO = 0x20; break;
    case  5: ADDR_LO = 0x40; break;
    case  6: ADDR_LO = 0x80; break;

    case  7: ADDR_HI = 0x01; break;
    case  8: ADDR_HI = 0x02; break;
    case  9: ADDR_HI = 0x04; break;
    case 10: ADDR_HI = 0x08; break;
    case 11: ADDR_HI = 0x10; break;
    case 12: ADDR_HI = 0x20; break;
    case 13: ADDR_HI = 0x40; break;    
  }
  
  IOA =  (BYTE)  val;
  IOB = ((BYTE) (val >> 8)) & 0x0F;   
}

/**
 * @brief  �������� �������� �������
 * @return ���������� �����, ���������� �������� �������
 */
#if 0
unsigned short UpdateBinary(void)
{
  typedef union {
    unsigned short word;  
    unsigned char byte[2];  
  } UNIONWORD;

  UNIONWORD ret;
  
  IOC &= 0xFC;

  IOC |=  0x02; // ������� ����� ����������
  ret.byte[1] = empty_data; // ������ � ����� ������
  IOC &= ~0x02; // ������ �������

  IOC |=  0x01; // ������� ������ ����������
  ret.byte[0] = empty_data; // ������ � ����� ������
  IOC &= ~0x01; // ������ �������

  return ret.word;
}
#endif
/** @}*/