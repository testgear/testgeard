/*
 * Framebuffer test plugin for Test Gear
 *
 * Copyright (c) 2012-2014, Martin Lund
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "testgear/plugin.h"

#define BIT8_RED   0xE0
#define BIT8_GREEN 0x1C
#define BIT8_BLUE  0x03

#define BIT16_RED   0xF800
#define BIT16_GREEN 0x07E0
#define BIT16_BLUE  0x001F

#define BIT24_RED   0xFF0000
#define BIT24_GREEN 0x00FF00
#define BIT24_BLUE  0x0000FF

#define BIT32_RED   0xFF0000
#define BIT32_GREEN 0x00FF00
#define BIT32_BLUE  0x0000FF

#define BLACK 0x000000
#define WHITE 0xFFFFFF

unsigned int red = BIT32_RED;
unsigned int green = BIT32_GREEN;
unsigned int blue = BIT32_BLUE;

// Framebuffer configuration data
struct fb
{
    int fd;
    void *mem;
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    struct fb_var_screeninfo vinfo_backup;
    unsigned long screensize;
} fb;

static int open_device(char *device_name)
{
    // Open framebuffer device
    fb.fd = open(device_name, O_RDWR);
    if (!fb.fd)
    {
        printf("Error opening framebuffer device %s (%s)\n", device_name, strerror(errno));
        return 1;
    }

    // Get variable screen information
    if (ioctl(fb.fd, FBIOGET_VSCREENINFO, &fb.vinfo))
    {
        printf("Error reading variable screen information from %s (%s)\n", device_name, strerror(errno));
        return 1;
    }

    // Store variable screen information (for later restore)
    memcpy(&fb.vinfo_backup, &fb.vinfo, sizeof(struct fb_var_screeninfo));

    // Get fixed screen information
    if (ioctl(fb.fd, FBIOGET_FSCREENINFO, &fb.finfo))
    {
        printf("Error reading fixed screen information from %s (%s)\n", device_name, strerror(errno));
        return 1;
    }

    // Map framebuffer to user memory
    fb.screensize = fb.finfo.smem_len;
    fb.mem = mmap(0, fb.screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb.fd, 0);
    if (fb.mem == MAP_FAILED)
    {
        printf("Failed to map framebuffer memory\n");
        return 1;
    }

    // Disable blinking cursor
    system("echo 0 > /sys/class/graphics/fbcon/cursor_blink");

    return 0;
}

static int close_device(void)
{
    // Unmap framebuffer memory
    munmap(fb.mem, fb.screensize);

    // Restore variable screen information
    if (ioctl(fb.fd, FBIOPUT_VSCREENINFO, &fb.vinfo_backup))
    {
        printf("Error restoring variable screen information\n");
        return 1;
    }

    // Close framebuffer device
    close(fb.fd);

    return 0;
}

static void put_pixel(struct fb *fb, int x, int y, unsigned int color)
{
    void *fbmem = fb->mem;

    if (fb->vinfo.bits_per_pixel == 8)
    {
        // 8 bit
        static unsigned char *pixel;

        fbmem += fb->finfo.line_length * y;
        pixel = fbmem;
        pixel += x;
        *pixel = color;
    }
    else if (fb->vinfo.bits_per_pixel == 16)
    {
        // 16 bit
        static unsigned short *pixel;

        fbmem += fb->finfo.line_length * y;
        pixel = fbmem;
        pixel += x;
        *pixel = color;
    }
    else
    {
        // 24/32 bit
        static unsigned int *pixel;

        fbmem += fb->finfo.line_length * y;
        pixel = fbmem;
        pixel += x;
        *pixel = color;
    }
}

static void put_solid(struct fb *fb, unsigned int color)
{
    int x, y;

    for (y = 0; y < (fb->vinfo.yres); y++)
    {
        for (x = 0; x < fb->vinfo.xres; x++)
            put_pixel(fb, x, y, color);
    }
}

static void put_pattern(struct fb *fb, int pattern)
{
    switch (pattern)
    {
        case 0:
            // Solid red
            put_solid(fb, BIT32_RED);
            break;
        case 1:
            // Solid green
            put_solid(fb, BIT32_GREEN);
            break;
        case 2:
            // Solid blue
            put_solid(fb, BIT32_BLUE);
            break;
        case 3:
            // Solid white
            put_solid(fb, WHITE);
            break;
        case 4:
            // Solid black
            put_solid(fb, BLACK);
            break;
        default:
            break;
    }
}

/*
 * Preliminary Test Gear plugin model
 *
 * All commands and variables are name based.
 *
 * Framebuffer test plugin example
 *
 * Example client command line usage:
 * testgearctl connect 192.168.0.42 pandaboard
 * testgearctl load fb
 * testgearctl set fb.device "/dev/fb0"
 * testgearctl set fb.xres=640
 * testgearctl set fb.yres=640
 * testgearctl run fb.set_resolution
 * testgearctl set fb.pattern=1
 * testgearctl run fb.draw_pattern
 * testgearctl set fb.filename="/images/640x480-color-pattern.png"
 * testgearctl run fb.show_image
 * testgearctl disconnect pandaboard
 *
 * Same as above but short hand example:
 * testgearctl connect 192.168.0.42 pandaboard
 * testgearctl load fb
 * testgearctl set fb.device "/dev/fb0"
 * testgearctl run fb.set_resolution (xres=640 yres=480)
 * testgearctl run fb.set_depth (depth=32)
 * testgearctl run fb.show_image (filename="/images/640x480-color-pattern.png")
 * testgearctl disconnect pandaboard
 *
 * Useful functions:
 * testgearctl describe fb_test       (returns plugin description + variables)
 * testgearctl describe fb_test.xres  (returns variable description)
 * testgearctl describe fb_test.set_resolution (returns command description)
 *
 */

