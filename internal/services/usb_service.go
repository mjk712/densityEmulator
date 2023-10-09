package services

import (
	"errors"
	"fmt"

	"github.com/amdf/ipk"
	"github.com/google/gousb"
)

func checkFasVersion() (string, error) {

	var fasVersion string

	_, ok := ipk.USBOpen(ipk.IDProductANL12bit)
	if ok {
		fasVersion = "12 Bit"
		return fasVersion, nil
	}
	_, ok = ipk.USBOpen(ipk.IDProductANL16bit)
	if ok {
		fasVersion = "16 Bit"
		return fasVersion, nil
	}
	return "", errors.New("fail open usb")
}

func ResetUsbDevice(vid gousb.ID, pid gousb.ID) error {
	ctx := gousb.NewContext()
	defer ctx.Close()

	device, err := ctx.OpenDeviceWithVIDPID(vid, pid)
	if err != nil {
		fmt.Println("fff")
		return err
	}
	defer device.Close()
	err = device.Reset()
	if err != nil {
		return err
	}
	return nil
}
