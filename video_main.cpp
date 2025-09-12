// Minimal V4L2 single-frame grabber (YUYV -> PPM)
// Usage: ./grab_one_v4l2 [device] [width] [height]
// Defaults: device=/dev/video0, width=640, height=480
#include "video.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    const char *dev_name = (argc > 1) ? argv[1] : "/dev/video5";
    int width  = (argc > 2) ? atoi(argv[2]) : 640;
    int height = (argc > 3) ? atoi(argv[3]) : 480;

    enum v4l2_buf_type type;

    size_t rgb_size = (size_t)width * height * 3;
    unsigned char *rgb = (unsigned char*)malloc(rgb_size);

     if (!rgb) {
        fprintf(stderr, "malloc rgb failed\n");
        return 1;
    }

    int err = init_video(dev_name, width, height,type);
    if (err != 0) return err;

    err = wait_for_frame();
    if (err != 0) return err;

    err = get_buffer(rgb,width,height);
    if (err != 0) return err;

   
    FILE *f = fopen("frame.ppm", "wb");
    if (!f) {
        perror("fopen frame.ppm");
        return 1;
    }
    // PPM header
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    if (fwrite(rgb, 1, rgb_size, f) != rgb_size) {
        perror("fwrite");
    }
    fclose(f);
    fprintf(stderr, "Saved frame.ppm (%dx%d)\n", width, height);

    stop_video(type);

    
   
    free(rgb);
    return 0;
}
