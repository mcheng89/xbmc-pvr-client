// MD5 code from libmongoose
//
// Copyright (c) 2004-2011 Sergey Lyubka
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "utils.h"
#include "client.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <sys/stat.h>
#include <windows.h>

typedef struct MD5Context {
    uint32_t buf[4];
    uint32_t bits[2];
    unsigned char in[64];
} MD5_CTX;

#if defined(__BYTE_ORDER) && (__BYTE_ORDER == 1234)
#define byteReverse(buf, len) // Do nothing
#else
static void byteReverse(unsigned char *buf, unsigned longs) {
    uint32_t t;
    do {
        t = (uint32_t) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
        ((unsigned) buf[1] << 8 | buf[0]);
        *(uint32_t *) buf = t;
        buf += 4;
    } while (--longs);
}
#endif

#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

#define MD5STEP(f, w, x, y, z, data, s) \
    ( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

// Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
// initialization constants.
static void MD5Init(MD5_CTX *ctx) {
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}
static void MD5Transform(uint32_t buf[4], uint32_t const in[16]) {
    register uint32_t a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}
static void MD5Update(MD5_CTX *ctx, unsigned char const *buf, unsigned len) {
    uint32_t t;

    t = ctx->bits[0];
    if ((ctx->bits[0] = t + ((uint32_t) len << 3)) < t)
        ctx->bits[1]++;
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;

    if (t) {
        unsigned char *p = (unsigned char *) ctx->in + t;

        t = 64 - t;
        if (len < t) {
            memcpy(p, buf, len);
            return;
        }
        memcpy(p, buf, t);
        byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (uint32_t *) ctx->in);
        buf += t;
        len -= t;
    }

    while (len >= 64) {
        memcpy(ctx->in, buf, 64);
        byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (uint32_t *) ctx->in);
        buf += 64;
        len -= 64;
    }

    memcpy(ctx->in, buf, len);
}
static void MD5Final(unsigned char digest[16], MD5_CTX *ctx) {
  unsigned count;
  unsigned char *p;

  count = (ctx->bits[0] >> 3) & 0x3F;

  p = ctx->in + count;
  *p++ = 0x80;
  count = 64 - 1 - count;
  if (count < 8) {
    memset(p, 0, count);
    byteReverse(ctx->in, 16);
    MD5Transform(ctx->buf, (uint32_t *) ctx->in);
    memset(ctx->in, 0, 56);
  } else {
    memset(p, 0, count - 8);
  }
  byteReverse(ctx->in, 14);

  ((uint32_t *) ctx->in)[14] = ctx->bits[0];
  ((uint32_t *) ctx->in)[15] = ctx->bits[1];

  MD5Transform(ctx->buf, (uint32_t *) ctx->in);
  byteReverse((unsigned char *) ctx->buf, 4);
  memcpy(digest, ctx->buf, 16);
  memset((char *) ctx, 0, sizeof(*ctx));
}
// Stringify binary data. Output buffer must be twice as big as input,
// because each byte takes 2 bytes in string representation
static void bin2str(char *to, const unsigned char *p, size_t len) {
  static const char *hex = "0123456789abcdef";

  for (; len--; p++) {
    *to++ = hex[p[0] >> 4];
    *to++ = hex[p[0] & 0x0f];
  }
  *to = '\0';
}
// Return stringified MD5 hash for list of vectors. Buffer must be 33 bytes.
void mg_md5(char *buf, ...) {
    unsigned char hash[16];
    const char *p;
    va_list ap;
    MD5_CTX ctx;

    MD5Init(&ctx);

    va_start(ap, buf);
    while ((p = va_arg(ap, const char *)) != NULL) {
        MD5Update(&ctx, (const unsigned char *) p, (unsigned) strlen(p));
    }
    va_end(ap);

    MD5Final(hash, &ctx);
    bin2str(buf, hash, sizeof(hash));
}

