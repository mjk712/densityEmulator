package services

import (
	"emulatortm/internal/models"
	"log"
	"os"
)

func ReplaceDrivers(fas *models.FAS) error {

	//вырубаем ипк

	var sOrigDriverFileName string
	var sModifDriverFileName string

	switch fas.Version {
	case "12 Bit":
		sOrigDriverFileName = "ANLBRT_3.spt"
		sModifDriverFileName = "DENS_ANLBRT.spt"
	case "16 Bit":
		sOrigDriverFileName = "ANLNEW_3.spt"
		sModifDriverFileName = "DENS_ANLNEW.spt"
	}

	binRoot := "bin"
	sys32root := "c:/Windows/System32/ipkload"
	sysWOW64root := "c:/Windows/SysWOW64/IPKLoad"
	//меняем дрова
	/*err := os.Rename(binRoot+"/"+sModifDriverFileName, sys32root+"/"+sOrigDriverFileName)
	if err != nil {
		log.Fatal(err)
	}
	err = os.Rename(binRoot+"/"+sModifDriverFileName, sysWOW64root+"/"+sOrigDriverFileName)
	if err != nil {
		log.Fatal(err)
	}*/

	os.Remove(sys32root + "/" + sOrigDriverFileName)

	os.Remove(sysWOW64root + "/" + sOrigDriverFileName)

	sourceDrive, err := os.ReadFile(binRoot + "/" + sModifDriverFileName)
	if err != nil {
		log.Fatal(err)
	}

	err = os.WriteFile(sys32root+"/"+sOrigDriverFileName, sourceDrive, 0666)
	if err != nil {
		log.Fatal(err)
	}
	err = os.WriteFile(sysWOW64root+"/"+sOrigDriverFileName, sourceDrive, 0666)
	if err != nil {
		log.Fatal(err)
	}

	//врубаем ипк
	return nil
}

func ReturnDrivers(fas *models.FAS) error {
	//вырубаем ипк

	var sOrigDriverFileName string
	//var sModifDriverFileName string

	switch fas.Version {
	case "12 Bit":
		sOrigDriverFileName = "ANLBRT_3.spt"
		//sModifDriverFileName = "DENS_ANLBRT.spt"
	case "16 Bit":
		sOrigDriverFileName = "ANLNEW_3.spt"
		//sModifDriverFileName = "DENS_ANLNEW.spt"
	}

	binRoot := "bin"
	sys32root := "c:/Windows/System32/ipkload"
	sysWOW64root := "c:/Windows/SysWOW64/IPKLoad"
	//меняем дрова
	/*err := os.Rename(sys32root+"/"+sOrigDriverFileName, binRoot+"/"+sModifDriverFileName)
	if err != nil {
		log.Fatal(err)
	}
	err = os.Rename(sysWOW64root+"/"+sOrigDriverFileName, binRoot+"/"+sModifDriverFileName)
	if err != nil {
		log.Fatal(err)
	}*/
	err := os.Remove(sys32root + "/" + sOrigDriverFileName)
	if err != nil {
		log.Fatal(err)
	}

	err = os.Remove(sysWOW64root + "/" + sOrigDriverFileName)
	if err != nil {
		log.Fatal(err)
	}

	sourceDrive, err := os.ReadFile(binRoot + "/" + sOrigDriverFileName)
	if err != nil {
		log.Fatal(err)
	}

	err = os.WriteFile(sys32root+"/"+sOrigDriverFileName, sourceDrive, 0666)
	if err != nil {
		log.Fatal(err)
	}
	err = os.WriteFile(sysWOW64root+"/"+sOrigDriverFileName, sourceDrive, 0666)
	if err != nil {
		log.Fatal(err)
	}
	return nil
}

func InstallDrivers() {
	anlBrtDriverFileName := "ANLBRT_3.spt"
	anlNewDriverFileName := "ANLNEW_3.spt"
	binRoot := "bin"
	sys32root := "c:/Windows/System32/ipkload"
	sysWOW64root := "c:/Windows/SysWOW64/IPKLoad"
	//-----------------------------------12 bit driver install----------------------
	sourceDrive, err := os.ReadFile(binRoot + "/" + anlBrtDriverFileName)
	if err != nil {
		log.Fatal(err)
	}

	err = os.WriteFile(sys32root+"/"+anlBrtDriverFileName, sourceDrive, 0666)
	if err != nil {
		log.Fatal(err)
	}
	err = os.WriteFile(sysWOW64root+"/"+anlBrtDriverFileName, sourceDrive, 0666)
	if err != nil {
		log.Fatal(err)
	}
	//-----------------------------------16 bit driver install----------------------
	sourceDrive2, err := os.ReadFile(binRoot + "/" + anlNewDriverFileName)
	if err != nil {
		log.Fatal(err)
	}

	err = os.WriteFile(sys32root+"/"+anlNewDriverFileName, sourceDrive2, 0666)
	if err != nil {
		log.Fatal(err)
	}
	err = os.WriteFile(sysWOW64root+"/"+anlNewDriverFileName, sourceDrive2, 0666)
	if err != nil {
		log.Fatal(err)
	}
}
