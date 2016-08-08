.PHONY: all debug clean

all :
	@cd module_mail && $(MAKE)
	@cd module_trigger && $(MAKE)
	@cd module_video && $(MAKE)
	@cd module_sms && $(MAKE)
	@cd module_noise_detection && $(MAKE)
	@cd module_streaming && $(MAKE)
	@cd module_temperature && $(MAKE)
	@cd module_firebase && $(MAKE)

debug :
	@cd module_mail && $(MAKE) debug
	@cd module_trigger && $(MAKE) debug
	@cd module_video && $(MAKE) debug
	@cd module_sms && $(MAKE) debug
	@cd module_noise_detection && $(MAKE) debug
	@cd module_streaming && $(MAKE) debug
	@cd module_temperature && $(MAKE) debug
	@cd module_firebase && $(MAKE) debug

test :
	@cd module_mail && $(MAKE) test
	@cd module_trigger && $(MAKE) test
	@cd module_video && $(MAKE) test
	@cd module_sms && $(MAKE) test
	@cd module_noise_detection && $(MAKE) test
	@cd module_streaming && $(MAKE) test
	@cd module_temperature && $(MAKE) test
	@cd module_firebase && $(MAKE) test

clean :
	@cd module_mail && $(MAKE) clean
	@cd module_trigger && $(MAKE) clean
	@cd module_video && $(MAKE) clean
	@cd module_sms && $(MAKE) clean
	@cd module_noise_detection && $(MAKE) clean
	@cd module_streaming && $(MAKE) clean
	@cd module_temperature && $(MAKE) clean
	@cd module_firebase && $(MAKE) clean
