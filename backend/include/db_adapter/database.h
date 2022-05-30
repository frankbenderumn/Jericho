#ifndef DATABASE_H_
#define DATABASE_H_

#define RECORDS std::vector<std::vector<std::string>>
#define RECORD std::vector<std::string>
// #include "config/defs.h"
// #include "prizm/prizm.h"
#include <stdexcept>
#include <pqxx/pqxx>
#include <unordered_map>
#include "db_adapter/table.h"

class Database {
  public:
    Database() {
        C = new pqxx::connection("dbname=demodb user=fbender \
                                    hostaddr=127.0.0.1 port=5432");
        if (C) {
            // Console::Log(SUCCESS, "Connected to database: postgres");
            printf("Connected to db\n");
        } else {
            // Console::Log(FAILURE, "Failed to connect to database: postgres");
            printf("Failed to connect to db\n");
        }
    }

    ~Database() {
        printf("destroying database instance\n");
    }

    Database(std::string name, 
            std::string role, 
            std::string password, 
            std::string host, 
            std::string port) {
        C = new pqxx::connection("dbname="+name+" user="+role+" password="+password +
                                "hostaddr="+host+" port="+port);
        if (C) {
            // Console::Log(SUCCESS, "Connected to database "+name+" as "+role+"");
        } else {
            // Console::Log(FAILURE, "Failed to connect to database "+name+"");
        }
    }

    /* mutates database */
    void LoadTable(std::string name) {
                pqxx::connection* D = new pqxx::connection("dbname=demodb user=fbender \
                                    hostaddr=127.0.0.1 port=5432");
        pqxx::work worker{*D};
        bool commited = true;
        pqxx::result res;
        try {
            res = worker.exec("SELECT * FROM " + name + ";");
        } 
        catch (const pqxx::undefined_table& e) {
            // Console::Log(FAILURE, "Undefined table on query: " + e.query());
            // Console::Log(FAILURE, "Table does not exist: " + name);
            commited = false;
        }

        if (commited) {
            std::cout << res.size() << std::endl;

            if (!IsTable(name)) {
                // Console::Log(INFO, "Adding table " + name + " to memory...");
                AddTable(name, res);
            }

            // for (int i = 0; i < res.size(); i++) {
            // pqxx::tuple row = res.at(0);
            // std::cout << row.size() << std::endl;
            // std::cout << row.at(0) << " -- " << row.at(1) << " -- " << row.at(2) << std::endl;
            // pqxx::field c = row.at(1);
            // std::cout << c.c_str() << std::endl;
            // for (auto const &field: row) {
            //     std::cout << field.c_str() << '\t';
            // }
            // std::cout << std::endl;

        }
    }

    RECORD QueryNames(std::string table) {
        pqxx::work worker{*C};
        bool commited = true;
        pqxx::result res;
        RECORD result;
        // YEL(printf("Running query\n"));
        try {
            // if (type == "where") {
            //     if (method == "like") {
            //         res = worker.exec("SELECT * FROM " + table + " WHERE " +  attribute + " ILIKE '%" + value + "%'  LIMIT 50;");
            //     } else {
            //         res = worker.exec("SELECT * FROM " + table + " WHERE " +  attribute + " = " + value + " LIMIT 50;");
            //     }
            // } else if (type == "ALL") {
                res = worker.exec("SELECT * FROM " + table + ";");
                worker.commit();
            // } else {
            //     res = worker.exec("SELECT * FROM " + table + " LIMIT 50;");
            // }
            for (const pqxx::row& r : res) {
                std::vector<pqxx::field> fields;
                for (pqxx::field f : r) {
                    result.push_back(std::string(f.c_str()));
                    // BCYA(printf("OOH LA LA: %s\n", f.name()));
                }
                // Row* row = new Row(fields);
                // _rows.push_back(row);
            }
        } 
        catch (const pqxx::undefined_table& e) {
            // Console::Log(FAILURE, "Undefined table on query: " + e.query());
            // Console::Log(FAILURE, "Table does not exist: " + table);
            commited = false;
        }
        return result;
    }

