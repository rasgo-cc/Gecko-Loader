# efm32 loader - a GUI and CLI utility for EFM32's bootloader

![](https://github.com/cidadao/Gecko-Loader/raw/master/img/win_scr.png)

A blog post about it: http://theramblingness.com/2015/07/16/a-gui-and-cli-utility-for-efm32s-uart-bootloader/

### CLI mode usage
```
efm32_loader.exe <port_name> <bin_file> <boot_pol>
```

### GUI mode
```
Don't pass any arguments and the GUI will show up.
```

###Hardware connections:
```
TX  -- BOOT_RX (E11)
RX  -- BOOT_TX (E10)
DTR -- BOOT_EN (DBG_SWCLK)
RTS -- RESET
```

### Other considerations:
In order to prevent the bootloader from being overwritten, the linkerscript must be modified as described on application note AN0003
