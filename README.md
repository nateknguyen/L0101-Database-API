# L0101 Database API

## Features
L0101 is a header-only noSQL Database API for C++ written by
- Fahed Elkhatib
- Tristan Naess
- Nathan Nguyen
- Israel Sanchez

for their Senior Software Design project.

L0101 provides a simple API and syntax to interface with a json document store database with O(n) document search and multi-threaded performance on filter operations

## Requirements
L0101 requires C++17 and OpenMP for compilation
make is used for the test and demo code
## Installation
Because L0101 is header-only, you only need to #include the file in your code and put the file in the a directory in your include path.
The header can be downloaded by running `curl -O https://github.com/CS179K/L0101/main/include/database.h`
## Demo Code
If the entire repository is cloned, the demo code is compiled with `make`
and the tests with `make test` to make the binaries `demo_main` and `test_main` respectively.

## Functionality
Database and Collection level operations are done through the Database class.
Extraction of data from Documents is done to Documents queried from the Database
Queried documents are either const reference if requested by id or a vector of copies if requested by filter. All changes must be made through the `update_document[s]()` functions
### Database Member Functions:
`Database(const std::string &filepath)`
    - Creates a database object. Inactive collections are written to file in `filepath`.
    
`std::vector<std::string> get_collection_names()`
    - Returns the collection names; throws if the filepath doesn't exist
    
`void change_collection_name(const std::string &old_name, const std::string &new_name)`
    - Changes the name of the collection from `old_name` to `new_name`
    
`void set_current_collection(const std::string &name)`
    - Sets the current collection to the one with the given name; throws if no collection with the name exists
    
`void add_collection(const std::string &name)`
    - Crate a collection with the provided name; throws if a collection with the name already exists
    
`void remove_collection(const std::string &name)`
    - Removes the collection with the provided name; throws if a collection with the name doesn't exist
    
`void add_collection_from_file(const std::string &name, const std::string &filepath)`
    - Creates a collection with the given name and prepares to load data from `filepath`; throws if the name already exists
    - The data in `filepath` is to be formatted as json objects with whitespace or no delimiter between objects

-- All further Database functions throw if no current collection is set --

`void save_current_collection(const std::string &filepath)`
    - Saves the contents as newline delimited json objects in the file at `filepath`; throws if it fails to open the file
    
`void load_current_collection(const std::string &filepath)`
    - Loads the contents of a file into the current collection. File formatting is the same as for `add_collection_from_file`
    - Throws if the file doesn't exist
    
`std::vector<size_t> get_ids()`
    - Returns the ids of the documents in the current collection in a vector
    
`std::vector<Document>::const_iterator begin() and end()`
    - Return iterators to the first and end + 1 elements of the current collection. Allows for range based iteration
    
`size_t add_document(const std::string &json)`
    - Adds a document to the current collection and return that document's id
    
`const Document &get_document(size_t id)`
    - Returns the document with given id or throws if the document doesn't exist
    
`const std::vector<Document> get_documents(const std::string &pattern, bool parallel = true)`
    - Returns all documents in the current collection that match the provided pattern, as defined later
    - parallel flag dictates whether the filter is run parallel, and defaults to true
    
`void update_document(size_t id, const std::string &data)`
    - Replaces the specified field in the document matching `id` with its specified value or throws if the document doesn't exist
    - The `data` string is formatted as '"key":value' where value is the entire value to be replaced, no sub-field access
    - If the key doesn't exist a new key-value pair will be appended to the object
    - If the value is specified as `delete` the field will be removed
    
`void update_documents(const std::string &pattern, const std::sring &data, bool parallel = true)`
    - Same behavior as the previous function but applied to all documents that match the filter
    
`void remove_document(size_t id)`
    - Removes the specified document or throws if the document doesn't exist or collection is empty
    
`void remove_documents(const std::string &pattern, bool parallel = true)`
    - Removes all documents that match the pattern

### Document Member Functions:
`size_t get_id()`
    - Returns the id of the document
    
`std::string to_string()`
    - Returns the contents in a tree-formatted string
    
`T get<T>(const std::string &field)`
    - Returns the requested field or throws if the type is incorrect or field doesn't exist
    - Legal types are:
- int
- double
- std::string
- bool
- json_object
- json_array

`T query<T>(const std::string &path)`
    - Returns the requested sub-field or throws if the type is incorrect or path is incorrectly formatted
    - Correct path format is specified later
    - Legal types are the same as `get<T>()`

`bool is_null(const std::string &field)`
    - Returns whether the field contains `null` or throws if the field doesn't exist


`json_object` and `json_array` are wrappers around the json data for an object or array respectively
both have the same `T get<T>()` function as `Document` expcept that `json_array` takes a `size_t` as its argument and the same `bool is_null()` function with the matching argument

### Paths and Patterns
Paths for path queries and filtering consist of quote surrounded object keys and bracket surrounded array indices. Excluding the field of the root document, all keys are preceded with a `'.'`
e.g. `"Root object field name"."sub-object field name"[3]."key"`
The patterns for the filter (`Database::*_documents()`) functions are a string consisting of any number of path queries, each followed by an `'='` and then the expected value. The individual queries are delimited with `&`
e.g. `"Active"=true&"Name"[1]="Smith`



