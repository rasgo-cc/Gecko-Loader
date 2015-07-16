# efm32_loader
CLI and GUI utility to upload programs to EFM32 microcontrollers using the UART bootloader.

![](https://github.com/cidadao/efm32_loader/blob/master/efm32_loader_winscr.png)

EFM32 microcontrollers have a factory programmed UART bootloader that may be used to upload programs into it instead of using a commercial programmer. The bootloader uses the [XMODEM-CRC](http://www.amulettechnologies.com/index.php/welcome/xmodem-crc) protocol to transfer data and, as refered on [AN003 UART Bootloader](http://www.silabs.com/Support%20Documents/TechnicalDocs/AN0003.pdf) application note, TeraTerm, which supports that kind of data transfers, may be used. However, if you want the capability of uploading a program through your own application a command-line utility is what you need. Look no further, you have efm32_loader. It can also be used in GUI mode if any parameters are provided.

Usage (*CLI mode*):
<p>
<code>
efm32_loader.exe &ltport_name&gt &ltbin_file&gt &ltboot_pol&gt
</code>
</p>

Regarding hardware, all you need is a USB-to-UART converter connected to your computer. Connections:

<pre><code>TX  -- BOOT_RX (E11)<br>
RX  -- BOOT_TX (E10)<br>
DTR -- BOOT_EN (DBG_SWCLK)<br>
RTS -- RESET<br>
</code></pre>

**IMPORTANT:** In order to prevent the bootloader from being overwritten, the linker script should be modified as described on AN0003.




