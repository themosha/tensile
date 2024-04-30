#include <iostream>

#include "tensilelib/tensile.h"

#include "duckdb/src/include/duckdb.h"
#include <pqxx/pqxx>
#include "sqlite3.h"

namespace {

class DuckDBProvider : public tensile::ISQLProvider {
public:
    std::string name() const override { return "DuckDB"; }

    bool Run(const std::string &sql, std::string *error_msg) override {
        duckdb_database database;
        duckdb_open(nullptr, &database);
        duckdb_connection connection;
        duckdb_connect(database, &connection);
        duckdb_result result;
        auto state = duckdb_query(connection, sql.c_str(), &result);
        if (state == DuckDBSuccess) {
            return true;
        }
        *error_msg = duckdb_result_error(&result);
        return false;
    }

};

class PostgresProvider : public tensile::ISQLProvider {
public:
    std::string name() const override { return "Postgres"; }

    bool Run(const std::string &sql, std::string *error_msg) override {
        try {
            pqxx::connection connection;
            pqxx::work w(connection);
            w.exec(sql);
            return true;
        }
        catch (std::exception &e) {
            *error_msg = e.what();
            return false;
        }
    }
};

class SQLiteProvider : public tensile::ISQLProvider {
public:
    std::string name() const override { return "SQLite"; }

    bool Run(const std::string &sql, std::string *error_msg) override {
        sqlite3 *db;
        int ok = sqlite3_open(
                ":memory:",   /* Database filename (UTF-8) */
                &db         /* OUT: SQLite db handle */
        );

        char *errmsg;
        ok = sqlite3_exec(
                db,                                  /* An open database */
                sql.c_str(),                           /* SQL to be evaluated */
                NULL,  /* Callback function */
                NULL,                                    /* 1st argument to callback */
                &errmsg                              /* Error msg written here */
        );
        if (ok != 0) {
            error_msg->assign(errmsg, strlen(errmsg));
            sqlite3_free(errmsg);
            return false;
        }
        return true;
    }
};

}  // namespace

int main(int argc, char** argv) {
    tensile::Driver driver(argc, argv);
    driver.AddProvider(std::make_unique<SQLiteProvider>());
    driver.AddProvider(std::make_unique<PostgresProvider>());
    driver.AddProvider(std::make_unique<DuckDBProvider>());

    if (!driver.perftrace()) {
        std::cout << "Tensile Test for SQL" << std::endl;
        std::cout << "====================" << std::endl;
    }

    auto results = driver.Run();
    if (!driver.perftrace()) {
        std::cout << "Results" << std::endl;
        std::cout << "=======" << std::endl;
        for (auto &result: results) {
            std::cout << result.provider << "," << result.feature << "," << result.limit << ","
                      << result.status.ToChar()
                      << "," << result.status.message() << std::endl;
        }
    }
}
