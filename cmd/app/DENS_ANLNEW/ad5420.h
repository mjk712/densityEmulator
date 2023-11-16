/**
 * @authors
 * © Степченко М. В., 2015
 * @brief Интерфейс модуля микросхемы AD5420
 * @file ad5420.h
 * @details
 * Проект: ИПК-3 
 * Микроконтроллер: CY7C64713
 * Отдел: СКБ 103.2
 */

#ifndef _AD5420_H_
#define _AD5420_H_

#include "fx2.h"
#include "fx2regs.h"

void Init_AD5420();
void Set_DC(BYTE uNumber, WORD uVal);
void Update_DC(void);

#endif //_AD5420_H_
