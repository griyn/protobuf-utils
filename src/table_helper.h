#pragma once

#include "protobuf_utils.h"

namespace garden {
namespace table_helper {

// <(0)key, (1)column_family name, (2)qualifier name, (3)version>
typedef std::tuple<std::string, std::string, std::string, int64_t> Key;

// <type, value>
typedef std::tuple<::google::protobuf::FieldDescriptor::CppType, std::string> Value;

typedef std::pair<Key, Value> KV;

// 写库
int message_to_kvs(const std::string& rowkey, 
        const ::google::protobuf::Message& message, std::vector<KV>* kvs);

int message_to_kvs(const std::string& rowkey, const std::string& field_path,
        const ::google::protobuf::Message& message, std::vector<KV>* kvs);

// 读库
int kvs_to_message(const std::vector<KV>& kvs, ::google::protobuf::Message* message);

} // table_helper
} // garden
