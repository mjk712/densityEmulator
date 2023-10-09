package services

import (
	"emulatortm/internal/models"
	"log"
)

func GetFAS(analogDevActive bool) (FAS *models.FAS) {
	version, err := checkFasVersion()
	if err != nil {
		log.Fatal(err)
	}

	return &models.FAS{
		Connected: analogDevActive,
		Version:   version,
	}
}
