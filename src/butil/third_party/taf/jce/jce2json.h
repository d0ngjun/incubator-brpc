#ifndef __JCE2RJSON_H__
#define __JCE2RJSON_H__

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

#include "jce/Jce.h"
#include "util/tc_common.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"


namespace rapidjson
{
class Input
{
public:
    static string getType(int t)
    {
        switch (t) {
        case kNullType:
            return "'null'";

        case kFalseType:
            return "'bool'";

        case kTrueType:
            return "'bool'";

        case kObjectType:
            return "'object'";

        case kArrayType:
            return "'array'";

        case kStringType:
            return "'string'";

        case kNumberType:
            return "'number'";

        default:
            return "'unknown'(" + taf::TC_Common::tostr(t) + ")";
        }
    }

    static void read(taf::Bool &b, const Value &val, bool isRequire = true)
    {
        if (val.IsBool()) {
            b = val.GetBool();
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'bool' type, get " + getType(val.GetType()));
        }
    }

    static void read(taf::Char &c, const Value &val, bool isRequire = true)
    {
        if (val.IsNumber()) {
            c = (taf::Char)val.GetInt();
        } else if (val.IsString()) {
            const string &sStr = val.GetString();

            if (sStr.empty() || !isdigit(sStr[0])) {
                if (isRequire) {
                    throw taf::JceDecodeException(" not 'number' type, get invalid 'string'");
                }

                return;
            }

            c = (taf::Char)atoi(sStr.c_str());
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'number' type, get " + getType(val.GetType()));
        }
    }

    static void read(taf::UInt8 &n, const Value &val, bool isRequire = true)
    {
        if (val.IsNumber()) {
            n = (taf::UInt8)val.GetUint();
        } else if (val.IsString()) {
            const string &sStr = val.GetString();

            if (sStr.empty() || !isdigit(sStr[0])) {
                if (isRequire) {
                    throw taf::JceDecodeException(" not 'number' type, get invalid 'string'");
                }

                return;
            }

            n = (taf::UInt8)strtoul(sStr.c_str(), NULL, 10);
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'number' type, get " + getType(val.GetType()));
        }
    }

    static void read(taf::Short &n, const Value &val, bool isRequire = true)
    {
        if (val.IsNumber()) {
            n = (taf::Short)val.GetInt();
        } else if (val.IsString()) {
            const string &sStr = val.GetString();

            if (sStr.empty() || !isdigit(sStr[0])) {
                if (isRequire) {
                    throw taf::JceDecodeException(" not 'number' type, get invalid 'string'");
                }

                return;
            }

            n = atoi(sStr.c_str());
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'number' type, get " + getType(val.GetType()));
        }
    }

    static void read(taf::UInt16 &n, const Value &val, bool isRequire = true)
    {
        if (val.IsNumber()) {
            n = (taf::UInt16)val.GetUint();
        } else if (val.IsString()) {
            const string &sStr = val.GetString();

            if (sStr.empty() || !isdigit(sStr[0])) {
                if (isRequire) {
                    throw taf::JceDecodeException(" not 'number' type, get invalid 'string'");
                }

                return;
            }

            n = strtoul(sStr.c_str(), NULL, 10);
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'number' type, get " + getType(val.GetType()));
        }
    }

    static void read(taf::Int32 &n, const Value &val, bool isRequire = true)
    {
        if (val.IsNumber()) {
            n = (taf::Int32)val.GetInt();
        } else if (val.IsString()) {
            const string &sStr = val.GetString();

            if (sStr.empty() || !isdigit(sStr[0])) {
                if (isRequire) {
                    throw taf::JceDecodeException(" not 'number' type, get invalid 'string'");
                }

                return;
            }

            n = atoi(sStr.c_str());
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'number' type, get " + getType(val.GetType()));
        }
    }

    static void read(taf::UInt32 &n, const Value &val, bool isRequire = true)
    {
        if (val.IsNumber()) {
            n = (taf::UInt32)val.GetUint();
        } else if (val.IsString()) {
            const string &sStr = val.GetString();

            if (sStr.empty() || !isdigit(sStr[0])) {
                if (isRequire) {
                    throw taf::JceDecodeException(" not 'number' type, get invalid 'string'");
                }

                return;
            }

            n = strtoul(sStr.c_str(), NULL, 10);
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'number' type, get " + getType(val.GetType()));
        }
    }

