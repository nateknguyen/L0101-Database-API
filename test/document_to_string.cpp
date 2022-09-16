#include <gtest/gtest.h>
#include "database.h"

TEST(DocToString, EmptyData)
{
    std::string data = "{}";
    std::string expected = "Document 0: {}";
    Document d(data);
    EXPECT_EQ(d.to_string(), expected) << "Failed to format empty object string";
}

TEST(DocToString, BasicData)
{
    std::string data = R"({"String":"foobar","Number":3.14159265358979,"Array":[1,2,3,4,5],"Object":{"field1":"data1","field2":"data2","field3":"data3"},"BoolTrue":true,"BoolFalse":false,"Null":null})";
    std::string expected = "Document 1: {\n\tString: \"foobar\",\n\tNumber: 3.14159265358979,\n\tArray: [\n\t\t1,\n\t\t2,\n\t\t3,\n\t\t4,\n\t\t5\n\t],\n\tObject: {\n\t\tfield1: \"data1\",\n\t\tfield2: \"data2\",\n\t\tfield3: \"data3\"\n\t},\n\tBoolTrue: true,\n\tBoolFalse: false,\n\tNull: null\n}";
    Document d(data);
    EXPECT_EQ(d.to_string(), expected) << "Failed to format basic object string";
}

TEST(DocToString, ArrayOfObjectData)
{
    std::string data = R"({"Array":[{"Foo":"foo","Bar":"bar","Baz":"baz","Quux":"quux"},{"Foo":"foo","Bar":"bar","Baz":"baz","Quux":"quux"},{"Foo":"foo","Bar":"bar","Baz":"baz","Quux":"quux"}]})";
    std::string expected = "Document 2: {\n\tArray: [\n\t\t{\n\t\t\tFoo: \"foo\",\n\t\t\tBar: \"bar\",\n\t\t\tBaz: \"baz\",\n\t\t\tQuux: \"quux\"\n\t\t},\n\t\t{\n\t\t\tFoo: \"foo\",\n\t\t\tBar: \"bar\",\n\t\t\tBaz: \"baz\",\n\t\t\tQuux: \"quux\"\n\t\t},\n\t\t{\n\t\t\tFoo: \"foo\",\n\t\t\tBar: \"bar\",\n\t\t\tBaz: \"baz\",\n\t\t\tQuux: \"quux\"\n\t\t}\n\t]\n}";
    Document d(data);
    EXPECT_EQ(d.to_string(), expected) << "Failed to format array of objects string";
}

TEST(DocToString, ObjectOfArraysData)
{
    std::string data = R"({"Object":{"Array1":[],"Array2":["string1","string2","string3"],"Array3":[[0,1,2],[3,4,5],[6,7,8],[9,10,11],[12,13,14]]}})";
    std::string expected = "Document 3: {\n\tObject: {\n\t\tArray1: [],\n\t\tArray2: [\n\t\t\t\"string1\",\n\t\t\t\"string2\",\n\t\t\t\"string3\"\n\t\t],\n\t\tArray3: [\n\t\t\t[\n\t\t\t\t0,\n\t\t\t\t1,\n\t\t\t\t2\n\t\t\t],\n\t\t\t[\n\t\t\t\t3,\n\t\t\t\t4,\n\t\t\t\t5\n\t\t\t],\n\t\t\t[\n\t\t\t\t6,\n\t\t\t\t7,\n\t\t\t\t8\n\t\t\t],\n\t\t\t[\n\t\t\t\t9,\n\t\t\t\t10,\n\t\t\t\t11\n\t\t\t],\n\t\t\t[\n\t\t\t\t12,\n\t\t\t\t13,\n\t\t\t\t14\n\t\t\t]\n\t\t]\n\t}\n}";
    Document d(data);
    EXPECT_EQ(d.to_string(), expected) << "Failed to format object of arrays string";
}
