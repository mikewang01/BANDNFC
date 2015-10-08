nrfjprog.exe --eraseall --reset --clockspeed 2000
nrfjprog.exe --clockspeed 2000 --programs .\s110_nrf51_8.0.0_softdevice.hex --program .\ClingBand_NFC_1_13.hex .\lemonBootloader.hex --verify --pinreset 
