package app

import (
	"emulatortm/internal/models"
	"emulatortm/internal/services"
	"fmt"
	"image/color"
	"os"
	"time"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/canvas"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/dialog"
	"github.com/amdf/ipk"
)

const (
	IDCURRENTDENSITY  = 0x7FD
	IDSTANDARDDENSITY = 0x7FC
)

var ipkBox ipk.IPK

// check
var fasDetected bool
var fdsDetected bool

// stages
var fasModelDetected bool
var driversInstalled bool
var readyToStart bool
var start bool

var fasVersion string

// shoud i add stage checking
func stage0Check() {
	for range time.Tick(time.Second / 2) {
		ipkBox.AnalogDev.Open()
		ipkBox.BinDev.Open()
		if ipkBox.AnalogDev.Active() && !start {
			//fmt.Println(start)
			start = true
			//fmt.Println("000")
			fas := services.GetFAS()
			readyToStart = true
			ipkBox.AnalogDev.Close()
			ipkBox.BinDev.Close()
			fasVersion = fas.Version
		}
	}

}
func check1Stage(checkFasModelLabel *canvas.Text) {
	for range time.Tick(time.Second / 2) {
		if readyToStart {
			//fmt.Println("asas")
			time.Sleep(50 * time.Millisecond)
			readyToStart = false
			checkFasModelLabel.Color = color.RGBA{R: 200, G: 200, B: 200, A: 255}
			checkFasModelLabel.Text = fmt.Sprintf("1.Модель ФАС: %s", fasVersion)
			checkFasModelLabel.Refresh()
			fasModelDetected = true
		}
	}
}
func check2Stage(checkIpkDisonnectLabel *canvas.Text, w fyne.Window) {
	for range time.Tick(time.Second / 2) {
		if fasModelDetected {
			if !ipkBox.AnalogDev.Active() {
				var fas *models.FAS
				//fmt.Println(start)
				fas = &models.FAS{Connected: true, Version: fasVersion}
				checkIpkDisonnectLabel.Color = color.RGBA{R: 200, G: 200, B: 200, A: 255}
				checkIpkDisonnectLabel.Text = "2.Питание ИПК выключено"
				checkIpkDisonnectLabel.Refresh()
				ipkBox.AnalogDev.Close()
				fasModelDetected = false
				//install drivers
				err := services.ReplaceDrivers(fas)
				if err != nil {
					fmt.Println("Error drive" + err.Error())
					services.ReturnDrivers(fas)
					break
				}
				time.Sleep(50 * time.Millisecond)
				driversInstalled = true
				dialog.ShowInformation("Эмулятор ТМ", "Включите ИПК СЕТЬ", w)
			}
		}
	}
}
func check3Stage(checkFdsLabel *canvas.Text) {
	for range time.Tick(time.Second / 2) {
		if driversInstalled {
			switch ipkBox.BinDev.Active() {
			case false:
				checkFdsLabel.Color = color.RGBA{R: 1, G: 1, B: 1, A: 255}
				checkFdsLabel.Text = "3.Ожидание соединения с ФДС"
				checkFdsLabel.Refresh()
				fdsDetected = false
			case true:
				checkFdsLabel.Color = color.RGBA{R: 200, G: 200, B: 200, A: 255}
				checkFdsLabel.Text = "3.Соединение с ФДС активно"
				checkFdsLabel.Refresh()
				channelN9 := new(ipk.DAC)
				channelN9.Init(ipkBox.AnalogDev, ipk.DAC9)
				SensorTC.Init(channelN9, ipk.DACAtmosphere, 10)
				ipkBox.BinDev.Set50V(1, true)
				fdsDetected = true
			}
		}
	}
}
func check4Stage(checkFasLabel *canvas.Text, ch chan (bool)) {
	for range time.Tick(time.Second / 2) {
		if driversInstalled {
			switch ipkBox.AnalogDev.Active() {
			case false:
				checkFasLabel.Color = color.RGBA{R: 1, G: 1, B: 1, A: 255}
				checkFasLabel.Text = "4.Ожидание соединения с ФАС"
				checkFasLabel.Refresh()
				fasDetected = false
			case true:
				checkFasLabel.Color = color.RGBA{R: 200, G: 200, B: 200, A: 255}
				checkFasLabel.Text = "4.Соединение с ФАС активно"
				checkFasLabel.Refresh()
				fasDetected = true
				ch <- fasDetected
			}
		}
	}
}
func transferToSecondWindow(w fyne.Window, w2 fyne.Window, errWindow fyne.Window) {
	for range time.Tick(time.Second / 2) {
		if fasDetected && fdsDetected {
			var fas *models.FAS
			fas = &models.FAS{Connected: true, Version: fasVersion}
			fasDetected = false
			fdsDetected = false
			//w.Close()
			w.Hide()
			app2(w2, fas, errWindow)
			w2.SetOnClosed(func() {
				go func() {
					services.ReturnDrivers(fas)
					errWindow.Show()
				}()
			})
		}
	}
}

