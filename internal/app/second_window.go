package app

import (
	"fmt"
	"os"
	"runtime"
	"strconv"
	"time"

	"github.com/amdf/ixxatvci3"
	"github.com/amdf/ixxatvci3/candev"
	"github.com/artlukm/common"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/layout"

	"fyne.io/fyne/v2/widget"
)

var canOk = make(chan int)
var can *candev.Device
var b candev.Builder

var pos byte
var posCounter int
var posName = make(map[byte]string)

var allErrors = container.NewVBox()
var allParams = container.NewVBox()
var connectLabel = widget.NewLabel("Не соединено")

var connectButton = widget.NewButton("Соединить", func() {
	var err error

	can, err = b.Speed(ixxatvci3.Bitrate25kbps).Get()

	if err == nil {
		can.Run()
		canOk <- 1
	} else {
		connectLabel.SetText(common.Decode1251String(err.Error()))
	}
})

var topPanel = container.NewHBox(
	widget.NewLabel(""),
)

var bottomPanel = container.NewHBox(
	connectLabel,
	connectButton,
	widget.NewButton("Выход", func() { os.Exit(0) }),
)

var labelTM = widget.NewLabel("0")
var slideTM = widget.NewSlider(70, 100)
var labelTC = widget.NewLabel("0")
var slideTC = widget.NewSlider(0, 100)
var editDensity = widget.NewEntry()

var p206n1 = widget.NewCheck("П206 № 1", func(on bool) {
	if on {
		outputBinary |= 0x04
	} else {
		outputBinary &^= 0x04
	}
	sendBinary()
})
var p206n2 = widget.NewCheck("П206 № 2", func(on bool) {
	if on {
		outputBinary |= 0x08
	} else {
		outputBinary &^= 0x08
	}
	sendBinary()
})

var desc = widget.NewLabel("Плотность:60.0")
var minDesc = widget.NewLabel("Минимум:57.6")
var maxDesc = widget.NewLabel("Максимум:62.4")

var secDesc = container.NewVBox(
	desc, minDesc, maxDesc,
)

var leftPanel = container.NewVBox(
	labelTM,
	slideTM,
	labelTC,
	slideTC,
	/*widget.NewCheck("тяга", func(on bool) {
		if on {
			outputBinary |= 0x01
		} else {
			outputBinary &^= 0x01
		}
		sendBinary()
	}),
	widget.NewCheck("скорость", func(on bool) {
		if on {
			outputBinary |= 0x02
		} else {
			outputBinary &^= 0x02
		}
		sendBinary()
	}),*/
	p206n1,
	p206n2,
	/*widget.NewCheck("дополнительный сигнал", func(on bool) {
		if on {
			outputBinary |= 0x10
		} else {
			outputBinary &^= 0x10
		}
		sendBinary()
	}),*/
	container.NewHBox(
		widget.NewLabel("Плотность, с:"),
		editDensity,
		widget.NewButton("Установить", func() {
			go func() {
				dens, err := time.ParseDuration(editDensity.Text + "s")

				if err != nil {
					return
				}

				slideTM.Value = 90
				time.Sleep(2 * time.Second)
				slideTM.Value = 87
				time.Sleep(2 * time.Second)
				slideTM.Value = 84

				time.Sleep(dens) //плотность (с)
				slideTM.Value = 79
			}()
		}),
	),
	secDesc,
)

const (
	namePos        = "КМ"
	nameTraction   = "Тяга "
	nameValve      = "Клапан "
	nameCompressor = "Компрессор "
	nameAlarm      = "Сигнализация "
)

var labelPos = widget.NewLabel(namePos)
var labelTraction = widget.NewLabel(nameTraction)
var labelValve = widget.NewLabel(nameValve)
var labelCompressor = widget.NewLabel(nameCompressor)
var labelAlarm = widget.NewLabel(nameAlarm)

var rightPanel = container.NewVBox(
	labelPos,
	labelTraction,
	labelValve,
	labelCompressor,
	labelAlarm,
)