    RECORDS Query(std::string name, std::string table, std::string type, std::string attribute, std::string method, std::string value) {
        pqxx::work worker{*C};
        bool commited = true;
        pqxx::result res;
        RECORDS result;
        printf("Running query\n");
        try {
            if (type == "where") {
                if (method == "like") {
                    res = worker.exec("SELECT * FROM " + table + " WHERE " +  attribute + " ILIKE '%" + value + "%'  LIMIT 50;");
                } else {
                    res = worker.exec("SELECT * FROM " + table + " WHERE " +  attribute + " = " + value + " LIMIT 50;");
                }
            } else if (type == "ALL") {
                res = worker.exec("SELECT * FROM " + table + ";");
            } else {
                res = worker.exec("SELECT * FROM " + table + " LIMIT 50;");
            }
        } 
        catch (const pqxx::undefined_table& e) {
            // Console::Log(FAILURE, "Undefined table on query: " + e.query());
            // Console::Log(FAILURE, "Table does not exist: " + table);
            commited = false;
        }

        if (commited) {
            std::cout << res.size() << std::endl;

            if (!IsTable(table)) {
                // Console::Log(INFO, "Adding table " + table + " to memory...");
                // Console::Log(INFO, "This is a test!");
                result = CacheQuery(name, res);
            }

            // for (int i = 0; i < res.size(); i++) {
            // pqxx::tuple row = res.at(0);
            // std::cout << row.size() << std::endl;
            // std::cout << row.at(0) << " -- " << row.at(1) << " -- " << row.at(2) << std::endl;
            // pqxx::field c = row.at(1);
            // std::cout << c.c_str() << std::endl;
            // for (auto const &field: row) {
            //     std::cout << field.c_str() << '\t';
            // }
            // std::cout << std::endl;

        }

        return result;
    }

    bool IsTable(std::string key) const {
        return (_tables.find(key) != _tables.end());
    }

    Table* GetTable(std::string name) {
        return _tables[name];
    }

    void Insert(std::string table, RECORD record) {

        /** TODO: sanity check for existence of table */
        /** TODO: retrieve column types of table */
        /** TODO: may be useful to use migration for type check instead of io call */ 
        pqxx::work worker{*C};
        bool commited = true;
        pqxx::result res;
        RECORDS result;
        printf("Running insert query...\n");
        try {
            /** TODO: quick workaround need to make dynamic */
            // res = worker.exec("DROP TABLE users;");
            // worker.commit();
            // res = worker.exec("CREATE TABLE IF NOT EXISTS USERS(ID INT PRIMARY KEY, username VARCHAR(16), password VARCHAR(128), salt VARCHAR(16));");
            // worker.commit();
            res = worker.exec("INSERT INTO Users (id, username, password, salt) VALUES(1, '"+record.at(0)+"', '"+record.at(1)+"', '"+record.at(2)+"');");
            worker.commit();
            printf("Commited transaction!\n");
        } 
        catch (const pqxx::undefined_table& e) {
            // printf("Undefined table on query: " + e.query());
            // Console::Log(FAILURE, "Table does not exist: " + table);
            commited = false;
            printf("Failed to commit!\n");
        }
    }

    void AddTable(std::string name, pqxx::result rows) {
        Table* table = new Table(name, rows);
        table->Dump();
        // Console::Log(SUCCESS, "Successfully added table " + name + " to memory.");
        _tables[name] = table;
    }

    std::vector<std::vector<std::string>> CacheQuery(std::string name, pqxx::result rows) {
        Table* table = new Table(name, rows);
        // table->Dump();
        // Console::Log(SUCCESS, "Successfully added table " + name + " to memory.");
        _tables[name] = table;
        // Console::Log(WARNING, "Caching query " + name + "");
        // std::vector<std::vector<std::string>> debugTemp = {};
        // return debugTemp;
        return table->Stringify();
    }

    std::unordered_map<std::string, Table*> GetTables() {
        return _tables;
    }

    void Print() { printf("TODO: needed for persistent data\n"); }


  private:
    pqxx::connection* C;
    std::unordered_map<std::string, Table*> _tables;

        // try {
        //     // Connect to the database.
        //     std::cout << "Connected to " << C->dbname() << '\n';

        //     // Start a transaction.
        //     pqxx::work W{*C};

        //     // Perform a query and retrieve all results.
        //     pqxx::result R{W.exec("SELECT name FROM stock")};

        //     // Iterate over results.
        //     std::cout << "Found " << R.size() << "employees:\n";
        //     for (auto row: R)
        //         std::cout << row[0].c_str() << '\n';

        //     // Perform a query and check that it returns no result.
        //     std::cout << "Doubling all employees' salaries...\n";
        //     W.exec("UPDATE employee SET salary = salary*2");

        //     // Commit the transaction.
        //     std::cout << "Making changes definite: ";
        //     W.commit();
        //     std::cout << "OK.\n";
        // }
        
};

#endif