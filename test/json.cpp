#include <gtest/gtest.h>
#include <vector>

#include "database.h"

// ---------------------------------------------------
//  match_quote
// ---------------------------------------------------
TEST(MatchQuote, NullString)
{
    std::string str("");
    EXPECT_EQ(match_quote(str, 0), std::string::npos) << "Failed to return npos if empty string";
}

TEST(MatchQuote, NoQuotes)
{
    std::string str("fdsabtasvasknjm");
    EXPECT_EQ(match_quote(str, 5), std::string::npos) << "Failed to return npos if quote not provided";
}

TEST(MatchQuote, NoClosingQuote)
{
    std::string str(R"(fdsa"dsagsasahfgfasga)");
    EXPECT_EQ(match_quote(str, 4), std::string::npos) << "Failed to return npos if no closing quote";
}

TEST(MatchQuote, BasicString)
{
    std::string str(R"(fdsfads"dsafdsabvsadvasfda"dsafdsaa)");
    size_t first = str.find('"');
    size_t second = str.find('"', first+1);

    EXPECT_EQ(match_quote(str, first), second) << "Failed to locate closing quote with no intermediate escaped quotes";
}

TEST(MatchQuote, EscapedQuotes)
{
    std::string str(R"(fdasfdsewa"adsfdsagads\"fdasgads\"fadgas\"fsdahd"gfsadhds)");
    size_t first = 10;
    size_t second = 48;

    EXPECT_EQ(match_quote(str, first), second) << "Failed to locate closing quote with intermediate escaped quotes";
}

// ---------------------------------------------------
//  match_bracket
// ---------------------------------------------------

TEST(MatchBracket, NullString)
{
    std::string str("");

    EXPECT_EQ(match_bracket(str, 0), std::string::npos) << "Failed to reutrn npos if empty string";
}

TEST(MatchBracket, NoBrackets)
{
    std::string str("fdsabtasvasknjm");
    EXPECT_EQ(match_bracket(str, 5), std::string::npos) << "Failed to return npos if bracket not provided";
}

TEST(MatchBracket, UnbalancedBrackets)
{
    std::string str("fas{adsgsad{aasd}ffdsd");
    EXPECT_EQ(match_bracket(str, 3), std::string::npos) << "Failed to return npos if brackets are unmatched";
}

TEST(MatchBracket, BasicBrackets)
{
    std::string str("fdsa[fdsagdfsgdsf]fasdgas");
    EXPECT_EQ(match_bracket(str, 4), 17) << "Failed to locate closing bracket with no intermediate brackets";
}

TEST(MatchBrakcet, NestedBrackets)
{
    std::string str("fasgsad{fadsgad{gsa{agdfas}fdas{fdawsgfsag}gfds}agfa}afdas");
    EXPECT_EQ(match_bracket(str, 7), 52) << "Failed to locate closing bracket with intermedaite brackets";
}

TEST(MatchBracket, BracketInQuotes)
{
    std::string str(R"(fdsafds{dsagadfa"ghafa}fdsagfdsa"dasgas}asgdas)");
    EXPECT_EQ(match_bracket(str, 7), 39) << "Failed to locate closing bracket with intermediate bracket in string";
}

// ---------------------------------------------------
//  tokenize_json
// ---------------------------------------------------

TEST(TokenizeJson, EmptyString)
{
    std::string json(R"()");
    std::vector<std::string> expected = {};
    auto result = tokenize_json(json);

    EXPECT_EQ(result.size(), expected.size()) << "Tokenized json did not contain expected number of fields";
    for (size_t i = 0; i < result.size(); i++)
    {
        EXPECT_EQ(expected[i], result[i]) << "Parsed value didn't match expected";
    }
}

TEST(TokenizeJson, EmptyJson)
{
    std::string json(R"({})");
    std::vector<std::string> expected = {};
    auto result = tokenize_json(json);

    EXPECT_EQ(result.size(), expected.size()) << "Tokenized json did not contain expected number of fields";
    for (size_t i = 0; i < result.size(); i++)
    {
        EXPECT_EQ(expected[i], result[i]) << "Parsed value didn't match expected";
    }
}

