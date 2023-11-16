/**
 * @brief Функции, относящиеся к USB
 * @file periph.c
 * @date 3/23/05 3:03p
 * @details
 * Проект: ИПК-3 
 * Микроконтроллер: CY7C64713
 * Отдел: СКБ 103
 */

//! Не генерировать векторы прерываний
#pragma NOIV

#include "fx2.h"
#include "fx2regs.h"

#include "AnlBrt.h"

extern xdata volatile DENSITY_TEST_DATA dtd;
xdata volatile DENSITY_TEST_DATA new_dtd;

//! Полученный флаг данных установок
extern BOOL   GotSUD;         
extern BOOL   Sleep;
extern BOOL   Rwuen;
extern BOOL   Selfpwr;

//! Текущая конфигурация
BYTE   Configuration;
//! Альтернативные настройки
BYTE   AlternateSetting;

/** @defgroup disphooks Хуки диспетчера задач
 *  Следующие хуки вызываются диспетчером задач
 *  @{
 */

/** 
 * @brief Вызывается один раз при старте 
 */
void TD_Init(void)
{
  InitAnlBrt();  
  Rwuen = TRUE;        // Enable remote-wakeup
  BREAKPT &= ~bmBPEN;  // to see BKPT LED go out TGE
}

/**
 * @brief Вызывается постоянно, пока устройство простаивает
 */
void TD_Poll(void)
{
}

/**
 * @brief Вызывается перед тем, как устройство перейдёт в режим сна
 */
BOOL TD_Suspend(void)
{
   return(FALSE);
}

/** 
 * @brief Вызывается перед тем, как устройство вернётся из режима сна
 */
BOOL TD_Resume(void)          // Called after the device resumes
{
   return(TRUE);
}

/** @}*/

/** @defgroup reqhooks Хуки запросов к устройству
 *  Следующие хуки вызываются парсером запросов к устройству "end point 0"
 *  @{
 */

BOOL DR_GetDescriptor(void)
{
   return(TRUE);
}

BOOL DR_SetConfiguration(void)   // Called when a Set Configuration command is received
{
   Configuration = SETUPDAT[2];
   return(TRUE);            // Handled by user code
}

BOOL DR_GetConfiguration(void)   // Called when a Get Configuration command is received
{
   EP0BUF[0] = Configuration;
   EP0BCH = 0;
   EP0BCL = 1;
   return(TRUE);            // Handled by user code
}

BOOL DR_SetInterface(void)       // Called when a Set Interface command is received
{
   AlternateSetting = SETUPDAT[2];
   return(TRUE);            // Handled by user code
}

BOOL DR_GetInterface(void)       // Called when a Set Interface command is received
{
   EP0BUF[0] = AlternateSetting;
   EP0BCH = 0;
   EP0BCL = 1;
   return(TRUE);            // Handled by user code
}

BOOL DR_GetStatus(void)
{
   return(TRUE);
}

BOOL DR_ClearFeature(void)
{
   return(TRUE);
}

BOOL DR_SetFeature(void)
{
   return(TRUE);
}

/** 
 * @brief Обработка запросов, приходящих от компьютера
 * Здесь, в зависимости от поступившего запроса, передаются или
 * принимаются данные с компьютера/на компьютер.
 */
BOOL DR_VendorCmnd(void)
{
  switch (SETUPDAT[1])
  {
    case 0xB0:  
      if (SETUPDAT[0] == 0xC0)
      {
        *(PAnlSignals)(EP0BUF) = valSignals;

        EP0BCH = 0; 
        EP0BCL = sizeof(valSignals);
      }
      if (SETUPDAT[0] == 0x40)
      {
        EP0BCH = 0; 
        EP0BCL = 0; 

        while(EP0CS & 0x02);

        valSignals = *(PAnlSignals)EP0BUF;
      }
    break;
    case 0xB3:
    /*
        if (SETUPDAT[0] == 0xC0) // out
        {
          
          *((unsigned long *)EP0BUF) = uLastValue;
          EP0BCH = 0;
          EP0BCL = sizeof(uLastValue);
        } else*/
        if (SETUPDAT[0] == 0x40) // in
        {
          EP0BCH = 0;
          EP0BCL = 0; 

          while(EP0CS & 0x02);

          new_dtd = *(DENSITY_TEST_DATA *)EP0BUF;
          dtd = new_dtd;
        }
    break;
    default:
      return (TRUE);
  }
  return(FALSE);
}

