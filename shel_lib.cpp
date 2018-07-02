#include <iostream>

#include "logger.hpp"
#include "shel_lib.hpp"

Native_Return_Data *call_native_function(std::string name, std::vector<Data_Atom *> args, Code_Site *site) {
    if (name == "print") {
        return print(args, site);
    } else if (name == "array_get") {
        return array_get(args, site);
    } else if (name == "array_set") {
        return array_set(args, site);
    } else if (name == "array_len") {
        return array_len(args, site);
    } else if (name == "array_add") {
        return array_add(args, site);
    }

    return new Native_Return_Data(false, NULL);
}

Native_Return_Data *print(std::vector<Data_Atom *> args, Code_Site *site) {
    if (args.size() == 1) {
        std::cout << atom_to_string(args[0]) << std::endl;
        return new Native_Return_Data(true, NULL);
    }

    std::string main_string = ((Str_Atom *)args[0])->value;
    std::vector<std::string> str_args;

    for (int i = 1; i < args.size(); i++) {
        str_args.push_back(atom_to_string(args[i]));
    }

    std::string build = main_string;

    for (auto other : str_args) {
        size_t start_pos = build.find("%");

        if (start_pos == std::string::npos) std::cerr << "Wrong number of args passed to print" << std::endl;

        build.replace(start_pos, 1, other);
    }

    std::cout << build << std::endl;

    return new Native_Return_Data(true, NULL);
}

Native_Return_Data *array_get(std::vector<Data_Atom *> args, Code_Site *site) {
    auto arr = (Array_Atom *)args[0];
    auto index = ((Num_Atom *)args[1])->value;

    if (index >= arr->items.size()) report_fatal_error("Index out of range", site);

    return new Native_Return_Data(true, arr->items.at(index));
}

Native_Return_Data *array_set(std::vector<Data_Atom *> args, Code_Site *site) {
    auto arr = (Array_Atom *)args[0];
    auto index = ((Num_Atom *)args[1])->value;
    auto value = args[2];

    arr->items.at(index) = value;

    return new Native_Return_Data(true, NULL);
}

Native_Return_Data *array_len(std::vector<Data_Atom *> args, Code_Site *site) {
    auto arr = (Array_Atom *)args[0];

    return new Native_Return_Data(true, new Num_Atom(arr->items.size()));
}

Native_Return_Data *array_add(std::vector<Data_Atom *> args, Code_Site *site) {
    auto arr = (Array_Atom *)args[0];
    auto value = args[1];

    arr->items.push_back(value);

    return new Native_Return_Data(true, NULL);
}
