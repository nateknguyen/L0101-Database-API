#include <gtest/gtest.h>
#include <vector>
#include <filesystem>
#include "database.h"

//

TEST(WriteToFile, PopulatedCollection)
{
    Database db("test/temps");

    db.add_collection("foo");
    db.set_current_collection("foo");
    db.add_document(R"({"test1":"string","Numbers":1234,"nms":[1,2,3]})");
    db.add_document(R"({"test2":"string2","Numbers":4321,"nms":[3,2,1]})");
    db.add_document(R"({"test3":"string3","Numbers":1243,"nms":[2,1,3]})");

    EXPECT_NO_THROW(db.save_current_collection("test/saves/test.json")) << "Failed to save collection";

    std::ifstream file("test/saves/test.json");
    if (!file.is_open())
    {
        EXPECT_TRUE(false) << "Failed to open saved file";
    }

    std::string buffer;
    std::vector<std::string> lines;
    while (std::getline(file, buffer))
    {
        lines.push_back(buffer);
    }

    std::vector<std::string> expected = {"[", R"(	{"test1":"string","Numbers":1234,"nms":[1,2,3]},)",R"(	{"test2":"string2","Numbers":4321,"nms":[3,2,1]},)",R"(	{"test3":"string3","Numbers":1243,"nms":[2,1,3]})", "]"};
    ASSERT_EQ(lines.size(), expected.size()) << "Failed to read correct number of lines";
    for (size_t i = 0; i < lines.size(); i++)
    {
        EXPECT_EQ(lines[i], expected[i]) << "Line from file did not match expected";
    }
}

TEST(WriteToFile, UnpopulatedCollection)
{
    Database db("test/temps");

    db.add_collection("foo");
    db.set_current_collection("foo");
  
    EXPECT_NO_THROW(db.save_current_collection("test/saves/test.json")) << "Failed to save collection";

    std::ifstream file("test/saves/test.json");
    if (!file.is_open())
    {
        FAIL() << "Failed to open saved file";
    }

    std::string buffer;
    std::vector<std::string> lines;
    while (std::getline(file, buffer))
    {
        lines.push_back(buffer);
    }

    std::vector<std::string> expected = {"[","]"};
    for (size_t i = 0; i < lines.size(); i++)
    {
        EXPECT_EQ(lines[i], expected[i]) << "Line in save file did not match expected";
    }
  
}

TEST(WriteToFile, NoCurrentCollection)
{
    Database db("test/temps");

    db.add_collection("foo");
  
    EXPECT_ANY_THROW(db.save_current_collection("test/saves/test.json")) << "Failed to throw when no current collection";
}

TEST(LoadNewCollectionFromFile, FileExist)
{
    Database db("test/temps");

    EXPECT_NO_THROW(db.add_collection_from_file("collection1","test/saves/test2.json")) << "failed to construct from file";
    db.set_current_collection("collection1");

    auto ids = db.get_ids(); // Can't hard code ids in test, because static makes test order dependent
    EXPECT_EQ(db.get_document(ids[0]).get<std::string>("field1"), "\"data1\"") << "first document field1 not what expected";
    EXPECT_EQ(db.get_document(ids[0]).get<std::string>("field2"), "\"data2\"") << "first document field2 not what expected";
    EXPECT_EQ(db.get_document(ids[1]).get<int>("field1"), 1234) << "second document field1 not what expected";
    EXPECT_EQ(db.get_document(ids[1]).get<bool>("field2"), true) << "second document field2 not what expected";
    EXPECT_EQ(db.get_document(ids[2]).get<std::string>("field1"), "\"Hello\"") << "third document field1 not what expected";
    EXPECT_EQ(db.get_document(ids[2]).get<std::string>("field2"), "\"World\"") << "third document field2 not what expected";
}

TEST(LoadNewCollectionFromFile, FileDoesNotExist)
{
    Database db("test/temps");
    EXPECT_ANY_THROW(db.add_collection_from_file("foo","test/saves/missing.json")) << "Failed to throw when file is missing";
}

TEST(InactiveCollectionCache, CacheFileExist)
{
    Database db("test/temps");
    db.add_collection_from_file("collection1","test/saves/test2.json");
    db.set_current_collection("collection1");
    auto ids = db.get_ids();

    db.add_collection("collection2");
    db.set_current_collection("collection2");
    ASSERT_TRUE(std::filesystem::exists("test/temps/collection1.json.tmp")) << "failed to find temp file for collection 1";
    
    std::ifstream file("test/temps/collection1.json.tmp");
    if (!file.is_open())
    {
        FAIL() << "Failed to open saved file";
    }

    std::string buffer;
    std::vector<std::string> lines;
    while (std::getline(file, buffer))
    {
        lines.push_back(buffer);
    }

    std::vector<std::string> expected = {"{",R"({"field1":"data1","field2":"data2"},)",R"({"field1":1234,"field2":true},)",R"({"field1":"Hello","field2":"World"})","}"};
    for (size_t i = 0; i < 3; i++)
    {
        expected[i+1] = '\"' + std::to_string(ids[i]) + "\":" + expected[i+1];
    }

    ASSERT_EQ(lines.size(), expected.size()) << "Failed to read correct number of lines";
    for (size_t i = 0; i < lines.size(); i++)
    {
        EXPECT_EQ(lines[i], expected[i]) << "Line from file did not match expected";
    }
    

}
