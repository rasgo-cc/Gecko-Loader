# efm32 loader - a GUI and CLI utility for EFM32's bootloader

A blog post about it: http://theramblingness.com/2015/07/16/a-gui-and-cli-utility-for-efm32s-uart-bootloader/

### CLI mode usage
<code>
efm32_loader.exe <port_name> <bin_file> <boot_pol>
</code>

### GUI mode
Don't pass any arguments and the GUI will show up.

###Hardware connections:
<code>
TX  -- BOOT_RX (E11)
RX  -- BOOT_TX (E10)
DTR -- BOOT_EN (DBG_SWCLK)
RTS -- RESET
</code>

### Other considerations:
In order to prevent the bootloader from being overwritten, the linkerscript must be modified as described on application note AN0003