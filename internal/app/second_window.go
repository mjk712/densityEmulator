package app

import (
	"bytes"
	"emulatortm/internal/models"
	"emulatortm/internal/services"
	"encoding/binary"
	"fmt"
	"strconv"
	"time"

	"github.com/amdf/ipk"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/layout"

	"fyne.io/fyne/v2/widget"
)

// SensorIR Переменная для задания давления ИР в кгс/см²
var SensorIR ipk.PressureOutput

// SensorTC Переменная для задания давления ТЦ в кгс/см²
var SensorTC ipk.PressureOutput

var topPanel = container.NewHBox(
	widget.NewLabel(""),
)

var editDensity = widget.NewEntry()

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

var uBasePressure uint16
var uTopPressure uint16
var uMidPressure uint16
var uLowPressure uint16

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

var desc = widget.NewLabel("Плотность:60.0")
var minDesc = widget.NewLabel("Минимум:57.6")
var maxDesc = widget.NewLabel("Максимум:62.4")

var secDesc = container.NewVBox(
	desc, minDesc, maxDesc,
)

var leftPanel = container.NewVBox(
	container.NewHBox(
		widget.NewLabel("Плотность, с:"),
		editDensity,
		widget.NewButton("Установить", func() {
			go func() {
				var fas *ipk.AnalogDevice
				fas = ipkBox.AnalogDev
				dens, err := time.ParseDuration(editDensity.Text + "s")

				if err != nil {
					return
				}

				uBasePressure = GetDacAmt(9.0, 4095, 20) //3900
				uTopPressure = GetDacAmt(8.8, 4095, 20)  //3850
				uMidPressure = GetDacAmt(8.4, 4095, 20)  //3580
				uLowPressure = GetDacAmt(7.9, 4095, 20)  //3167

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
			}()
		}),
		widget.NewButton("Сброс", func() {
			go func() {
				var fas *ipk.AnalogDevice
				fas = ipkBox.AnalogDev
				dens, err := time.ParseDuration(editDensity.Text + "s")

				if err != nil {
					return
				}

				uBasePressure = GetDacAmt(9.0, 4095, 20) //3900
				uTopPressure = GetDacAmt(8.8, 4095, 20)  //3850
				uMidPressure = GetDacAmt(8.4, 4095, 20)  //3580
				uLowPressure = GetDacAmt(7.9, 4095, 20)  //3167

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
			}()
		}),
	),
	secDesc,
)

func app2(w2 fyne.Window, fas *models.FAS, errWindow fyne.Window) {

	bottomPanel := container.NewHBox(

		widget.NewButton("Выход", func() {
			go func() {
				services.ReturnDrivers(fas)
				errWindow.Show()
			}()
		}),
	)

	myContent :=
		container.New(layout.NewBorderLayout(topPanel, bottomPanel, nil, nil),
			topPanel, bottomPanel, leftPanel,
		)

	w2.SetContent(
		myContent,
	)
	w2.Resize(fyne.Size{Width: 400, Height: 300})
	w2.CenterOnScreen()
	editDensity.SetText("10")

	go func() {
		for range time.Tick(time.Second / 4) {
			editDensity.OnChanged = func(input string) {
				intValue, _ := strconv.Atoi(input)
				descValue := float64(intValue) * 6
				minDescValue := (float64(intValue) - 0.4) * 6
				maxDescValue := (float64(intValue) + 0.4) * 6
				desc.SetText(fmt.Sprintf("Плотность: %.1f", descValue))
				minDesc.SetText(fmt.Sprintf("Минимум: %.1f", minDescValue))
				maxDesc.SetText(fmt.Sprintf("Максимум: %.1f", maxDescValue))
			}
		}
	}()

	w2.Show()
}
