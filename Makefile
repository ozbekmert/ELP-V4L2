EXPENDABLES =  test H264Handler.o RecordH264HD.h264 0FFH264.h264 1FFH264.h264 2FFH264.h264 3FFH264.h264 4FFH264.h264 5FFH264.h264
 
# Add any necessary include directories here.
INCLUDE_DIRS = -I /home/pi/App/ELP-USBFHD06H-L180/Recorder/main
LIB_DIRS = -I /home/pi/App/ELP-USBFHD06H-L180/Recorder/sdk

all: Project


Project: Test.cpp H264Handler.o 
	     g++ -Wall -g  -o test Test.cpp H264Handler.o

H264Handler.o:  USBFHD06H.cpp USBFHD06H.h
				 g++ -Wall -g $(INCLUDE_DIRS) $(LIB_DIRS) -c USBFHD06H.cpp -o H264Handler.o 


clean:
	rm -f $(EXPENDABLES)