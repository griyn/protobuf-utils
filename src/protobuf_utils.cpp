#include "protobuf_utils.h"
#include "google/protobuf/repeated_field.h"

namespace garden {
namespace protobuf_utils {

ERRTYPE set_message_field(
        const std::string& field, 
        const std::string& value, 
        google::protobuf::Message* message, 
        bool is_force) {
    if (message == nullptr) {
        return ERRTYPE::OUTPUT_IS_NULLPTR;
    }

    auto* desc = message->GetDescriptor();
    auto* refl = message->GetReflection();

    static std::string s_token = ".";
    auto iter = field.find_first_of(s_token);
    // A.B.C 存在嵌套类型的情况
    if (iter != std::string::npos) {
        std::string cur = field.substr(0, iter);
        std::string oth = field.substr(iter + 1);
        auto nested_field_desc = desc->FindFieldByName(cur);
        if (nested_field_desc == nullptr) {
            return ERRTYPE::FIELD_NOT_FOUND;
        }
        if (nested_field_desc->is_repeated()) {
            return ERRTYPE::FIELD_FROM_REPEATED;
        }
        if (nested_field_desc->type() != google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE) {
            return ERRTYPE::FIELD_NOT_FOUND;
        }

        auto* nested_message = refl->MutableMessage(message, nested_field_desc);
        if (nested_message == nullptr) {
            ERRTYPE::INERNAL_ERROR;
        }

        return set_message_field(oth, value, nested_message, is_force);
    }

    // 目标字段
    auto* field_desc = desc->FindFieldByName(field);
    if (field_desc == nullptr) {
        return ERRTYPE::FIELD_NOT_FOUND;
    }

    if (field_desc->is_repeated()) {
        std::unique_ptr<google::protobuf::Message> ptr(message->New());
        if (!ptr->ParsePartialFromString(value)) {
            return ERRTYPE::FIELD_NOT_FOUND;
        }
        message->MergeFrom(*ptr);
        return ERRTYPE::SUCCESS;
    }

    if (!is_force && refl->HasField(*message, field_desc)) {
        return ERRTYPE::FIELD_HAS_SET;
    }

    switch (field_desc->cpp_type()) {
#define SET_PROTOBUF(PBTYPE, PBFUNC, CPPTYPE)                                                \
    case google::protobuf::FieldDescriptor::CPPTYPE_##PBTYPE: {                              \
        refl->PBFUNC(message, field_desc, *reinterpret_cast<const CPPTYPE*>(value.c_str())); \
        break;                                                                               \
    }
        SET_PROTOBUF(DOUBLE, SetDouble, double  );
        SET_PROTOBUF(FLOAT , SetFloat , float   );
        SET_PROTOBUF(INT64 , SetInt64 , int64_t );
        SET_PROTOBUF(UINT64, SetUInt64, uint64_t);
        SET_PROTOBUF(INT32 , SetInt32 , int32_t );
        SET_PROTOBUF(UINT32, SetUInt32, uint32_t);
        SET_PROTOBUF(BOOL  , SetBool  , bool    );
#undef SET_PROTOBUF
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
        refl->SetString(message, field_desc, value);
        break;
    }
    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
        if (!refl->MutableMessage(message, field_desc)->ParseFromString(value)) {
            return ERRTYPE::PARSE_FAILED;
        }
        break;
    }

    default:
        return ERRTYPE::INERNAL_ERROR;
    }

    return ERRTYPE::SUCCESS;
}

