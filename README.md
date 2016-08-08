# fyrelab sentri
## Target
fyrelab sentri is a system using additional hardware such as webcams, microphones, temperatures sensors etc.
to provide an intelligent home monitoring system as a baby monitor, burglar alarm or pet watch.
It is easily configurable through [sentri-webtool](https://github.com/fyrelab/sentri-webtool)

## Install
For a full installation follow our [installation guide](https://fyrelab.de/sentri/install#advanced)

If all dependencies are satisfied you can simply run `./configure` first,
then `make all` and then `make install` with root privileges to build the system.

### Building
Building with `make` with the following targets

* all: build as release build
* install: install system with all services
* run: Build as debug and run automatically
* debug: Build as debug build
* check: check code style
* clean: clean object files
* distclean: clean all build files (including binaries)

## Coding style
Google Coding style with 120 letters expansion and disabled constant references is used.
https://google.github.io/styleguide/cppguide.html

## Licensing
fyrelab sentri is licensed under GPLv3. See `LICENSE` for further information.

### Used libraries:
* Boost (Boost license) [headers only, lib itself dynamically linked]
* jsonxx (MIT) [statically linked]
* libquickmail (GPLv3) [statically linked]
* embeddedRest (MIT) [statically linked]
* rapidjson (MIT, JSON) [statically linked]
* v4l2loopback (GPLv2) [used as external binary, completely independent]
