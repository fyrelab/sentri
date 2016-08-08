include Definitions.inc

.PHONY: all run debug clean test runtest distclean install uninstall install-core install-webtool install-daemon install-auto-wifi install-core uninstall-webtool uninstall-daemon uninstall-auto-wifi v4l2loopback install-v4l2loopback install-v4l2loopback-daemon unstall-v4l2loopback-daemon

all :
	@cd src/lib_module && $(MAKE)
	@cd src/core && $(MAKE)
	@cd src/modules && $(MAKE)

run :
	@mkdir -p "/tmp/sentri"
	@mkdir -p "/tmp/sentri/stream"
	@mkdir -p "/tmp/sentri/video"
	@mkdir -p "/tmp/sentri/audio"
	@mkdir -p "/tmp/sentri/img"
	@cd src/lib_module && $(MAKE) debug
	@cd src/modules && $(MAKE) debug
	@cd src/core && $(MAKE) run

debug :
	@cd src/lib_module && $(MAKE) debug
	@cd src/core && $(MAKE) debug
	@cd src/modules && $(MAKE) debug

clean :
	@cd src/lib_module && $(MAKE) clean
	@cd src/core && $(MAKE) clean
	@cd src/modules && $(MAKE) clean
	@cd src/testing && $(MAKE) clean

test :
	@cd src/lib_module && $(MAKE) test
	@cd src/core && $(MAKE) test
	@cd src/modules && $(MAKE) test
	@cd src/testing && $(MAKE) test

runtest : test
	@cd src/testing && $(MAKE) runtest

distclean :
	@rm -rf bin/
	@rm -rf log/

### Installation on linux

install : v4l2loopback
	@sudo make install-core
	@sudo make install-v4l2loopback
	$(MAKE) install-webtool
	$(MAKE) install-daemon
	@sudo make install-v4l2loopback-daemon
	@sudo make install-auto-wifi

uninstall :
	$(MAKE) uninstall-daemon
	$(MAKE) uninstall-webtool
	@sudo make uninstall-core
	@sudo make uninstall-v4l2loopback-daemon
	@sudo make uninstall-auto-wifi

install-core :
	@echo 'Installing $(SYSTEM_NAME)...'
	@mkdir -p /usr/share/$(SYSTEM_NAME)/ && cp -r ./res/share/* /usr/share/$(SYSTEM_NAME)
	@mkdir -p /opt/$(SYSTEM_NAME)/bin/release/ && cp -r ./bin/release/* /opt/$(SYSTEM_NAME)/bin/release
	@cp ./bin/release/lib_module/libmodule$(SYSTEM_NAME).so /usr/lib/libmodule$(SYSTEM_NAME).so
	@mkdir -p /opt/$(SYSTEM_NAME)/scripts/ && cp ./scripts/videoaudiosetup /opt/$(SYSTEM_NAME)/scripts/
	@chmod +x /opt/$(SYSTEM_NAME)/scripts/videoaudiosetup

install-webtool :
	@echo 'Installing $(SYSTEM_NAME)-webtool...'
	@mkdir -p ~/opt/$(SYSTEM_NAME)/$(SYSTEM_NAME)-webtool/ && cp -r ./$(SYSTEM_NAME)-webtool/* ~/opt/$(SYSTEM_NAME)/$(SYSTEM_NAME)-webtool
	@cd ~/opt/$(SYSTEM_NAME)/$(SYSTEM_NAME)-webtool/ && mv config.release.js config.js
	@cd ~/opt/$(SYSTEM_NAME)/$(SYSTEM_NAME)-webtool/ && npm install

install-daemon :
	@echo 'Installing daemon...'
	@mkdir -p ~/.config/systemd/user/ && cp ./systemd/$(SYSTEM_NAME).service ./systemd/$(SYSTEM_NAME)-webtool.service ./systemd/$(SYSTEM_NAME)-videoaudio.service  ~/.config/systemd/user/
	@systemctl --user daemon-reload && systemctl --user enable $(SYSTEM_NAME)

install-auto-wifi :
	@echo 'Installing auto-wifi...'
	@mkdir -p /etc/udev/rules.d && cp ./udev/99-sentri-auto-wifi-config.rules /etc/udev/rules.d/99-sentri-auto-wifi-config.rules
	@mkdir -p /usr/local/bin && cp ./udev/sentri-auto-wifi-config.sh /usr/local/bin/sentri-auto-wifi-config.sh
	@chmod +x /usr/local/bin/sentri-auto-wifi-config.sh

uninstall-auto-wifi :
	@echo 'Uninstalling auto-wifi...'
	@rm -rf /etc/udev/rules.d/99-sentri-auto-wifi-config.rules
	@rm -rf /usr/local/bin/sentri-auto-wifi-config.sh

uninstall-daemon :
	@echo 'Uninstalling daemon...'
	@systemctl --user stop $(SYSTEM_NAME) $(SYSTEM_NAME)-webtool
	@systemctl --user disable $(SYSTEM_NAME)
	@rm -rf ~/.config/systemd/user/$(SYSTEM_NAME).service ~/.config/systemd/user/$(SYSTEM_NAME)-webtool.service ~/.config/systemd/user/$(SYSTEM_NAME)-videoaudio.service

uninstall-webtool :
	@echo 'Uninstalling $(SYSTEM_NAME)-webtool...'
	@rm -rf ~/opt/$(SYSTEM_NAME)
	@rm -rf ~/.config/$(SYSTEM_NAME)

uninstall-core :
	@echo 'Uninstalling...'
	@rm -rf /usr/share/$(SYSTEM_NAME)/ /opt/$(SYSTEM_NAME)/
	@rm -rf /usr/lib/libmodule$(SYSTEM_NAME).so
#@rm -rf ~/.log/$(SYSTEM_NAME)

v4l2loopback :
	$(MAKE) -C ./dep/v4l2loopback

install-v4l2loopback :
	@sudo make install -C ./dep/v4l2loopback

install-v4l2loopback-daemon :
	@cp ./scripts/virtualvideodevicesetup /usr/local/bin
	@chmod +x /usr/local/bin/virtualvideodevicesetup
	@cp ./systemd/$(SYSTEM_NAME)-virtualdevicesetup.service /etc/systemd/system
	@chmod 664  /etc/systemd/system/$(SYSTEM_NAME)-virtualdevicesetup.service
	@systemctl daemon-reload && systemctl enable $(SYSTEM_NAME)-virtualdevicesetup

uninstall-v4l2loopback-daemon :
	@@systemctl stop $(SYSTEM_NAME)-virtualdevicesetup
	@systemctl disable $(SYSTEM_NAME)-virtualdevicesetup
	@rm -rf /etc/systemd/system/$(SYSTEM_NAME)-virtualdevicesetup.service
	@rm -rf /usr/local/bin/virtualvideodevicesetup
