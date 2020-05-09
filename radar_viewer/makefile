defualt: radar_viewer
	
radar_viewer: radar_viewer.cpp
	g++ radar_viewer.cpp -o radar_viewer `pkg-config opencv --cflags --libs` -lm -lpcap -lfftw3 -lSDL2 -lSDL2_image -lSDL2_ttf
	
clean:
	rm radar_viewer