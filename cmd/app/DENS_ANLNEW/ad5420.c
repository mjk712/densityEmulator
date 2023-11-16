/**
 * @authors
 * © Степченко М. В., 2015
 * @brief Микросхема AD5420
 * @file ad5420.c
 * @details
 * Проект: ИПК-3 
 * Микроконтроллер: CY7C64713
 * Отдел: СКБ 103.2
 */

//*****************************************************************************
//                            ЗАГОЛОВОЧНЫЕ ФАЙЛЫ
//*****************************************************************************

#include <string.h>
#include <stdio.h>
#include <math.h>
#include "anlbrt.h"
#include "ad5420.h"
#include "syncdly.h" // Макрос /*SYNCDELAY*/

extern xdata BYTE debug_string[48];

//*****************************************************************************
//                            МАКРОСЫ
//*****************************************************************************

//! Размер цепочки микросхем, соединенных по принципу daisy chain.
#define AD_CHAIN_SIZE 7

// Ножки для управления микросхемой
#define AD1_CLEAR_HI  IOC |=  bmBIT2
#define AD1_CLEAR_LO  IOC &= ~bmBIT2
#define AD1_LATCH_HI  IOC |=  bmBIT3
#define AD1_LATCH_LO  IOC &= ~bmBIT3
#define AD1_SCLK_HI   IOC |=  bmBIT4
#define AD1_SCLK_LO   IOC &= ~bmBIT4
#define AD1_SDATA_HI  IOD |=  bmBIT0
#define AD1_SDATA_LO  IOD &= ~bmBIT0

#define AD2_CLEAR_HI  IOB |=  bmBIT0
#define AD2_CLEAR_LO  IOB &= ~bmBIT0
#define AD2_LATCH_HI  IOB |=  bmBIT3/**/
#define AD2_LATCH_LO  IOB &= ~bmBIT3/**/
#define AD2_SCLK_HI   IOB |=  bmBIT2
#define AD2_SCLK_LO   IOB &= ~bmBIT2
#define AD2_SDATA_HI  IOB |=  bmBIT1
#define AD2_SDATA_LO  IOB &= ~bmBIT1

// Включение и отключение прерываний. Чтобы программно генерируемый SPI не прерывался.
// TODO: проверить, можно ли без этого обойтись
#define INT_ON  IE |=  0x80
#define INT_OFF IE &= ~0x80

//*****************************************************************************
//                            ТИПЫ
//*****************************************************************************

//! Регистры микросхемы
typedef enum
{
  REG_NOP       = 0x00, // No operation (NOP) 
  REG_DATA      = 0x01, // Data register 
  REG_READBACK  = 0x02, // Readback register value as per read address (see Table 8) 
  REG_CONTROL   = 0x55, // Control register 
  REG_RESET     = 0x56 // Reset register
} AD_REG;

//! Диапазон работы микросхемы
typedef enum
{
  AD_RANGE_4_20 = 5, //4 mA to 20 mA current range 
  AD_RANGE_0_20 = 6, //0 mA to 20 mA current range 
  AD_RANGE_0_24 = 7  //0 mA to 24 mA current range
} AD_RANGES;

//! Тип для выбора битов, которые нужно задать в конфигурационном регистре REG_CONTROL
typedef union
{
  WORD word;
  struct 
  { // сверху младшие биты
    unsigned Range:3;    /* Output range select. See Table 11. здесь использовать одно из значений AD_RANGES */
    unsigned DCEN:1;     /* Daisy-chain enable. */
    unsigned SREN:1;     /* Digital slew rate control enable. */
    unsigned SR_Step:3;  /* Digital slew rate control. See the AD5410/AD5420 Features section. */
    unsigned SR_Clock:4; /* Digital slew rate control. See the AD5410/AD5420 Features section. */
    unsigned OUTEN:1;    /* Output enable. This bit must be set to enable the output. */
    unsigned REXT:1;     /* Setting this bit selects the external current setting 
                            resistor. See the AD5410/AD5420 Features section 
                            for further details. When using an external current 
                            setting resistor, it is recommended to only set REXT 
                            when also setting the OUTEN bit. Alternately, REXT 
                            can be set before the OUTEN bit is set, but the range 
                            (see Table 11) must be changed on the write in which 
                            the output is enabled. See Figure 40 for best practice.  */
    unsigned unused:2;
  } fields;
} AD_CONTROL_WORD;

