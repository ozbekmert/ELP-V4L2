#include "/home/pi/App/ELP-USBFHD06H-L180/Recorder/main/USBFHD06H.h"
#include <iostream>
#include <cstring>
#include <string>
#include <stdint.h>
#include <stdlib.h>
///home/pi/App/ELP-USBFHD06H-L180/Recorder/Videos
/*
    fmt.fmt.pix.width = witdh;//1920;
	fmt.fmt.pix.height = height;//= 1080;
*/
using namespace std;
int main()
{ 
    int width = 1920 , height = 1080;
    USBFHD06H *t1 = new USBFHD06H();


    t1 ->setCamSetting(width,height);
    for(int l = 0 ; l < 3 ; l++ )
    {
        if(t1->saveCam(l,200)==1)
        {
            std::cout << "error" << std::endl;
        }

    }
     t1-> closeCapture();
    
}