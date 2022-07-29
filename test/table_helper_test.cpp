#include <iostream>
#include "example.pb.h"
#include "table_helper.h" 
#include "gtest/gtest.h"

class TableHelperTest : public ::testing::Test {
protected:
    void SetUp() override {
        person.set_name("Tom");
        person.set_age(10);
        person.mutable_address()->set_street("St.honolulu");
        person.mutable_address()->set_number(999);
        person.add_score(88);
        person.add_friends()->set_name("Jessica");

        kvs.emplace_back(std::make_tuple("person1", "cf0", "name", 0), 
                std::make_tuple(google::protobuf::FieldDescriptor::CPPTYPE_STRING, "John"));
        kvs.emplace_back(std::make_tuple("person1", "cf0", "address.street", 0), 
                std::make_tuple(google::protobuf::FieldDescriptor::CPPTYPE_STRING, "St.louis"));
        int number = 101;
        kvs.emplace_back(std::make_tuple("person1", "cf0", "address.number", 0), 
                std::make_tuple(google::protobuf::FieldDescriptor::CPPTYPE_INT32, 
                    std::string{reinterpret_cast<char*>(&number), sizeof(int32_t)}));
    }

protected:
    example::Person person;
    std::vector<garden::table_helper::KV> kvs;
};

TEST_F(TableHelperTest, kvs_to_message) {
    example::Person person1;
    ASSERT_EQ(0, garden::table_helper::kvs_to_message(kvs, &person1));
    std::cout << "kvs_to_message, person1:" << person1.ShortDebugString() << std::endl;
    EXPECT_EQ("John", person1.name());
    EXPECT_EQ("St.louis", person1.address().street());
    EXPECT_EQ(101, person1.address().number());
}

TEST_F(TableHelperTest, message_to_kvs) {
    std::string rowkey = "rowkey17";
    std::vector<garden::table_helper::KV> kvs1;
    ASSERT_EQ(0, garden::table_helper::message_to_kvs(rowkey, person, &kvs1));
    EXPECT_EQ(6, kvs1.size());
    EXPECT_EQ(rowkey, std::get<0>(kvs1[0].first));
    EXPECT_EQ("cf0", std::get<1>(kvs1[0].first));
    EXPECT_EQ("name", std::get<2>(kvs1[0].first));
    EXPECT_EQ(person.name(), std::get<1>(kvs1[0].second));

    EXPECT_EQ("address.street", std::get<2>(kvs1[2].first));
    EXPECT_EQ(person.address().street(), std::get<1>(kvs1[2].second));

    EXPECT_EQ("address.number", std::get<2>(kvs1[3].first));
    EXPECT_EQ(person.address().number(), 
            *reinterpret_cast<const int32_t*>(std::get<1>(kvs1[3].second).c_str()));
}