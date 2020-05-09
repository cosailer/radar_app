defualt: radar_tracker

radar_tracker: radar_tracker.c myvar.h myfft.h mysdl.h mydsp.h interpolate.h kalman.h ssa.hpp
	g++ radar_tracker.c -o radar_tracker -lm -lpcap -lfftw3 -lSDL2 -lSDL2_image -lSDL2_ttf

clean:
	rm radar_tracker


