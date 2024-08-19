
#ifndef SRC_POLYFILL_BYTESWAP_HPP_
#define SRC_POLYFILL_BYTESWAP_HPP_
#include <windows.h>
#ifdef _WIN32
#if __BYTE_ORDER == __LITTLE_ENDIAN

inline unsigned short htons(unsigned short value) {
  return _byteswap_ushort(value);
}

inline unsigned short ntohs(unsigned short value) {
  return _byteswap_ushort(value);
}

inline unsigned int htonl(unsigned int value) {
  return _byteswap_ulong(value); // "long" is always 32-bit on Windows (lol)
}

inline unsigned int ntohl(unsigned int value) { return _byteswap_ulong(value); }

#else
inline unsigned short htons(unsigned short value) { return value; }
inline unsigned short ntohs(unsigned int value) { return value; }
inline unsigned short htonl(unsigned short value) { return value; }
inline unsigned short ntohl(unsigned int value) { return value; }
#endif
#else
#include <netinet/in.h>
#endif

#endif
