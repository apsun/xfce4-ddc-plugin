# xfce4-ddc-plugin

Control your monitor volume and brightness settings ~~from the Xfce panel~~
using hotkeys.

## Dependencies

- xfce4 >= 4.18
- [keybinder](https://github.com/kupferlauncher/keybinder) >= 0.3.2
- [ddcutil](https://github.com/rockowitz/ddcutil) >= 1.3.0

## Installation

Ensure that the `i2c-dev` module is loaded at boot:

```Bash
sudo tee /etc/modules-load.d/i2c-dev.conf <<< i2c-dev
```

Add your user to the `i2c` group:

```Bash
sudo usermod -aG i2c ${USER}
```

Reboot to make the above changes take effect. Then, build and install the
plugin:

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

Note: if the plugin crashes on load, it's probably because another
application has registered the volume/brightness hotkeys. Try
stopping/uninstalling `xfce4-pulseaudio-plugin` (volume) and
`xfce4-power-manager` (brightness).

## License

GPLv3
