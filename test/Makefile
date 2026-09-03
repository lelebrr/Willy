all: test_willy_splash

willy_extracted.cpp: ../include/willy_config.h ../src/ui/willy_config.cpp
	echo '#include <cstdint>' > willy_extracted.cpp
	sed -n '/struct WillyConfig/,/^};/p' ../include/willy_config.h >> willy_extracted.cpp
	echo 'static WillyConfig willyCfg;' >> willy_extracted.cpp
	sed -n '/bool load_willy_config()/,/^}/p' ../src/ui/willy_config.cpp >> willy_extracted.cpp

test_runner: test_willy_splash.cpp willy_extracted.cpp
	g++ -I. test_willy_splash.cpp -o test_runner

test_willy_splash: test_runner
	./test_runner

clean:
	rm -f test_runner willy_extracted.cpp
