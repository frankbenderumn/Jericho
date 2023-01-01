#include "util/encoding.h"
#include "server/defs.h"

int UTF8_to_ascii(unsigned char* out, int *outlen,
              const unsigned char* in, int *inlen) {
    const unsigned char* processed = in;
    const unsigned char* outend;
    const unsigned char* outstart = out;
    const unsigned char* instart = in;
    const unsigned char* inend;
    unsigned int c, d;
    int trailing;

    if (in == NULL) {
        /*
	 * initialization nothing to do
	 */
	*outlen = 0;
	*inlen = 0;
	return(0);
    }
    inend = in + (*inlen);
    outend = out + (*outlen);
    while (in < inend) {
	d = *in++;
	if      (d < 0x80)  { c= d; trailing= 0; }
	else if (d < 0xC0) {
	    /* trailing byte in leading position */
	    *outlen = out - outstart;
	    *inlen = processed - instart;
	    return(-2);
        } else if (d < 0xE0)  { c= d & 0x1F; trailing= 1; }
        else if (d < 0xF0)  { c= d & 0x0F; trailing= 2; }
        else if (d < 0xF8)  { c= d & 0x07; trailing= 3; }
	else {
	    /* no chance for this in Ascii */
	    *outlen = out - outstart;
	    *inlen = processed - instart;
	    return(-2);
	}

	if (inend - in < trailing) {
	    break;
	} 

	for ( ; trailing; trailing--) {
	    if ((in >= inend) || (((d= *in++) & 0xC0) != 0x80))
		break;
	    c <<= 6;
	    c |= d & 0x3F;
	}

	/* assertion: c is a single UTF-4 value */
	if (c < 0x80) {
	    if (out >= outend)
		break;
	    *out++ = c;
	} else {
	    /* no chance for this in Ascii */
	    *outlen = out - outstart;
	    *inlen = processed - instart;
	    return(-2);
	}
	processed = in;
    }
    *outlen = out - outstart;
    *inlen = processed - instart;
    return(0);
}

#include <string>
#include <codecvt>

std::string to_utf8(const std::string& str, const std::locale& loc = std::locale{}) {
//   using wcvt = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>;
//   std::u32string wstr(str.size(), U'\0');
//   std::use_facet<std::ctype<char32_t>>(loc).widen(str.data(), str.data() + str.size(), &wstr[0]);
//   return wcvt{}.to_bytes(wstr.data(),wstr.data() + wstr.size());
	return "does not work on mac";
}

std::string from_utf8(const std::string& str, const std::locale& loc) {
//   using wcvt = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>;
//   auto wstr = wcvt{}.from_bytes(str);
//   std::string result(wstr.size(), '0');
//   std::use_facet<std::ctype<char32_t>>(loc).narrow(wstr.data(), wstr.data() + wstr.size(), '?', &result[0]);
	return "does not work on mac";
}