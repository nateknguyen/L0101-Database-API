#include <iostream>
#include <ios>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <algorithm>
#include <limits>

#define AVG_FIELDS 10
#define AVG_RECURSIONS 5

// random setup is global to avoid constructing mt19997 repeatedly
std::random_device rnd;
std::mt19937 mt(rnd());
std::normal_distribution<float> norm(1, .15);
std::bernoulli_distribution coin(0.5);
std::uniform_int_distribution rand_char(0,35);
std::uniform_int_distribution int_distribution(-2000000, 2000000);
std::uniform_real_distribution double_distribution(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
std::uniform_int_distribution all_fields(0,6);  // includes array and object
std::uniform_int_distribution data_fields(0,4); // does not 

std::string rand_string()
{
    size_t len = std::max((size_t)std::floor(norm(mt) * 5), 5ul);
    std::string ret(len+2, '"');
    size_t n;
    for (size_t i = 1; i <= len; i++)
    {
        n = rand_char(mt);
        if (n < 10)
        {
            ret[i] = '0' + n;
            continue;
        }

        ret[i] = 'a' + n - 10;
    }
    return ret;
}

std::string rand_int()
{
    return std::to_string(int_distribution(mt));
}

std::string rand_double()
{
    std::stringstream ss;
    ss << std::scientific << double_distribution(mt);
    return ss.str();
}
std::string rand_bool()
{
    if (coin(mt)) return "true";
    return "false";
}

std::string null_field()
{
    return "null";
}

std::string construct_array(size_t max_recursion_depth);

std::string construct_object(size_t max_recursion_depth, const std::vector<std::string>& must_include)
{
    size_t field_count = std::max((size_t)(norm(mt) * AVG_FIELDS), must_include.size());

    std::vector<std::string> fields;
    fields.reserve(field_count);

    for (const auto& f : must_include)
        fields.push_back(f);

    for (size_t i = must_include.size(); i < field_count; i++)
    {
        size_t type;
        std::string data;
        if(max_recursion_depth > 0)
            type = all_fields(mt);
        else
            type = data_fields(mt);

        switch (type)
        {
            case 0: data = rand_string(); break;
            case 1: data = rand_int(); break;
            case 2: data = rand_double(); break;
            case 3: data = rand_bool(); break;
            case 4: data = null_field(); break;
            case 5: data = construct_object(max_recursion_depth-1, std::vector<std::string>()); break;
            case 6: data = construct_array(max_recursion_depth-1); break;
        }
        data = rand_string() + ":" + data;
        fields.push_back(data);
    }

    std::random_shuffle(fields.begin(), fields.end());

    std::stringstream ss;
    ss << '{';

    for (const auto& f : fields)
        ss << f << ',';

    std::string ret = ss.str();
    ret.back() = '}';

    return ret;
}

std::string construct_array(size_t max_recursion_depth)
{
    size_t field_count = std::floor(norm(mt) * 10);
    std::vector<std::string> fields;
    fields.reserve(field_count);
    
    for (size_t i = 0; i < field_count; i++)
    {
        size_t type;
        std::string data;
        if(max_recursion_depth > 0)
            type = all_fields(mt);
        else
            type = data_fields(mt);

        switch (type)
        {
            case 0: data = rand_string(); break;
            case 1: data = rand_int(); break;
            case 2: data = rand_double(); break;
            case 3: data = rand_bool(); break;
            case 4: data = null_field(); break;
            case 5: data = construct_object(max_recursion_depth-1, std::vector<std::string>()); break;
            case 6: data = construct_array(max_recursion_depth-1); break;
        }
        fields.push_back(data);
    }

    std::stringstream ss;
    ss << '[';

    for (const auto& f : fields)
        ss << f << ',';

    std::string ret = ss.str();
    ret.back() = ']';

    return ret;
}

int main(int argc, char ** argv)
{
    std::vector<std::string> necessary_fields = {
        R"("Get filter field 1":{"field g11":true,"field g12":"String g12","field g13":["string g131",true,false,4.8997]})",
        R"("get filter field 2":["This","was","a","triumph.","I'm","making","a","note","here:","Huge","Success.","It's","hard","to","overstate","my","satisfaction."])",
        R"("Update filter field 1":"This is update filter 1")",
        R"("Update filter field 2":{"field u21":false,"filter u22":123})",
        R"("Update filter field 3":false)",
        R"("Remove filter field 1":12345)",
        R"("Remove filter field 2":["Remove","this","object"])",
        R"("Remove filter field 3":"Hello there.")",
        R"("Remove filter field 4":2.998e8)"
    };

    size_t entries = 200000;
    std::string filepath;

    std::cout << "Enter target file name: ";
    std::getline(std::cin, filepath);
    std::cout << "Enter number of entries (default 200000): ";
    std::string buffer;
    std::getline(std::cin, buffer);
    if (buffer.size())
    {
        try
        {
            entries = std::stoi(buffer);
        }
        catch(std::invalid_argument& e)
        {
            std::cerr << "Error: " << e.what() << '\n';
            exit(EXIT_FAILURE);
        }
    }

    std::ofstream file("test/saves/" + filepath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file \"" << filepath << "\" to write" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (entries > 5000)
    {
        std::cout << "This may take a while\n";
    }

    std::cout << "\e[?25l";

    bool get_hit, update_hit, remove_hit;

    std::string object;
    std::vector<std::string> hit_fields;
    for (size_t i = 1; i < entries; i++)
    {
        get_hit = coin(mt);
        update_hit = coin(mt);
        remove_hit = coin(mt);

        hit_fields.clear();
        if (get_hit)
        {
            hit_fields.push_back(necessary_fields[0]);
            hit_fields.push_back(necessary_fields[1]);
        }
        if (update_hit)
        { 
            hit_fields.push_back(necessary_fields[2]);
            hit_fields.push_back(necessary_fields[3]);
            hit_fields.push_back(necessary_fields[4]);
        }
        if (remove_hit)
        {
            hit_fields.push_back(necessary_fields[5]);
            hit_fields.push_back(necessary_fields[6]);
            hit_fields.push_back(necessary_fields[7]);
            hit_fields.push_back(necessary_fields[8]);
        }
       
        object = construct_object((size_t)(norm(mt) * AVG_RECURSIONS), hit_fields);
            
        file << object << '\n';

        std::cout << "Entries generated: " << i << '\r';
    }
    object = construct_object((size_t)(norm(mt) * AVG_RECURSIONS), necessary_fields);

    file << object;

    file.close();

    std::cout << "\e[?25h";
}