ERRTYPE get_message_field(
        const std::string& field, 
        const google::protobuf::Message& message,
        std::string* value) {
    auto* desc = message.GetDescriptor();
    auto* refl = message.GetReflection();

    static std::string s_token = ".";
    auto iter = field.find_first_of(s_token);
    // A.B.C 存在嵌套类型的情况
    if (iter != std::string::npos) {
        std::string cur = field.substr(0, iter);
        std::string oth = field.substr(iter + 1);
        auto nested_field_desc = desc->FindFieldByName(cur);
        if (nested_field_desc == nullptr) {
            return ERRTYPE::FIELD_NOT_FOUND;
        }
        if (nested_field_desc->is_repeated()) {
            return ERRTYPE::FIELD_FROM_REPEATED;
        }
        if (nested_field_desc->type() != google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE) {
            return ERRTYPE::FIELD_NOT_FOUND;
        }

        auto& nested_message = refl->GetMessage(message, nested_field_desc);

        return get_message_field(oth, nested_message, value);
    }

    // 目标字段
    auto* field_desc = desc->FindFieldByName(field);
    if (field_desc == nullptr) {
        return ERRTYPE::FIELD_NOT_FOUND;
    }

    if (field_desc->is_repeated()) {
        if (refl->FieldSize(message, field_desc) == 0) {
            return ERRTYPE::FIELD_HAS_SET;
        }
        std::unique_ptr<google::protobuf::Message> ptr(message.New());
        if (copy_message_field(field, &message, ptr.get())) {
            return ERRTYPE::INERNAL_ERROR;
        }
        google::protobuf::io::StringOutputStream os(value);
        ptr->SerializePartialToZeroCopyStream(&os);
        return ERRTYPE::SUCCESS;
    }

    if (!refl->HasField(message, field_desc)) {
        return ERRTYPE::FIELD_HAS_SET;
    }

    switch (field_desc->cpp_type()) {
#define GET_PROTOBUF(PBTYPE, PBFUNC, CPPTYPE)                           \
    case google::protobuf::FieldDescriptor::CPPTYPE_##PBTYPE: {         \
        CPPTYPE tmp = refl->PBFUNC(message, field_desc);                \
        value->assign(reinterpret_cast<char*>(&tmp), sizeof(CPPTYPE));  \
        break;                                                          \
    }
        GET_PROTOBUF(DOUBLE, GetDouble, double  );
        GET_PROTOBUF(FLOAT , GetFloat , float   );
        GET_PROTOBUF(INT64 , GetInt64 , int64_t );
        GET_PROTOBUF(UINT64, GetUInt64, uint64_t);
        GET_PROTOBUF(INT32 , GetInt32 , int32_t );
        GET_PROTOBUF(UINT32, GetUInt32, uint32_t);
        GET_PROTOBUF(BOOL  , GetBool  , bool    );
#undef GET_PROTOBUF
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
        *value = refl->GetStringReference(message, field_desc, value);
        break;
    }
    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
        *value = refl->GetMessage(message, field_desc).SerializeAsString();
        break;
    }

    default:
        return ERRTYPE::INERNAL_ERROR;
    }

    return ERRTYPE::SUCCESS;
}

ERRTYPE copy_message_field(
        const std::string& field,
        const google::protobuf::Message* src_message,
        google::protobuf::Message* dst_message) {
    if (src_message == nullptr || dst_message == nullptr) {
        return ERRTYPE::OUTPUT_IS_NULLPTR;
    }

    if (src_message->GetDescriptor() != dst_message->GetDescriptor()) {
        return ERRTYPE::FIELD_NOT_SAME;
    }

    auto* desc = src_message->GetDescriptor();
    auto* refl = src_message->GetReflection();

    static std::string s_token = ".";
    auto iter = field.find_first_of(s_token);
    // A.B.C 存在嵌套类型的情况
    if (iter != std::string::npos) {
        std::string cur = field.substr(0, iter);
        std::string oth = field.substr(iter + 1);
        auto nested_field_desc = desc->FindFieldByName(cur);
        if (nested_field_desc == nullptr) {
            return ERRTYPE::FIELD_NOT_FOUND;
        }
        if (nested_field_desc->is_repeated()) {
            return ERRTYPE::FIELD_FROM_REPEATED;
        }
        if (nested_field_desc->type() != google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE) {
            return ERRTYPE::FIELD_NOT_FOUND;
        }

        auto* src_nested_message = &refl->GetMessage(*src_message, nested_field_desc);
        auto* dst_nested_message = refl->MutableMessage(dst_message, nested_field_desc);

        return copy_message_field(oth, src_nested_message, dst_nested_message);
    }

    // 目标字段
    auto* field_desc = desc->FindFieldByName(field);
    if (field_desc == nullptr) {
        return ERRTYPE::FIELD_NOT_FOUND;
    }

    if (field_desc->is_repeated()) {
        int count = refl->FieldSize(*src_message, field_desc);
        for (int i = 0; i < count; ++i) {
            switch (field_desc->cpp_type()) {
#define HANDLE_TYPE(CPPTYPE, METHOD)                                \
    case google::protobuf::FieldDescriptor::CPPTYPE_##CPPTYPE:      \
        refl->Add##METHOD(dst_message, field_desc,                  \
            refl->GetRepeated##METHOD(*src_message, field_desc, i));\
    break;

                HANDLE_TYPE(INT32 , Int32 );
                HANDLE_TYPE(INT64 , Int64 );
                HANDLE_TYPE(UINT32, UInt32);
                HANDLE_TYPE(UINT64, UInt64);
                HANDLE_TYPE(FLOAT , Float );
                HANDLE_TYPE(DOUBLE, Double);
                HANDLE_TYPE(BOOL  , Bool  );
                HANDLE_TYPE(STRING, String);
                HANDLE_TYPE(ENUM  , Enum  );
#undef HANDLE_TYPE
                case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
                refl->AddMessage(dst_message, field_desc)->MergeFrom(
                        refl->GetRepeatedMessage(*src_message, field_desc, i));
                break;
            }
        }
    } else {
        switch (field_desc->cpp_type()) {
#define HANDLE_TYPE(CPPTYPE, METHOD)                            \
    case google::protobuf::FieldDescriptor::CPPTYPE_##CPPTYPE:  \
        refl->Set##METHOD(dst_message, field_desc,              \
            refl->Get##METHOD(*src_message, field_desc));       \
    break;

            HANDLE_TYPE(INT32 , Int32 );
            HANDLE_TYPE(INT64 , Int64 );
            HANDLE_TYPE(UINT32, UInt32);
            HANDLE_TYPE(UINT64, UInt64);
            HANDLE_TYPE(FLOAT , Float );
            HANDLE_TYPE(DOUBLE, Double);
            HANDLE_TYPE(BOOL  , Bool  );
            HANDLE_TYPE(STRING, String);
            HANDLE_TYPE(ENUM  , Enum  );
#undef HANDLE_TYPE

            case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
            refl->MutableMessage(dst_message, field_desc)->MergeFrom(
                    refl->GetMessage(*src_message, field_desc));
            break;
        }
    }

    return ERRTYPE::SUCCESS;
}

