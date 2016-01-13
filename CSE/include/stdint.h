#ifndef _STD_INT_H_
#define _STD_INT_H_

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;
typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed long    int32_t;

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

#define BOOL_CHECK(flag) \
	if (!(flag)) {                 \
		return false;              \
	}

#endif