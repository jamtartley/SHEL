#include <vector>

#include "typer.hpp"

struct Native_Return_Data {
    bool was_success;
    void *return_value;

    Native_Return_Data(bool was_success, void *return_value) {
        this->was_success = was_success;
        this->return_value = return_value;
    }
};

Native_Return_Data *call_native_function(std::string name, std::vector<Data_Atom *> args, Code_Site *site);
Native_Return_Data *print(std::vector<Data_Atom *> args, Code_Site *site);
Native_Return_Data *array_get(std::vector<Data_Atom *> args, Code_Site *site);
Native_Return_Data *array_set(std::vector<Data_Atom *> args, Code_Site *site);
Native_Return_Data *array_len(std::vector<Data_Atom *> args, Code_Site *site);
Native_Return_Data *array_add(std::vector<Data_Atom *> args, Code_Site *site);
