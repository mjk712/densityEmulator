/**
 * @authors
 * © Сучилин   М. Л., 2004-2006
 * © Степченко М. В., 2011-2015
 * @brief Формирователь аналоговых сигналов (ФАС)
 * @file AnlBrt.c
 * @details
 * Проект: ИПК-3 
 * Микроконтроллер: CY7C64713
 * Отдел: СКБ 103
 */

//*****************************************************************************
//                            ЗАГОЛОВОЧНЫЕ ФАЙЛЫ
//****************************************************************************/

#include "fx2.h"
#include "fx2regs.h"
#include "syncdly.h"            // макрос SYNCDELAY
#include "AnlBrt.h"
#include "ad5420.h"
#include "myi2c.h"
#include <string.h>
#include <intrins.h>

//*****************************************************************************
//                            ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
//****************************************************************************/

//! Значения аналоговых сигналов
xdata AnlSignals valSignals;
xdata volatile ANL_CORRECTION corr_data;
xdata volatile DENSITY_TEST_DATA dtd;
extern BOOL bAnlSgnlsUpdated;

//*****************************************************************************
//                            ОБЪЯВЛЕНИЯ ФУНКЦИЙ
//****************************************************************************/

void UpdateAnalog();
unsigned short UpdateBinary();

//*****************************************************************************
//                            МАКРОСЫ
//****************************************************************************/

//! Коэффициент для подсчёта частоты
#define FRQ_COEFF 4000

//*****************************************************************************
//                            РЕАЛИЗАЦИЯ ФУНКЦИЙ
//****************************************************************************/

/** @defgroup maingroup Основные функции
 *  Эти функции реализуют основную функциональность устройства
 *  @{
 */

/**
 * @brief  Инициализация формирователя аналоговых сигналов
 *
 * Здесь инициализируется процессор, порты ввода-вывода. Происходит начальная
 * инициализация аналоговых сигналов.
 */
void InitAnlBrt(void)
{
  BYTE i;
  BYTE * pBytePtr;
  
  CPUCS |= bmCLKSPD1; // Микроконтроллер функционирует на частоте 48 МГц.
                      // Данный параметр влияет на генерацию частот.
  
  // максимально быстрый доступ к памяти - MOVX за два такта (по умолчанию 3)
  CKCON &= 0xF8; 

  AUTOPTRSETUP |= 7;

  //=======================================================

  // Отключение всех альтернативных функций портов A, E и С:
  PORTACFG = 0;
  PORTECFG = 0;
  PORTCCFG = 0;   
  WAKEUPCS &= ~bmWU2EN;
  IFCONFIG &= ~bmIFCFG0;
  IFCONFIG &= ~bmIFCFG1;

  //=======================================================
  // Конфигурация портов
  
  OEA = 0x00; //входы
  OEE = 0x00; //входы
  OEC |= bmBIT2|bmBIT3|bmBIT4|bmBIT5; // выходы
  OEB |= bmBIT0|bmBIT1|bmBIT2|bmBIT3; // выходы, bmBIT4 вход
  OED |= bmBIT0|bmBIT4|bmBIT5|bmBIT6|bmBIT7; // выходы, bmBIT1 вход  
  
  //=======================================================

  IP = 0x08;
  TMOD = 0x21;
  TH0 = 0x3C;
  TL0 = 0xBF;
  TH1 = 0x06;
  TL1 = 0x06;
  TR0 = 1;
  ET0 = 1;
  //TR1 = 1; отключен
  //ET1 = 1;

  memset((void*)&valSignals, 0x00, sizeof(valSignals)); 
  memset((void*)&dtd, 0x00, sizeof(dtd)); 
  dtd.base_pressure = 60292;
  
  Init_AD5420();

  EZUSB_InitI2C();
  
  pBytePtr = (BYTE*)&corr_data.corr[0];
  for (i = 0; i < sizeof(corr_data); i++)
  {
    *pBytePtr++ = EEPROMReadOneByte(i + EEPROM_LOC);
  }

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
* @brief  Первый таймер.
*
* Обновление аналоговых сигналов. Подтверждение работы
* микроконтроллера для предотвращения его сброса.
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
  
  TH0 = 0x3C; // 80 раз в секунду
  TL0 = 0xBF;

  if (++counter > 20) // 4 раза в секунду
  {
    counter = 0;
    ready = !ready;
  }
  if (ready)
  {    
    IOC |= bmBIT5;
  }
  else
  {
    IOC &= ~bmBIT5;
  }

  ///if (dtd.enable)
  ///{
    ///valSignals.anlgSgnls[8] = 3000;
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
        bAnlSgnlsUpdated = TRUE;
      break;
      
      case DTD_HIGH:
        valSignals.anlgSgnls[7] = dtd.top_pressure;
        bAnlSgnlsUpdated = TRUE;
        if (0 == counter_sec)
        {
          dtd_state = DTD_MID;
          counter_sec = dtd.num_sec * 80;
        }
      break;
        
      case DTD_MID:
        valSignals.anlgSgnls[7] = dtd.mid_pressure;
        bAnlSgnlsUpdated = TRUE;
        if (0 == counter_sec)
        {
          dtd_state = DTD_LOW;
        }
      break;
      
      case DTD_LOW:
        valSignals.anlgSgnls[7] = dtd.low_pressure;
        bAnlSgnlsUpdated = TRUE;
      break;
    }
    valSignals.anlgSgnls[8] = 20000;
    
    if (counter_sec > 0) 
    {
      counter_sec--; //80 раз в секунду
    }
  ///}
}
#if 0
/**
 * @brief  Второй таймер.
 * Здесь обрабатываются частотные и двоичные сигналы.
 */
void int_timer_1(void) interrupt 3
{
  static USHORT counter[4] = { 0, 0, 0, 0 };
  static USHORT bits[4] = { 0x10, 0x20, 0x40, 0x80 };
  static BYTE state_frq = 0xFF;
  static BYTE half[4] = { 0, 0, 0, 0 }; // делитель частоты на 2.
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

          // обнулить все биты, которые равны 0 в state_frq (но не трогая младшие 4 бита)
          IOD &= (state_frq | 0x0F);
          // поставить 1 на все биты, которые равны 1 в state_frq
          IOD |= state_frq;

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

/**
 * @brief  Обновить двоичные сигналы
 * @return Возвращает слово, содержащее двоичные сигналы
 */
unsigned short UpdateBinary(void)
{
  typedef union {
    unsigned short word;  
    unsigned char byte[2];  
  } UNIONWORD;

  UNIONWORD ret;

  // 8 входов ВХ-ДСБУС
  ret.byte[1] = ( ((IOE & 0x1F) << 3) | ((IOA & 0xE0) >> 5) ); // PA5-PA7 младшая часть, PE0-PE4 старшая часть

  // 5 входов ВХ-ДСБУ
  ret.byte[0] = IOA & 0x1F; // PA0-PA4

  return ret.word;
}
#endif
/** @}*/
