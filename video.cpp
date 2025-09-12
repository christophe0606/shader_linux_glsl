#include "video.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

static int xioctl(int fd, int request, void *arg)
{
    int r;
    do
    {
        r = ioctl(fd, request, arg);
    } while (r == -1 && errno == EINTR);
    return r;
}

struct buffer
{
    void *start;
    size_t length;
};

static void yuyv_to_rgb24(unsigned char *dst, const unsigned char *src, int width, int height)
{
    // YUYV: pairs of pixels: Y0 U Y1 V
    // Convert to RGB using integer math; clamp to 0..255
    int frame_pixels = width * height;
    for (int i = 0, j = 0; i < frame_pixels; i += 2, j += 4)
    {
        int y0 = src[j + 0] & 0xFF;
        int u = src[j + 1] & 0xFF;
        int y1 = src[j + 2] & 0xFF;
        int v = src[j + 3] & 0xFF;

        int c0 = y0 - 16;
        int c1 = y1 - 16;
        int d = u - 128;
        int e = v - 128;

        int r0 = (298 * c0 + 409 * e + 128) >> 8;
        int g0 = (298 * c0 - 100 * d - 208 * e + 128) >> 8;
        int b0 = (298 * c0 + 516 * d + 128) >> 8;

        int r1 = (298 * c1 + 409 * e + 128) >> 8;
        int g1 = (298 * c1 - 100 * d - 208 * e + 128) >> 8;
        int b1 = (298 * c1 + 516 * d + 128) >> 8;

#define CLAMP(x) ((x) < 0 ? 0 : ((x) > 255 ? 255 : (x)))
        r0 = CLAMP(r0);
        g0 = CLAMP(g0);
        b0 = CLAMP(b0);
        r1 = CLAMP(r1);
        g1 = CLAMP(g1);
        b1 = CLAMP(b1);
#undef CLAMP

        // Write RGB for two pixels
        int p = (i * 3);
        dst[p + 0] = (unsigned char)r0;
        dst[p + 1] = (unsigned char)g0;
        dst[p + 2] = (unsigned char)b0;

        dst[p + 3] = (unsigned char)r1;
        dst[p + 4] = (unsigned char)g1;
        dst[p + 5] = (unsigned char)b1;
    }
}

static struct v4l2_requestbuffers req;
static struct buffer *buffers;

static int fd;

static int make_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int init_video(const char *dev_name, int &width, int &height, enum v4l2_buf_type &type)
{
    fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
    if (fd == -1)
    {
        perror("open video device");
        return 1;
    }

    // 1) Query capabilities

    struct v4l2_capability cap;
    if (xioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
        perror("VIDIOC_QUERYCAP");
        close(fd);
        return 1;
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        fprintf(stderr, "Device does not support video capture\n");
        close(fd);
        return 1;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        fprintf(stderr, "Device does not support streaming i/o\n");
        close(fd);
        return 1;
    }

    // 2) Set format to YUYV
    struct v4l2_format fmt;
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (xioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
    {
        perror("VIDIOC_S_FMT");
        close(fd);
        return 1;
    }
    // The driver may adjust width/height
    width = fmt.fmt.pix.width;
    height = fmt.fmt.pix.height;
    if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV)
    {
        fprintf(stderr, "Device did not accept YUYV; got fourcc %08x\n", fmt.fmt.pix.pixelformat);
        close(fd);
        return 1;
    }

    // 3) Request buffers (MMAP)
    CLEAR(req);
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (xioctl(fd, VIDIOC_REQBUFS, &req) == -1)
    {
        perror("VIDIOC_REQBUFS");
        close(fd);
        return 1;
    }
    if (req.count < 2)
    {
        fprintf(stderr, "Insufficient buffer memory (got %u)\n", req.count);
        close(fd);
        return 1;
    }

    buffers = (buffer *)calloc(req.count, sizeof(*buffers));
    if (!buffers)
    {
        fprintf(stderr, "Out of memory\n");
        close(fd);
        return 1;
    }

    for (unsigned int n = 0; n < req.count; n++)
    {
        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type = req.type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n;

        if (xioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            perror("VIDIOC_QUERYBUF");
            return 1;
        }
        buffers[n].length = buf.length;
        buffers[n].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (buffers[n].start == MAP_FAILED)
        {
            perror("mmap");
            return 1;
        }
    }

    // 4) Queue all buffers
    for (unsigned int n = 0; n < req.count; n++)
    {
        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type = req.type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n;
        if (xioctl(fd, VIDIOC_QBUF, &buf) == -1)
        {
            perror("VIDIOC_QBUF");
            return 1;
        }
    }

    // 5) Start streaming
    type = (enum v4l2_buf_type)req.type;
    if (xioctl(fd, VIDIOC_STREAMON, &type) == -1)
    {
        perror("VIDIOC_STREAMON");
        return 1;
    }

    make_nonblocking(fd);

    return 0;
}

int try_get_buffer(unsigned char *rgb, int width, int height)
{
    // 7) Dequeue one filled buffer
    struct v4l2_buffer buf;
    CLEAR(buf);
    buf.type = req.type;
    buf.memory = V4L2_MEMORY_MMAP;

    if (xioctl(fd, VIDIOC_DQBUF, &buf) == -1)
    {
        if (errno == EAGAIN)
        {
            // No frame available right now
            return 0;
        }
        else if (errno == EINTR)
        {
            return 0;
        }
        else
        {
            perror("VIDIOC_DQBUF");
            return 1;
        }
        return 1;
    }

    // 8) Convert YUYV to RGB and save as PPM

    yuyv_to_rgb24(rgb, (const unsigned char *)buffers[buf.index].start, width, height);

    // 9) Re-queue (optional) and stop streaming
    if (xioctl(fd, VIDIOC_QBUF, &buf) == -1)
    {
        perror("VIDIOC_QBUF (requeue)");
    }

    return 0;
}

void stop_video(enum v4l2_buf_type &type)
{
    if (xioctl(fd, VIDIOC_STREAMOFF, &type) == -1)
    {
        perror("VIDIOC_STREAMOFF");
    }

    // 10) Cleanup
    for (unsigned int n = 0; n < req.count; n++)
    {
        if (buffers[n].start && buffers[n].start != MAP_FAILED)
        {
            munmap(buffers[n].start, buffers[n].length);
        }
    }
    free(buffers);
    close(fd);
}