ERRTYPE add_repeated_field(
        const std::string& field, 
        const std::string& value, 
        google::protobuf::Message* message) {
    if (message == nullptr) {
        return ERRTYPE::OUTPUT_IS_NULLPTR;
    }

    auto* desc = message->GetDescriptor();
    auto* refl = message->GetReflection();

    static std::string s_token = ".";
    auto iter = field.find_first_of(s_token);
    // A.B.C 存在嵌套类型的情况
    if (iter != std::string::npos) {
        std::string cur = field.substr(0, iter);
        std::string oth = field.substr(iter + 1);
        auto nested_field_desc = desc->FindFieldByName(cur);
        if (nested_field_desc == nullptr) {
            return ERRTYPE::FIELD_NOT_FOUND;
        }
        if (nested_field_desc->is_repeated()) {
            return ERRTYPE::FIELD_FROM_REPEATED;
        }
        if (nested_field_desc->type() != google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE) {
            return ERRTYPE::FIELD_NOT_FOUND;
        }

        auto* nested_message = refl->MutableMessage(message, nested_field_desc);
        if (nested_message == nullptr) {
            ERRTYPE::INERNAL_ERROR;
        }

        return add_repeated_field(oth, value, nested_message);
    }

    // 目标字段
    auto* field_desc = desc->FindFieldByName(field);
    if (field_desc == nullptr) {
        return ERRTYPE::FIELD_NOT_FOUND;
    }

    if (!field_desc->is_repeated()) {
        return ERRTYPE::FIELD_NOT_REPEATED;
    }

    switch (field_desc->cpp_type()) {
#define ADD_PROTOBUF(PBTYPE, PBFUNC, CPPTYPE)                                                \
    case google::protobuf::FieldDescriptor::CPPTYPE_##PBTYPE: {                              \
        refl->PBFUNC(message, field_desc, *reinterpret_cast<const CPPTYPE*>(value.c_str())); \
        break;                                                                               \
    }
        ADD_PROTOBUF(DOUBLE, AddDouble, double  );
        ADD_PROTOBUF(FLOAT , AddFloat , float   );
        ADD_PROTOBUF(INT64 , AddInt64 , int64_t );
        ADD_PROTOBUF(UINT64, AddUInt64, uint64_t);
        ADD_PROTOBUF(INT32 , AddInt32 , int32_t );
        ADD_PROTOBUF(UINT32, AddUInt32, uint32_t);
        ADD_PROTOBUF(BOOL  , AddBool  , bool    );
#undef ADD_PROTOBUF
    case google::protobuf::FieldDescriptor::CPPTYPE_STRING: {
        refl->AddString(message, field_desc, value);
        break;
    }
    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE: {
        if (!refl->AddMessage(message, field_desc)->ParseFromString(value)) {
            return ERRTYPE::PARSE_FAILED;
        }
        break;
    }

    default:
        return ERRTYPE::INERNAL_ERROR;
    }

    return ERRTYPE::SUCCESS;
}

} // protobuf_utils
} // garden