TEST(TokenizeJson, NumberFields)
{
    std::string json(R"({"Field1":1,"Field2":.123,"Field3":2.99e8,"Field4":-3.14159265358979})");
    std::vector<std::string> expected = {"Field1","1","Field2",".123","Field3","2.99e8","Field4","-3.14159265358979"};
    auto result = tokenize_json(json);

    EXPECT_EQ(result.size(), expected.size()) << "Tokenized json did not contain expected number of fields";
    for (size_t i = 0; i < result.size(); i++)
    {
        EXPECT_EQ(expected[i], result[i]) << "Parsed value didn't match expected";
    }
}

TEST(TokenizeJson, StringFields)
{
    std::string json(R"({"field1":"data1","field2":"data with \"quotes\" in it","field3":"data3"})");
    std::vector<std::string> expected = {"field1", "\"data1\"", "field2", R"("data with \"quotes\" in it")", "field3", "\"data3\""};
    auto result = tokenize_json(json);

    EXPECT_EQ(result.size(), expected.size()) << "Tokenized json did not contain expected number of fields";
    for (size_t i = 0; i < result.size(); i++)
    {
        EXPECT_EQ(expected[i], result[i]) << "Parsed value didn't match expected";
    }
}

TEST(TokenizeJson, ListFields)
{
    std::string json(R"({"field1":[1,2,3,4,5],"field2":["string1","string2","string3","string4"],"field3":[true,false,true]})");
    std::vector<std::string> expected = {"field1","[1,2,3,4,5]","field2","[\"string1\",\"string2\",\"string3\",\"string4\"]","field3","[true,false,true]"};
    auto result = tokenize_json(json);

    EXPECT_EQ(result.size(), expected.size()) << "Tokenized json did not contain expected number of fields";
    for (size_t i = 0; i < result.size(); i++)
    {
        EXPECT_EQ(expected[i], result[i]) << "Parsed value didn't match expected";
    }
}

TEST(TokenizeJson, ObjectFields)
{
    std::string json(R"({"field1":{"field1.1":true,"field1.2":false},"field2":{},"field3":{"field3.1":"label","field3.2":["data1"."data2","data3"]}})");
    std::vector<std::string> expected = {"field1",R"({"field1.1":true,"field1.2":false})","field2","{}","field3",R"({"field3.1":"label","field3.2":["data1"."data2","data3"]})"};
    auto result = tokenize_json(json);

    EXPECT_EQ(result.size(), expected.size()) << "Tokenized json did not contain expected number of fields";
    for (size_t i = 0; i < result.size(); i++)
    {
        EXPECT_EQ(expected[i], result[i]) << "Parsed value didn't match expected";
    }
}

TEST(TokenizeJson, NextedObjectTokenization)
{
    std::string json(R"({"field1":{"field1.1":true,"field1.2":false},"field2":{},"field3":{"field3.1":"label","field3.2":["data1"."data2","data3"]}})");
    std::vector<std::string> expected2 = {"field1.1","true","field1.2","false"};
    std::vector<std::string> expected3 = {"field3.1","\"label\"","field3.2","[\"data1\".\"data2\",\"data3\"]"};
    auto result1 = tokenize_json(json);
    auto result2 = tokenize_json(result1[1]);
    EXPECT_EQ(expected2.size(), result2.size()) << "Subfield 1 was not the expected size";
    for (size_t i = 0; i < result2.size(); i++)
    {
        EXPECT_EQ(expected2[i], result2[i]) << "Parsed value didn't match expected";
    }

    auto result3 = tokenize_json(result1[5]);
    EXPECT_EQ(expected3.size(), result3.size()) << "Subfield 5 was not the expected size";
    for (size_t i = 0; i < result3.size(); i++)
    {
        EXPECT_EQ(expected3[i], result3[i]) << "Parsed value didn't match expected";
    }
}

// ---------------------------------------------------
//  smash_json
// ---------------------------------------------------

TEST(SmashJson, EmptyFields)
{
    std::vector<std::string> fields{};
    std::string json = smash_json(fields);
    std::string expected = R"({})";
    EXPECT_EQ(json, expected) << "Failed to return empty json when passed no fields";
}

TEST(SmashJson, UnbalancedFields)
{
    std::vector<std::string> fields{"field1", "data1", "field2", "data2", "field3"};
    std::string json = smash_json(fields);
    std::string expected = R"()";
    EXPECT_EQ(json, expected) << "Failed to return empty string when fields are not balanced";
}

