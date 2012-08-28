/* This is from the BIND 4.9.4 release, modified to fit as a drop-in replacement */

/* Copyright (c) 1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include "snet.hpp"

#define IN6ADDRSZ       16
#define INADDRSZ         4
#define INT16SZ          2

#ifdef WIN32
    #define ERRNO         ((int)GetLastError())
    #define SET_ERRNO(x)  (SetLastError((DWORD)(x)))
    #undef  EAFNOSUPPORT     /* override definition in errno.h */
    #define EAFNOSUPPORT     WSAEAFNOSUPPORT
#else
    #define ERRNO         (errno)
    #define SET_ERRNO(x)  (errno = (x))
#endif

static int      _inet_pton4(const char* src, unsigned char* dst);
static int      _inet_pton6(const char* src, unsigned char* dst);

int snet::inet_pton (int af, const char *src, void *dst)
{
    switch (af)
    {
        case AF_INET:
            return _inet_pton4(src, reinterpret_cast<unsigned char *>(dst));
        case AF_INET6:
            return _inet_pton6(src, reinterpret_cast<unsigned char *>(dst));
        default:
            SET_ERRNO(EAFNOSUPPORT);
            return -1;
    }
}

static int _inet_pton4(const char* src, unsigned char* dst)
{
  static const char digits[] = "0123456789";
  int saw_digit, octets, ch;
  unsigned char tmp[INADDRSZ], *tp;

  saw_digit = 0;
  octets = 0;
  tp = tmp;
  *tp = 0;
  while((ch = *src++) != '\0')
  {
    const char *pch;

    if ((pch = strchr(digits, ch)) != NULL)
    {
      unsigned int val = *tp * 10 + static_cast<unsigned int>(pch - digits);

      if (saw_digit && *tp == 0) return (0);
      if (val > 255) return (0);
      *tp = static_cast<unsigned char>(val);
      if (!saw_digit)
      {
        if (++octets > 4) return (0);
        saw_digit = 1;
      }
    }
    else if(ch == '.' && saw_digit)
    {
      if (octets == 4) return (0);
      *++tp = 0;
      saw_digit = 0;
    }
    else return (0);
  }
  if (octets < 4) return (0);
  memcpy(dst, tmp, INADDRSZ);
  return (1);
}

static int _inet_pton6(const char* src, unsigned char* dst)
{
  static const char xdigits_l[] = "0123456789abcdef", xdigits_u[] = "0123456789ABCDEF";
  unsigned char tmp[IN6ADDRSZ], *tp, *endp, *colonp;
  const char *xdigits, *curtok;
  int ch, saw_xdigit;
  size_t val;

  memset((tp = tmp), 0, IN6ADDRSZ);
  endp = tp + IN6ADDRSZ;
  colonp = NULL;
  /* Leading :: requires some special handling. */
  if ((*src == ':') && (*++src != ':')) return 0;
  curtok = src;
  saw_xdigit = 0;
  val = 0;
  while((ch = *src++) != '\0')
  {
    const char *pch;

    if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL) pch = strchr((xdigits = xdigits_u), ch);
    if (pch != NULL)
    {
      val <<= 4;
      val |= (pch - xdigits);
      if (++saw_xdigit > 4) return (0);
      continue;
    }
    if (ch == ':')
    {
      curtok = src;
      if (!saw_xdigit)
      {
        if (colonp) return (0);
        colonp = tp;
        continue;
      }
      if (tp + INT16SZ > endp) return (0);
      *tp++ = static_cast<unsigned char>((val >> 8) & 0xff);
      *tp++ = static_cast<unsigned char>(val & 0xff);
      saw_xdigit = 0;
      val = 0;
      continue;
    }
    if (ch == '.' && ((tp + INADDRSZ) <= endp) && _inet_pton4(curtok, tp) > 0)
    {
      tp += INADDRSZ;
      saw_xdigit = 0;
      break;    /* '\0' was seen by inet_pton4(). */
    }
    return (0);
  }
  if (saw_xdigit) {
    if (tp + INT16SZ > endp) return (0);
    *tp++ = static_cast<unsigned char>((val >> 8) & 0xff);
    *tp++ = static_cast<unsigned char>(val & 0xff);
  }
  if (colonp != NULL)
  {
    /*
     * Since some memmove()'s erroneously fail to handle
     * overlapping regions, we'll do the shift by hand.
     */
    const ssize_t n = tp - colonp;
    ssize_t i;

    if (tp == endp) return (0);
    for(i = 1; i <= n; i++)
    {
      *(endp - i) = *(colonp + n - i);
      *(colonp + n - i) = 0;
    }
    tp = endp;
  }
  if (tp != endp) return (0);
  memcpy(dst, tmp, IN6ADDRSZ);
  return (1);
}