/** @}*/

//-----------------------------------------------------------------------------
// USB Interrupt Handlers
//   The following functions are called by the USB interrupt jump table.
//-----------------------------------------------------------------------------

/** @defgroup usbhooks Хуки прерываний USB
 *  Следующие инструкции вызываются из таблицы переходов USB
 *  @{
 */

//! Обработчик прерывания доступности данных установки
void ISR_Sudav(void) interrupt 0
{
   GotSUD = TRUE;            // Set flag
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSUDAV;         // Clear SUDAV IRQ
}

//! Обработчик прерывания токена установки
void ISR_Sutok(void) interrupt 0
{
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSUTOK;         // Clear SUTOK IRQ
}

void ISR_Sof(void) interrupt 0
{
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSOF;            // Clear SOF IRQ
}

void ISR_Ures(void) interrupt 0
{
   // whenever we get a USB reset, we should revert to full speed mode
   pConfigDscr = pFullSpeedConfigDscr;
   ((CONFIGDSCR xdata *) pConfigDscr)->type = CONFIG_DSCR;
   pOtherConfigDscr = pHighSpeedConfigDscr;
   ((CONFIGDSCR xdata *) pOtherConfigDscr)->type = OTHERSPEED_DSCR;
   
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmURES;         // Clear URES IRQ
}

void ISR_Susp(void) interrupt 0
{
   Sleep = TRUE;
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSUSP;
}

void ISR_Highspeed(void) interrupt 0
{
   if (EZUSB_HIGHSPEED())
   {
      pConfigDscr = pHighSpeedConfigDscr;
      ((CONFIGDSCR xdata *) pConfigDscr)->type = CONFIG_DSCR;
      pOtherConfigDscr = pFullSpeedConfigDscr;
      ((CONFIGDSCR xdata *) pOtherConfigDscr)->type = OTHERSPEED_DSCR;
   }

   EZUSB_IRQ_CLEAR();
   USBIRQ = bmHSGRANT;
}
void ISR_Ep0ack(void) interrupt 0
{
}
void ISR_Stub(void) interrupt 0
{
}
void ISR_Ep0in(void) interrupt 0
{
}
void ISR_Ep0out(void) interrupt 0
{
}
void ISR_Ep1in(void) interrupt 0
{
}
void ISR_Ep1out(void) interrupt 0
{
}
void ISR_Ep2inout(void) interrupt 0
{
}
void ISR_Ep4inout(void) interrupt 0
{
}
void ISR_Ep6inout(void) interrupt 0
{
}
void ISR_Ep8inout(void) interrupt 0
{
}
void ISR_Ibn(void) interrupt 0
{
}
void ISR_Ep0pingnak(void) interrupt 0
{
}
void ISR_Ep1pingnak(void) interrupt 0
{
}
void ISR_Ep2pingnak(void) interrupt 0
{
}
void ISR_Ep4pingnak(void) interrupt 0
{
}
void ISR_Ep6pingnak(void) interrupt 0
{
}
void ISR_Ep8pingnak(void) interrupt 0
{
}
void ISR_Errorlimit(void) interrupt 0
{
}
void ISR_Ep2piderror(void) interrupt 0
{
}
void ISR_Ep4piderror(void) interrupt 0
{
}
void ISR_Ep6piderror(void) interrupt 0
{
}
void ISR_Ep8piderror(void) interrupt 0
{
}
void ISR_Ep2pflag(void) interrupt 0
{
}
void ISR_Ep4pflag(void) interrupt 0
{
}
void ISR_Ep6pflag(void) interrupt 0
{
}
void ISR_Ep8pflag(void) interrupt 0
{
}
void ISR_Ep2eflag(void) interrupt 0
{
}
void ISR_Ep4eflag(void) interrupt 0
{
}
void ISR_Ep6eflag(void) interrupt 0
{
}
void ISR_Ep8eflag(void) interrupt 0
{
}
void ISR_Ep2fflag(void) interrupt 0
{
}
void ISR_Ep4fflag(void) interrupt 0
{
}
void ISR_Ep6fflag(void) interrupt 0
{
}
void ISR_Ep8fflag(void) interrupt 0
{
}
void ISR_GpifComplete(void) interrupt 0
{
}
void ISR_GpifWaveform(void) interrupt 0
{
}

/** @}*/