static int fb_load(void)
{
    // Set defaults
    set_string("device", "/dev/fb0");
    set_int("depth", 32);
    set_int("xres", 640);
    set_int("yres", 480);
    set_int("xoffset", 0);
    set_int("yoffset", 0);
    set_int("pattern", 1);
    set_string("filename", "/tmp/test-pattern.png");

    return 0;
}

static int fb_unload(void)
{
    // Action on unload
    printf(" xres=%d\n", get_int("xres"));
    printf(" yres=%d\n", get_int("yres"));
    printf(" filename=%s\n", get_string("filename"));
    printf(" name=%s\n", get_string("name"));
    printf(" version=%s\n", get_string("version"));
    printf(" description=%s\n", get_string("description"));
    printf(" author=%s\n", get_string("author"));
    printf(" license=%s\n", get_string("license"));

    return 0;
}

static int fb_set_resolution(void)
{
    int status;
    char *device = get_string("device");
    int xres = get_int("xres");
    int yres = get_int("yres");

    // Open fb device
    status = open_device(device);
    if (status != 0)
        return status;

    fb.vinfo.xres = xres;
    fb.vinfo.yres = yres;

    // Set variable screen information
    if (ioctl(fb.fd, FBIOPUT_VSCREENINFO, &fb.vinfo))
    {
        printf("Error setting variable screen information in %s (%s)\n", device, strerror(errno));
        return 1;
    }

    // Get fixed screen information
    if (ioctl(fb.fd, FBIOGET_FSCREENINFO, &fb.finfo))
    {
        printf("Error reading fixed screen information from %s (%s)\n", device, strerror(errno));
        return 1;
    }

    printf("Resolution %dx%d, depth %d bpp\n",
            fb.vinfo.xres, fb.vinfo.yres,
            fb.vinfo.bits_per_pixel);

    // Close device
    status = close_device();
    if (status != 0)
        return status;

    return 0;
}

