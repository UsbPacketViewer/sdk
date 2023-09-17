all:usbpv_test_lib
clean:
	rm usbpv_test_lib main.o ./x64/usbpv_test_gcc
usbpv_test_lib:main.o
	g++ -Wl,-O1 -o usbpv_test_lib main.o -lpthread
	cp usbpv_test_lib ./x64/usbpv_test_gcc
main.o:
	g++ -c -pipe -O2 -std=gnu++11 -Wall -Wextra -Wno-format -Wno-unused-but-set-variable -D_REENTRANT -fPIC -DQT_NO_DEBUG -I../usbpv_lib -I. -o main.o ../usbpv_test_lib/main.cpp

test:usbpv_test_lib
	cd x64 && ./usbpv_test_gcc