std::string intToString(int x) {
    std::stringstream out;
    out << x;
    return out.str();
}
int hexToInt(std::string str) {
    std::istringstream ss( str );
    int n;
    ss >> std::hex >> n;
    return n;
}
std::string toUpper(const std::string & s)
{
    std::string ret(s.size(), char());
    for(unsigned int i = 0; i < s.size(); ++i)
        ret[i] = (s[i] <= 'z' && s[i] >= 'a') ? s[i]-('a'-'A') : s[i];
    return ret;
}
size_t findIgnoreCase(std::string s1, std::string s2) {
    std::string s1_copy = toUpper(s1), s2_copy = toUpper(s2);
    return s1_copy.find(s2_copy);
}
int createSocket(std::string hostname, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        return 0;
    }
    struct sockaddr_in server_address;
    memset((char *) &server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    struct hostent *host = gethostbyname( hostname.c_str() );
    server_address.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
    if (SOCKET_ERROR == connect(sock, (struct sockaddr *)&server_address, sizeof(server_address))) {
        closesocket(sock);
        return 0;
    }
    return sock;
}
std::string httpRequest(std::string host, int port, std::string path, std::string headers, std::string request) {
    int sock = createSocket(host, port);
    if (sock == 0) return "";

    char sPort[6];
    snprintf(sPort, 6, "%i", port);
    std::string getReqeust = (request==""?"GET ":"POST ")+path+" HTTP/1.1\r\n"
        "Host: "+host+":"+sPort+"\r\n"
        "Connection: close\r\n"+headers+
        (request==""?"":"Content-Length: "+intToString(request.length())+"\r\n")+
        "\r\n"+request;
    send(sock, getReqeust.c_str(), getReqeust.length(), 0);

    int readLen;
    char buffer[1024];
    std::string getHeaders = "";
    std::string getResponse = "";
    while (getHeaders.find("\r\n\r\n")==std::string::npos) {
        readLen = recv(sock, buffer, 1024, 0);
        if (readLen <= 0)
            goto httpGetRequestError;
        getHeaders.append(buffer,readLen);
    }
    getResponse = getHeaders.substr(getHeaders.find("\r\n\r\n")+4);
    getHeaders = getHeaders.substr(0, getHeaders.find("\r\n\r\n")+2);

    if (findIgnoreCase(getHeaders,"Transfer-Encoding: chunked")!=std::string::npos) {
        std::string chunkedData = getResponse;
        int chunkSize = 0;
        getResponse = "";
        for (;;) {
            if (chunkSize==0 && chunkedData.find("\r\n")==std::string::npos) {
                ///wait for next chunk
                readLen = recv(sock, buffer, 1024, 0);
                if (readLen <= 0)
                    goto httpGetRequestError;
                chunkedData.append(buffer,readLen);
            } else if (chunkSize>0) {
                ///still in middle of reading chunk data
                readLen = recv(sock, buffer, (chunkSize>1024?1024:chunkSize), 0);
                if (readLen <= 0)
                    goto httpGetRequestError;
                getResponse.append(buffer,readLen);
                chunkSize -= readLen;
                if (chunkSize == 0)
                    if (recv(sock, buffer, 2, 0)!=2)
                        goto httpGetRequestError;
            } else {
                ///found next chunk
                chunkSize = hexToInt(chunkedData.substr(0,chunkedData.find("\r\n")));
                if (chunkSize == 0) break;
                chunkedData = chunkedData.substr(chunkedData.find("\r\n")+2);
                int appendLen = (chunkedData.length()<chunkSize?chunkedData.length():chunkSize);
                getResponse.append(chunkedData,0,appendLen);
                if (chunkedData.length() == appendLen) {
                    chunkedData = "";
                } else chunkedData = chunkedData.substr(appendLen);
                chunkSize -= appendLen;
                if (chunkSize == 0) {
                    if (chunkedData.length() > 2)
                        chunkedData = chunkedData.substr(2);
                    else {
                        if (recv(sock, buffer, 2-chunkedData.length(), 0)!=2-chunkedData.length())
                            goto httpGetRequestError;
                        chunkedData = "";
                    }
                }
            }
        }
    } else if (findIgnoreCase(getHeaders,"Content-Length: ")!=std::string::npos) {
        int start_pos = findIgnoreCase(getHeaders,"Content-Length: ");
        int end_pos = getHeaders.find("\r\n", start_pos+16);
        int contentLen = atoi(getHeaders.substr(start_pos+16, end_pos-(start_pos+16)).c_str());
        contentLen -= getResponse.length();
        while (contentLen > 0) {
            readLen = recv(sock, buffer, (contentLen>1024?1024:contentLen), 0);
            if (readLen <= 0) {
                closesocket(sock);
                return "";
            }
            getResponse.append(buffer,readLen);
            contentLen -= readLen;
        }
    } else getResponse = "";

    closesocket(sock);
    return getResponse;

    httpGetRequestError:
    closesocket(sock);
    return "";
}

std::string httpDigestHeaders(std::string username, std::string password, std::string method, std::string path) {
    static int nc = 0;
    char nonce[33], nonce_counter[9], cnonce[33], response[33], nonce1[33], nonce2[33];
    //generate nonce, cnonce here...
    snprintf(nonce1, 33, "%i%i%i", rand()%100000, time(0), rand()%100000);
    mg_md5(nonce, nonce1, 0);
    snprintf(nonce1, 33, "%i%i%i", rand()%100000, time(0), rand()%100000);
    mg_md5(cnonce, nonce1, 0);
    //...
    nc = (nc + 1)%100000000;
    snprintf(nonce_counter, 9, "%i", nc);

    mg_md5(nonce1, username.c_str(), ":CetonTV:", password.c_str(), 0);
    mg_md5(nonce2, method.c_str(), ":", path.c_str(), 0);
    mg_md5(response, nonce1, ":", nonce, ":", nonce_counter, ":", cnonce, ":auth:", nonce2, 0);
    return "Authorization: Digest username=\""+username+"\", realm=\"CetonTV\", nonce=\""+nonce+"\", path=\""+path+"\", qop=auth, nc="+nonce_counter+", cnonce=\""+cnonce+"\", response=\""+response+"\", uri=\""+path+"\"\r\n";
}

std::vector<std::string> split(const std::string& s, const std::string& delim, const bool keep_empty) {
    std::vector<std::string> result;
    if (delim.empty()) {
        result.push_back(s);
        return result;
    }
    std::string::const_iterator substart = s.begin(), subend;
    while (true) {
        subend = search(substart, s.end(), delim.begin(), delim.end());
        std::string temp(substart, subend);
        if (keep_empty || !temp.empty()) {
            result.push_back(temp);
        }
        if (subend == s.end()) {
            break;
        }
        substart = subend + delim.size();
    }
    return result;
}
