package app

import (
	"emulatortm/internal/services"
	"time"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"github.com/amdf/ipk"
)

const (
	IDCURRENTDENSITY  = 0x7FD
	IDSTANDARDDENSITY = 0x7FC
)

var ipkBox ipk.IPK

// check
var fasDetected bool = false
var fdsDetected bool = false

// stages
var fasModelDetected bool = false
var driversInstalled bool = false
var readyToStart bool = false
var start bool = false

var fasVersion string = ""

// SensorIR Переменная для задания давления ИР в кгс/см²
var SensorIR ipk.PressureOutput

// SensorTC Переменная для задания давления ТЦ в кгс/см²
var SensorTC ipk.PressureOutput

func Run() {

	//start
	ipkBox.AnalogDev = new(ipk.AnalogDevice)
	ipkBox.BinDev = new(ipk.BinaryDevice)
	ipkBox.FreqDev = new(ipk.FreqDevice)
	//При повторном включении ИПК надо заного инициализировать ФАС и ФДС
	go func() {
		for range time.Tick(time.Second / 2) {
			if driversInstalled {
				ipkBox.AnalogDev.Open() //открываем ФАС-3
				ipkBox.BinDev.Open()    //открываем ФДС-3
			}
		}
	}()

	ipkBox.BinDev.Open() //открываем ФДС-3
	defer ipkBox.BinDev.Close()
	ipkBox.FreqDev.Open() //открываем ФЧС-3
	defer ipkBox.FreqDev.Close()

	//fyne
	a := app.New()
	w := a.NewWindow("Эмулятор ТМ")
	w.Resize(fyne.NewSize(300, 300))
	w2 := a.NewWindow("Эмулятор ТМ")
	errWindow := a.NewWindow("Эмулятор ТМ")
	startCheck(w, w2, errWindow)

	//Перед началом стадий, устанавливаем актуальные версии драйверов ФАС
	services.InstallDrivers()
	a.Run()

}
