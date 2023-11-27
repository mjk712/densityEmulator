package app

import (
	"emulatortm/internal/models"
	"emulatortm/internal/services"
	"fmt"
	"image/color"
	"os"
	"os/exec"
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

// Проверка на версию ФАС, 12 или 16 бит
func stage0Check() {
	for range time.Tick(time.Second / 2) {
		ipkBox.AnalogDev.Open()
		ipkBox.BinDev.Open()
		if ipkBox.AnalogDev.Active() && !start {
			start = true
			fas := services.GetFAS()
			readyToStart = true
			ipkBox.AnalogDev.Close()
			ipkBox.BinDev.Close()
			fasVersion = fas.Version
		}
	}

}

// Определили версию ФАС, теперь показываем её пользователю
func check1Stage(checkFasModelLabel *canvas.Text) {
	for range time.Tick(time.Second / 2) {
		if readyToStart {
			time.Sleep(50 * time.Millisecond)
			readyToStart = false
			checkFasModelLabel.Color = color.RGBA{R: 200, G: 200, B: 200, A: 255}
			checkFasModelLabel.Text = fmt.Sprintf("1.Модель ФАС: %s", fasVersion)
			checkFasModelLabel.Refresh()
			fasModelDetected = true
		}
	}
}

// Ждём пока будет выключен ИПК и устанавливаем правильные драйвера
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

// Ожидаем соединения с ФДС, а также устанавливаем режим стоянки
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
				// подключаем ЦАП8 и ЦАП9
				channelN9 := new(ipk.DAC)
				channelN9.Init(ipkBox.AnalogDev, ipk.DAC9)
				channelN8 := new(ipk.DAC)
				channelN8.Init(ipkBox.AnalogDev, ipk.DAC8)
				SensorIR.Init(channelN8, ipk.DACAtmosphere, 10)
				SensorTC.Init(channelN9, ipk.DACAtmosphere, 10)
				ipkBox.BinDev.Set50V(1, true)
				SensorIR.Set(8.6)
				fdsDetected = true
			}
		}
	}
}

// Ожидаем соединения с ФАС
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

// функция перехода в окно основной программы
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

// Справка
func aboutHelp() {
	err := exec.Command("cmd", "/C", ".\\bin\\index.html").Run()
	if err != nil {
		fmt.Println(err)
		fmt.Println("Ошибка открытия файла справки")
	}
}

func Run() {
	c3 := make(chan bool)
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
	w2 := a.NewWindow("Эмулятор ТМ")
	errWindow := a.NewWindow("Эмулятор ТМ")
	errWindow.SetMaster()
	w.Resize(fyne.NewSize(300, 300))

	err1 := canvas.NewText("Внимание! После закрытия программы прежде чем продолжать работу с ИПК-3,", color.RGBA{255, 0, 0, 190})
	err2 := canvas.NewText("необходимо в обязательном порядке выключить питание ФПС-3 (СЕТЬ ВЫКЛ).", color.RGBA{255, 0, 0, 190})
	err3 := canvas.NewText("После этого можно снова включить питание и продолжать работу.", color.RGBA{255, 0, 0, 190})
	err4 := canvas.NewText("Если этого не сделать, ФАС будет работать некорректно до тех пор,", color.RGBA{255, 0, 0, 190})
	err5 := canvas.NewText("пока питание не будет выключено.", color.RGBA{255, 0, 0, 170})
	errWindow.SetOnClosed(func() { os.Exit(0) })

	faqMenu := fyne.NewMenuItem("Справка", func() {
		aboutHelp()
	})

	errWindow.Resize(fyne.Size{Width: 400, Height: 400})
	errWindow.CenterOnScreen()
	errWindow.SetContent(container.NewVBox(err1, err2, err3, err4, err5))

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
	//Перед началом стадий, устанавливаем актуальные версии драйверов ФАС
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
