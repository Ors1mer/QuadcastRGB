# QuadcastRGB
## About
The program (or driver, if you wish) allows changing the RGB colors of the
microphone HyperX Quadcast S just like NGenuity does. Should support all
platforms, Unix-like preferably. Only Linux and partially FreeBSD were tested
so far.

### Features:
- *open source (GPLv3)*
- *cross-platform*
- *cli*
- *no need to run in the background*

### Things yet to be done:
- *all modes except **solid***
- *brightness & speed*
- *delay - **blink**-specific option*
- *actual installation*
- *save the previous settings*

### Examples:
```bash
quadcastrgb solid # set default solid color (#f20000) for the whole micro

quadcastrgb -a cycle # set default cycle (rainbow) mode for the whole micro

quadcastrgb -u solid 4c0099 -l solid ff6000 # set purple color for the upper part and yellow for the lower

quadcastrgb -u -b 50 cycle -l lightning ff6000 # set default cycle mode for the upper diode with 50% brightness and yellow lightning for the lower
```
## Install
### Compiling from source
If you are lucky, this should be enough:
```bash
make install # for linux
gmake install OS=freebsd # for freebsd
```
## Basic problems during&after Install
### Problem 1: make failed
Check the dependencies:  
 - gcc v12.2.0 OR clang v14.0.6 (most versions should do fine)
 - libusb-1.0 v1.0.26
 - gettext v0.21-2

### Problem 2: command not found
Check the $PATH and manpath. The program follows XDG specifications, so the
binary is stored in $HOME/.local/bin (should be in $PATH) and the man is in
$HOME/.local/share/man (should be in $MANPATH). It is possible to move them,
of course.

### Problem 3: couldn't open the microphone
Check the error code. If it is 4, try running the program under superuser
privileges. If that was the problem, you should eventually create a dev rule
for the microphone to allow certain users access to it.

### How to create the udev rule
```bash
cd /etc/udev/rules.d # here the rules are stored
 vi 10-quadcast-perm.rules # ...under superuser
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0951", ATTRS{idProduct}=="171f", MODE="0660", GROUP="hyperrgb" # write this line, save & exit (:wq)
```
Now the microphone is accessible for the group "hyperrgb". Add your user to the
group and it's done.
