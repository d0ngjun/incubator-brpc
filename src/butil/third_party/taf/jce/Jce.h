#ifndef __JCE_H__
#define __JCE_H__

#include <netinet/in.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>



//支持iphone
#ifdef __APPLE__
#include "JceType.h"
#elif defined ANDROID  // android
#include "JceType.h"
#else
#include "JceType.h"
#endif


#ifndef jce_likely
#if defined(__GNUC__) && __GNUC__ >= 4
#define jce_likely(x)			(__builtin_expect(!!(x),1))
#else
#define jce_likely(x)   (x)
#endif
#endif

#ifndef jce_unlikely
#if defined(__GNUC__) && __GNUC__ >= 4
#define jce_unlikely(x)			(__builtin_expect(!!(x),0))
#else
#define jce_unlikely(x)   (x)
#endif
#endif

//数据头类型
#define TafHeadeChar  0
#define TafHeadeShort 1
#define TafHeadeInt32 2
#define TafHeadeInt64 3
#define TafHeadeFloat 4
#define TafHeadeDouble 5
#define TafHeadeString1 6
#define TafHeadeString4 7
#define TafHeadeMap 8
#define TafHeadeList 9
#define TafHeadeStructBegin 10
#define TafHeadeStructEnd 11
#define TafHeadeZeroTag 12
#define TafHeadeSimpleList 13


//////////////////////////////////////////////////////////////////
//// 保留接口版本Jce宏定义
//编码相应的宏
#define TafReserveBuf(os, len) \
do{ \
	if((os)._reverse) \
	{ \
		if(jce_unlikely((os)._buf_len < (len))) \
		{ \
			size_t len1 = (len)<<1; \
			char * p = new char[(len1)]; \
			memcpy(p, (os)._buf, (os)._len); \
			delete[] (os)._buf; \
			(os)._buf = p; \
			(os)._buf_len = (len1); \
		} \
	} \
}while(0)

#define TafWriteToHead(os, type, tag) \
do { \
	if (jce_likely((tag) < 15)) \
	{ \
		TafWriteUInt8TTypeBuf( os, (type) + ((tag)<<4) , (os)._len); \
	} \
	else \
	{ \
		TafWriteUInt8TTypeBuf( os, (type) + (240) , (os)._len); \
		TafWriteUInt8TTypeBuf( os, (tag),  (os)._len);\
	} \
} while(0)

#define TafWriteCharTypeBuf(os, val, osLen) \
do { \
	TafReserveBuf(os, (osLen)+sizeof(Char)); \
	(*(Char *)((os)._buf + (osLen))) = (val); \
	(osLen) += sizeof(Char); \
} while(0)

#define TafWriteInt32TypeBuf(os, val, osLen) \
do { \
	TafReserveBuf(os, (osLen)+sizeof(Int32)); \
	(*(Int32 *)((os)._buf + (osLen))) = (val); \
	(osLen) += sizeof(Int32); \
} while(0)

#define TafWriteInt64TypeBuf(os, val, osLen) \
do { \
	TafReserveBuf(os, (osLen)+sizeof(Int64)); \
	(*(Int64 *)((os)._buf + (osLen))) = (val); \
	(osLen) += sizeof(Int64); \
} while(0)

#define TafWriteFloatTypeBuf(os, val, osLen) \
do { \
	TafReserveBuf(os, (osLen)+sizeof(Float)); \
	(*(Float *)((os)._buf + (osLen))) = (val); \
	(osLen) += sizeof(Float); \
} while(0)

#define TafWriteDoubleTypeBuf(os, val, osLen) \
do { \
	TafReserveBuf(os, (osLen)+sizeof(Double)); \
	(*(Double *)((os)._buf + (osLen))) = (val); \
	(osLen) += sizeof(Double); \
} while(0)

#define TafWriteUInt32TTypeBuf(os, val, osLen) \
do { \
	TafReserveBuf(os, (osLen)+sizeof(uint32_t)); \
	(*(uint32_t *)((os)._buf + (osLen))) = (val); \
	(osLen) += sizeof(uint32_t); \
} while(0)

#define TafWriteUInt8TTypeBuf(os, val, osLen) \
do { \
	TafReserveBuf(os, (osLen)+sizeof(uint8_t)); \
	(*(uint8_t *)((os)._buf + (osLen))) = (val); \
	(osLen) += sizeof(uint8_t); \
} while(0)

#define TafWriteUIntTypeBuf(os, val, osLen) \
do { \
	TafReserveBuf(os, (osLen)+sizeof(unsigned int)); \
	(*(unsigned int *)((os)._buf + (osLen))) = (val); \
	(osLen) += sizeof(unsigned int); \
} while(0)

#define TafWriteShortTypeBuf(os, val, osLen) \
do { \
	TafReserveBuf(os, (osLen)+sizeof(Short)); \
	(*(Short *)((os)._buf + (osLen))) = (val); \
	(osLen) += sizeof(Short); \
} while(0)

#define TafWriteTypeBuf(os, buf, len) \
do { \
	TafReserveBuf(os, (os)._len + (len)); \
	memcpy((os)._buf + (os)._len, (const void *)(buf), (len)); \
	(os)._len += (len); \
} while(0)


//解码相应的宏
#define TafPeekTypeBuf(is, buf, offset, type) \
do { \
	if(jce_unlikely((is)._cur+(offset)+sizeof(type)>(is)._buf_len) )\
	{ \
		char s[64]; \
		snprintf(s, sizeof(s), "buffer overflow when peekBuf, over %u.", (uint32_t)((is)._buf_len)); \
		throw JceDecodeException(s); \
	} \
	(buf) = (*((type *)((is)._buf+(is)._cur+(offset)))); \
} while(0)

#define TafPeekTypeBufNoTag(is, offset, type) \
do { \
	if(jce_unlikely((is)._cur+(offset)+sizeof(type)>(is)._buf_len) )\
	{ \
		char s[64]; \
		snprintf(s, sizeof(s), "buffer overflow when peekBuf, over %u.", (uint32_t)((is)._buf_len)); \
		throw JceDecodeException(s); \
	} \
} while(0)

#define TafReadCharTypeBuf(is, buf) \
do { \
	TafPeekTypeBuf(is, buf, 0, Char); \
	(is)._cur += sizeof(Char); \
} while(0)

#define TafReadShortTypeBuf(is, buf) \
do { \
	TafPeekTypeBuf(is, buf, 0, Short); \
	(is)._cur += sizeof(Short); \
} while(0)

#define TafReadInt32TypeBuf(is, buf) \
do { \
	TafPeekTypeBuf(is, buf, 0, Int32); \
	(is)._cur += sizeof(Int32); \
} while(0)

#define TafReadInt64TypeBuf(is, buf) \
do { \
	TafPeekTypeBuf(is, buf, 0, Int64); \
	(is)._cur += sizeof(Int64); \
} while(0)

#define TafReadFloatTypeBuf(is, buf) \
do { \
	TafPeekTypeBuf(is, buf, 0, Float); \
	(is)._cur += sizeof(Float); \
} while(0)

#define TafReadDoubleTypeBuf(is, buf) \
do { \
	TafPeekTypeBuf(is, buf, 0, Double); \
	(is)._cur += sizeof(Double); \
} while(0)

