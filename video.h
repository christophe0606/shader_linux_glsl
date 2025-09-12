#ifndef video_h
#define video_h

#include <linux/videodev2.h>

extern int init_video(const char* dev_name, 
                      int &width, 
                      int &height,
                      enum v4l2_buf_type &type);

extern int try_get_buffer(unsigned char *rgb,int width, int height);
extern void stop_video(enum v4l2_buf_type &type);

#endif