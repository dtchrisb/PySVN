rem test 01 test commands
call test-01.cmd >test-01.new.log 2>&1
rem test 02 is not a regression test
rem can only run 03 on barry's LAN
if "%USERNAME%" == "barry" call test-03.cmd >test-03.new.log 2>&1

python benchmark_diff test-01.known_good.log test-01.new.log
if "%USERNAME%" == "barry" python benchmark_diff test-03.known_good.log test-03.new.log
