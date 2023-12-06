package services

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"time"

	"github.com/amdf/ipk"
)

var uBasePressure uint16
var uTopPressure uint16
var uMidPressure uint16
var uLowPressure uint16

// Структура для функции измерения давления
type densityTestData struct {
	enable        uint16
	start         uint16
	reset         uint16
	num_sec       uint16
	base_pressure uint16
	top_pressure  uint16
	mid_pressure  uint16
	low_pressure  uint16
}

func GetDacAmt(val float64, nBitData uint16, nMaxCurrent uint16) uint16 {
	var mA, dac float64
	mA = val*16/10 + 4
	dac = (mA / float64(nMaxCurrent) * float64(nBitData))
	return uint16(dac)
}

// функция перевода данный структуры в байты, для последующей отправки
func (data *densityTestData) toBytes() []byte {
	if nil == data {
		return nil
	}
	buf := new(bytes.Buffer)

	binary.Write(buf, binary.BigEndian, data.enable)
	binary.Write(buf, binary.BigEndian, data.start)
	binary.Write(buf, binary.BigEndian, data.reset)
	binary.Write(buf, binary.BigEndian, data.num_sec)
	binary.Write(buf, binary.BigEndian, data.base_pressure)
	binary.Write(buf, binary.BigEndian, data.top_pressure)
	binary.Write(buf, binary.BigEndian, data.mid_pressure)
	binary.Write(buf, binary.BigEndian, data.low_pressure)

	return buf.Bytes()
}

func InitDensity(editDensityText string, fas *ipk.AnalogDevice) {

	dens, err := time.ParseDuration(editDensityText + "s")

	if err != nil {
		return
	}
	fass := GetFAS()

	switch fass.Version {
	case "12 Bit":
		uBasePressure = GetDacAmt(9.0, 4095, 20) //3900
		uTopPressure = GetDacAmt(8.8, 4095, 20)  //3850
		uMidPressure = GetDacAmt(8.4, 4095, 20)  //3580
		uLowPressure = GetDacAmt(7.9, 4095, 20)  //3167
	case "16 Bit":
		uBasePressure = GetDacAmt(9.0, 0xFFFF, 20) //3900
		uTopPressure = GetDacAmt(8.8, 0xFFFF, 20)  //3850
		uMidPressure = GetDacAmt(8.4, 0xFFFF, 20)  //3580
		uLowPressure = GetDacAmt(7.9, 0xFFFF, 20)  //3167
	}

	var data densityTestData

	data.low_pressure = uLowPressure
	data.mid_pressure = uMidPressure
	data.top_pressure = uTopPressure
	data.base_pressure = uBasePressure
	data.num_sec = uint16(dens.Seconds())

	data.enable = 1
	data.start = 1
	//start начинает измерение
	dtd := data.toBytes()
	err = fas.DensEmulate(0x40, 0xB3, dtd, len(dtd))
	if err != nil {
		fmt.Println(err)
	}
	data.start = 0
}

func ResetDensity(editDensityText string, fas *ipk.AnalogDevice) {

	dens, err := time.ParseDuration(editDensityText + "s")

	if err != nil {
		return
	}
	fass := GetFAS()

	switch fass.Version {
	case "12 Bit":
		uBasePressure = GetDacAmt(9.0, 4095, 20) //3900
		uTopPressure = GetDacAmt(8.8, 4095, 20)  //3850
		uMidPressure = GetDacAmt(8.4, 4095, 20)  //3580
		uLowPressure = GetDacAmt(7.9, 4095, 20)  //3167
	case "16 Bit":
		uBasePressure = GetDacAmt(9.0, 0xFFFF, 20) //3900
		uTopPressure = GetDacAmt(8.8, 0xFFFF, 20)  //3850
		uMidPressure = GetDacAmt(8.4, 0xFFFF, 20)  //3580
		uLowPressure = GetDacAmt(7.9, 0xFFFF, 20)  //3167
	}

	var data densityTestData

	data.low_pressure = uLowPressure
	data.mid_pressure = uMidPressure
	data.top_pressure = uTopPressure
	data.base_pressure = uBasePressure
	data.num_sec = uint16(dens.Seconds())

	data.enable = 1
	data.reset = 1
	//reset отменяет и возвращает давление
	dtd := data.toBytes()
	err = fas.DensEmulate(0x40, 0xB3, dtd, len(dtd))
	if err != nil {
		fmt.Println(err)
	}
	data.reset = 0

}
