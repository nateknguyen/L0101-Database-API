#include <gtest/gtest.h>

#include "database.h"


/*
    // put code for Monday demo here
    Document d (R"({"Field One":"Foobar","Int Field 1":23,"Double Field 1":-3.1479658694523,"Bool Field 1":false,"Bool Field 2":true,"Null Field":null,"Array Field":["test",123,-3e8,true,false,null,["1","2","3"],{"field_name":true}]})");
    Document d2 (R"({"Object Field":{"Field One":"Foobar","Int Field 1":23,"Double Field 1":-3.1479658694523,"Bool Field 1":false,"Bool Field 2":true,"Null Field":null,"Array Field":["test"],"Object Field":{"field_name":false}}})");
    std::cout << d.get<std::string>("Field One") << '\n';
    std::cout << d.get<int>("Int Field 1") << '\n';
    std::cout << d.get<double>("Double Field 1") << '\n';
    std::cout << d.get<bool>("Bool Field 1") << '\n';
    std::cout << d.get<bool>("Bool Field 2") << '\n';
    std::cout << d.get<bool>("Null Field") << '\n';
    std::cout << d.get<json_array>("Array Field").get<std::string>(0) << '\n';
    std::cout << d.get<json_array>("Array Field").get<int>(1) << '\n';
    std::cout << d.get<json_array>("Array Field").get<double>(2) << '\n';
    std::cout << d.get<json_array>("Array Field").get<bool>(3) << '\n';
    std::cout << d.get<json_array>("Array Field").get<bool>(4) << '\n';
    std::cout << d.get<json_array>("Array Field").get<bool>(5) << '\n';
    std::cout << d.get<json_array>("Array Field").get<json_array>(6).get<std::string>(0) << '\n';
    std::cout << d.get<json_array>("Array Field").get<json_object>(7).get<bool>("field_name") << '\n';
    
    std::cout << d2.get<json_object>("Object Field").get<std::string>("Field One") << '\n';
    std::cout << d2.get<json_object>("Object Field").get<int>("Int Field 1") << '\n';
    std::cout << d2.get<json_object>("Object Field").get<double>("Double Field 1") << '\n';
    std::cout << d2.get<json_object>("Object Field").get<bool>("Bool Field 1") << '\n';
    std::cout << d2.get<json_object>("Object Field").get<bool>("Bool Field 2") << '\n';
    std::cout << d2.get<json_object>("Object Field").get<bool>("Null Field") << '\n';
    std::cout << d2.get<json_object>("Object Field").get<json_array>("Array Field").get<std::string>(0) << '\n';
    std::cout << d2.get<json_object>("Object Field").get<json_object>("Object Field").get<bool>("field_name") << '\n';
*/
