/** \mainpage ���-3
 *
 * ������������� ���������� �������� ���-3
 *
 * \section usage_sec �������������
 *
 * ��������� ������������� ��� ���������������� CY7C64713. �������� ��� ������������� � ����
 * ANLBRT_3.hex, ������� � ������� ������� CyScript ������������� � ���� <b>ANLBRT_3.spt</b>.
 *
 * \section drv_sec ������� ����������
 *
 * ������� �������� ���������� IPK3_Driver �������� ��������� �����:
 <ul>
  <li><b>ezusb.sys</b> �������, ���������� �� ������������ � �����������;</li>
  <li><b>IPKLoad.sys</b> ������� �������� �������� � ���������� ��� ��� ���������;<li>
  <li><b>*.inf</b> �����, ����������� ��� ��������� ��������� � �������;<li>
  <li><b>ANLLoad.iic</b> ����, ���������� ������������� ����������. ���������� ����������� � EEPROM
  ����������, ����� ���� ���������� ��������� ����������� ���������� ��������� IPKLoad.sys
  ��� ����������� � ����������.<li>
 </ul>
 */

/**
 * @authors
 * � �������   �. �., 2004-2006
 * � ��������� �. �., 2011
 * @brief ��������� ������������� ���������� �������� (���)
 * @file AnlBrt.h
 * @details
 * ������: ���-3 
 * ���������������: CY7C64713
 * �����: ��� 103
 */
#ifndef _ANL_BRT_H_
#define _ANL_BRT_H_

//! ���������� ���������� ��������
#define ANLG_SGNLS_COUNT 14 
//! ���������� ��������� ��������
#define FREQ_SGNLS_COUNT 4  

//! ���������, ���������� ����������, ��������� � �������� �������
typedef struct _AnlSignals
{
  //! ���������� �������
  unsigned short anlgSgnls[ANLG_SGNLS_COUNT]; 
  //! ��������� �������
  unsigned short freqSgnls[FREQ_SGNLS_COUNT]; 
  //! �������� �������
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
