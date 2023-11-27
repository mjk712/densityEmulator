Программа для измерения плотности "Эмулятор ТМ"
Программа отправялет данные на микроконтроллер через модифицированную версию драйвера ФАС.
Модифицированные драйвера: DENS_ANLBRT(12 bit) и DENS_ANLNEW(16 bit).
При работе с этой программой, в ИПТМ-395 настройка "Датчик ТЦ" должна быть настроена как "АЦП".
Драйвера ИПК должны быть установлены по этим путям:
C:/Windows/System32/ipkload
C:/Windows/SysWOW64/IPKLoad
Для работы программы требуется libusb-1.0
Инструкция по установке:
1. https://github.com/google/gousb#notes-for-installation-on-windows
2. Для начала скачиваем mingw с gcc под капотом.(https://sourceforge.net/projects/mingw/) (Не забудьте добавить mingw/bin в path переменных сред)
3. Потом устанавливаем pkg-config(https://stackoverflow.com/questions/1710922/how-to-install-pkg-config-in-windows)
4. Далее настраиваем pkg-config-path, нужно указывать папку в которой будет лежать lib с libusb (пример команды set PKG_CONFIG_PATH=F:\+GTK-SOURCES\gnu-windows\lib\pkgconfig; у меня это был C:\mingw64\lib\pkgconfig, в mingw удобнее всего)
5. В папку, путь к которой указали, кидаем libusb-1.0.pc, остальные файлы libusb закиньте в папку lib 

