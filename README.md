## EZ Cat 5

EZ Cat 5 is a disk cataloguer. Any USB pen drive, hard drive, directory, disk, mountpoint etc. can be indexed and later browsed or searched without the original media present. See the project web site (link below) for more information including screenshot(s).

### Latest Release Version

5.0

### O/S Environment

It is designed to run on Qt 5.8+ and its home operating system is Kubuntu 18.04 / 18.10. It also runs on Debian 10 (Buster) and may also run on other KDE (and maybe non-KDE) desktops.

### Licence
GPLv3

### Known Issues

Toolbar and menu icons are unlikely to work well on non-KDE desktops. The search functionality needs improvement.

### Download

Tarballs, deb files and an apt repository are available [on the project website](https://www.loggytronic.com/ezcat5). Source code is available [on GitHub](https://github.com/ChrisTallon/ezcat).

### Build From Source

On Kubuntu:

	sudo apt install build-essential qtbase5-dev libblkid-dev
	tar -zxvf ezcat-5.0.tar.gz
	export QT_SELECT=qt5
	mkdir ezcat-build
	cd ezcat-build
	qmake ../ezcat-5.0/EZCat.pro
	make

An executable 'ezcat' will be built in the ezcat-build folder.

### Links

Web: https://www.loggytronic.com/ezcat5

Source: https://github.com/ChrisTallon/ezcat

Bug / issue tracker: https://github.com/ChrisTallon/ezcat/issues
