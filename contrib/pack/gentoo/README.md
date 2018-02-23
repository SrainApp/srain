# Before installation
- Python target is hard-coded to python3_5, which is the default python interpreter now. For python3_6 and higher, you need to change srain ebuilds with correct python header path in function `src_prepare()`.
- Live ebuild, that is srain-9999, is designed to build from upstream and help developers debug programs. If you think so, please consider srain-9999.ebuild, else just use srain-0.06.x.ebuild. In order to use live 9999 ebuild, tell portage to autounmask it by `emerge --ask --verbose --autounmask-write =net-irc/srain-9999` then run `dispatch-conf` to update package.accept_keywords.

# Installation
- Create a local overlay and put srain ebuilds in category `net-irc/srain`. For creating a local overlay, please read [gentoo local overlay wiki](https://wiki.gentoo.org/wiki/Handbook:AMD64/Portage/CustomTree#Defining_a_custom_repository) and [overlay development](https://wiki.gentoo.org/wiki/Custom_repository).

# Configuration
- Check out [srainrc.example](https://raw.githubusercontent.com/SilverRainZ/srain/master/srainrc.example) and `/etc/xdg/srain/builtin.cfg` sample configuration file. You can copy and rename it to `~/.config/srain/srain.cfg` , change `srain.cfg` as whatever you like, then (re)start srain client.

If you find any bugs or ideas, file an issue [here](https://github.com/SilverRainZ/srain/issues).
