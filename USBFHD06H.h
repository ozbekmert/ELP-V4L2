#ifndef USBFHD06H_H_
#define USBFHD06H_H_

#define V4L_BUFFERS_DEFAULT	6//16
#define V4L_BUFFERS_MAX		16//32


class USBFHD06H {
public:
	USBFHD06H();
	~USBFHD06H();
	char* rec_filename;
	void *mem0[V4L_BUFFERS_MAX];
	unsigned int nbufs = V4L_BUFFERS_DEFAULT;
	int dev;
	int freeram;
	int ret;
	int i;
	int video_reqbufs(int dev, int nbufs);
	int video_enable(int dev, int enable);
	int closeCapture();
	int setCamSetting(int witdh, int height);
	int saveCam(int file_n,int frame_count);
};

#endif /* USBFHD06H_H_ */