//*****************************************************************************
//                            ПЕРЕМЕННЫЕ
//*****************************************************************************

// Массивы хранят значения для вывода на аналоговые выходы
// Инициализация этих массивов происходит в Init_AD5420
static xdata volatile WORD chain_1[AD_CHAIN_SIZE]; // первая группа микросхем, выходы 0-5 мА
static xdata volatile WORD chain_2[AD_CHAIN_SIZE]; // вторая группа микросхем, выходы 4-20 мА
static xdata WORD control_array[AD_CHAIN_SIZE];
static AD_CONTROL_WORD acw;

//*****************************************************************************
//                            РЕАЛИЗАЦИЯ ФУНКЦИЙ
//*****************************************************************************

/**
  * @brief Заполнить массив для отправки в группу микросхем одинаковыми значениями
  * @param[in]   uValue     Значение, помещаемое в элементы массива
  * @param[out] uDataArray  Заполняемый массив
  * @param[in]   uArraySize Размер массива uDataArray = 7
  * @retval Возвращаемое значение отсутствует
*/
static void FillArray_AD5420(WORD uValue, WORD * pDataArray, BYTE uArraySize)
{
  BYTE i;
  if ((NULL == pDataArray) || (uArraySize == 0)) return;
  for (i = 0; i < uArraySize; i++)
    pDataArray[i] = uValue;
}

/**
  * @brief Запись в цепочку микросхем
  * @param[in] uGroupNumber 1 - Первая группа микросхем, 2 - Вторая группа микросхем.
  * @param[in] uAddr        Адрес внутреннего регистра микросхемы
  * @param[in] uDataArray   Массив данных, передаваемых в микросхему. Каждый элемент попадает в свою микросхему. Номер элемента = Номер микросхемы.
  * @param[in] uArraySize   Размер массива uDataArray = 7
  * @retval Возвращаемое значение отсутствует
*/
static void WriteChain_AD5420(BYTE uGroupNumber, AD_REG uAddr, WORD * pDataArray, BYTE uArraySize)
{
  BYTE i, j;
  WORD mask;

  if ((NULL == pDataArray) || (uArraySize == 0)) return;

  INT_OFF;

  // Режим daisy chain.
  // 7 передач по 3 байта (24 бита).
  // В самом конце LATCH высокий уровень.
  // Запись бита по повышению уровня CLK
  
  switch (uGroupNumber)
  {
    case 1:
      AD1_LATCH_LO;
      AD1_SCLK_LO;

      for (i = 0; i < uArraySize; i++) 
      {
        for (j = 0; j < 64; j++)
        {
          SYNCDELAY;
        }
        
        mask = 0x80;
        for (j = 0; j < 8; j++) // передача 8 бит адреса начиная со старшего бита к младшим
        {
          AD1_SCLK_LO;
          if (uAddr & mask)
          {
            AD1_SDATA_HI;
          } else
          {
            AD1_SDATA_LO;
          }
          SYNCDELAY;
          AD1_SCLK_HI;
          
          mask >>= 1;
        }
        
        mask = 0x8000;
        for (j = 0; j < 16; j++) // передача 16 бит данных начиная со старшего бита к младшим
        {
          AD1_SCLK_LO;
          if (pDataArray[(uArraySize - 1) - i] & mask)
          {
            AD1_SDATA_HI;
          } else
          {
            AD1_SDATA_LO;
          }
          SYNCDELAY;
          AD1_SCLK_HI;
          
          mask >>= 1;
        }
      }
      AD1_SCLK_LO;
      AD1_LATCH_HI;
      for (j = 0; j < 64; j++)
      {
        SYNCDELAY;
      }
      AD1_LATCH_LO;
    break;
    
    case 2:
      AD2_LATCH_LO;
      AD2_SCLK_LO;

      for (i = 0; i < uArraySize; i++) 
      {
        for (j = 0; j < 64; j++)
        {
          SYNCDELAY;
        }
        
        mask = 0x80;
        for (j = 0; j < 8; j++) // передача 8 бит адреса начиная со старшего бита к младшим
        {
          AD2_SCLK_LO;
          if (uAddr & mask)
          {
            AD2_SDATA_HI;
          } else
          {
            AD2_SDATA_LO;
          }
          SYNCDELAY;
          AD2_SCLK_HI;
          
          mask >>= 1;
        }
        
        mask = 0x8000;
        for (j = 0; j < 16; j++) // передача 16 бит данных начиная со старшего бита к младшим
        {
          AD2_SCLK_LO;
          if (pDataArray[(uArraySize - 1) - i] & mask)
          {
            AD2_SDATA_HI;
          } else
          {
            AD2_SDATA_LO;
          }
          SYNCDELAY;
          AD2_SCLK_HI;
          
          mask >>= 1;
        }
      }
      AD2_SCLK_LO;
      AD2_LATCH_HI;
      for (j = 0; j < 64; j++)
      {
        SYNCDELAY;
      }
      AD2_LATCH_LO;
    break;
  }

  INT_ON;
}