var outputBinary uint8

func sendBinary() {
	msg := candev.Message{ID: 0x7F0, Len: 1}
	msg.Data[0] = outputBinary
	can.Send(msg)
}

func sendPressure() {
	msg := candev.Message{ID: 0x7F2, Len: 4}
	msg.Data[0] = uint8(slideTM.Value)
	msg.Data[2] = uint8(slideTC.Value)
	can.Send(msg)
}

func app2(w2 fyne.Window) {
	posName[0] = "I"
	posName[1] = "II"
	posName[2] = "III"
	posName[3] = "IV"
	posName[4] = "VA"
	posName[5] = "V"
	posName[6] = "VI"

	//app := app.New()
	p206n1.Hide()
	p206n2.Hide()

	myContent :=
		container.New(layout.NewBorderLayout(topPanel, bottomPanel, nil, rightPanel),
			topPanel, bottomPanel, leftPanel, rightPanel,
		)

	//w := app.NewWindow("Эмулятор ТМ")

	w2.SetContent(
		myContent,
	)
	w2.Resize(fyne.Size{Width: 800, Height: 600})
	w2.CenterOnScreen()

	p206n1.SetChecked(true)
	p206n2.SetChecked(true)
	slideTM.Value = 90
	slideTC.Value = 11
	editDensity.SetText("10")

	go func() {
		<-canOk

		connectButton.Hide()
		connectLabel.SetText("Соединено")

		go func() {
			for range time.Tick(time.Second / 2) {
				sendBinary()
			}
		}()

		go func() {
			for range time.Tick(time.Second / 2) {
				sendPressure()
			}
		}()

		go func() {
			for range time.Tick(time.Second) {
				if posCounter < 2*60 {
					posCounter++
				}
			}
		}()

		ch, _ := can.GetMsgChannelCopy()

		///counter := 0
		for msg := range ch {
			switch msg.ID {
			case 0x570:
				if pos != msg.Data[1] {
					posCounter = 0
				}
				pos = msg.Data[1]
				labelPos.SetText(namePos + ":" + posName[pos] + fmt.Sprintf("(%d)", posCounter))
			case 0x7F1:
				if 1 == msg.Len {
					stTraction := "РАЗРЫВ"
					stValve := "ЗАКРЫТ"
					stCompressor := "ВЫКЛ"
					stAlarm := "ВЫКЛ"
					if 0 != msg.Data[0]&0x01 {
						stTraction = "ТЯГА"
					}
					if 0 != msg.Data[0]&0x02 {
						stValve = "ОТКРЫТ"
					}
					if 0 != msg.Data[0]&0x04 {
						stCompressor = "ВКЛ"
					}
					if 0 != msg.Data[0]&0x08 {
						stAlarm = "ВКЛ"
					}

					labelTraction.SetText(fmt.Sprint(nameTraction, stTraction))
					labelValve.SetText(fmt.Sprint(nameValve, stValve))
					labelCompressor.SetText(fmt.Sprint(nameCompressor, stCompressor))
					labelAlarm.SetText(fmt.Sprint(nameAlarm, stAlarm))
				}
			}
			runtime.Gosched()
		}

	}()
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

	go func() {
		time.Sleep(time.Second)
		connectButton.OnTapped()

		for range time.Tick(time.Second / 4) {
			kgcTM := common.AtToKiloPascal(slideTM.Value / 10)
			kgcTC := common.AtToKiloPascal(slideTC.Value / 10)
			labelTM.SetText(fmt.Sprint("Давление ТМ: ", slideTM.Value/10, " кгс/см²       (", kgcTM, " кПа)"))
			labelTC.SetText(fmt.Sprint("Давление ТЦ: ", slideTC.Value/10, " кгс/см²       (", kgcTC, " кПа)"))
			slideTM.Refresh()
			slideTC.Refresh()
		}
	}()

	w2.Show()
}
