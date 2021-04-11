# i3-winmotion

i3-winmotion is a tool that allows you to move between [i3](https://i3wm.org/)
windows using a using specified focus keys (see the demo below).

The credits for the idea go to [pcharest2000/winswitch](https://github.com/pcharest2000/winswitch).
i3-winmotion is a complete rewrite using the i3-ipc bindings provided by
[i3-ipcpp](https://github.com/Iskustvo/i3-ipcpp), which allows i3-winmotion to
**only highlight visible windows**, avoiding overlapping hints. This is done by
 traversing the i3 JSON workspace layout and enabling hints only for the visible
 windows in a tab or stack.

## How it works
i3-winmotion uses the hint keys (default: 'asdfjkl') to create 'hints'
consisting of the minimum number of characters to create unique hints that are
assigned to the visible windows. Entering the displayed key combination will
cause the corresponding window to become focussed. Pressing escape exits the
application and keeps the focus unchanged.

For example, if there are 2 hint keys and 2 visible windows, each window will have one
unique hint key assigned. But if there are 2 hint keys and 3 visible windows
i3-winmotion will assign hints consisting of 2 keys to each window as each hint
has to be an unique combination of the hint keys.

_Note that the hints are displayed as uppercase, but the expected key presses are
lower case._

## Requirements
* i3
* [RapidJSON](https://rapidjson.org/)

## Building
Simply run `make` in the root of the project.

It will automatically initialize the
i3-ipcpp submodule and compile i3-ipcpp (if it's not already compiled).
Then it will compile i3-winmotion and place the resulting binary in `./bin`.

## TODO
Add argument parsing to make everything more configurable.

## Demo
![i3-winmotion in action 1](screenshots/demo.gif)
![i3-winmotion in action 2](screenshots/screenshot_default.png)
![i3-winmotion in action 3](screenshots/screenshot_floating.png)

