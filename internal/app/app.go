package app

import (
	"emulatortm/internal/services"
	"fmt"
	"image/color"
	"time"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/canvas"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/dialog"
	"github.com/amdf/ipk"
	"github.com/google/gousb"
)

var ipkBox ipk.IPK

var vid gousb.ID

var pid gousb.ID

// check
var fasDetected bool
var fdsDetected bool

// stages
var fasModelDetected bool
var driversInstalled bool
var readyToEmulate bool

//shoud i add stage checking

//var can candev.Device

func Run() {

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
	ipkBox.AnalogDev.Open() //открываем ФАС-3

	ipkBox.BinDev.Open() //открываем ФДС-3
	defer ipkBox.BinDev.Close()
	ipkBox.FreqDev.Open() //открываем ФЧС-3
	defer ipkBox.FreqDev.Close()

	fas := services.GetFAS(ipkBox.AnalogDev.Active())
	//fyne
	a := app.New()
	w := a.NewWindow("Эмулятор ТМ")
	w2 := a.NewWindow("Эмулятор ТМ")
	w.Resize(fyne.NewSize(300, 300))
	checkFasModelLabel := canvas.NewText("1.Определение модели ФАС", color.RGBA{R: 1, G: 1, B: 1, A: 255})
	checkIpkDisonnectLabel := canvas.NewText("2.Выключите питание ИПК", color.RGBA{R: 1, G: 1, B: 1, A: 255})
	checkFdsLabel := canvas.NewText("3.Ожидание соединения с ФДС", color.RGBA{R: 1, G: 1, B: 1, A: 255})
	checkFasLabel := canvas.NewText("4.Ожидание соединения с ФАС", color.RGBA{R: 1, G: 1, B: 1, A: 255})
	contentBox := container.NewVBox(checkFasModelLabel, checkIpkDisonnectLabel, checkFdsLabel, checkFasLabel)

	fasDetected = false
	fdsDetected = false

	driversInstalled = false
	fasModelDetected = false
	readyToEmulate = false

	go func() {
		checkFasModelLabel.Color = color.RGBA{R: 200, G: 200, B: 200, A: 255}
		checkFasModelLabel.Text = fmt.Sprintf("1.Модель ФАС: %s", fas.Version)
		checkFasModelLabel.Refresh()
		fasModelDetected = true
	}()
	go func() {
		for range time.Tick(time.Second / 2) {
			if fasModelDetected {
				switch ipkBox.AnalogDev.Active() {
				case false:
					checkIpkDisonnectLabel.Color = color.RGBA{R: 200, G: 200, B: 200, A: 255}
					checkIpkDisonnectLabel.Text = "2.Питание ИПК выключено"
					checkIpkDisonnectLabel.Refresh()
					ipkBox.AnalogDev.Close()
					fasModelDetected = false
					//install drivers
					err := services.ReplaceDrivers(fas)
					if err != nil {
						services.ReturnDrivers(fas)
						break
					}
					driversInstalled = true
					dialog.ShowInformation("Эмулятор ТМ", "Включите ИПК СЕТЬ", w)
				}
			}
		}
	}()

	go func() {
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
					fdsDetected = true
				}
			}
		}
	}()

	go func() {
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
				}
			}
		}
	}()
	go func() {
		for range time.Tick(time.Second / 2) {
			if fasDetected && fdsDetected {
				fasDetected = false
				fdsDetected = false
				//w.Close()
				w.Hide()
				readyToEmulate = true
				app2(w2)
			}
		}
	}()
	go func() {
		if readyToEmulate {
			fmt.Println("Finally cameto part 2")
		}
	}()
	w.SetContent(contentBox)
	w.Show()
	a.Run()

	/*err := can.Init(0x1F, 0x16)
	if err != nil {
		/*wiz.Error(lang.Str("Не удалось произвести инициализацию CAN:"))
		wiz.Error(err.Error())
		time.Sleep(time.Second * 2)
		return
	}
	can.Run()*/

	go func() {
		for range time.Tick(time.Second / 2) {
			if !ipkBox.AnalogDev.Active() {
				fas.Connected = false
			}
		}
	}()
	fmt.Printf("Определена модель фас:%s", fas.Version)
	fmt.Println("Отключите питание ФАС")

	ipkBox.AnalogDev.Open() //открываем ФАС-3
	defer ipkBox.AnalogDev.Close()

	/*err := services.ReturnDrivers(fas)
	if err != nil {
		//function to exit
	}*/
	vid = 0x0547
	pid = 0x0894
	/*err := services.ResetUsbDevice(vid, pid)
	if err != nil {
		fmt.Println(err)
	}*/

}