TEST(SmashJson, BalancedFields)
{
    std::vector<std::string> fields{"field1","\"data1\"","field2","3.14159","field3","[1,2,3,4,5]","field4",R"({"field4.1":true,"field4.2":null,"field4.3":{"field4.3.1":"data4.3.1","field4.3.2":true}})"};
    std::string json = smash_json(fields);
    std::string expected = R"({"field1":"data1","field2":3.14159,"field3":[1,2,3,4,5],"field4":{"field4.1":true,"field4.2":null,"field4.3":{"field4.3.1":"data4.3.1","field4.3.2":true}}})";
    EXPECT_EQ(json, expected) << "Failed to return concatinated json string";
}

// ---------------------------------------------------
//  tokenize_array
// ---------------------------------------------------

TEST(TokenizeArray, EmptyString)
{
    std::string json(R"()");
    std::vector<std::string> expected = {};
    auto result = tokenize_array(json);

    EXPECT_EQ(result.size(), expected.size()) << "Tokenized array did not contain expected number of fields";
    for (size_t i = 0; i < result.size(); i++)
    {
        EXPECT_EQ(expected[i], result[i]) << "Parsed value didn't match expected";
    }
}

TEST(TokenizeArray, EmptyArray)
{
    std::string json(R"([])");
    std::vector<std::string> expected = {};
    auto result = tokenize_array(json);

    EXPECT_EQ(result.size(), expected.size()) << "Tokenized array did not contain expected number of fields";
    for (size_t i = 0; i < result.size(); i++)
    {
        EXPECT_EQ(expected[i], result[i]) << "Parsed value didn't match expected";
    }
}

TEST(TokenizeArray, NumberFields)
{
    std::string json(R"([1,.123,2.99e8,-3.14159265358979])");
    std::vector<std::string> expected = {"1",".123","2.99e8","-3.14159265358979"};
    auto result = tokenize_array(json);

    EXPECT_EQ(result.size(), expected.size()) << "Tokenized array did not contain expected number of fields";
    for (size_t i = 0; i < result.size(); i++)
    {
        EXPECT_EQ(expected[i], result[i]) << "Parsed value didn't match expected";
    }
}

TEST(TokenizeArray, StringFields)
{
    std::string json(R"(["data1","data with \"quotes\" in it","data3"])");
    std::vector<std::string> expected = {"\"data1\"", R"("data with \"quotes\" in it")", "\"data3\""};
    auto result = tokenize_array(json);

    EXPECT_EQ(result.size(), expected.size()) << "Tokenized array did not contain expected number of fields";
    for (size_t i = 0; i < result.size(); i++)
    {
        EXPECT_EQ(expected[i], result[i]) << "Parsed value didn't match expected";
    }
}

TEST(TokenizeArray, ListFields)
{
    std::string json(R"([[1,2,3,4,5],["string1","string2","string3","string4"],[true,false,true]])");
    std::vector<std::string> expected = {"[1,2,3,4,5]","[\"string1\",\"string2\",\"string3\",\"string4\"]","[true,false,true]"};
    auto result = tokenize_array(json);

    EXPECT_EQ(result.size(), expected.size()) << "Tokenized array did not contain expected number of fields";
    for (size_t i = 0; i < result.size(); i++)
    {
        EXPECT_EQ(expected[i], result[i]) << "Parsed value didn't match expected";
    }
}

TEST(TokenizeArray, ObjectFields)
{
    std::string json(R"([{"field1.1":true,"field1.2":false},{},{"field3.1":"label","field3.2":["data1"."data2","data3"]}])");
    std::vector<std::string> expected = {R"({"field1.1":true,"field1.2":false})","{}",R"({"field3.1":"label","field3.2":["data1"."data2","data3"]})"};
    auto result = tokenize_array(json);

    EXPECT_EQ(result.size(), expected.size()) << "Tokenized array did not contain expected number of fields";
    for (size_t i = 0; i < result.size(); i++)
    {
        EXPECT_EQ(expected[i], result[i]) << "Parsed value didn't match expected";
    }
}

