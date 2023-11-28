package app

import (
	"emulatortm/internal/models"
	"emulatortm/internal/services"
	"fmt"
	"strconv"
	"time"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/layout"

	"fyne.io/fyne/v2/widget"
)

var topPanel = container.NewHBox(
	widget.NewLabel(""),
)

var editDensity = widget.NewEntry()

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
			go services.InitDensity(editDensity.Text, ipkBox.AnalogDev)
		}),
		widget.NewButton("Сброс", func() {
			go services.ResetDensity(editDensity.Text, ipkBox.AnalogDev)
		}),
	),
	secDesc,
)

func app2(w2 fyne.Window, fas *models.FAS, errWindow fyne.Window) {
	//Меню справки
	faqMenu := fyne.NewMenuItem("Справка", func() {
		aboutHelp()
	})
	newMenu := fyne.NewMenu("Info", faqMenu)
	mainMenu := fyne.NewMainMenu(newMenu)
	w2.SetMainMenu(mainMenu)

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
	w2.Resize(fyne.Size{Width: 500, Height: 400})
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
