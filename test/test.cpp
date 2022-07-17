#include <iostream>
#include "example.pb.h"
#include "protobuf_utils.h" 
#include "gtest/gtest.h"

class ProtobufUtilsTest : public ::testing::Test {
protected:
    void SetUp() override {
        person.set_name("Tom");
        person.set_age(10);
        person.mutable_address()->set_street("St.honolulu");
        person.mutable_address()->set_number(999);
        person.add_score(88);
        person.add_friends()->set_name("Jessica");
    }

protected:
    example::Person person;
};

TEST_F(ProtobufUtilsTest, set_message_field) {
    EXPECT_EQ(0, ::garden::protobuf_utils::set_message_field("name", "Kira", &person, true));
    EXPECT_EQ("Kira", person.name());

    EXPECT_EQ(0, ::garden::protobuf_utils::set_message_field("address.street", "St.louis", &person, true));
    EXPECT_EQ("St.louis", person.address().street());

    uint32_t number = 39;
    EXPECT_EQ(0, ::garden::protobuf_utils::set_message_field("address.number", 
            std::string{reinterpret_cast<char*>(&number), sizeof(int)}, &person, true));
    EXPECT_EQ(number, person.address().number());

    std::cout << person.ShortDebugString() << std::endl;

    // failed
    // 没有对应字段名
    EXPECT_NE(0, ::garden::protobuf_utils::set_message_field("name2", "Kira2", &person, true));
    // is_force=false && 字段已被设定过
    EXPECT_NE(0, ::garden::protobuf_utils::set_message_field("name", "Elieen", &person, false));
}

TEST_F(ProtobufUtilsTest, get_message_field) {
    std::string getv;
    EXPECT_EQ(0, ::garden::protobuf_utils::get_message_field("name", person, &getv));
    EXPECT_EQ(person.name(), getv);

    EXPECT_EQ(0, ::garden::protobuf_utils::get_message_field("address.street", person, &getv));
    EXPECT_EQ(person.address().street(), getv);
}

TEST_F(ProtobufUtilsTest, copy_message_field) {
    example::Person replicant;
    EXPECT_EQ(0, ::garden::protobuf_utils::copy_message_field("name", &person, &replicant));
    EXPECT_EQ(person.name(), replicant.name());

    EXPECT_EQ(0, ::garden::protobuf_utils::copy_message_field("address.street", &person, &replicant));
    EXPECT_EQ(person.address().street(), replicant.address().street());
}

TEST_F(ProtobufUtilsTest, add_repeated_field) {
    EXPECT_EQ(0, ::garden::protobuf_utils::add_repeated_field("score", "", &person));
    EXPECT_EQ(2, person.score_size());

    int32_t number = 4;
    EXPECT_EQ(0, ::garden::protobuf_utils::add_repeated_field("score", 
            std::string{reinterpret_cast<char*>(&number), sizeof(int32_t)}, &person));
    EXPECT_EQ(3, person.score_size());
    EXPECT_EQ(number, person.score(2));

    EXPECT_EQ(0, ::garden::protobuf_utils::add_repeated_field("friends", "", &person));
    EXPECT_EQ(2, person.friends_size());

    example::Friend frd;
    frd.set_name("Maria");
    EXPECT_EQ(0, ::garden::protobuf_utils::add_repeated_field("friends", frd.SerializeAsString(), &person));
    EXPECT_EQ(3, person.friends_size());
    EXPECT_EQ(frd.name(), person.friends(2).name());

    std::cout << person.ShortDebugString() << std::endl;
}