#define TafReadTypeBuf(is, buf, type) \
do { \
	TafPeekTypeBuf(is, buf, 0, type); \
	(is)._cur += sizeof(type); \
} while(0)

#define TafReadHeadSkip(is, len) \
do {\
 	(is)._cur += (len); \
} while(0)

#define TafPeekFromHead(is, type, tag, n) \
do { \
	(n) = 1; \
	uint8_t typeTag, tmpTag; \
	TafPeekTypeBuf(is, typeTag, 0, uint8_t); \
	tmpTag = typeTag >> 4; \
	(type) = (typeTag & 0x0F); \
	if(jce_unlikely(tmpTag == 15)) \
	{ \
		TafPeekTypeBuf(is, tag, 1, uint8_t); \
		(n) += 1; \
	} \
	else \
	{ \
		(tag) = tmpTag; \
	} \
} while(0)

#define readFromHead(is, type ,tag) \
do { \
	size_t n = 0; \
	TafPeekFromHead(is, type, tag, n); \
	TafReadHeadSkip(is, n); \
} while(0)

#define TafPeekFromHeadNoTag(is, type, n) \
do { \
	(n) = 1; \
	uint8_t typeTag, tmpTag; \
	TafPeekTypeBuf(is, typeTag, 0, uint8_t); \
	tmpTag = typeTag >> 4; \
	(type) = (typeTag & 0x0F); \
	if(jce_unlikely(tmpTag == 15)) \
	{ \
		TafPeekTypeBufNoTag(is, 1, uint8_t); \
		(n) += 1; \
	} \
} while(0)

#define readFromHeadNoTag(is, type) \
do { \
	size_t n = 0; \
	TafPeekFromHeadNoTag(is, type, n); \
	TafReadHeadSkip(is, n); \
} while(0)

#define TafPeekBuf(is ,buf, len, offset) \
do {\
	if (jce_unlikely((is)._cur + (offset) + (len) > (is)._buf_len)) \
	{ \
		char s[64]; \
		snprintf(s, sizeof(s), "buffer overflow when peekBuf, over %u.", (uint32_t)((is)._buf_len)); \
		throw JceDecodeException(s); \
	} \
	::memcpy(buf, (is)._buf + (is)._cur + (offset), (len)); \
} while(0)

#define TafReadBuf(is, buf, len) \
do { \
		TafPeekBuf(is, buf, len, 0); \
	    (is)._cur += (len); \
} while(0)

#define TafReadStringBuf(is, str, len) \
do{\
	if (jce_unlikely((is)._cur + (len) > (is)._buf_len)) \
	{ \
		char s[64]; \
		snprintf(s, sizeof(s), "buffer overflow when peekBuf, over %u.", (uint32_t)((is)._buf_len)); \
		throw JceDecodeException(s); \
	} \
	str.assign((is)._buf + (is)._cur, (is)._buf + (is)._cur + (len)); \
	(is)._cur += len; \
} while (0)

#define TafSkipToTag(flag, tag, retHeadType, retHeadTag) \
do { \
	try \
	{ \
		uint8_t nextHeadType, nextHeadTag; \
		while (!ReaderT::hasEnd()) \
		{ \
			size_t len = 0; \
			TafPeekFromHead(*this, nextHeadType, nextHeadTag, len); \
			if (jce_unlikely(nextHeadType == TafHeadeStructEnd || tag < nextHeadTag)) \
			{ \
				break; \
			} \
			if (tag == nextHeadTag) \
			{ \
				(retHeadType) = nextHeadType; \
				(retHeadTag) = nextHeadTag; \
				TafReadHeadSkip(*this, len); \
				(flag) = true; \
				break; \
			} \
			TafReadHeadSkip(*this, len); \
			skipField(nextHeadType); \
		} \
	} \
	catch (JceDecodeException& e) \
	{ \
	} \
} while(0)

namespace taf
{
//////////////////////////////////////////////////////////////////
struct JceStructBase {
protected:
    JceStructBase() {}

    ~JceStructBase() {}
};

struct JceException : public std::runtime_error {
    JceException(const std::string &s) : std::runtime_error(s) {}
};

struct JceEncodeException : public JceException {
    JceEncodeException(const std::string &s) : JceException(s) {}
};

struct JceDecodeException : public JceException {
    JceDecodeException(const std::string &s) : JceException(s) {}
};

struct JceDecodeMismatch : public JceDecodeException {
    JceDecodeMismatch(const std::string &s) : JceDecodeException(s) {}
};

struct JceDecodeRequireNotExist : public JceDecodeException {
    JceDecodeRequireNotExist(const std::string &s) : JceDecodeException(s) {}
};

struct JceDecodeInvalidValue : public JceDecodeException {
    JceDecodeInvalidValue(const std::string &s) : JceDecodeException(s) {}
};

struct JceNotEnoughBuff : public JceException {
    JceNotEnoughBuff(const std::string &s) : JceException(s) {}
};

//////////////////////////////////////////////////////////////////
namespace
{
/// 数据头信息的封装，包括类型和tag
class DataHead
{
    uint8_t _type;
    uint8_t _tag;
public:
    enum {
        eChar = 0,
        eShort = 1,
        eInt32 = 2,
        eInt64 = 3,
        eFloat = 4,
        eDouble = 5,
        eString1 = 6,
        eString4 = 7,
        eMap = 8,
        eList = 9,
        eStructBegin = 10,
        eStructEnd = 11,
        eZeroTag = 12,
        eSimpleList = 13,
    };

    struct helper {
        uint8_t     type : 4;
        uint8_t     tag  : 4;
    } __attribute__((packed));

public:
    DataHead() : _type(0), _tag(0) {}
    DataHead(uint8_t type, uint8_t tag) : _type(type), _tag(tag) {}

    uint8_t getTag() const
    {
        return _tag;
    }
    void setTag(uint8_t t)
    {
        _tag = t;
    }
    uint8_t getType() const
    {
        return _type;
    }
    void setType(uint8_t t)
    {
        _type = t;
    }

    /// 读取数据头信息
    template<typename InputStreamT>
    void readFrom(InputStreamT &is)
    {
        size_t n = peekFrom(is);
        is.skip(n);
    }

    /// 读取头信息，但不前移流的偏移量
    template<typename InputStreamT>
    size_t peekFrom(InputStreamT &is)
    {
        helper h;
        size_t n = sizeof(h);
        is.peekBuf(&h, sizeof(h));
        _type = h.type;

        if (h.tag == 15) {
            is.peekBuf(&_tag, sizeof(_tag), sizeof(h));
            n += sizeof(_tag);
        } else {
            _tag = h.tag;
        }

        return n;
    }

    /// 写入数据头信息
    template<typename OutputStreamT>
    void writeTo(OutputStreamT &os)
    {
        /*
        helper h;
        h.type = _type;
        if(_tag < 15){
            h.tag = _tag;
            os.writeBuf(&h, sizeof(h));
        }else{
            h.tag = 15;
            os.writeBuf(&h, sizeof(h));
            os.writeBuf(&_tag, sizeof(_tag));
        }
        */
        writeTo(os, _type, _tag);
    }

