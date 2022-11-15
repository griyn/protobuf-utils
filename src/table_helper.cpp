#include "table_helper.h"
#include "protobuf_utils.h"
#include "table_extensions.pb.h"

namespace garden {
namespace table_helper {

int message_to_kvs(const std::string& rowkey, 
        const ::google::protobuf::Message& message, std::vector<KV>* kvs) {
    return message_to_kvs(rowkey, "", message, kvs);
};

int message_to_kvs(const std::string& rowkey, const std::string& field_path,
        const ::google::protobuf::Message& message, std::vector<KV>* kvs) {
    auto* desc = message.GetDescriptor();
    auto* refl = message.GetReflection();

    for (auto i = 0; i < desc->field_count(); i++) {
        auto* field_desc = desc->field(i);
        // 嵌套的情况
        if (!field_desc->is_repeated() &&
                field_desc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
            std::string path = field_path.empty() ? 
                    field_desc->name() : field_path + "." + field_desc->name();
            int ret = message_to_kvs(rowkey, path, refl->GetMessage(message, field_desc), kvs);
            if (ret != 0) {
                return ret;
            }
        } else {
            std::string value{""};
            int ret = protobuf_utils::get_message_field(field_desc->name(), message, &value);
            if (ret == protobuf_utils::ERRTYPE::FIELD_NOT_SET) {
                // 跳过未设定的字段
                continue;
            }
            if (ret != 0) {
                return ret;
            }
            std::string family("cf0");
            if (field_desc->options().HasExtension(google::protobuf::column_family)) {
                family = field_desc->options().GetExtension(google::protobuf::column_family);
            }
            std::string field_name_with_path = field_desc->name();
            if (!field_path.empty()) {
                field_name_with_path = field_path + "." + field_desc->name();
            }
            // 默认版本号写0
            kvs->emplace_back(std::make_tuple(rowkey, std::move(family), field_name_with_path, 0), 
                std::make_tuple(field_desc->cpp_type(), std::move(value)));
        }
    }
    return 0;
}

// 读库
int kvs_to_message(const std::vector<KV>& kvs, ::google::protobuf::Message* message) {
    for (const KV& kv : kvs) {
        const std::string& field = std::get<2>(kv.first);
        const std::string& value = std::get<1>(kv.second);
        int ret = protobuf_utils::set_message_field(field, value, message);
        if (ret != 0) {
            return ret;
        }
    }
    return 0;
}

} // table_helper
} // garden
