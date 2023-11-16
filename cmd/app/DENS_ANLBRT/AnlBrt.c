/**
 * @authors
 * © Сучилин   М. Л., 2004-2006
 * © Степченко М. В., 2011
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
#include <string.h>
#include <intrins.h>

//*****************************************************************************
//                            ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
//****************************************************************************/

//! Значения аналоговых сигналов
xdata AnlSignals valSignals;
//! Старые значения аналоговых сигналов
xdata AnlSignals oldSignals;

xdata volatile DENSITY_TEST_DATA dtd;

/** 
 * При обращении к переменной, объявленной по адресу 0x8000 происходит чтение
 * из порта данных "D" из внешней памяти. Не путать с портом ввода-вывода IOD.
 * В этот момент на порту "D" появляются те или иные данные, см. схему. 
 */
xdata volatile BYTE empty_data _at_ 0x8000;

//*****************************************************************************
//                            ОБЪЯВЛЕНИЯ ФУНКЦИЙ
//****************************************************************************/

void UpdateAnlSignal(PAnlSignals newVal, PAnlSignals oldVal);
void SetDCVal(unsigned short val, unsigned char addr);
unsigned short UpdateBinary();

//*****************************************************************************
//                            МАКРОСЫ
//****************************************************************************/

//! Порт D - младшая часть адреса микросхемы
#define ADDR_LO IOD
//! Порт E - старшая часть адреса микросхемы
#define ADDR_HI IOE

//! Включить бит управления частоты
#define FRQ_ON  ADDR_LO |=  0x10
//! Выключить бит управления частоты
#define FRQ_OFF ADDR_LO &= ~0x10

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
  CPUCS |= bmCLKSPD1; // Микроконтроллер функционирует на частоте 48 МГц.
                      // Данный параметр влияет на генерацию частот.
  
  // максимально быстрый доступ к памяти - MOVX за два такта (по умолчанию 3)
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
  //ET1 = 1; таймер 1 не включаем

  // начальная инициализация сигналов или точнее значений
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
    IOC |= 0x20;
  }
  else
  {
    IOC &= ~0x20;
  }

  ///if (dtd.enable)
  ///{
    ///valSignals.anlgSgnls[8] = 3000; // ТЦ
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
      counter_sec--; //80 раз в секунду
    }
  ///}

  UpdateAnlSignal(&valSignals, &oldSignals);
}

/**
 * @brief  Второй таймер.
 * Здесь обрабатываются частотные и двоичные сигналы.
 */
#if 0
void int_timer_1(void) interrupt 3
{
  static USHORT counter[4] = { 0, 0, 0, 0 };
  static USHORT bits[4] = { 0x01, 0x02, 0x04, 0x08 };
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
  
          empty_data = state_frq; // на D появляется значение state_frq                
          FRQ_ON;                 // выборка частоты      
          FRQ_OFF;                // выключить выборку

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
 * @brief  Обновление аналоговых сигналов
 * @param[in] newVal Новые значения аналоговых сигналов
 * @param[in] oldVal Старые значения аналоговых сигналов
 *
 * Задача этой функции - обновить значения и
 * установить новые значения для выходов аналоговых сигналов
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
 * @brief  Выборка микросхемы и установка для неё значения
 * @param[in] val Значение
 * @param[in] addr Адрес микросхемы
 */
void SetDCVal(unsigned short val, unsigned char addr)
{ 
  // 4-й бит (считая от 0) - отвечает за частоту.
  // Остальные по порядку (аналоговые) после него сдвигаются

  ADDR_LO = 0x00;
  ADDR_HI = 0x00;

  switch (addr)
  {
    case  0: ADDR_LO = 0x01; break;
    case  1: ADDR_LO = 0x02; break;
    case  2: ADDR_LO = 0x04; break;
    case  3: ADDR_LO = 0x08; break;
    // бит 0x10 отвечает за частоту
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
 * @brief  Обновить двоичные сигналы
 * @return Возвращает слово, содержащее двоичные сигналы
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

  IOC |=  0x02; // выборка одной микросхемы
  ret.byte[1] = empty_data; // чтение с порта данных
  IOC &= ~0x02; // отмена выборки

  IOC |=  0x01; // выборка другой микросхемы
  ret.byte[0] = empty_data; // чтение с порта данных
  IOC &= ~0x01; // отмена выборки

  return ret.word;
}
#endif
/** @}*/