/**
  * @brief Инициализация группы микросхем AD5420
  * @retval Возвращаемое значение отсутствует
*/
void Init_AD5420(void) 
{
  acw.fields.Range = AD_RANGE_0_20;
  acw.fields.DCEN = 1;
  acw.fields.SREN = 0;
  acw.fields.SR_Step = 0;
  acw.fields.SR_Clock = 0;
  acw.fields.OUTEN = 1;
  acw.fields.REXT = 1;
  acw.fields.unused = 0;
  
  AD1_CLEAR_LO;
  AD2_CLEAR_LO;
  AD1_LATCH_LO;
  AD2_LATCH_LO;
  AD1_SCLK_LO;
  AD2_SCLK_LO;

  // Сброс
  FillArray_AD5420(0x0001, control_array, AD_CHAIN_SIZE);
  WriteChain_AD5420(1, REG_RESET, control_array, AD_CHAIN_SIZE);
  WriteChain_AD5420(2, REG_RESET, control_array, AD_CHAIN_SIZE);
  FillArray_AD5420(acw.word, control_array, AD_CHAIN_SIZE);
  FillArray_AD5420(0x0000, chain_1, AD_CHAIN_SIZE);
  FillArray_AD5420(0x0000, chain_2, AD_CHAIN_SIZE);
  WriteChain_AD5420(1, REG_CONTROL, control_array, AD_CHAIN_SIZE);
  WriteChain_AD5420(2, REG_CONTROL, control_array, AD_CHAIN_SIZE);
}

/**
  * @brief Установка значений во внутренний массив
  * @param[in] uNumber Номер аналогового выхода (< ANLG_SGNLS_COUNT)
  * @param[in] uVal Значение, пришедшее с компьютера.
  * @retval Возвращаемое значение отсутствует
*/
void Set_DC(BYTE uNumber, WORD uVal)
{
  float tmp;
  if (uNumber < ANLG_SGNLS_COUNT)
  {
    tmp = (float)uVal;
    //corr_data.corr[uNumber] = 0.99;
    tmp = tmp * corr_data.corr[uNumber];
    
    
    //sprintf(debug_string, "value %2.6f", tmp);
    if (uNumber < AD_CHAIN_SIZE) // Выход принадлежит первой группе микросхем
    {
      chain_1[uNumber] = (WORD)tmp;
    }
      else  // Выход принадлежит второй группе микросхем
    {
      chain_2[uNumber - AD_CHAIN_SIZE] = (WORD)tmp;
    }
  }
}

/**
  * @brief Установка тока на выходе
  * @retval Возвращаемое значение отсутствует
*/
void Update_DC(void)
{
  FillArray_AD5420(acw.word, control_array, AD_CHAIN_SIZE);
  WriteChain_AD5420(1, REG_CONTROL, control_array, AD_CHAIN_SIZE);
  WriteChain_AD5420(1, REG_DATA, chain_1, AD_CHAIN_SIZE);
  WriteChain_AD5420(2, REG_CONTROL, control_array, AD_CHAIN_SIZE);
  WriteChain_AD5420(2, REG_DATA, chain_2, AD_CHAIN_SIZE);
}