    static void read(taf::Int64 &n, const Value &val, bool isRequire = true)
    {
        if (val.IsNumber()) {
            n = (taf::Int64)val.GetInt64();
        } else if (val.IsString()) {
            const string &sStr = val.GetString();

            if (sStr.empty() || !isdigit(sStr[0])) {
                if (isRequire) {
                    throw taf::JceDecodeException(" not 'number' type, get invalid 'string'");
                }

                return;
            }

            n = atol(sStr.c_str());
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'number' type, get " + getType(val.GetType()));
        }
    }

    static void read(taf::Float &n, const Value &val, bool isRequire = true)
    {
        if (val.IsNumber()) {
            n = (taf::Float)val.GetFloat();
        } else if (val.IsString()) {
            const string &sStr = val.GetString();

            if (sStr.empty() || !isdigit(sStr[0])) {
                if (isRequire) {
                    throw taf::JceDecodeException(" not 'number' type, get invalid 'string'");
                }

                return;
            }

            n = atof(sStr.c_str());
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'number' type, get " + getType(val.GetType()));
        }
    }

    static void read(taf::Double &n, const Value &val, bool isRequire = true)
    {
        if (val.IsNumber()) {
            n = (taf::Double)val.GetDouble();
        } else if (val.IsString()) {
            const string &sStr = val.GetString();

            if (sStr.empty() || !isdigit(sStr[0])) {
                if (isRequire) {
                    throw taf::JceDecodeException(" not 'number' type, get invalid 'string'");
                }

                return;
            }

            n = atof(sStr.c_str());
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'number' type, get " + getType(val.GetType()));
        }
    }

    static void read(std::string &s, const Value &val, bool isRequire = true)
    {
        if (val.IsString()) {
            s = val.GetString();
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'string' type, get " + getType(val.GetType()));
        }
    }

