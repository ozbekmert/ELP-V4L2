/*
 *      H.264 USB Video Class Stream to File with Fractions
 *      Mert Özbek
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 * 		I've discovered that I can called STREAMOFF then queue up the buffers again then call STREAMON and it works again.
 * 	    This isn't ideal because I'd prefer it not to crash in the first place but for the time being it's what I'm stuck with
 *
 */

//-----------------System Includes-----------------------
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>
#include <linux/videodev2.h>
#include <linux/version.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <iostream>
#include <chrono>
#include <cstring>
//-----------------SDK Includes-----------------------
extern "C"
{
#include "/home/pi/App/ELP-USBFHD06H-L180/Recorder/sdk/v4l2uvc.h"
#include "/home/pi/App/ELP-USBFHD06H-L180/Recorder/sdk/h264_xu_ctrls.h"
#include "/home/pi/App/ELP-USBFHD06H-L180/Recorder/sdk/nalu.h"
#include "/home/pi/App/ELP-USBFHD06H-L180/Recorder/main/USBFHD06H.h"
}

struct v4l2_buffer buf0; 
struct v4l2_capability cap;
struct v4l2_format fmt;
struct v4l2_streamparm parm;
FILE *rec_fp1 = NULL;

USBFHD06H::USBFHD06H()
{
	 
	 rec_filename = "FFH264.h264";	/*"H264.ts"*/

};
	
USBFHD06H::~USBFHD06H()
{


};

int USBFHD06H::setCamSetting(int witdh, int height)
{
	dev = open("/dev/video2", O_RDWR);
	memset(&cap, 0, sizeof cap);
	ret = ioctl(dev, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) 
	{
		std::cout<<"Unable to open camera" << std::endl;
		return 1;
	}

	memset(&fmt, 0, sizeof fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = witdh;//1920;
	fmt.fmt.pix.height = height;//= 1080;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;
	ret = ioctl(dev, VIDIOC_S_FMT, &fmt);
	if (ret < 0) 
	{
		std::cout<<"Unable to save format" << std::endl;
		return 1;
	}

	memset(&parm, 0, sizeof parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(dev, VIDIOC_G_PARM, &parm);
	if (ret < 0) 
	{
		std::cout<<"Unable to get frame rate" << std::endl;
		return 1;
	}
	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = 30;


	std::cout<<"Current frame rate: " <<parm.parm.capture.timeperframe.numerator << "x" << parm.parm.capture.timeperframe.denominator<< std::endl;
	ret = ioctl(dev, VIDIOC_S_PARM, &parm);
	if (ret < 0) 
	{
		std::cout<<"Unable to set frame rate" << std::endl;
		return 1;
	}
	std::cout<<"Frame rate set" << std::endl;


	if ((int)(nbufs = video_reqbufs(dev, nbufs)) < 0) 
	{
			close(dev);		
			return 1;
	}
	
	/* Map the buffers. */
	for (i = 0; i < nbufs; ++i) 
	{
		memset(&buf0, 0, sizeof buf0);
		buf0.index = i;
		buf0.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf0.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(dev, VIDIOC_QUERYBUF, &buf0);
		if (ret < 0) {
			std::cout << "Unable to query buffer" << std::endl;
			close(dev);			
			return 1;
		}
		std::cout << "length - offset: " <<  buf0.length <<" "<< buf0.m.offset << std::endl;

		mem0[i] = mmap(0, buf0.length, PROT_READ, MAP_SHARED, dev, buf0.m.offset);
		if (mem0[i] == MAP_FAILED) {
			std::cout << "Unable to map buffer" << std::endl;
			close(dev);			
			return 1;
		}
		std::cout << "Buffer mapped at address " <<  i <<" "<< mem0[i] << std::endl;
	}

	return 0;

}

int USBFHD06H::saveCam(int file_n,int frame_count)
{
	std::string posfix = std::to_string(file_n);
	char* c = const_cast<char*>(posfix.c_str());
	strcat(c, rec_filename);
	
	/* Queue the buffers. */
	for (i = 0; i < nbufs; ++i) 
	{
		memset(&buf0, 0, sizeof buf0);
		buf0.index = i;
		buf0.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf0.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(dev, VIDIOC_QBUF, &buf0);
		if (ret < 0) 
		{
			std::cout << "Unable to Queue buffer" << std::endl;
			close(dev);			
			return 1;
		}
	} 

	video_enable(dev, 1);
	std::chrono::time_point<std::chrono::system_clock> t_start;
	std::chrono::time_point<std::chrono::system_clock> t_end;
	//for (i = 0; i < nframes; ++i) 
	for (i = 0; i < frame_count; ++i) 
	{
		
		if(i==0)
		{
			t_start = std::chrono::high_resolution_clock::now();
		}
		/* Dequeue a buffer. */
		memset(&buf0, 0, sizeof buf0);
		buf0.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf0.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(dev, VIDIOC_DQBUF, &buf0);
		if (ret < 0) 
		{
			close(dev);
			return 1;
		}
		/* Record the H264 video file */
		if(rec_fp1 == NULL){
			rec_fp1 = fopen(c, "a+b");
		}
		if(rec_fp1 != NULL){
			fwrite(mem0[buf0.index], buf0.bytesused, 1, rec_fp1);
		}


		/* Requeue the buffer. */
		ret = ioctl(dev, VIDIOC_QBUF, &buf0);
		if (ret < 0) {
			close(dev);		
			return 1;
		}
		
		if(i==frame_count-1)
		{
			t_end = std::chrono::high_resolution_clock::now();	
		} 



	}
	video_enable(dev, 0);

	if(rec_fp1 != NULL)
	{
		fclose(rec_fp1);
		rec_fp1 = NULL;
	}	
	auto s_dur = std::chrono::duration_cast<std::chrono::seconds>(t_end-t_start);
	double sec = std::chrono::duration<double>(s_dur).count();
	
	double fps = (i-1)/(sec);
	std::cout << "Captured frames: " << i-1 << "fps: " << fps << std::endl;
	
	
	return 0;

};

int USBFHD06H::closeCapture()
{
	close(dev);
	fflush(stdout);
	return 0;
}

int USBFHD06H::video_reqbufs(int dev, int nbufs)
{
	struct v4l2_requestbuffers rb;
	int ret;

	memset(&rb, 0, sizeof rb);
	rb.count = nbufs;
	rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rb.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(dev, VIDIOC_REQBUFS, &rb);
	if (ret < 0) {
		std::cout<< "Unable to allocate buffers:" << std::endl;
	}

	std::cout << "Buffers allocated" << rb.count << std::endl;
	return rb.count;
}

int USBFHD06H::video_enable(int dev, int enable)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret;

	ret = ioctl(dev, enable ? VIDIOC_STREAMON : VIDIOC_STREAMOFF, &type);
	if (ret < 0) {
		std::cout<< "Unable to capture Enable stat" << enable << std::endl;
		return ret;
	}

	return 0;
}






