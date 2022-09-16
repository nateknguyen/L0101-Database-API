#include "database.h"
#include <iostream>
#include <stdio.h>
#include <random>

void press_enter_to_continue()
{
    int c;
    printf("Press Enter To Continue... ");
    fflush(stdout);
    do c = getchar(); while ((c != '\n') && (c != EOF));
    printf("\033[A\r                          \n");
}

void print_error(const std::string &message)
{
    std::cout << "\033[31mError: " << message << "\033[0m\n\n"; 
}

void print_success()
{
    std::cout << "\033[32mSuccess\033[0m\n\n";
}

int main(int argc, char ** argv)
{
    Database db("test/temps");

    std::cout << "-- Loading two collections from file and creating one empty --\n\n";
    press_enter_to_continue();

    db.add_collection_from_file("Collection 1", "datasets/collection1.json");
    db.add_collection_from_file("Collection 2", "datasets/collection2.json");
    db.add_collection("Collection 3");

    print_success();

    std::cout << "-- Trying to load collection from missing file --\n\n";
    press_enter_to_continue();
    try
    {
        db.add_collection_from_file("Collection 4", "this/file/doesn't/exist");
    }
    catch (std::runtime_error &e)
    {
        print_error(e.what());
    }


    std::cout << "-- Listing collections in database --\n\n";
    press_enter_to_continue();
    for (const auto &name : db.get_collection_names())
    {
        std::cout << "\t" << name << '\n';
    }
    std::cout << '\n';

    std::cout << "-- Changing the name of \"Collection 2\" to \"Second Ccllection\" --\n\n";
    press_enter_to_continue();
    db.change_collection_name("Collection 2", "Second Collection");
    for (const auto &name : db.get_collection_names())
    {
        std::cout << "\t" << name << '\n';
    }
    std::cout << '\n';

    std::cout << "-- Trying to remove document without current collection set --\n\n";
    press_enter_to_continue();
    try
    {
        db.remove_document(123);
    }
    catch (std::runtime_error &e)
    {
        print_error(e.what());
    }


    std::cout << "-- Trying to set the current collection to a non-existent collection --\n\n";
    press_enter_to_continue();
    try
    {
        db.set_current_collection("Collection foo");
    }
    catch(std::runtime_error &e)
    {
        print_error(e.what());
    }

    std::cout << "-- Setting current collection --\n\n";
    press_enter_to_continue();
    db.set_current_collection("Collection 1");
    print_success();

    std::cout << "-- Iterate Collection 1 with iterators and print with to_string() --\n\n";
    press_enter_to_continue();
    for (const auto &d : db)
    {
        std::cout << d.to_string() << '\n';
    }

    std::cout << "-- Changing to the empty collection --\n\n";
    press_enter_to_continue();
    db.set_current_collection("Collection 3");

    std::cout << "\n-- Adding documents to the collection --\n\n";
    press_enter_to_continue();
    db.add_document(R"({"Field 1": "Hello World", "Pi": 3.14159265358979, "Sub-Object": {"Key 1": true, "Key 2": false}})");
    try
    {
        db.add_document(R"({"Field 1": "Hello There", "Field 2":["This", "was", "a". "triumph.", "I'm", "Making ", "a", "note", "here:", "\"", "Huge", "Succcess.", "\""], "Sub-Object": {"Key 1": false, "Key 2": true}})");
    }
    catch(std::runtime_error &e)
    {
        print_error(e.what());
    }
    press_enter_to_continue();

    std::cout << "-- Typo in one of the strings. I put a . instead of a , --\n\n";

    db.add_document(R"({"Field 1": "Hello There", "Field 2":["This", "was", "a", "triumph.", "I'm", "Making ", "a", "note", "here:", "\"", "Huge", "Succcess.", "\""], "Pi": 3.14159265358979, "Sub-Object": {"Key 1": false, "Key 2": false}})");
    db.add_document(R"({"Field 1": "General Kenobi", "Sub-Object": {"Key 1": true, "Key 2": false}})");
    db.add_document(R"({"Field 1": "foo", "Field 2": "bar", "Field 3": "baz", "Field 4": "quux"})");
    db.add_document(R"({"Title": "The Canterbury Tales", "Prologue": [
        "Whan that Aprille with his shoures soote,",
        "The droghte of March hath perced to the roote,",
        "And bathed every veyne in swich licóur",
        "Of which vertú engendred is the flour;",
        "Whan Zephirus eek with his swete breeth",
        "Inspired hath in every holt and heeth",
        "The tendre croppes, and the yonge sonne",
        "Hath in the Ram his halfe cours y-ronne,",
        "And smale foweles maken melodye,",
        "That slepen al the nyght with open ye,",
        "So priketh hem Natúre in hir corages,",
        "Thanne longen folk to goon on pilgrimages,",
        "And palmeres for to seken straunge strondes,",
        "To ferne halwes, kowthe in sondry londes;",
        "And specially, from every shires ende",
        "Of Engelond, to Caunterbury they wende,",
        "The hooly blisful martir for to seke,",
        "That hem hath holpen whan that they were seeke."]})");

    std::cout << "-- Getting ids of documents in current collection--\n\n";
    press_enter_to_continue();
    auto ids = db.get_ids();
    for (auto id : ids)
    {
        std::cout << "id: " << id << '\n';
    }
    
    std::cout << "\n-- Getting document with id: 3 --\n\n";
    press_enter_to_continue();
    const Document &d = db.get_document(3);
    std::cout << d.to_string() << '\n';

    std::cout << "\n-- Get the value of \"field 1\" --\n\n";
    press_enter_to_continue();
    try
    {
        std::cout << d.get<std::string>("field 1");
    }
    catch (std::runtime_error &e)
    {
        print_error(e.what());
    }

    std::cout << "-- Typo. It's Field, not field --\n\n";

    press_enter_to_continue();
    std::cout << d.get<std::string>("Field 1") << '\n';

    std::cout << "\n-- Getting the value of Sub-Object element \"Key 1\" --\n\n";
    press_enter_to_continue();
    std::cout << std::boolalpha<<d.get<json_object>("Sub-Object").get<bool>("Key 1") << '\n';

    std::cout << db.get_document(5).to_string() << '\n';
    std::cout << "-- Updating document 5. removing \"Field 4\", making \"Field 2\" = \"2.99e+8\" --\n\n";
    press_enter_to_continue();
    db.update_document(5, R"({"Field 4": delete, "Field 2": 2.99e+8})");
    std::cout << db.get_document(5).to_string() << '\n';

    std::cout << "-- removing document 5 --\n\n";
    press_enter_to_continue();
    db.remove_document(5);

    for (const auto &d : db)
    {
        std::cout << d.to_string() << '\n';
    }

    std::cout << "-- Getting all documents with \"Pi\" = 3.14159265358979 and \"Sub-Object\".\"Key 2\" = false --\n\n";
    press_enter_to_continue();
    auto docs = db.get_documents(R"("Pi" = 3.14159265358979 & "Sub-Object"."Key 2" = false)");
    for (const auto &d : docs)
    {
        std::cout << d.to_string() << '\n';
    }

    std::cout << "-- Updating all documents with \"Sub-Object\".\"Key 1\" = true to have a new field: \"New field\":\"This is new Data\" --\n\n";
    press_enter_to_continue();
    db.update_documents(R"("Sub-Object"."Key 1" = true)", R"({"New field": "This is new data"})");
    for (const auto &d : db)
    {
        std::cout << d.to_string() << '\n';
    }

    std::cout << "-- Removing all documents with \"Sub-Object\".\"Key 2\" = false --\n\n";
    press_enter_to_continue();
    db.remove_documents(R"("Sub-Object"."Key 2" = false)");
    for (const auto &d : db)
    {
        std::cout << d.to_string() << '\n';
    }

    std::cout << "-- Saving the collection to file \"datasets/canterbury.json\" --\n\n";
    press_enter_to_continue();
    db.save_current_collection("datasets/canterbury.json");
}