    static void read(char *buf, const taf::UInt32 bufLen, taf::UInt32 &readLen, const Value &val, bool isRequire = true)
    {
        if (val.IsString()) {
            SizeType strSize = val.GetStringLength();

            if (strSize > bufLen) {
                throw taf::JceDecodeException(" invalid string size");
            }

            memcpy(buf, val.GetString(), strSize);
            readLen = strSize;
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'string' type, get " + getType(val.GetType()));
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void read(std::map<std::string, V, Cmp, Alloc> &m, const Value &val, bool isRequire = true)
    {
        if (val.IsObject()) {
            Value::ConstMemberIterator iter = val.MemberBegin();

            for (; iter != val.MemberEnd(); ++iter) {
                std::pair<std::string, V> pr;
                pr.first = iter->name.GetString();
                read(pr.second, iter->value);
                m.insert(pr);
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'object' type, get " + getType(val.GetType()));
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void read(std::map<taf::Bool, V, Cmp, Alloc> &m, const Value &val, bool isRequire = true)
    {
        if (val.IsObject()) {
            Value::ConstMemberIterator iter = val.MemberBegin();

            for (; iter != val.MemberEnd(); ++iter) {
                std::pair<taf::Bool, V> pr;
                pr.first = taf::TC_Common::strto<taf::Bool>(iter->name.GetString());
                read(pr.second, iter->value);
                m.insert(pr);
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'object' type, get " + getType(val.GetType()));
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void read(std::map<taf::Char, V, Cmp, Alloc> &m, const Value &val, bool isRequire = true)
    {
        if (val.IsObject()) {
            Value::ConstMemberIterator iter = val.MemberBegin();

            for (; iter != val.MemberEnd(); ++iter) {
                std::pair<taf::Char, V> pr;
                pr.first = taf::TC_Common::strto<taf::Int32>(iter->name.GetString());
                read(pr.second, iter->value);
                m.insert(pr);
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'object' type, get " + getType(val.GetType()));
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void read(std::map<taf::UInt8, V, Cmp, Alloc> &m, const Value &val, bool isRequire = true)
    {
        if (val.IsObject()) {
            Value::ConstMemberIterator iter = val.MemberBegin();

            for (; iter != val.MemberEnd(); ++iter) {
                std::pair<taf::UInt8, V> pr;
                pr.first = taf::TC_Common::strto<taf::UInt32>(iter->name.GetString());
                read(pr.second, iter->value);
                m.insert(pr);
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'object' type, get " + getType(val.GetType()));
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void read(std::map<taf::Short, V, Cmp, Alloc> &m, const Value &val, bool isRequire = true)
    {
        if (val.IsObject()) {
            Value::ConstMemberIterator iter = val.MemberBegin();

            for (; iter != val.MemberEnd(); ++iter) {
                std::pair<taf::Short, V> pr;
                pr.first = taf::TC_Common::strto<taf::Short>(iter->name.GetString());
                read(pr.second, iter->value);
                m.insert(pr);
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'object' type, get " + getType(val.GetType()));
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void read(std::map<taf::UInt16, V, Cmp, Alloc> &m, const Value &val, bool isRequire = true)
    {
        if (val.IsObject()) {
            Value::ConstMemberIterator iter = val.MemberBegin();

            for (; iter != val.MemberEnd(); ++iter) {
                std::pair<taf::UInt16, V> pr;
                pr.first = taf::TC_Common::strto<taf::UInt16>(iter->name.GetString());
                read(pr.second, iter->value);
                m.insert(pr);
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'object' type, get " + getType(val.GetType()));
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void read(std::map<taf::Int32, V, Cmp, Alloc> &m, const Value &val, bool isRequire = true)
    {
        if (val.IsObject()) {
            Value::ConstMemberIterator iter = val.MemberBegin();

            for (; iter != val.MemberEnd(); ++iter) {
                std::pair<taf::Int32, V> pr;
                pr.first = taf::TC_Common::strto<taf::Int32>(iter->name.GetString());
                read(pr.second, iter->value);
                m.insert(pr);
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'object' type, get " + getType(val.GetType()));
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void read(std::map<taf::UInt32, V, Cmp, Alloc> &m, const Value &val, bool isRequire = true)
    {
        if (val.IsObject()) {
            Value::ConstMemberIterator iter = val.MemberBegin();

            for (; iter != val.MemberEnd(); ++iter) {
                std::pair<taf::UInt32, V> pr;
                pr.first = taf::TC_Common::strto<taf::UInt32>(iter->name.GetString());
                read(pr.second, iter->value);
                m.insert(pr);
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'object' type, get " + getType(val.GetType()));
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void read(std::map<taf::Int64, V, Cmp, Alloc> &m, const Value &val, bool isRequire = true)
    {
        if (val.IsObject()) {
            Value::ConstMemberIterator iter = val.MemberBegin();

            for (; iter != val.MemberEnd(); ++iter) {
                std::pair<taf::Int64, V> pr;
                pr.first = taf::TC_Common::strto<taf::Int64>(iter->name.GetString());
                read(pr.second, iter->value);
                m.insert(pr);
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'object' type, get " + getType(val.GetType()));
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void read(std::map<taf::Float, V, Cmp, Alloc> &m, const Value &val, bool isRequire = true)
    {
        if (val.IsObject()) {
            Value::ConstMemberIterator iter = val.MemberBegin();

            for (; iter != val.MemberEnd(); ++iter) {
                std::pair<taf::Float, V> pr;
                pr.first = taf::TC_Common::strto<taf::Float>(iter->name.GetString());
                read(pr.second, iter->value);
                m.insert(pr);
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'object' type, get " + getType(val.GetType()));
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void read(std::map<taf::Double, V, Cmp, Alloc> &m, const Value &val, bool isRequire = true)
    {
        if (val.IsObject()) {
            Value::ConstMemberIterator iter = val.MemberBegin();

            for (; iter != val.MemberEnd(); ++iter) {
                std::pair<taf::Double, V> pr;
                pr.first = taf::TC_Common::strto<taf::Double>(iter->name.GetString());
                read(pr.second, iter->value);
                m.insert(pr);
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'object' type, get " + getType(val.GetType()));
        }
    }

    template<typename K, typename V, typename Cmp, typename Alloc>
    static void read(std::map<K, V, Cmp, Alloc> &m, const Value &val, bool isRequire = true)
    {
        if (val.IsObject()) {
            Value::ConstMemberIterator iter = val.MemberBegin();

            for (; iter != val.MemberEnd(); ++iter) {
                std::pair<K, V> pr;
                rapidjson::Document _jDoc;
                _jDoc.Parse(iter->name.GetString());

                if (_jDoc.GetParseError() != rapidjson::kParseErrorNone) {
                    throw taf::JceDecodeException(" parse member name failed, error code: " + taf::TC_Common::tostr(_jDoc.GetParseError()) + ". name: " + iter->name.GetString());
                }

                read(pr.first, _jDoc);
                read(pr.second, iter->value);
                m.insert(pr);
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'object' type, get " + getType(val.GetType()));
        }
    }

    template<typename T, typename Alloc>
    static void read(std::vector<T, Alloc> &v, const Value &val, bool isRequire = true)
    {
        if (val.IsArray()) {
            Value::ConstValueIterator iter = val.Begin();

            for (; iter != val.End(); ++iter) {
                T t;
                read(t, *iter);
                v.push_back(t);
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'array' type, get " + getType(val.GetType()));
        }
    }

    /// 读取结构数组
    template<typename T>
    static void read(T *v, const taf::UInt32 len, taf::UInt32 &readLen, const Value &val, bool isRequire = true)
    {
        if (val.IsArray()) {
            if (val.Size() > len) {
                throw taf::JceDecodeException("invalid array size");
            }

            SizeType i = 0;

            for (; i < val.Size(); ++i) {
                read(v[i], val[i]);
            }

            readLen = i;
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'array' type, get " + getType(val.GetType()));
        }
    }

    template<typename T>
    static void read(T &v, const Value &val, bool isRequire = true, typename taf::jce::disable_if<taf::jce::is_convertible<T *, taf::JceStructBase *>, void ***>::type dummy = 0)
    {
        taf::Int32 n = 0;
        read(n, val, isRequire);
        v = (T) n;
    }

    /// 读取结构
    template<typename T>
    static void read(T &v, const Value &val, bool isRequire = true, typename taf::jce::enable_if<taf::jce::is_convertible<T *, taf::JceStructBase *>, void ***>::type dummy = 0)
    {
        if (val.IsObject()) {
            v.readFromJson(val);
        } else if (isRequire) {
            throw taf::JceDecodeException(" not 'object' type, get " + getType(val.GetType()));
        }
    }

    static const Value &getMember(const Value &obj, const char *name, bool isRequire = true)
    {
        Document doc;
        Value::ConstMemberIterator it;

        if ((it = obj.FindMember(name)) != obj.MemberEnd()) {
            return it->value;
        } else if (isRequire) {
            throw taf::JceDecodeException(" object member not found: " + string(name));
        }

        static Value NullValue;
        return NullValue;
    }

    template<typename T>
    static void getMember(const Value &obj, const char *key, T &val, bool isRequire = true)
    {
        Value::ConstMemberIterator it;

        if ((it = obj.FindMember(key)) != obj.MemberEnd()) {
            try {
                read(val, it->value, isRequire);
            } catch (taf::JceDecodeException &ex) {
                throw taf::JceDecodeException(string(key) + "->" + ex.what());
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(string(key) + ": object member not found");
        }
    }

    template<typename T>
    static void getMember(const Value &obj, const char *key, T *val, const taf::UInt32 len, taf::UInt32 &readLen, bool isRequire = true)
    {
        Value::ConstMemberIterator it;

        if ((it = obj.FindMember(key)) != obj.MemberEnd()) {
            try {
                read(val, len, readLen, it->value, isRequire);
            } catch (taf::JceDecodeException &ex) {
                throw taf::JceDecodeException(string(key) + "->" + ex.what());
            }
        } else if (isRequire) {
            throw taf::JceDecodeException(string(key) + ": object member not found");
        }
    }
};

class Output
{
public:
    static void write(Value &val, Document::AllocatorType &alloc, taf::Bool b)
    {
        val.SetBool(b);
    }

    static void write(Value &val, Document::AllocatorType &alloc, taf::Char n)
    {
        val.SetInt(n);
    }

    static void write(Value &val, Document::AllocatorType &alloc, taf::UInt8 n)
    {
        val.SetUint(n);
    }

    static void write(Value &val, Document::AllocatorType &alloc, taf::Short n)
    {
        val.SetInt(n);
    }

    static void write(Value &val, Document::AllocatorType &alloc, taf::UInt16 n)
    {
        val.SetUint(n);
    }

    static void write(Value &val, Document::AllocatorType &alloc, taf::Int32 n)
    {
        val.SetInt(n);
    }

    static void write(Value &val, Document::AllocatorType &alloc, taf::UInt32 n)
    {
        val.SetUint(n);
    }

    static void write(Value &val, Document::AllocatorType &alloc, taf::Int64 n)
    {
        val.SetInt64(n);
    }

    static void write(Value &val, Document::AllocatorType &alloc, taf::Float n)
    {
        val.SetFloat(n);
    }

    static void write(Value &val, Document::AllocatorType &alloc, taf::Double n)
    {
        val.SetDouble(n);
    }

    static void write(Value &val, Document::AllocatorType &alloc, const std::string &s)
    {
        val.SetString(s.data(), s.length());
    }

    static void write(Value &val, Document::AllocatorType &alloc, const char *buf, const taf::UInt32 len)
    {
        val.SetString(buf, len);
    }

    template<typename V, typename Cmp, typename Alloc>
    static void write(Value &val, Document::AllocatorType &alloc, const std::map<std::string, V, Cmp, Alloc> &m)
    {
        val.SetObject();
        typedef typename std::map<std::string, V, Cmp, Alloc>::const_iterator IT;

        for (IT i = m.begin(); i != m.end(); ++i) {
            Value vVal;
            write(vVal, alloc, i->second);
            val.AddMember(Value(i->first.c_str(), alloc).Move(), vVal, alloc);
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void write(Value &val, Document::AllocatorType &alloc, const std::map<taf::Bool, V, Cmp, Alloc> &m)
    {
        val.SetObject();
        typedef typename std::map<taf::Bool, V, Cmp, Alloc>::const_iterator IT;

        for (IT i = m.begin(); i != m.end(); ++i) {
            Value vVal;
            write(vVal, alloc, i->second);
            val.AddMember(Value(taf::TC_Common::tostr(i->first).c_str(), alloc).Move(), vVal, alloc);
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void write(Value &val, Document::AllocatorType &alloc, const std::map<taf::Char, V, Cmp, Alloc> &m)
    {
        val.SetObject();
        typedef typename std::map<taf::Char, V, Cmp, Alloc>::const_iterator IT;

        for (IT i = m.begin(); i != m.end(); ++i) {
            Value vVal;
            write(vVal, alloc, i->second);
            val.AddMember(Value(taf::TC_Common::tostr((taf::Int32)i->first).c_str(), alloc).Move(), vVal, alloc);
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void write(Value &val, Document::AllocatorType &alloc, const std::map<taf::UInt8, V, Cmp, Alloc> &m)
    {
        val.SetObject();
        typedef typename std::map<taf::UInt8, V, Cmp, Alloc>::const_iterator IT;

        for (IT i = m.begin(); i != m.end(); ++i) {
            Value vVal;
            write(vVal, alloc, i->second);
            val.AddMember(Value(taf::TC_Common::tostr((taf::UInt32)i->first).c_str(), alloc).Move(), vVal, alloc);
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void write(Value &val, Document::AllocatorType &alloc, const std::map<taf::Short, V, Cmp, Alloc> &m)
    {
        val.SetObject();
        typedef typename std::map<taf::Short, V, Cmp, Alloc>::const_iterator IT;

        for (IT i = m.begin(); i != m.end(); ++i) {
            Value vVal;
            write(vVal, alloc, i->second);
            val.AddMember(Value(taf::TC_Common::tostr(i->first).c_str(), alloc).Move(), vVal, alloc);
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void write(Value &val, Document::AllocatorType &alloc, const std::map<taf::UInt16, V, Cmp, Alloc> &m)
    {
        val.SetObject();
        typedef typename std::map<taf::UInt16, V, Cmp, Alloc>::const_iterator IT;

        for (IT i = m.begin(); i != m.end(); ++i) {
            Value vVal;
            write(vVal, alloc, i->second);
            val.AddMember(Value(taf::TC_Common::tostr(i->first).c_str(), alloc).Move(), vVal, alloc);
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void write(Value &val, Document::AllocatorType &alloc, const std::map<taf::Int32, V, Cmp, Alloc> &m)
    {
        val.SetObject();
        typedef typename std::map<taf::Int32, V, Cmp, Alloc>::const_iterator IT;

        for (IT i = m.begin(); i != m.end(); ++i) {
            Value vVal;
            write(vVal, alloc, i->second);
            val.AddMember(Value(taf::TC_Common::tostr(i->first).c_str(), alloc).Move(), vVal, alloc);
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void write(Value &val, Document::AllocatorType &alloc, const std::map<taf::UInt32, V, Cmp, Alloc> &m)
    {
        val.SetObject();
        typedef typename std::map<taf::UInt32, V, Cmp, Alloc>::const_iterator IT;

        for (IT i = m.begin(); i != m.end(); ++i) {
            Value vVal;
            write(vVal, alloc, i->second);
            val.AddMember(Value(taf::TC_Common::tostr(i->first).c_str(), alloc).Move(), vVal, alloc);
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void write(Value &val, Document::AllocatorType &alloc, const std::map<taf::Int64, V, Cmp, Alloc> &m)
    {
        val.SetObject();
        typedef typename std::map<taf::Int64, V, Cmp, Alloc>::const_iterator IT;

        for (IT i = m.begin(); i != m.end(); ++i) {
            Value vVal;
            write(vVal, alloc, i->second);
            val.AddMember(Value(taf::TC_Common::tostr(i->first).c_str(), alloc).Move(), vVal, alloc);
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void write(Value &val, Document::AllocatorType &alloc, const std::map<taf::Float, V, Cmp, Alloc> &m)
    {
        val.SetObject();
        typedef typename std::map<taf::Float, V, Cmp, Alloc>::const_iterator IT;

        for (IT i = m.begin(); i != m.end(); ++i) {
            Value vVal;
            write(vVal, alloc, i->second);
            val.AddMember(Value(taf::TC_Common::tostr(i->first).c_str(), alloc).Move(), vVal, alloc);
        }
    }

    template<typename V, typename Cmp, typename Alloc>
    static void write(Value &val, Document::AllocatorType &alloc, const std::map<taf::Double, V, Cmp, Alloc> &m)
    {
        val.SetObject();
        typedef typename std::map<taf::Double, V, Cmp, Alloc>::const_iterator IT;

        for (IT i = m.begin(); i != m.end(); ++i) {
            Value vVal;
            write(vVal, alloc, i->second);
            val.AddMember(Value(taf::TC_Common::tostr(i->first).c_str(), alloc).Move(), vVal, alloc);
        }
    }

    template<typename K, typename V, typename Cmp, typename Alloc>
    static void write(Value &val, Document::AllocatorType &alloc, const std::map<K, V, Cmp, Alloc> &m)
    {
        val.SetObject();
        typedef typename std::map<K, V, Cmp, Alloc>::const_iterator IT;

        for (IT i = m.begin(); i != m.end(); ++i) {
            rapidjson::Document _jDoc;
            write(_jDoc, _jDoc.GetAllocator(), i->first);
            rapidjson::StringBuffer _jBuf;
            rapidjson::Writer<rapidjson::StringBuffer> _jWriter(_jBuf);
            _jDoc.Accept(_jWriter);

            Value vVal;
            write(vVal, alloc, i->second);
            val.AddMember(Value(_jBuf.GetString(), alloc).Move(), vVal, alloc);
        }
    }

    template<typename T, typename Alloc>
    static void write(Value &val, Document::AllocatorType &alloc, const std::vector<T, Alloc> &v)
    {
        val.SetArray();
        typedef typename std::vector<T, Alloc>::const_iterator IT;

        for (IT i = v.begin(); i != v.end(); ++i) {
            Value sval;
            write(sval, alloc, *i);
            val.PushBack(sval, alloc);
        }
    }

    template<typename T>
    static void write(Value &val, Document::AllocatorType &alloc, const T *v, const taf::UInt32 len)
    {
        val.SetArray();

        for (taf::UInt32 i = 0; i < len; ++i) {
            Value sval;
            write(sval, alloc, v[i]);
            val.PushBack(sval, alloc);
        }
    }

    template<typename T>
    static void write(Value &val, Document::AllocatorType &alloc, const T &v, typename taf::jce::disable_if<taf::jce::is_convertible<T *, taf::JceStructBase *>, void ***>::type dummy = 0)
    {
        write(val, alloc, (taf::Int32) v);
    }

    template<typename T>
    static void write(Value &val, Document::AllocatorType &alloc, const T &v, typename taf::jce::enable_if<taf::jce::is_convertible<T *, taf::JceStructBase *>, void ***>::type dummy = 0)
    {
        v.writeToJson(val, alloc);
    }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
}


#endif//__JCE2RJSON_H__
