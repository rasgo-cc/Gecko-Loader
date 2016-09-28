

## Gecko Loader
CLI and GUI utility to upload code to EFM32 microcontrollers (Silicon Labs) through the UART or USB bootloader.

A blog post about it: http://theramblingness.com/2015/07/16/a-gui-and-cli-utility-for-efm32s-uart-bootloader/

<img src="https://github.com/cidadao/Gecko-Loader/raw/master/img/win_scr.png">

#### CLI mode usage
```
UART: gecko_loader.exe <port_name> <bin_file> uart <boot_pol>
USB:  gecko_loader.exe <port_name> <bin_file> usb
```

#### GUI mode
```
Don't pass any arguments and the GUI will show up.
```

#### Hardware connections:
```
TX  -- BOOT_RX (E11)
RX  -- BOOT_TX (E10)
DTR -- BOOT_EN (DBG_SWCLK)
RTS -- RESET
```

### Other considerations:
In order to prevent the bootloader from being overwritten, the **linker script must be modified** as described on application note AN0003
