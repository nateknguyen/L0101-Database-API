#include <gtest/gtest.h>
#include <chrono>
#include <random>
#include "database.h"	// optimized version
// separate headers need to be defined and namespaced

using hr_clock = std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using ms = std::chrono::milliseconds;

std::random_device rnd;
std::mt19937 mt(rnd());

TEST(RuntimeTest, get_documentsAveragePath)
{
	auto t1 = hr_clock::now();
	auto t2 = hr_clock::now();
	auto t3 = hr_clock::now();
	double parallel_time = 0.00;
	double linear_time = 0.00;
    
    Database db("test/temps");
    Database lin_db ("test/temps");
    db.add_collection_from_file("Collection 1", "test/saves/RuntimeTestData.json"); // RuntimeTestData.json needs to be written. Make several thousand objects long
    lin_db.add_collection_from_file("Collection 2", "test/saves/RuntimeTestData.json");
    db.set_current_collection("Collection 1");
    lin_db.set_current_collection("Collection 2");

	std::string path = R"("Get filter field 1"={"field g11":true,"field g12":"String g12","field g13":["string g131",true,false,4.8997]}&"get filter field 2"=["This","was","a","triumph.","I'm","making","a","note","here:","Huge","Success.","It's","hard","to","overstate","my","satisfaction."])";

	t1 = hr_clock::now();
	auto rp = db.get_documents(path);
	t2 = hr_clock::now();
	auto urp = lin_db.get_documents(path, false);
	t3 = hr_clock::now();

	parallel_time = duration<double, std::milli>(t2 - t1).count();
	linear_time = duration<double, std::milli>(t3 - t2).count();

	ASSERT_LT(parallel_time, linear_time);
}

TEST(RuntimeTest, update_documentsAveragePath)
{
	auto t1 = hr_clock::now();
	auto t2 = hr_clock::now();
	auto t3 = hr_clock::now();
	double parallel_time = 0.00;
	double linear_time = 0.00;
    
    Database db("test/temps");
    Database lin_db ("test/temps");
    db.add_collection_from_file("Collection 1", "test/saves/RuntimeTestData.json"); // RuntimeTestData.json needs to be written. Make several thousand objects long
    lin_db.add_collection_from_file("Collection 2", "test/saves/RuntimeTestData.json");
    db.set_current_collection("Collection 1");
    lin_db.set_current_collection("Collection 2");

	std::string path = R"("Update filter field 1"="This is update filter 1"&"Update filter field 2"={"field u21":false,"filter u22":123}&"Update filter field 3"=false)";

	t1 = hr_clock::now();
	db.update_documents(path, "{\"Update filter field 1\":delete}");
	t2 = hr_clock::now();
	lin_db.update_documents(path, "{\"Update filter field 1\":delete}", false);
	t3 = hr_clock::now();

	parallel_time = duration<double, std::milli>(t2 - t1).count();
	linear_time = duration<double, std::milli>(t3 - t2).count();

	ASSERT_LT(parallel_time, linear_time);
}

TEST(RuntimeTest, remove_documentsAveragePath)
{
	auto t1 = hr_clock::now();
	auto t2 = hr_clock::now();
	auto t3 = hr_clock::now();
	double parallel_time = 0.00;
	double linear_time = 0.00;
    
    Database db("test/temps");
    Database lin_db ("test/temps");
    db.add_collection_from_file("Collection 1", "test/saves/RuntimeTestData.json"); // RuntimeTestData.json needs to be written. Make several thousand objects long
    lin_db.add_collection_from_file("Collection 2", "test/saves/RuntimeTestData.json");
    db.set_current_collection("Collection 1");
    lin_db.set_current_collection("Collection 2");

	std::string path = R"("Remove filter field 1"=12345&"Remove filter field 2"=["Remove","this","object"]&"Remove filter field 3"="Hello there."&"Remove filter field 4"=2.998e8)";

	t1 = hr_clock::now();
	db.remove_documents(path);
	t2 = hr_clock::now();
	lin_db.remove_documents(path, false);
	t3 = hr_clock::now();

	parallel_time = duration<double, std::milli>(t2 - t1).count();
	linear_time = duration<double, std::milli>(t3 - t2).count();

	ASSERT_LT(parallel_time, linear_time);
}
