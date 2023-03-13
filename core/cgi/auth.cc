#include "celerity/celerity.h"

int main(int argc, char* argv[]) {

    Postgres* postgres = new Postgres("demodb", "fbender");
    postgres->schema("schema.lock");
    postgres->tables();
    Table* stocket = postgres->table("stocket");
    Table* transaction = postgres->table("transaction");
    Table* enduser = postgres->table("enduser");
    transaction->all();
    transaction->describe();
    std::vector<std::string> args = {"1234", "23", "Something", "01-01-2022", "a0eebc99-9c0b-4ef8-bb6d-6bb9bd380a12", "Target", "alc"};
    transaction->insert(args);
    delete postgres;

    return 0;
}