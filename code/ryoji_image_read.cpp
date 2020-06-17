#ifndef __RYOJI_IMAGE_READ__
#define __RYOJI_IMAGE_READ__

#define MapTo16(buffer, index) (buffer[index] | (buffer[index+1] << 8))
#define MapTo32(buffer, index) (buffer[index] | (buffer[index+1] << 8) |  (buffer[index+2] << 16) | (buffer[index+3] << 24)) 
// NOTE(Momo): Only 32 bit BMP version 5






#undef MapTo16
#undef MapTo32
#endif