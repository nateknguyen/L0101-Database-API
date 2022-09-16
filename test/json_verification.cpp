#include <gtest/gtest.h>

#include "database.h"

TEST(JsonVerification, EmptyString) // ""
{
	//std::string json = "";
    auto res = verify_json("");
    EXPECT_TRUE(res) << "Empty string didn't fail";
}

TEST(JsonVerification, EmptyJson) // "{}"
{
	std::string json = "{}";
    auto res = verify_json(json);
	EXPECT_FALSE(res) << "Failed when passed empty json object";
}

TEST(JsonVerification, BasicJson) // no arrays or objects
{	
	std::string json = R"({"String":"string","Number":3.14159,"True":true,"False":false,"Null":null})";
    auto res = verify_json(json);
	EXPECT_FALSE(res) << "Basic json object did not succeed";
}

TEST(JsonVerification, JsonWithArray) // contains array
{
	std::string json = R"({"arrayField":["Hello",123,0.12,true,false,null]})";
    auto res = verify_json(json);
	EXPECT_FALSE(res) << "Threw with json containing array";
}

TEST(JsonVerification, JsonWithObject) // contains object
{
	std::string json = R"({"objectField":{"String":"string","Number":3.14159,"True":true,"False":false,"Null":null}})";
    auto res = verify_json(json);
	EXPECT_FALSE(res) << "Threw with json containing object";
}

TEST(JsonVerification, NestedArray) // array containing array and object
{
	std::string json = R"({"array":[[1,2,3,4,5],{"foo":true,"bar":false}]})";
    auto res = verify_json(json);
	EXPECT_FALSE(res) << "Threw with json containing array of complex types";
}

TEST(JsonVerification, NextedObject) // object containing array and object
{
	std::string json = R"({"object":{"object":{"foo":true,"bar":false},"array":[1,2,3,4,5]}})";
    auto res = verify_json(json);
	EXPECT_FALSE(res) << "Threw with json containing object of complex types";
}

TEST(JsonVerification, MissingTopLevelBrace) // {... and ...}
{
	std::string json1 = R"("foo":true,"bar":false})";
	std::string json2 = R"({"foo":true,"bar":false)";
    auto res1 = verify_json(json1);
    auto res2 = verify_json(json2);
	EXPECT_TRUE(res1) << "string missing initial brace didn't throw";
	EXPECT_TRUE(res2) << "string missing closing brace didn't throw";
}

TEST(JsonVerification, MissingKVPair) // {"...":"...",}
{
	std::string json = R"({"foo":"string","bar":42,})";
    auto res = verify_json(json);
	EXPECT_TRUE(res) << "string missing kv-pair didn't throw";
}

TEST(JsonVerification, MissingInternalComma) //{"...":foo"...":bar} 
{
	std::string json = R"({"foo":42"bar":3.14159})";
    auto res = verify_json(json);
	EXPECT_TRUE(res) << "string missing internal comma didn't throw";
}

TEST(JsonVerification, MissingColon) // {"field"foo}
{
	std::string json = R"({"foo"42})";
    auto res = verify_json(json);
	EXPECT_TRUE(res) << "string missing colon didn't throw";
}

TEST(JsonVerification, MissingKey) // {:"..."}
{
	std::string json1 = R"({:"value"})";
	std::string json2 = R"({"foo":42,:"value"})";
    auto res1 = verify_json(json1);
    auto res2 = verify_json(json2);
	EXPECT_TRUE(res1) << "string missing first key didn't throw";
	EXPECT_TRUE(res2) << "string missing internal key didn't throw";
}

TEST(JsonVerification, EmptyKey) // {"":foo}
{
	std::string json = R"({"":value})";
    auto res = verify_json(json);
	EXPECT_TRUE(res) << "string with empty key didn't throw";
}

TEST(JsonVerification, MissingValue) // {"...":}
{
	std::string json1 = R"({"key":})";
	std::string json2 = R"({"key1":"value1","key2":,"key3":"value3"})";
    auto res1 = verify_json(json1);
    auto res2 = verify_json(json2);
	EXPECT_TRUE(res1) << "string missing final value didn't throw";
	EXPECT_TRUE(res2) << "string missing internal value didn't throw";
}

TEST(JsonVerification, MissingStringFieldQuote) // {"...":"...} and {"...":..."}
{
	std::string json1 = R"({"field1":"string1","field2":"string2,"field3":"string3"})";
	std::string json2 = R"({"field1":"string1","field2":string2","field3":"string3"})";
    auto res1 = verify_json(json1);
    auto res2 = verify_json(json2);
	EXPECT_TRUE(res1) << "string missing opening field quote didn't throw";
	EXPECT_TRUE(res2) << "string missing closing field quote didn't throw";
}

TEST(JsonVerification, MissingArrayBracket) // {"...":[...} and {"...":...]}
{
	std::string json1 = R"({"Array":[foo,bar,baz,"field2":"string"})";
	std::string json2 = R"({"Array":foo,bar,baz],"field2":"string"})";
    auto res1 = verify_json(json1);
    auto res2 = verify_json(json2);
	EXPECT_TRUE(res1) << "string missing closing array bracket didn't throw";
	EXPECT_TRUE(res2) << "string missing opening array bracket didn't throw";
}

TEST(JsonVerification, MissingArrayField) // {"...":[...,...,]}
{
	std::string json1 = R"({"Array":[1,2,3,,5]})";
	std::string json2 = R"({"Array":[1,2,3,]})";
	std::string json3 = R"({"Array":[,1,2,3,4]})";
    auto res1 = verify_json(json1);
    auto res2 = verify_json(json2);
    auto res3 = verify_json(json3);
	EXPECT_TRUE(res1) << "string missing internal array value didn't throw";
	EXPECT_TRUE(res2) << "string missing last array value didn't throw";
	EXPECT_TRUE(res3) << "string initial array value didn't throw";

}

TEST(JsonVerification, MissingArrayInternalComma) // {"...":[...,......]}
{
	std::string json = R"({"Array":["foo","bar""baz","quux"]})";
    auto res = verify_json(json);
	EXPECT_TRUE(res) << "string missing array comma didn't throw";
}

TEST(JsonVerification, MissingObjectBracket) // {"...":{...} and {"...":...}}
{
	std::string json1 = R"({{"field":{"subfield1":true,"subfield2":42,"field2":123})";
	std::string json2 = R"({{"field":"subfield1":true,"subfield2":42},"field2":123})";
    auto res1 = verify_json(json1);
    auto res2 = verify_json(json2);
	EXPECT_TRUE(res1) << "string missing closing object brace didn't throw";
	EXPECT_TRUE(res2) << "string missing opening object brace didn't throw";
}

TEST(JsonVerification, IncorrectlyFormattedNumber) // number contains illegal characters/bad formatting
{
	std::string json1 = R"({"bad_number":-123d5})";
	std::string json2 = R"({"bad_number":1.23e})";
	std::string json3 = R"({"bad_number":e42})";
    auto res1 = verify_json(json1);
    auto res2 = verify_json(json2);
    auto res3 = verify_json(json3);
	EXPECT_TRUE(res1) << "non legal character in number didn't throw";
	EXPECT_TRUE(res2) << "missing exponent didn't throw";
	EXPECT_TRUE(res3) << "missing mantissa didn't throw";
}

TEST(JsonVerification, OtherIncorrectData) // {"...":nottrue/false/null}
{
	std::string json = R"({"field":foo})";
    auto res = verify_json(json);
	EXPECT_TRUE(res) << "non-legal value didn't throw";
}
