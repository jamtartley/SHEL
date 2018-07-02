#include <iostream>

#include "shel_lib.hpp"

Native_Return_Data *call_native_function(std::string name, std::vector<Data_Atom *> args) {
    if (name == "print") {
        return print(args);
    } else if (name == "array_get") {
        return array_get(args);
    } else if (name == "array_set") {
        return array_set(args);
    } else if (name == "array_add") {
        return array_add(args);
    }

    return new Native_Return_Data(true, NULL);
}

Native_Return_Data *print(std::vector<Data_Atom *> args) {
    // @TODO(LOW) Handle not having main string in print
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

Native_Return_Data *array_get(std::vector<Data_Atom *> args) {
    auto arr = (Array_Atom *)args[0];
    auto index = ((Num_Atom *)args[1])->value;

    return new Native_Return_Data(true, arr->items.at(index));
}

Native_Return_Data *array_set(std::vector<Data_Atom *> args) {
    auto arr = (Array_Atom *)args[0];
    auto index = ((Num_Atom *)args[1])->value;
    auto value = args[2];

    arr->items.at(index) = value;

    return new Native_Return_Data(true, NULL);
}

Native_Return_Data *array_add(std::vector<Data_Atom *> args) {
    auto arr = (Array_Atom *)args[0];
    auto value = args[1];

    arr->items.push_back(value);

    return new Native_Return_Data(true, NULL);
}
