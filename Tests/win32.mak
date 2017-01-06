#
#	PySVN win32 tests - win32.mak
#
all: test-01.win32.new.log test-03.win32.new.log test-04.win32.new.log test-05.win32.new.log test-06.win32.new.log test-07.win32.new.log

clean: clean-01 clean-03 clean-04 clean-05 clean-06 clean-07

#
#	Helpers while developing and verifying tests
#
WB_DIFF=wb-diff.cmd
test-01.win32.new.log: test-01.cmd test-01.win32.known_good-$(KNOWN_GOOD_VERSION).log
	-subst b: /d >nul 2>&1
	if exist testroot-01 rmdir /s /q testroot-01
	test-01.cmd >test-01.win32.new.log 2>&1
	python benchmark_diff.py test-01.win32.known_good-$(KNOWN_GOOD_VERSION).log test-01.win32.new.log

clean-01:
	-subst b: /d >nul 2>&1
	if exist test-01.win32.new.log del test-01.win32.new.log
	if exist testroot-01 rmdir /s /q testroot-01

diff-01: test-01.win32.new.log
	$(WB_DIFF) test-01.win32.known_good-$(KNOWN_GOOD_VERSION).log.clean test-01.win32.new.log.clean

new-01: test-01.win32.new.log
	copy test-01.win32.new.log test-01.win32.known_good-$(KNOWN_GOOD_VERSION).log

test-03.win32.new.log: test-03.cmd test-03.win32.known_good-$(KNOWN_GOOD_VERSION).log
	-subst b: /d >nul 2>&1
	if exist testroot-03 rmdir /s /q testroot-03
	if "%USERNAME%" == "barry" test-03.cmd >test-03.win32.new.log 2>&1
	if "%USERNAME%" == "barry" python benchmark_diff.py test-03.win32.known_good-$(KNOWN_GOOD_VERSION).log test-03.win32.new.log

clean-03:
	-subst b: /d >nul 2>&1
	if exist test-03.win32.new.log del test-03.win32.new.log
	if exist testroot-03 rmdir /s /q testroot-03

diff-03: test-03.win32.new.log
	$(WB_DIFF) test-03.win32.known_good-$(KNOWN_GOOD_VERSION).log.clean test-03.win32.new.log.clean

new-03: test-03.win32.new.log
	copy test-03.win32.new.log test-03.win32.known_good-$(KNOWN_GOOD_VERSION).log

test-04.win32.new.log: test-04.cmd test-04.win32.known_good-$(KNOWN_GOOD_VERSION).log
	-subst b: /d >nul 2>&1
	if exist testroot-04 rmdir /s /q testroot-04
	test-04.cmd >test-04.win32.new.log 2>&1
	python benchmark_diff.py test-04.win32.known_good-$(KNOWN_GOOD_VERSION).log test-04.win32.new.log

clean-04:
	-subst b: /d >nul 2>&1
	if exist test-04.win32.new.log del test-04.win32.new.log
	if exist testroot-04 rmdir /s /q testroot-04

diff-04: test-04.win32.new.log
	$(WB_DIFF) test-04.win32.known_good-$(KNOWN_GOOD_VERSION).log.clean test-04.win32.new.log.clean

new-04: test-04.win32.new.log
	copy test-04.win32.new.log test-04.win32.known_good-$(KNOWN_GOOD_VERSION).log

test-05.win32.new.log: test-05.cmd test-05.win32.known_good-$(KNOWN_GOOD_VERSION).log
	-subst b: /d >nul 2>&1
	if exist testroot-05 rmdir /s /q testroot-05
	test-05.cmd >test-05.win32.new.log 2>&1
	python benchmark_diff.py test-05.win32.known_good-$(KNOWN_GOOD_VERSION).log test-05.win32.new.log

clean-05:
	-subst b: /d >nul 2>&1
	if exist test-05.win32.new.log del test-05.win32.new.log
	if exist testroot-05 rmdir /s /q testroot-05

diff-05: test-05.win32.new.log
	$(WB_DIFF) test-05.win32.known_good-$(KNOWN_GOOD_VERSION).log.clean test-05.win32.new.log.clean

new-05: test-05.win32.new.log
	copy test-05.win32.new.log test-05.win32.known_good-$(KNOWN_GOOD_VERSION).log

test-06.win32.new.log: test-06.cmd test-06.win32.known_good-$(KNOWN_GOOD_VERSION).log
	-subst b: /d >nul 2>&1
	if exist testroot-06 rmdir /s /q testroot-06
	test-06.cmd >test-06.win32.new.log 2>&1
	python benchmark_diff.py test-06.win32.known_good-$(KNOWN_GOOD_VERSION).log test-06.win32.new.log

clean-06:
	-subst b: /d >nul 2>&1
	if exist test-06.win32.new.log del test-06.win32.new.log
	if exist testroot-06 rmdir /s /q testroot-06

diff-06: test-06.win32.new.log
	$(WB_DIFF) test-06.win32.known_good-$(KNOWN_GOOD_VERSION).log.clean test-06.win32.new.log.clean

new-06: test-06.win32.new.log
	copy test-06.win32.new.log test-06.win32.known_good-$(KNOWN_GOOD_VERSION).log

test-07.win32.new.log: test-07.cmd test-07.win32.known_good-$(KNOWN_GOOD_VERSION).log
	-subst b: /d >nul 2>&1
	if exist testroot-07 rmdir /s /q testroot-07
	test-07.cmd >test-07.win32.new.log 2>&1
	python benchmark_diff.py test-07.win32.known_good-$(KNOWN_GOOD_VERSION).log test-07.win32.new.log

clean-07:
	-subst b: /d >nul 2>&1
	if exist test-07.win32.new.log del test-07.win32.new.log
	if exist testroot-07 rmdir /s /q testroot-07

diff-07: test-07.win32.new.log
	$(WB_DIFF) test-07.win32.known_good-$(KNOWN_GOOD_VERSION).log.clean test-07.win32.new.log.clean

new-07: test-07.win32.new.log
	copy test-07.win32.new.log test-07.win32.known_good-$(KNOWN_GOOD_VERSION).log