    /// 写入数据头信息
    template<typename OutputStreamT>
    static void writeTo(OutputStreamT &os, uint8_t type, uint8_t tag)
    {
        helper h;
        h.type = type;

        if (tag < 15) {
            h.tag = tag;
            os.writeBuf(&h, sizeof(h));
        } else {
            h.tag = 15;
            os.writeBuf(&h, sizeof(h));
            os.writeBuf(&tag, sizeof(tag));
        }
    }
};
}


//////////////////////////////////////////////////////////////////
/// 缓冲区读取器封装
class BufferReader
{
public:
    const char         *_buf;        ///< 缓冲区
    size_t              _buf_len;    ///< 缓冲区长度
    size_t              _cur;        ///< 当前位置

public:

    BufferReader() : _buf(NULL), _buf_len(0), _cur(0) {}

    void reset()
    {
        _cur = 0;
    }

    /// 读取缓存
    void readBuf(void *buf, size_t len)
    {
        if (len <= _buf_len && (_cur + len) <= _buf_len) {
            peekBuf(buf, len);
            _cur += len;
        } else {
            char s[64];
            snprintf(s, sizeof(s), "buffer overflow when skip, over %u.", (uint32_t)_buf_len);
            throw JceDecodeException(s);
        }
    }

    /// 针对读取vector<char>的优化
    template<typename Alloc>
    void readBuf(std::vector<Char, Alloc> &v, size_t len)
    {
        if (len <= _buf_len && (_cur + len) <= _buf_len) {
            peekBuf(v, len);
            _cur += len;
        } else {
            char s[64];
            snprintf(s, sizeof(s), "buffer overflow when skip, over %u.", (uint32_t)_buf_len);
            throw JceDecodeException(s);
        }
    }
    /// 针对读取vector<char>的优化
    /// 读取缓存，但不改变偏移量
    template<typename Alloc>
    void peekBuf(std::vector<Char, Alloc> &v, size_t len, size_t offset = 0)
    {
        if (_cur + offset + len > _buf_len) {
            char s[64];
            snprintf(s, sizeof(s), "buffer overflow when peekBuf, over %u.", (uint32_t)_buf_len);
            throw JceDecodeException(s);
        }

        const char *begin = _buf + _cur + offset;
        v.assign(begin, begin + len);
    }

    /// 读取缓存，但不改变偏移量
    void peekBuf(void *buf, size_t len, size_t offset = 0)
    {
        if (_cur + offset + len > _buf_len) {
            char s[64];
            snprintf(s, sizeof(s), "buffer overflow when peekBuf, over %u.", (uint32_t)_buf_len);
            throw JceDecodeException(s);
        }

        ::memcpy(buf, _buf + _cur + offset, len);
    }

    /// 跳过len个字节
    void skip(size_t len)
    {
        if (len <= _buf_len && (_cur + len) <= _buf_len) {
            _cur += len;
        } else {
            char s[64];
            snprintf(s, sizeof(s), "buffer overflow when skip, over %u.", (uint32_t)_buf_len);
            throw JceDecodeException(s);
        }
    }

    /// 设置缓存
    void setBuffer(const char *buf, size_t len)
    {
        _buf = buf;
        _buf_len = len;
        _cur = 0;
    }

    /// 设置缓存
    template<typename Alloc>
    void setBuffer(const std::vector<char, Alloc> &buf)
    {
        _buf = &buf[0];
        _buf_len = buf.size();
        _cur = 0;
    }

    /**
     * 判断是否已经到BUF的末尾
     */
    bool hasEnd()
    {
        return _cur >= _buf_len;
    }
    size_t tellp() const
    {
        return _cur;
    }
    const char *base() const
    {
        return _buf;
    }
    size_t size() const
    {
        return _buf_len;
    }
};

//当jce文件中含有指针型类型的数据用MapBufferReader读取
//在读数据时利用MapBufferReader提前分配的内存 减少运行过程中频繁内存分配
//结构中定义byte指针类型，指针用*来定义，如下：
//byte *m;
//指针类型使用时需要MapBufferReader提前设定预分配内存块setMapBuffer()，
//指针需要内存时通过偏移指向预分配内存块，减少解码过程中的内存申请
class MapBufferReader : public BufferReader
{

public:
    MapBufferReader() : _buf_m(NULL), _buf_len_m(0), _cur_m(0) {}

    void reset()
    {
        _cur_m = 0;
        BufferReader::reset();
    }

    char *cur()
    {
        if (jce_unlikely(_buf_m == NULL)) {
            char s[64];
            snprintf(s, sizeof(s), "MapBufferReader's buff not set,_buf = null");
            throw JceDecodeException(s);
        }

        return _buf_m + _cur_m;
    }

    size_t left()
    {
        return _buf_len_m - _cur_m;
    }

    /// 跳过len个字节
    void mapBufferSkip(size_t len)
    {
        if (jce_unlikely(_cur_m + len > _buf_len_m)) {
            char s[64];
            snprintf(s, sizeof(s), "MapBufferReader's buffer overflow when peekBuf, over %u.", (uint32_t)_buf_len_m);
            throw JceDecodeException(s);
        }

        _cur_m += len;
    }

    /// 设置缓存
    void setMapBuffer(char *buf, size_t len)
    {
        _buf_m = buf;
        _buf_len_m = len;
        _cur_m = 0;
    }

    /// 设置缓存
    template<typename Alloc>
    void setMapBuffer(std::vector<char, Alloc> &buf)
    {
        _buf_m = &buf[0];
        _buf_len_m = buf.size();
        _cur_m = 0;
    }
public:
    char               *_buf_m;        ///< 缓冲区
    size_t              _buf_len_m;    ///< 缓冲区长度
    size_t              _cur_m;        ///< 当前位置
};

//////////////////////////////////////////////////////////////////
/// 缓冲区写入器封装
class BufferWriter
{
public:
    char   *_buf;
    size_t  _len;
    size_t  _buf_len;
    bool	_reverse;

public:
    BufferWriter(const BufferWriter &bw)
    {
        _buf = NULL;
        _len = 0;
        _buf_len = 0;
        _reverse = true;

        writeBuf(bw._buf, bw._len);
        _len = bw._len;
        //_buf_len    = bw._buf_len;
    }

    BufferWriter &operator=(const BufferWriter &buf)
    {
        _reverse = true;
        writeBuf(buf._buf, buf._len);
        _len = buf._len;
        //_buf_len = buf._buf_len;
        return *this;
    }

    BufferWriter()
        : _buf(NULL)
        , _len(0)
        , _buf_len(0)
        , _reverse(true)
    {}
    ~BufferWriter()
    {
        delete[] _buf;
    }

