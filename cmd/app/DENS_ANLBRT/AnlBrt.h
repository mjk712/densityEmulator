/** \mainpage ФАС-3
 *
 * Формирователь аналоговых сигналов ИПК-3
 *
 * \section usage_sec Использование
 *
 * Программа предназначена для микроконтроллера CY7C64713. Исходный код компилируется в файл
 * ANLBRT_3.hex, который с помощью утилиты CyScript преобразуется в файл <b>ANLBRT_3.spt</b>.
 *
 * \section drv_sec Драйвер устройства
 *
 * Каталог драйвера устройства IPK3_Driver содержит следующие файлы:
 <ul>
  <li><b>ezusb.sys</b> Драйвер, отвечающий за коммуникацию с устройством;</li>
  <li><b>IPKLoad.sys</b> Драйвер загрузки прошивки в устройство при его включении;<li>
  <li><b>*.inf</b> Файлы, необходимые для установки драйверов в систему;<li>
  <li><b>ANLLoad.iic</b> Файл, содержащий идентификатор устройства. Однократно прошивается в EEPROM
  устройства, после чего становится возможным определение устройства драйвером IPKLoad.sys
  при подключении к компьютеру.<li>
 </ul>
 */

/**
 * @authors
 * © Сучилин   М. Л., 2004-2006
 * © Степченко М. В., 2011
 * @brief Интерфейс формирователя аналоговых сигналов (ФАС)
 * @file AnlBrt.h
 * @details
 * Проект: ИПК-3 
 * Микроконтроллер: CY7C64713
 * Отдел: СКБ 103
 */
#ifndef _ANL_BRT_H_
#define _ANL_BRT_H_

//! Количество аналоговых сигналов
#define ANLG_SGNLS_COUNT 14 
//! Количество частотных сигналов
#define FREQ_SGNLS_COUNT 4  

//! Структура, содержащая аналоговые, частотные и двоичные сигналы
typedef struct _AnlSignals
{
  //! Аналоговые сигналы
  unsigned short anlgSgnls[ANLG_SGNLS_COUNT]; 
  //! Частотные сигналы
  unsigned short freqSgnls[FREQ_SGNLS_COUNT]; 
  //! Двоичные сигналы
  unsigned short binarySgnls;
} AnlSignals, *PAnlSignals;

typedef struct _DENSITY_TEST_DATA
{
  short enable;
  short start;
  short reset;
  short num_sec;
  short base_pressure;
  short top_pressure;
  short mid_pressure;
  short low_pressure;
} DENSITY_TEST_DATA, *PDENSITY_TEST_DATA;

typedef enum 
{
  DTD_BASE,
  DTD_HIGH,
  DTD_MID,
  DTD_LOW
} DTD_STATE;

extern void InitAnlBrt();
extern xdata AnlSignals valSignals;

typedef unsigned short USHORT;

#endif // _ANL_BRT_H_
