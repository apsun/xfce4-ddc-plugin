# xfce4-ddc-plugin

Control your monitor volume and brightness settings ~~from the Xfce panel~~
using hotkeys.

## Dependencies

- xfce4 >= 4.18
- [keybinder](https://github.com/kupferlauncher/keybinder) >= 0.3.2
- [ddcutil](https://github.com/rockowitz/ddcutil) >= 1.4.1

## Installation

After installing ddcutil, ensure that the udev rules are loaded:

```Bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Load the `i2c-dev` kernel module immediately and at future boot:

```Bash
sudo modprobe i2c-dev
sudo tee /etc/modules-load.d/i2c-dev.conf <<< i2c-dev
```

Then, build and install the plugin:

```Bash
./autogen.sh --prefix=/usr
make
sudo make install
```

"DDC Plugin" should now show up in the available Xfce panel items. To view
the plugin logs, run `xfce4-panel` with `G_MESSAGES_DEBUG` set:

```Bash
killall xfce4-panel; G_MESSAGES_DEBUG=xfce4-ddc-plugin xfce4-panel
```

## License

GPLv3