    void reserve(size_t len)
    {
        if (jce_unlikely(_buf_len < len)) {
            len <<= 1;

            if (len < 128) {
                len = 128;
            }

            char *p = new char[len];
            memcpy(p, _buf, _len);
            delete[] _buf;
            _buf = p;
            _buf_len = len;
        }
    }
    void reset()
    {
        _len = 0;
    }
    void writeBuf(const void *buf, size_t len)
    {
        TafReserveBuf(*this, _len + len);
        memcpy(_buf + _len, buf, len);
        _len += len;
    }
    //const std::vector<char> &getByteBuffer() const      { return _buf; }
    std::vector<char> getByteBuffer() const
    {
        return std::vector<char>(_buf, _buf + _len);
    }
    const char *getBuffer() const
    {
        return _buf;
    }//{ return &_buf[0]; }
    size_t getLength() const
    {
        return _len;
    }	//{ return _buf.size(); }
    //void swap(std::vector<char>& v)                     { _buf.swap(v); }
    void swap(std::vector<char> &v)
    {
        v.assign(_buf, _buf + _len);
    }
    void swap(BufferWriter &buf)
    {
        std::swap(_buf, buf._buf);
        std::swap(_buf_len, buf._buf_len);
        std::swap(_len, buf._len);
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// 预先设定缓存的封装器
class BufferWriterBuff
{
public:
    char   *_buf;
    size_t  _len;
    size_t  _buf_len;
    bool	_reverse;
private:
    BufferWriterBuff(const BufferWriterBuff &);
public:

    BufferWriterBuff &operator=(const BufferWriterBuff &buf)
    {
        _reverse = false;
        writeBuf(buf._buf, buf._len);
        _len = buf._len;
        _buf_len = buf._buf_len;
        return *this;
    }

    BufferWriterBuff()
        : _buf(NULL)
        , _len(0)
        , _buf_len(0)
        , _reverse(false)
    {}
    ~BufferWriterBuff()
    {

    }

    void setBuffer(char *buffer, size_t size_buff)
    {
        _buf = buffer;
        _len = 0;
        _buf_len = size_buff;
        _reverse = false;
    }

    /*
    void reserve(size_t len)
    {
        if(_buf_len < len)
        {

        }
    }
    */
    void reset()
    {
        _len = 0;
    }

    void writeBuf(const void *buf, size_t len)
    {
        if (jce_unlikely(_buf_len < _len + len)) {
            throw JceNotEnoughBuff("not enough buffer");
        }

        memcpy(_buf + _len, buf, len);
        _len += len;
    }

    std::vector<char> getByteBuffer() const
    {
        return std::vector<char>(_buf, _buf + _len);
    }
    const char *getBuffer() const
    {
        return _buf;
    }
    size_t getLength() const
    {
        return _len;
    }
    void swap(std::vector<char> &v)
    {
        v.assign(_buf, _buf + _len);
    }
    void swap(BufferWriterBuff &buf)
    {
        std::swap(_buf, buf._buf);
        std::swap(_buf_len, buf._buf_len);
        std::swap(_len, buf._len);
    }
};

//////////////////////////////////////////////////////////////////
template<typename ReaderT = BufferReader>
class JceInputStream : public ReaderT
{
public:
    //taf自动伸缩使用的接口，其它不会使用
    bool getTagType(uint8_t tag, uint8_t &type)
    {
        try {
            DataHead h;

            while (!ReaderT::hasEnd()) {
                size_t len = h.peekFrom(*this);

                if (tag <= h.getTag() || h.getType() == DataHead::eStructEnd) {
                    if (h.getType() == DataHead::eStructEnd) {
                        return false;
                    } else if (tag == h.getTag()) {
                        type = h.getType();
                        return true;
                    } else {
                        return false;
                    }
                }

                this->skip(len);
                skipField(h.getType());
            }
        } catch (JceDecodeException &e) {
        }

        return false;
    }

    //taf自动伸缩使用的接口，其它不会使用
    int getFieldValue(uint8_t type, int64_t &value)
    {
        int flag = -1;

        value = 0;

        switch (type) {
        case DataHead::eChar: {
            Char c = 0;
            read(c, 0);

            value = c;

            flag = 0;
        }
        break;

        case DataHead::eShort: {
            Short s = 0;
            read(s, 0);

            value = s;

            flag = 0;
        }
        break;

        case DataHead::eInt32: {
            Int32 i = 0;
            read(i, 0);

            value = i;

            flag = 0;
        }
        break;

        case DataHead::eInt64: {
            Int64 i = 0;
            read(i, 0);

            value = i;

            flag = 0;
        }
        break;

        case DataHead::eFloat: {
            flag = -1;
        }
        break;

        case DataHead::eDouble: {
            flag = -1;
        }
        break;

        case DataHead::eString1: {
            DataHead h;
            h.readFrom(*this);

            uint8_t len = readByType<uint8_t>();
            value = len;

            flag = 1;
        }
        break;

        case DataHead::eString4: {
            DataHead h;
            h.readFrom(*this);

            uint32_t len = ntohl(readByType<uint32_t>());
            value = len;

            flag = 1;
        }
        break;

        case DataHead::eMap: {
            DataHead h;
            h.readFrom(*this);

            Int32 size = 0;
            read(size, 0);

            value = size;

            flag = 2;
        }
        break;

        case DataHead::eList: {
            DataHead h;
            h.readFrom(*this);

            Int32 size = 0;
            read(size, 0);

            value = size;

            flag = 3;
        }
        break;

        case DataHead::eSimpleList: {
            DataHead h;
            h.readFrom(*this);

            DataHead hh;
            hh.readFrom(*this);

            UInt32 size = 0;
            read(size, 0);

            value = size;

            flag = 4;
        }
        break;

        case DataHead::eStructBegin: {
            flag = -1;
        }
        break;

        case DataHead::eStructEnd: {
            flag = -1;
        }
        break;

        case DataHead::eZeroTag: {
            flag = 0;
        }
        break;

        default: {
            char s[64];
            snprintf(s, sizeof(s), "getFieldValue with invalid type, type value:%d.", type);
            throw JceDecodeMismatch(s);
        }
        }

        return flag;
    }

    /// 跳到指定标签的元素前
    bool skipToTag(uint8_t tag)
    {
        try {
            uint8_t headType = 0, headTag = 0;

            while (!ReaderT::hasEnd()) {
                size_t len = 0;
                TafPeekFromHead(*this, headType, headTag, len);

                if (tag <= headTag || headType == TafHeadeStructEnd) {
                    return headType == TafHeadeStructEnd ? false : (tag == headTag);
                }

                TafReadHeadSkip(*this, len);
                skipField(headType);
            }
        } catch (JceDecodeException &e) {
        }

        return false;
    }

    /// 跳到当前结构的结束
    void skipToStructEnd()
    {
        uint8_t headType = 0;

        do {
            readFromHeadNoTag(*this, headType);
            skipField(headType);
        } while (headType != TafHeadeStructEnd);
    }

    /// 跳过一个字段
    void skipField()
    {
        uint8_t headType = 0;
        readFromHeadNoTag(*this, headType);
        skipField(headType);
    }

    /// 跳过一个字段，不包含头信息
    void skipField(uint8_t type)
    {
        switch (type) {
        case TafHeadeChar:
            TafReadHeadSkip(*this, sizeof(Char));
            break;

        case TafHeadeShort:
            TafReadHeadSkip(*this, sizeof(Short));
            break;

        case TafHeadeInt32:
            TafReadHeadSkip(*this, sizeof(Int32));
            break;

        case TafHeadeInt64:
            TafReadHeadSkip(*this, sizeof(Int64));
            break;

        case TafHeadeFloat:
            TafReadHeadSkip(*this, sizeof(Float));
            break;

        case TafHeadeDouble:
            TafReadHeadSkip(*this, sizeof(Double));
            break;

        case TafHeadeString1: {
            size_t len = 0;
            TafReadTypeBuf(*this, len, uint8_t);
            TafReadHeadSkip(*this, len);
        }
        break;

        case TafHeadeString4: {
            size_t len = 0;
            TafReadTypeBuf(*this, len, uint32_t);
            len = ntohl(len);
            TafReadHeadSkip(*this, len);
        }
        break;

        case TafHeadeMap: {
            Int32 size = 0;
            read(size, 0);

            for (Int32 i = 0; i < size * 2; ++i) {
                skipField();
            }
        }
        break;

        case TafHeadeList: {
            Int32 size = 0;
            read(size, 0);

            for (Int32 i = 0; i < size; ++i) {
                skipField();
            }
        }
        break;

        case TafHeadeSimpleList: {
            uint8_t headType = 0, headTag = 0;
            readFromHead(*this, headType, headTag);

            if (jce_unlikely(headType != TafHeadeChar)) {
                char s[64];
                snprintf(s, sizeof(s), "skipField with invalid type, type value: %d, %d, %d.", type, headType, headTag);
                throw JceDecodeMismatch(s);
            }

            Int32 size = 0;
            read(size, 0);
            TafReadHeadSkip(*this, size);
        }
        break;

        case TafHeadeStructBegin:
            skipToStructEnd();
            break;

        case TafHeadeStructEnd:
        case TafHeadeZeroTag:
            break;

        default: {
            char s[64];
            snprintf(s, sizeof(s), "skipField with invalid type, type value:%d.", type);
            throw JceDecodeMismatch(s);
        }
        }
    }

    /// 读取一个指定类型的数据（基本类型）
    template<typename T>
    inline T readByType()
    {
        T n;
        this->readBuf(&n, sizeof(n));
        return n;
    }
    void readUnknown(std::string &sUnkown, uint8_t tag)
    {

        size_t start = ReaderT::tellp();
        size_t last  = start;

        try {
            uint8_t lasttag = tag;
            DataHead h;

            while (!ReaderT::hasEnd()) {
                size_t len = h.peekFrom(*this);

                if ( h.getTag() <= lasttag) {
                    break;
                }

                lasttag = h.getTag();
                this->skip(len);
                skipField(h.getType());
                last = ReaderT::tellp(); //记录下最后一次正常到达的位置
            }
        } catch (...) { //
            throw;
        }

        std::string s(ReaderT::base() + start, last - start);
        sUnkown = s;
        return ;

    }
    friend class XmlProxyCallback;

    void read(Bool &b, uint8_t tag, bool isRequire = true)
    {
        Char c = b;
        read(c, tag, isRequire);
        b = c ? true : false;
    }

    void read(Char &c, uint8_t tag, bool isRequire = true)
    {
        uint8_t headType = 0, headTag = 0;
        bool skipFlag = false;
        TafSkipToTag(skipFlag, tag, headType, headTag);

        if (jce_likely(skipFlag)) {
            switch (headType) {
            case TafHeadeZeroTag:
                c = 0;
                break;

            case TafHeadeChar:
                TafReadTypeBuf(*this, c, Char);
                break;

            default: {
                char s[64];
                snprintf(s, sizeof(s), "read 'Char' type mismatch, tag: %d, get type: %d.", tag, headType);
                throw JceDecodeMismatch(s);
            }

            }
        } else if (jce_unlikely(isRequire)) {
            char s[64];
            snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d.", tag, headTag);
            throw JceDecodeRequireNotExist(s);
        }
    }

    void read(UInt8 &n, uint8_t tag, bool isRequire = true)
    {
        Short i = (Short)n;
        read(i, tag, isRequire);
        n = (UInt8)i;
    }

    void read(Short &n, uint8_t tag, bool isRequire = true)
    {
        uint8_t headType = 0, headTag = 0;
        bool skipFlag = false;
        TafSkipToTag(skipFlag, tag, headType, headTag);

        if (jce_likely(skipFlag)) {
            switch (headType) {
            case TafHeadeZeroTag:
                n = 0;
                break;

            case TafHeadeChar:
                TafReadTypeBuf(*this, n, Char);
                break;

            case TafHeadeShort:
                TafReadTypeBuf(*this, n, Short);
                n = ntohs(n);
                break;

            default: {
                char s[64];
                snprintf(s, sizeof(s), "read 'Short' type mismatch, tag: %d, get type: %d.", tag, headType);
                throw JceDecodeMismatch(s);
            }
            }
        } else if (jce_unlikely(isRequire)) {
            char s[64];
            snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
            throw JceDecodeRequireNotExist(s);
        }
    }

    void read(UInt16 &n, uint8_t tag, bool isRequire = true)
    {
        Int32 i = (Int32)n;
        read(i, tag, isRequire);
        n = (UInt16)i;
    }

    void read(Int32 &n, uint8_t tag, bool isRequire = true)
    {
        uint8_t headType = 1, headTag = 1;
        bool skipFlag = false;
        TafSkipToTag(skipFlag, tag, headType, headTag);

        if (jce_likely(skipFlag)) {
            switch (headType) {
            case TafHeadeZeroTag:
                n = 0;
                break;

            case TafHeadeChar:
                TafReadTypeBuf(*this, n, Char);
                break;

            case TafHeadeShort:
                TafReadTypeBuf(*this, n, Short);
                n = (Short)ntohs(n);
                break;

            case TafHeadeInt32:
                TafReadTypeBuf(*this, n, Int32);
                n = ntohl(n);
                break;

            default: {
                char s[64];
                snprintf(s, sizeof(s), "read 'Int32' type mismatch, tag: %d, get type: %d.", tag, headType);
                throw JceDecodeMismatch(s);
            }
            }
        } else if (jce_unlikely(isRequire)) {
            char s[64];
            snprintf(s, sizeof(s), "require field not exist, tag: %d headType: %d, headTag: %d", tag, headType, headTag);
            throw JceDecodeRequireNotExist(s);
        }
    }

    void read(UInt32 &n, uint8_t tag, bool isRequire = true)
    {
        Int64 i = (Int64)n;
        read(i, tag, isRequire);
        n = (UInt32)i;
    }

    void read(Int64 &n, uint8_t tag, bool isRequire = true)
    {
        uint8_t headType = 0, headTag = 0;
        bool skipFlag = false;
        TafSkipToTag(skipFlag, tag, headType, headTag);

        if (jce_likely(skipFlag)) {
            switch (headType) {
            case TafHeadeZeroTag:
                n = 0;
                break;

            case TafHeadeChar:
                TafReadTypeBuf(*this, n, Char);
                break;

            case TafHeadeShort:
                TafReadTypeBuf(*this, n, Short);
                n = (Short) ntohs(n);
                break;

            case TafHeadeInt32:
                TafReadTypeBuf(*this, n, Int32);
                n = (Int32) ntohl(n);
                break;

            case TafHeadeInt64:
                TafReadTypeBuf(*this, n, Int64);
                n = jce_ntohll(n);
                break;

            default: {
                char s[64];
                snprintf(s, sizeof(s), "read 'Int64' type mismatch, tag: %d, get type: %d.", tag, headType);
                throw JceDecodeMismatch(s);
            }

            }
        } else if (jce_unlikely(isRequire)) {
            char s[64];
            snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
            throw JceDecodeRequireNotExist(s);
        }
    }

    void read(Float &n, uint8_t tag, bool isRequire = true)
    {
        uint8_t headType = 0, headTag = 0;
        bool skipFlag = false;
        TafSkipToTag(skipFlag, tag, headType, headTag);

        if (jce_likely(skipFlag)) {
            switch (headType) {
            case TafHeadeZeroTag:
                n = 0;
                break;

            case TafHeadeFloat:
                TafReadTypeBuf(*this, n, float);
                n = jce_ntohf(n);
                break;

            default: {
                char s[64];
                snprintf(s, sizeof(s), "read 'Float' type mismatch, tag: %d, get type: %d.", tag, headType);
                throw JceDecodeMismatch(s);
            }
            }
        } else if (jce_unlikely(isRequire)) {
            char s[64];
            snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
            throw JceDecodeRequireNotExist(s);
        }
    }

    void read(Double &n, uint8_t tag, bool isRequire = true)
    {
        uint8_t headType = 0, headTag = 0;
        bool skipFlag = false;
        TafSkipToTag(skipFlag, tag, headType, headTag);

        if (jce_likely(skipFlag)) {
            switch (headType) {
            case TafHeadeZeroTag:
                n = 0;
                break;

            case TafHeadeFloat:
                TafReadTypeBuf(*this, n, float);
                n = jce_ntohf(n);
                break;

            case TafHeadeDouble:
                TafReadTypeBuf(*this, n, double);
                n = jce_ntohd(n);
                break;

            default: {
                char s[64];
                snprintf(s, sizeof(s), "read 'Double' type mismatch, tag: %d, get type: %d.", tag, headType);
                throw JceDecodeMismatch(s);
            }
            }
        } else if (jce_unlikely(isRequire)) {
            char s[64];
            snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
            throw JceDecodeRequireNotExist(s);
        }
    }

    /*void read(std::string& s, uint8_t tag, bool isRequire = true)
    {
    	uint8_t headType = 0, headTag = 0;
    	bool skipFlag = false;
    	TafSkipToTag(skipFlag, tag, headType, headTag);
    	if (jce_likely(skipFlag))
    	{
    		switch(headType)
    		{
    		case TafHeadeString1:
    			{
    				size_t len = 0;
    				TafReadTypeBuf(*this, len, uint8_t);
    				char ss[256];
    				//s.resize(len);
                    //this->readBuf((void *)s.c_str(), len);
    				TafReadStringBuf(*this, s, len);
    				//TafReadBuf(*this, s, len);
    				//s.assign(ss, ss + len);
    			}
    			break;
    		case TafHeadeString4:
    			{
    				uint32_t len = 0;
    				TafReadTypeBuf(*this, len, uint32_t);
    				len = ntohl(len);
    				if (jce_unlikely(len > JCE_MAX_STRING_LENGTH))
    				{
    					char s[128];
    					snprintf(s, sizeof(s), "invalid string size, tag: %d, size: %d", tag, len);
    					throw JceDecodeInvalidValue(s);
    				}
    				//char *ss = new char[len];
                    //s.resize(len);
                    //this->readBuf((void *)s.c_str(), len);

    				char *ss = new char[len];
    				try
    				{
    					TafReadBuf(*this, ss, len);
    					s.assign(ss, ss + len);
    				}
    				catch (...)
    				{
    					delete[] ss;
    					throw;
    				}
    				delete[] ss;
    TafReadStringBuf(*this, s, len);
    			}
    			break;
    		default:
    		{
    				   char s[64];
    				   snprintf(s, sizeof(s), "read 'string' type mismatch, tag: %d, get type: %d.", tag, headType);
    				   throw JceDecodeMismatch(s);
    		}
    		}
    	}
    	else if (jce_unlikely(isRequire))
    	{
    		char s[64];
    		snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
    		throw JceDecodeRequireNotExist(s);
    	}
    }*/

    void read(std::string &s, uint8_t tag, bool isRequire = true)
    {
        uint8_t headType = 0, headTag = 0;
        bool skipFlag = false;
        TafSkipToTag(skipFlag, tag, headType, headTag);

        if (jce_likely(skipFlag)) {
            uint32_t strLength = 0;

            switch (headType) {
            case TafHeadeString1: {
                TafReadTypeBuf(*this, strLength, uint8_t);
            }
            break;

            case TafHeadeString4: {
                TafReadTypeBuf(*this, strLength, uint32_t);
                strLength = ntohl(strLength);

                if (jce_unlikely(strLength > JCE_MAX_STRING_LENGTH)) {
                    char s[128];
                    snprintf(s, sizeof(s), "invalid string size, tag: %d, size: %d", tag, strLength);
                    throw JceDecodeInvalidValue(s);
                }
            }
            break;

            default: {
                char s[64];
                snprintf(s, sizeof(s), "read 'string' type mismatch, tag: %d, get type: %d, tag: %d.", tag, headType, headTag);
                throw JceDecodeMismatch(s);
            }
            }

            TafReadStringBuf(*this, s, strLength);
        } else if (jce_unlikely(isRequire)) {
            char s[64];
            snprintf(s, sizeof(s), "require field not exist, tag: %d", tag);
            throw JceDecodeRequireNotExist(s);
        }
    }

    void read(char *buf, const UInt32 bufLen, UInt32 &readLen, uint8_t tag, bool isRequire = true)
    {
        uint8_t headType = 0, headTag = 0;
        bool skipFlag = false;
        TafSkipToTag(skipFlag, tag, headType, headTag);

        if (jce_likely(skipFlag)) {
            switch (headType) {
            case TafHeadeSimpleList: {
                uint8_t hheadType, hheadTag;
                readFromHead(*this, hheadType, hheadTag);

                if (jce_unlikely(hheadType != TafHeadeChar)) {
                    char s[128];
                    snprintf(s, sizeof(s), "type mismatch, tag: %d, type: %d, %d, %d", tag, headType, hheadType, hheadTag);
                    throw JceDecodeMismatch(s);
                }

                UInt32 size = 0;
                read(size, 0);

                if (jce_unlikely(size > bufLen)) {
                    char s[128];
                    snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, %d, size: %d", tag, headType, hheadType, size);
                    throw JceDecodeInvalidValue(s);
                }

                //TafReadTypeBuf(*this, size, UInt32);
                this->readBuf(buf, size);
                readLen = size;
            }
            break;

            default: {
                char s[128];
                snprintf(s, sizeof(s), "type mismatch, tag: %d, type: %d", tag, headType);
                throw JceDecodeMismatch(s);
            }
            }
        } else if (jce_unlikely(isRequire)) {
            char s[128];
            snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
            throw JceDecodeRequireNotExist(s);
        }
    }


    template<typename K, typename V, typename Cmp, typename Alloc>
    void read(std::map<K, V, Cmp, Alloc> &m, uint8_t tag, bool isRequire = true)
    {
        uint8_t headType = 0, headTag = 0;
        bool skipFlag = false;
        TafSkipToTag(skipFlag, tag, headType, headTag);

        if (jce_likely(skipFlag)) {
            switch (headType) {
            case TafHeadeMap: {
                UInt32 size = 0;
                read(size, 0);

                if (jce_unlikely(size > this->size())) {
                    char s[128];
                    snprintf(s, sizeof(s), "invalid map, tag: %d, size: %d", tag, size);
                    throw JceDecodeInvalidValue(s);
                }

                m.clear();

                for (UInt32 i = 0; i < size; ++i) {
                    std::pair<K, V> pr;
                    read(pr.first, 0);
                    read(pr.second, 1);
                    m.insert(pr);
                }
            }
            break;

            default: {
                char s[64];
                snprintf(s, sizeof(s), "read 'map' type mismatch, tag: %d, get type: %d.", tag, headType);
                throw JceDecodeMismatch(s);
            }
            }
        } else if (jce_unlikely(isRequire)) {
            char s[64];
            snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
            throw JceDecodeRequireNotExist(s);
        }
    }

    template<typename Alloc>
    void read(std::vector<Char, Alloc> &v, uint8_t tag, bool isRequire = true)
    {
        uint8_t headType = 0, headTag = 0;
        bool skipFlag = false;
        TafSkipToTag(skipFlag, tag, headType, headTag);

        if (jce_likely(skipFlag)) {
            switch (headType) {
            case TafHeadeSimpleList: {
                uint8_t hheadType, hheadTag;
                readFromHead(*this, hheadType, hheadTag);

                if (jce_unlikely(hheadType != TafHeadeChar)) {
                    char s[128];
                    snprintf(s, sizeof(s), "type mismatch, tag: %d, type: %d, %d, %d", tag, headType, hheadType, hheadTag);
                    throw JceDecodeMismatch(s);
                }

                UInt32 size = 0;
                read(size, 0);

                if (jce_unlikely(size > this->size())) {
                    char s[128];
                    snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, %d, size: %d", tag, headType, hheadType, size);
                    throw JceDecodeInvalidValue(s);
                }

                this->readBuf(v, size);
            }
            break;

            case TafHeadeList: {
                UInt32 size = 0;
                read(size, 0);

                if (jce_unlikely(size > this->size())) {
                    char s[128];
                    snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d", tag, headType, size);
                    throw JceDecodeInvalidValue(s);
                }

                v.reserve(size);
                v.resize(size);

                for (UInt32 i = 0; i < size; ++i) {
                    read(v[i], 0);
                }
            }
            break;

            default: {
                char s[128];
                snprintf(s, sizeof(s), "type mismatch, tag: %d, type: %d", tag, headType);
                throw JceDecodeMismatch(s);
            }
            }
        } else if (jce_unlikely(isRequire)) {
            char s[128];
            snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
            throw JceDecodeRequireNotExist(s);
        }
    }

    template<typename T, typename Alloc>
    void read(std::vector<T, Alloc> &v, uint8_t tag, bool isRequire = true)
    {
        uint8_t headType = 0, headTag = 0;
        bool skipFlag = false;
        TafSkipToTag(skipFlag, tag, headType, headTag);

        if (jce_likely(skipFlag)) {
            switch (headType) {
            case TafHeadeList: {
                UInt32 size = 0;
                read(size, 0);

                if (jce_unlikely(size > this->size())) {
                    char s[128];
                    snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d", tag, headType, size);
                    throw JceDecodeInvalidValue(s);
                }

                v.reserve(size);
                v.resize(size);

                for (UInt32 i = 0; i < size; ++i) {
                    read(v[i], 0);
                }
            }
            break;

            default: {
                char s[64];
                snprintf(s, sizeof(s), "read 'vector' type mismatch, tag: %d, get type: %d.", tag, headType);
                throw JceDecodeMismatch(s);
            }
            }
        } else if (jce_unlikely(isRequire)) {
            char s[64];
            snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
            throw JceDecodeRequireNotExist(s);
        }
    }

    /// 读取结构数组
    template<typename T>
    void read(T *v, const UInt32 len, UInt32 &readLen, uint8_t tag, bool isRequire = true)
    {
        uint8_t headType = 0, headTag = 0;
        bool skipFlag = false;
        TafSkipToTag(skipFlag, tag, headType, headTag);

        if (jce_likely(skipFlag)) {
            switch (headType) {
            case TafHeadeList: {
                UInt32 size = 0;
                read(size, 0);

                if (jce_unlikely(size > this->size())) {
                    char s[128];
                    snprintf(s, sizeof(s), "invalid size, tag: %d, type: %d, size: %d", tag, headType, size);
                    throw JceDecodeInvalidValue(s);
                }

                for (UInt32 i = 0; i < size; ++i) {
                    read(v[i], 0);
                }

                readLen = size;
            }
            break;

            default: {
                char s[64];
                snprintf(s, sizeof(s), "read 'vector struct' type mismatch, tag: %d, get type: %d.", tag, headType);
                throw JceDecodeMismatch(s);
            }
            }
        } else if (jce_unlikely(isRequire)) {
            char s[64];
            snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
            throw JceDecodeRequireNotExist(s);
        }
    }

    template<typename T>
    void read(T &v, uint8_t tag, bool isRequire = true, typename jce::disable_if<jce::is_convertible<T *, JceStructBase *>, void ***>::type dummy = 0)
    {
        Int32 n = 0;
        read(n, tag, isRequire);
        v = (T) n;
    }

    /// 读取结构
    template<typename T>
    void read(T &v, uint8_t tag, bool isRequire = true, typename jce::enable_if<jce::is_convertible<T *, JceStructBase *>, void ***>::type dummy = 0)
    {
        uint8_t headType = 0, headTag = 0;
        bool skipFlag = false;
        TafSkipToTag(skipFlag, tag, headType, headTag);

        if (jce_likely(skipFlag)) {
            if (jce_unlikely(headType != TafHeadeStructBegin)) {
                char s[64];
                snprintf(s, sizeof(s), "read 'struct' type mismatch, tag: %d, get type: %d.", tag, headType);
                throw JceDecodeMismatch(s);
            }

            v.readFrom(*this);
            skipToStructEnd();
        } else if (jce_unlikely(isRequire)) {
            char s[64];
            snprintf(s, sizeof(s), "require field not exist, tag: %d, headTag: %d", tag, headTag);
            throw JceDecodeRequireNotExist(s);
        }
    }
};

//////////////////////////////////////////////////////////////////
template<typename WriterT = BufferWriter>
class JceOutputStream : public WriterT
{
public:
    void writeUnknown(const std::string &s)
    {
        this->writeBuf(s.data(), s.size());
    }
    void writeUnknownV2(const std::string &s)
    {
        DataHead::writeTo(*this, DataHead::eStructBegin, 0);
        this->writeBuf(s.data(), s.size());
        DataHead::writeTo(*this, DataHead::eStructEnd, 0);
    }
    void write(Bool b, uint8_t tag)
    {
        write((Char) b, tag);
    }

    void write(Char n, uint8_t tag)
    {
        /*
        DataHead h(DataHead::eChar, tag);
        if(n == 0){
            h.setType(DataHead::eZeroTag);
            h.writeTo(*this);
        }else{
            h.writeTo(*this);
            this->writeBuf(&n, sizeof(n));
        }
        */
        if (jce_unlikely(n == 0)) {
            TafWriteToHead(*this, TafHeadeZeroTag, tag);
        } else {
            TafWriteToHead(*this, TafHeadeChar, tag);
            TafWriteCharTypeBuf(*this, n, (*this)._len);
        }
    }

    void write(UInt8 n, uint8_t tag)
    {
        write((Short) n, tag);
    }

    void write(Short n, uint8_t tag)
    {
        //if(n >= CHAR_MIN && n <= CHAR_MAX){
        if (n >= (-128) && n <= 127) {
            write((Char) n, tag);
        } else {
            /*
            DataHead h(DataHead::eShort, tag);
            h.writeTo(*this);
            n = htons(n);
            this->writeBuf(&n, sizeof(n));
            */
            TafWriteToHead(*this, TafHeadeShort, tag);
            n = htons(n);
            TafWriteShortTypeBuf(*this, n, (*this)._len);
        }
    }

    void write(UInt16 n, uint8_t tag)
    {
        write((Int32) n, tag);
    }

    void write(Int32 n, uint8_t tag)
    {
        //if(n >= SHRT_MIN && n <= SHRT_MAX){
        if (n >= (-32768) && n <= 32767) {
            write((Short) n, tag);
        } else {
            //DataHead h(DataHead::eInt32, tag);
            //h.writeTo(*this);
            TafWriteToHead(*this, TafHeadeInt32, tag);
            n = htonl(n);
            TafWriteInt32TypeBuf(*this, n, (*this)._len);
        }
    }

    void write(UInt32 n, uint8_t tag)
    {
        write((Int64) n, tag);
    }

    void write(Int64 n, uint8_t tag)
    {
        //if(n >= INT_MIN && n <= INT_MAX){
        if (n >= (-2147483647 - 1) && n <= 2147483647) {
            write((Int32) n, tag);
        } else {
            //DataHead h(DataHead::eInt64, tag);
            //h.writeTo(*this);
            TafWriteToHead(*this, TafHeadeInt64, tag);
            n = jce_htonll(n);
            TafWriteInt64TypeBuf(*this, n, (*this)._len);
        }
    }

    void write(Float n, uint8_t tag)
    {
        //DataHead h(DataHead::eFloat, tag);
        //h.writeTo(*this);
        TafWriteToHead(*this, TafHeadeFloat, tag);
        n = jce_htonf(n);
        TafWriteFloatTypeBuf(*this, n, (*this)._len);
    }

    void write(Double n, uint8_t tag)
    {
        //DataHead h(DataHead::eDouble, tag);
        //h.writeTo(*this);
        TafWriteToHead(*this, TafHeadeDouble, tag);
        n = jce_htond(n);
        TafWriteDoubleTypeBuf(*this, n, (*this)._len);
    }

    void write(const std::string &s, uint8_t tag)
    {
        if (jce_unlikely(s.size() > 255)) {
            if (jce_unlikely(s.size() > JCE_MAX_STRING_LENGTH)) {
                char ss[128];
                snprintf(ss, sizeof(ss), "invalid string size, tag: %d, size: %u", tag, (uint32_t)s.size());
                throw JceDecodeInvalidValue(ss);
            }

            TafWriteToHead(*this, TafHeadeString4, tag);
            uint32_t n = htonl(s.size());
            TafWriteUInt32TTypeBuf(*this, n, (*this)._len);
            //this->writeBuf(s.data(), s.size());
            TafWriteTypeBuf(*this, s.data(), s.size());
        } else {
            TafWriteToHead(*this, TafHeadeString1, tag);
            uint8_t n = s.size();
            TafWriteUInt8TTypeBuf(*this, n, (*this)._len);
            //this->writeBuf(s.data(), s.size());
            TafWriteTypeBuf(*this, s.data(), s.size());
        }
    }

    void write(const char *buf, const UInt32 len, uint8_t tag)
    {
        TafWriteToHead(*this, TafHeadeSimpleList, tag);
        TafWriteToHead(*this, TafHeadeChar, 0);
        write(len, 0);
        //this->writeBuf(buf, len);
        TafWriteTypeBuf(*this, buf, len);
    }

    template<typename K, typename V, typename Cmp, typename Alloc>
    void write(const std::map<K, V, Cmp, Alloc> &m, uint8_t tag)
    {
        //DataHead h(DataHead::eMap, tag);
        //h.writeTo(*this);
        TafWriteToHead(*this, TafHeadeMap, tag);
        Int32 n = m.size();
        write(n, 0);
        typedef typename std::map<K, V, Cmp, Alloc>::const_iterator IT;

        for (IT i = m.begin(); i != m.end(); ++i) {
            write(i->first, 0);
            write(i->second, 1);
        }
    }

    template<typename T, typename Alloc>
    void write(const std::vector<T, Alloc> &v, uint8_t tag)
    {
        //DataHead h(DataHead::eList, tag);
        //h.writeTo(*this);
        TafWriteToHead(*this, TafHeadeList, tag);
        Int32 n = v.size();
        write(n, 0);
        typedef typename std::vector<T, Alloc>::const_iterator IT;

        for (IT i = v.begin(); i != v.end(); ++i) {
            write(*i, 0);
        }
    }

    template<typename T>
    void write(const T *v, const UInt32 len, uint8_t tag)
    {
        TafWriteToHead(*this, TafHeadeList, tag);
        write(len, 0);

        for (Int32 i = 0; i < (Int32)len; ++i) {
            write(v[i], 0);
        }
    }

    template<typename Alloc>
    void write(const std::vector<Char, Alloc> &v, uint8_t tag)
    {
        //DataHead h(DataHead::eSimpleList, tag);
        //h.writeTo(*this);
        //DataHead hh(DataHead::eChar, 0);
        //hh.writeTo(*this);
        TafWriteToHead(*this, TafHeadeSimpleList, tag);
        TafWriteToHead(*this, TafHeadeChar, 0);
        Int32 n = v.size();
        write(n, 0);
        //writeBuf(&v[0], v.size());
        TafWriteTypeBuf(*this, &v[0], v.size());
    }

    template<typename T>
    void write(const T &v, uint8_t tag, typename jce::disable_if<jce::is_convertible<T *, JceStructBase *>, void ***>::type dummy = 0)
    {
        write((Int32) v, tag);
    }

    template<typename T>
    void write(const T &v, uint8_t tag, typename jce::enable_if<jce::is_convertible<T *, JceStructBase *>, void ***>::type dummy = 0)
    {
        //DataHead h(DataHead::eStructBegin, tag);
        //h.writeTo(*this);
        TafWriteToHead(*this, TafHeadeStructBegin, tag);
        v.writeTo(*this);
        TafWriteToHead(*this, TafHeadeStructEnd, 0);
        /*
        h.setType(DataHead::eStructEnd);
        h.setTag(0);
        h.writeTo(*this);
        */
    }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}

//支持iphone
#ifdef __APPLE__
#include "JceDisplayer.h"
#else
#include "JceDisplayer.h"
#endif

#endif