TEST(TokenizeArray, NextedArrayTokenization)
{
    std::string json(R"([["foo","bar","baz","quux"],[[1,2,3,4,5],true,false,null]])");
    std::vector<std::string> expected1 = {"[\"foo\",\"bar\",\"baz\",\"quux\"]","[[1,2,3,4,5],true,false,null]"};
    std::vector<std::string> expected2 = {"[1,2,3,4,5]","true","false","null"};
    std::vector<std::string> expected3 = {"1","2","3","4","5"};
    auto result1 = tokenize_array(json);

    EXPECT_EQ(result1.size(), expected1.size()) << "Tokenized array did not contain expected number of fields";
    for (size_t i = 0; i < result1.size(); i++)
    {
        EXPECT_EQ(expected1[i], result1[i]) << "Parsed value didn't match expected";
    }
    auto result2 = tokenize_array(result1[1]);
    
    EXPECT_EQ(result2.size(), expected2.size()) << "Tokenized sub-array did not contain expected number of fields";
    for (size_t i = 0; i < result2.size(); i++)
    {
        EXPECT_EQ(expected2[i], result2[i]) << "Parsed value didn't match expected";
    }
    auto result3 = tokenize_array(result2[0]);

    EXPECT_EQ(result3.size(), expected3.size()) << "Tokenized sub-sub-array did not contain expected number of fields";
    for (size_t i = 0; i < result3.size(); i++)
    {
        EXPECT_EQ(expected3[i], result3[i]) << "Parsed value didn't match expected";
    }
}

// ---------------------------------------------------
//  smash_array
// ---------------------------------------------------

TEST(SmashArray, EmptyFields)
{
    std::vector<std::string> fields{};
    std::string json = smash_array(fields);
    std::string expected = R"([])";
    EXPECT_EQ(json, expected) << "Failed to return empty json when passed no fields";
}

TEST(SmashArray, NormalFields)
{
    std::vector<std::string> fields{"3.14159","\"string 1\"","[1,2,3,4,5]","{\"field1\":\"data1\",\"field2\":true}","true","false","null"};
    std::string json = smash_array(fields);
    std::string expected = R"([3.14159,"string 1",[1,2,3,4,5],{"field1":"data1","field2":true},true,false,null])";
    EXPECT_EQ(json, expected) << "Failed to return concatinated array string";
}

// ---------------------------------------------------
//  de_whitespace_json
// ---------------------------------------------------

TEST(DeWhitespaceJson, EmptyStringReturnsEmpty)
{
    std::string json("");
    EXPECT_EQ(json, de_whitespace_json(json)) << "Somehow created a string from empty string";
}

TEST(DeWhitespaceJson, EmptyJsonReturnsunmodified)
{
    std::string json("{}");
    EXPECT_EQ(json, de_whitespace_json(json)) << "Empty json did not return same";
}

TEST(DeWhitespaceJson, NoWhitespaceUnchanged)
{
    std::string json(R"({"field1":"string1","array1":[1,2,3,4,5],"field3":true})");
    EXPECT_EQ(json, de_whitespace_json(json)) << "json without whitespace was modified";
}

TEST(DeWhitespaceJson, WhitespaceInStringUnmodified)
{
    std::string json("{\"field 1\":\"string 1\",\"field 2\":\"string\t2\",\"field 3\":\"this\ris\nstring three\"}");
    EXPECT_EQ(json, de_whitespace_json(json)) << "Whitespace in string(s) was modified";
}

TEST(DeWhitespaceJson, WhitespaceBetweenFieldsRemoved)
{
    std::string json("{\n\t\"field 1\" : \"string one\",\n\t\"field 2\" :\n\t[\n\t\t\"string 2.1\",\n\t\t\"string 2.2\",\n\t\t\"string 2.3\"\n\t],\n\t\"field 3\" :\n\t{\n\t\t\"field 3.1\" : true,\n\t\t\"field 3.2\" : \"foobar\"\n\t}\n}");
    std::string res("{\"field 1\":\"string one\",\"field 2\":[\"string 2.1\",\"string 2.2\",\"string 2.3\"],\"field 3\":{\"field 3.1\":true,\"field 3.2\":\"foobar\"}}");
    EXPECT_EQ(res, de_whitespace_json(json)) << "Failed to remove whitespace between fields";
}
