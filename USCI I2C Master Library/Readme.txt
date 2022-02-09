Using the USCI I2C Master

|--DMA Support : All files in this folder can be used ONLY ON MSP430 devices that have the USCI module AND DMA support.
    |------main.c : Application level C file that interfaces with slave MSP430 device. (Use TI_USCI_I2C_slave.c)
    |------TI_USCI_I2C_master_dma.c : Hardware interface level C file that contains all function definitions needed for USCI             initialization and USCI I2C communication.
    |------TI_USCI_I2C_master_dma.h : Header file that contains all definitions used in application/hardware interface files.

|--Without DMA Support : All files in this folder can be used on ANY MSP430 device with the USCI module
    |------main.c : Application level C file that interfaces with slave MSP430 device. (Use TI_USCI_I2C_slave.c)
    |------TI_USCI_I2C_master.c : Hardware interface level C file that contains all function definitions needed for USCI initialization      and USCI I2C communication.
    |------TI_USCI_I2C_master.h : Header file that contains all definitions used in application/hardware interface files.      