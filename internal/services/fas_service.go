package services

import (
	"emulatortm/internal/models"
	"fmt"
)

func GetFAS() (FAS *models.FAS) {
	version, err := checkFasVersion()
	if err != nil {
		fmt.Println(err)
	}
	return &models.FAS{
		Version: version,
	}
}