func Run() {
	//c4 := make(chan string)
	//	c2 := make(chan string)
	c3 := make(chan bool)
	//start
	ipkBox.AnalogDev = new(ipk.AnalogDevice)
	ipkBox.BinDev = new(ipk.BinaryDevice)
	ipkBox.FreqDev = new(ipk.FreqDevice)
	go func() {
		for range time.Tick(time.Second / 2) {
			if driversInstalled {
				ipkBox.AnalogDev.Open() //открываем ФАС-3
				ipkBox.BinDev.Open()    //открываем ФДС-3
			}
		}
	}()
	//ipkBox.AnalogDev.Open() //открываем ФАС-3

	ipkBox.BinDev.Open() //открываем ФДС-3
	defer ipkBox.BinDev.Close()
	ipkBox.FreqDev.Open() //открываем ФЧС-3
	defer ipkBox.FreqDev.Close()

	//fyne
	a := app.New()
	w := a.NewWindow("Эмулятор ТМ")
	w2 := a.NewWindow("Эмулятор ТМ")
	errWindow := a.NewWindow("Эмулятор ТМ")
	faqMenuWindow := a.NewWindow("Справка Эмулятор ТМ")
	errWindow.SetMaster()
	w.Resize(fyne.NewSize(300, 300))
	//img := canvas.NewImageFromFile("re.png")
	attention1 := canvas.NewText("1. Драйвера ИПК должны быть установлены по путям: ", color.Black)
	attention2 := canvas.NewText("C:/Windows/System32/ipkload", color.Black)
	attention3 := canvas.NewText("C:/Windows/SysWOW64/IPKLoad", color.Black)
	attention4 := canvas.NewText("2. Программа измеряет плотность последством отправки данных на модифицированный драйвер.", color.Black)
	attention5 := canvas.NewText("3. После закрытия программы рекомендуется перезагрузить ИПК.", color.Black)
	err1 := canvas.NewText("Внимание! После закрытия программы прежде чем продолжать работу с ИПК-3, необходимо в обязательном порядке", color.RGBA{255, 0, 0, 170})
	err2 := canvas.NewText("выключить питание ФПС-3 (СЕТЬ ВЫКЛ). После этого можно снова включить питание и продолжать работу.", color.RGBA{255, 0, 0, 170})
	err3 := canvas.NewText("Если этого не сделать, ФАС будет работать некорректно до тех пор, пока питание не будет выключено.", color.RGBA{255, 0, 0, 170})
	errWindow.SetOnClosed(func() { os.Exit(0) })
	content := container.NewVBox(attention1, attention2, attention3, attention4, attention5)
	faqMenuWindow.Resize(fyne.Size{Width: 670, Height: 420})
	faqMenuWindow.CenterOnScreen()
	faqMenuWindow.SetContent(content)
	faqMenuWindow.SetCloseIntercept(func() {
		faqMenuWindow.Hide()
	})
	faqMenu := fyne.NewMenuItem("Справка", func() {
		fmt.Println("fff")
		faqMenuWindow.Show()
	})

	errWindow.Resize(fyne.Size{Width: 400, Height: 400})
	errWindow.CenterOnScreen()
	errWindow.SetContent(container.NewVBox(err1, err2, err3))

	newMenu := fyne.NewMenu("Info", faqMenu)
	mainMenu := fyne.NewMainMenu(newMenu)
	w2.SetMainMenu(mainMenu)
	checkFasModelLabel := canvas.NewText("1.Включите питание ИПК", color.RGBA{R: 1, G: 1, B: 1, A: 255})
	checkIpkDisonnectLabel := canvas.NewText("2.Выключите питание ИПК", color.RGBA{R: 1, G: 1, B: 1, A: 255})
	checkFdsLabel := canvas.NewText("3.Ожидание соединения с ФДС", color.RGBA{R: 1, G: 1, B: 1, A: 255})
	checkFasLabel := canvas.NewText("4.Ожидание соединения с ФАС", color.RGBA{R: 1, G: 1, B: 1, A: 255})
	contentBox := container.NewVBox(checkFasModelLabel, checkIpkDisonnectLabel, checkFdsLabel, checkFasLabel)

	fasVersion = ""
	start = false
	fasDetected = false
	fdsDetected = false

	driversInstalled = false
	fasModelDetected = false
	readyToStart = false
	services.InstallDrivers()
	go stage0Check()
	go check1Stage(checkFasModelLabel)
	go check2Stage(checkIpkDisonnectLabel, w)
	go check3Stage(checkFdsLabel)
	go check4Stage(checkFasLabel, c3)
	go transferToSecondWindow(w, w2, errWindow)

	w.SetContent(contentBox)
	w.Show()
	a.Run()

}