static int fb_set_depth(void)
{
    int status;
    char *device = get_string("device");
    int depth = get_int("depth");

    // Open fb device
    status = open_device(device);
    if (status != 0)
        return status;

    switch (depth)
    {
        case 8:
        case 16:
        case 24:
        case 32:
            break;
        default:
            printf("Error: depth must be 8, 16, 24 or 32\n");
            return 1;
    }

    // Set color depth
    fb.vinfo.bits_per_pixel = depth;

    // Set color encoding
    switch (fb.vinfo.bits_per_pixel)
    {
        case 8:
            // RGB332
            fb.vinfo.red.offset    = 0;
            fb.vinfo.red.length    = 3;
            fb.vinfo.green.offset  = 3;
            fb.vinfo.green.length  = 3;
            fb.vinfo.blue.offset   = 6;
            fb.vinfo.blue.length   = 2;
            fb.vinfo.transp.offset = 0;
            fb.vinfo.transp.length = 0;
            break;
        case 16: // RGB 565
            fb.vinfo.red.offset    = 0;
            fb.vinfo.red.length    = 5;
            fb.vinfo.green.offset  = 5;
            fb.vinfo.green.length  = 6;
            fb.vinfo.blue.offset   = 11;
            fb.vinfo.blue.length   = 5;
            fb.vinfo.transp.offset = 0;
            fb.vinfo.transp.length = 0;
            break;
        case 24: // RGB 888
            fb.vinfo.red.offset    = 0;
            fb.vinfo.red.length    = 8;
            fb.vinfo.green.offset  = 8;
            fb.vinfo.green.length  = 8;
            fb.vinfo.blue.offset   = 16;
            fb.vinfo.blue.length   = 8;
            fb.vinfo.transp.offset = 0;
            fb.vinfo.transp.length = 0;
            break;
        case 32: // RGBA 8888
            fb.vinfo.red.offset    = 0;
            fb.vinfo.red.length    = 8;
            fb.vinfo.green.offset  = 8;
            fb.vinfo.green.length  = 8;
            fb.vinfo.blue.offset   = 16;
            fb.vinfo.blue.length   = 8;
            fb.vinfo.transp.offset = 24;
            fb.vinfo.transp.length = 8;
            break;
    }

    // Set variable screen information
    if (ioctl(fb.fd, FBIOPUT_VSCREENINFO, &fb.vinfo))
    {
        printf("Error setting variable screen information in %s (%s)\n", device, strerror(errno));
        return 1;
    }

    // Close device
    status = close_device();
    if (status != 0)
        return status;

    return 0;
}

static int fb_draw_pattern(void)
{
    int status;
    char *device = get_string("device");
    int pattern = get_int("pattern");

    // Open fb device
    status = open_device(device);
    if (status != 0)
        return status;

    // Draw pattern
    put_pattern(&fb, pattern);

    // Close device
    status = close_device();
    if (status != 0)
        return status;

    return 0;
}

static int fb_show_image(void)
{
    int status;
    char *device = get_string("device");
    char *filename = get_string("filename");

    // Open fb device
    status = open_device(device);
    if (status != 0)
        return status;

    // Show image

    // Close device
    status = close_device();
    if (status != 0)
        return status;

    return 0;
}

// Plugin properties
static struct plugin_properties fb_properties[] =
{
    {   .name = "device",
        .type = STRING,
        .description = "Name of device" },

    {   .name = "xres",
        .type = INT,
        .description = "Horizontal resolution [pixels]" },

    {   .name = "yres",
        .type = INT,
        .description = "Vertical resolution [pixels]" },

    {   .name = "depth",
        .type = INT,
        .description = "Color depth" },

    {   .name = "xoffset",
        .type = INT,
        .description = "Horizontal offset [pixels]" },

    {   .name = "yoffset",
        .type = INT,
        .description = "Vertical offset [pixels]" },

    {   .name = "pattern",
        .type = INT,
        .description = "Drawing pattern" },

    {   .name = "filename",
        .type = STRING,
        .description = "Image filename" },

    {   .name = "set_resolution",
        .type = COMMAND,
        .function = fb_set_resolution,
        .description = "Set resolution command" },

    {   .name = "set_depth",
        .type = COMMAND,
        .function = fb_set_depth,
        .description = "Set depth command" },

    {   .name = "draw_pattern",
        .type = COMMAND,
        .function = fb_draw_pattern,
        .description = "Fill screen with pattern" },

    {   .name = "show_image",
        .type = COMMAND,
        .function = fb_show_image,
        .description = "Show image on screen" },

    { }
};

// Plugin configuration
struct plugin fb_plugin = {
    .name = "fb",
    .version = "0.2",
    .description = "Framebuffer test plugin",
    .author = "Martin Lund",
    .license = "BSD-3",
    .load = fb_load,
    .unload = fb_unload,
    .properties = fb_properties,
};

plugin_register(fb_plugin);
