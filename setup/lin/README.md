# EmulatorTM

https://losst.pro/sozdanie-deb-paketov

Создайте файл манифеста DEBIAN/control со следующим содержимым:

Package: EmulatorTM
Version: 1.0.0
Section: unknown
Priority: optional
Depends: 
Architecture: amd64
Essential: no
Installed-Size: 20
Maintainer: Elmeh
Description: Эмулятор плотности ТМDEBIAN/control
Oт каких пакетов зависит программа Depends:

Определяем от каких пакетов будет зависеть программа.

objdump -p .//usr/share/testSrs3/testSrs3 | grep NEEDED

        NEEDED               libpthread.so.0
        NEEDED               libusb-1.0.so.0
        NEEDED               libresolv.so.2
        NEEDED               libGL.so.1
        NEEDED               libX11.so.6
        NEEDED               libXrandr.so.2
        NEEDED               libXxf86vm.so.1
        NEEDED               libXi.so.6
        NEEDED               libXcursor.so.1
        NEEDED               libm.so.6
        NEEDED               libXinerama.so.1
        NEEDED               libdl.so.2
        NEEDED               librt.so.1
        NEEDED               libc.so.6

 Чтобы посмотреть в каком пакете находятся выполните:

    dpkg -S libpthread.so.0

    libc6-i386: /lib32/libpthread.so.0
    libc6:amd64: /lib/x86_64-linux-gnu/libpthread.so.0

    Пакет называется libc6, libc6-i386



control

    Package: EmulatorTM
    Version: 1.0.0
    Provides: ixxat-socketcan
    Maintainer: Elmeh Development Team
    Architecture: all
    Installed-Size: 23000
    Priority: required
    Depends: libc6, libusb-1.0-0, ia32-libs, libc6-i386, libgl1, libx11-6, libxrandr2, libxxf86vm1, libxi6, libxcursor1, libxinerama1         
    Essential: no
    Description: Эмулятор плотности ТМ


## .desktop

Общие ярлыки приложений хранят в /usr/share/applications; свои - в ~/.local/share/applications. 

### Общее название группы для всех файлов ".desktop".
 Строка [Desktop Entry] - первая; прочие - в любом порядке.

[Desktop Entry]


### Какой версии спецификации соответствует сам этот файл.
Свежая - 1.1. Не обязательно.

Version=1.0


### Кодировка самого файла. Обычно - UTF-8.
Списки есть, например, у iconv. Не обязательно (устарело).

Encoding=UTF-8


### Тип объекта: Application - приложение;
Directory - категория; Link - ссылка на ресурс Интернета.

Type=Application


### "Категория" здесь - это заголовок подменю
в общем меню приложений. Здесь не переводится.
В значении может быть несколько частей,
их отделять символом ;.
И в конце строки рекомендуется поставить символ ;.
Если символ ; используется сам по себе - экранировать: \;.

Categories=System;Utility;


### Команда для запуска. Желательно указать полный путь.
Можно короткое имя, если программа доступна через $PATH.
Если нужно запустить с правами суперпользователя,
то нужно начинать команду, например, с gksudo -gk.
Понадобится установить программу gksudo или kdesudo.

Exec=top10t.sh


### Рабочий каталог. Не обязательно.

Path=/home/student


### Нужно ли сначала открыть окно эмулятора терминала,
а потом запустить в нём значение Exec.
"Да" - true; "нет" - false. Обычно "нет".

Terminal=true


### Файл значка. Обычно указывают короткое имя без расширения.
Стандартные форматы файлов: PNG, SVG (SVGZ).
Значки обычно хранят в /usr/share/icons.

Icon=utilities-terminal


### Нужно ли оповещать о запуске: помигать указателем мыши
или аплетом списка задач и тому подобное. Обычно "да".

StartupNotify=true


### Название ярлыка, видимое как подпись к значку или
как имя пункта в меню. Здесь на английском.

Name=Top 10 greedy threads


### Желательно перевести. Список условных обозначений
языков есть, например, у locale.

Name[ru]=Десять самых жадных потоков


### Описание, обычно видимое как всплывающая подсказка.

Comment=Shows Top 10 cpu eating processes/threads


### Желательно перевести.

Comment[ru]=Показывает 10 самых жрущих ЦПУ процессов/потоков


### Не показывать в меню. Обычно "нет".
Файловые ассоциации, если есть, будут работать.
NoDisplay=false


### Hе показывать в меню, убрать из файловых ассоциаций.
И вообще сделать вид, что приложения не существует.
Обычно "нет".

Hidden=false


### Показывать только в указанной рабочей среде:
GNOME, KDE, Xfce, ещё какие-нибудь через ;.

OnlyShowIn=GNOME;

### Не показывать в указанных рабочих средах.

В файле должен быть только один из параметров:
либо OnlyShowIn, либо NotShowIn.

NotShowIn=KDE;

---


    [Desktop Entry]
    Type=Application
    Name=EmulatorTM 
    Exec=/usr/share/EmulatorTM/EmulatorTM
    Icon=/usr/share/pixmaps/EmulatorTM.png
    Path=/usr/share/EmulatorTM
    Version=1.0.0
    Categories=System;
    Terminal=false
    StartupNotify=true
    Comment[ru]=Эмулятор плотности ТМ



## Сборка

    fakeroot dpkg-deb --build EmulatorTM


Удалить пакет

    sudo dpkg -r EmulatorTM