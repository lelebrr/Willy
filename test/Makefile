all: test_willy_splash

willy_extracted.cpp: ../src/ui/willy_splash.cpp
	echo '#include <cstdint>' > willy_extracted.cpp
	sed -n '/struct WillyConfig/,/^};/p' ../src/ui/willy_splash.cpp >> willy_extracted.cpp
	echo 'static WillyConfig willyCfg;' >> willy_extracted.cpp
	sed -n '/void load_willy_config()/,/^}/p' ../src/ui/willy_splash.cpp >> willy_extracted.cpp

test_runner: test_willy_splash.cpp willy_extracted.cpp
	g++ -I. test_willy_splash.cpp -o test_runner

test_willy_splash: test_runner
	./test_runner

clean:
	rm -f test_runner willy_extracted.cpp
