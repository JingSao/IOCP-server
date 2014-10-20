#ifndef _IOCP_SOCKET_MACROS_H_
#define _IOCP_SOCKET_MACROS_H_

#include <stdint.h>

#define MAKE_BODY_SIZE(a0, a1, a2, a3) ((((uint32_t)(uint8_t)(a0)) << 24) | ((uint32_t)(uint8_t)(a1) << 16) | ((uint32_t)(uint8_t)(a2) << 8) | ((uint32_t)(uint8_t)(a3)))
#define BODY_SIZE_GET0(s) (uint8_t)(((s) >> 24) & 0xFF)
#define BODY_SIZE_GET1(s) (uint8_t)(((s) >> 16) & 0xFF)
#define BODY_SIZE_GET2(s) (uint8_t)(((s) >> 8) & 0xFF)
#define BODY_SIZE_GET3(s) (uint8_t)((s) & 0xFF)
#define BODY_SIZE_GET(s, n) (uint8_t)(((s) >> (((uint32_t)(3 - (n))) << 3)) & 0xFF)

#endif
