#pragma once

#include <vector>
#include <string>
#include "google/protobuf/message.h"

namespace garden {
namespace protobuf_utils {

enum ERRTYPE {
    // 成功
    SUCCESS             = 0,
    // 错误
    FIELD_NOT_FOUND     = -1,
    FIELD_IS_REPEATED   = -2,
    FIELD_IS_MESSAGE    = -3,
    FIELD_NOT_SAME      = -4,
    FIELD_FROM_REPEATED = -5,
    FIELD_NOT_REPEATED  = -6,
    FIELD_NOT_SET       = -7,
    FIELD_HAS_SET       = -8,
    OUTPUT_IS_NULLPTR   = -9,
    SERIALIZE_FAILED    = -10,
    PARSE_FAILED        = -11,
    INERNAL_ERROR       = -100 // 函数内部错误，逻辑未实现或者不符合预期的异常
};

// 给指定字段设置value, 字段名可以是full_name: A.B.C。
//  field对应字段对应类型行为:
//  数字类型：value从string被强制转换成对应类型；
//  字符串类型：正常使用；
//  数组类型：value作为整体使用，反序列化到对应字段
//  内嵌类型：value作为整体使用，反序列化到对应字段
ERRTYPE set_message_field(
        const std::string& field, 
        const std::string& value, 
        google::protobuf::Message* message,
        bool is_force = false);

ERRTYPE get_message_field(
        const std::string& field, 
        const google::protobuf::Message& message,
        std::string* value);

ERRTYPE copy_message_field(
        const std::string& field,
        const google::protobuf::Message* src_message,
        google::protobuf::Message* dst_message);

// 给数组类型添加元素
//  数字类型：value从string被强制转换成对应类型；
//  字符串类型：正常使用；
//  内嵌类型：value作为整体使用，反序列化到对应字段
ERRTYPE add_repeated_field(
        const std::string& field, 
        const std::string& value, 
        google::protobuf::Message* message);

} // protobuf_utils
} // garden