#include "EEPROMData.h"

////////////////////////////////////////
///  Group function Write to EEPROM  ///
////////////////////////////////////////
template <class T> void EEPROMData::EEPROMSave(int StartAddress,T *storageVar){
    EEPROM.begin(MAX_EEPROM_SIZE);
	uint8_t * bytesToWriteEEPROM = (uint8_t *)storageVar;
	const int STORAGE_SIZE = sizeof(*storageVar);
	for (int i = 0; i < STORAGE_SIZE; i++) {
		EEPROM.write(StartAddress + i,bytesToWriteEEPROM[i]);
	}
	EEPROM.commit();
    EEPROM.end();
};

//////////////////////////////////////
/// Group function Read to EEPROM  ///
//////////////////////////////////////
template <class T> void EEPROMData::EEPROMRead(int StartAddress,T *storageVar){
    EEPROM.begin(MAX_EEPROM_SIZE);
	uint8_t * bytesToReadEEPROM = (uint8_t *)storageVar;
	const int STORAGE_SIZE = sizeof(*storageVar);
	for (int i = 0; i < STORAGE_SIZE; i++) {
		bytesToReadEEPROM[i] = EEPROM.read(StartAddress + i);
	}
    EEPROM.end();
};
