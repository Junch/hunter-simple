#include <gtest/gtest.h>
#include <sqlite3.h>
#include <sstream>

// https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm

class sqlite_exception : public std::exception
{
  public:
    sqlite_exception(const char *text)
    {
        std::stringstream ss;
        ss << "SQL error: " << text;
        _szMsg = ss.str();
    }

    const char *what() const noexcept override { return _szMsg.c_str(); }

  private:
    std::string _szMsg;
};

#define THROW_IF_FAILED(rc, szErrMsg)                                                                                          \
    {                                                                                                                          \
        if (rc != SQLITE_OK)                                                                                                   \
        {                                                                                                                      \
            throw sqlite_exception(szErrMsg);                                                                                  \
        }                                                                                                                      \
    }

TEST(sqlite, create)
{
    try
    {
        sqlite3 *db = NULL;
        int rc = sqlite3_open("test.db", &db);
        THROW_IF_FAILED(rc, sqlite3_errmsg(db));

        /* Create SQL statement */
        const char *sql = "CREATE TABLE COMPANY("
                    "ID INT PRIMARY KEY     NOT NULL,"
                    "NAME           TEXT    NOT NULL,"
                    "AGE            INT     NOT NULL,"
                    "ADDRESS        CHAR(50),"
                    "SALARY         REAL );";

        char *szErrMsg = 0;

        /* Execute SQL statement */
        rc = sqlite3_exec(db, sql, NULL, 0, &szErrMsg);
        THROW_IF_FAILED(rc, szErrMsg);

        /* Create SQL statement */
        sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "
              "VALUES (1, 'Paul', 32, 'California', 20000.00 ); "
              "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "
              "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "
              "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)"
              "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );"
              "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)"
              "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";
        rc = sqlite3_exec(db, sql, NULL, 0, &szErrMsg);
        THROW_IF_FAILED(rc, szErrMsg);

        sqlite3_close(db);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "Encounter a generic exception\n";
    }
}

TEST(sqlite, drop)
{
    try
    {
        sqlite3 *db = NULL;
        int rc = sqlite3_open("test.db", &db);
        THROW_IF_FAILED(rc, sqlite3_errmsg(db));

        char *szErrMsg = 0;
        const char *sql = "DROP TABLE COMPANY";
        rc = sqlite3_exec(db, sql, 0, 0, &szErrMsg);
        THROW_IF_FAILED(rc, szErrMsg);
        sqlite3_close(db);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

static int callback(void *data, int argc, char **argv, char **azColName)
{
    fprintf(stdout, "%s: \n", (const char *)data);

    for (int i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

TEST(sqlite, query)
{
    try
    {
        sqlite3 *db = NULL;
        int rc = sqlite3_open("test.db", &db);
        THROW_IF_FAILED(rc, sqlite3_errmsg(db));

        char *szErrMsg = 0;
        const char *data = "Callback function called";
        const char *sql = "SELECT * from COMPANY";
        rc = sqlite3_exec(db, sql, callback, (void *)data, &szErrMsg);
        THROW_IF_FAILED(rc, szErrMsg);
        sqlite3_close(db);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

TEST(sqlite, update)
{
    try
    {
        sqlite3 *db = NULL;
        int rc = sqlite3_open("test.db", &db);
        THROW_IF_FAILED(rc, sqlite3_errmsg(db));

        char *szErrMsg = 0;
        const char *data = "Callback function called";
        const char *sql = "UPDATE COMPANY set SALARY = 25000.00 where ID=1; "
                          "SELECT * from COMPANY";
        rc = sqlite3_exec(db, sql, callback, (void *)data, &szErrMsg);
        THROW_IF_FAILED(rc, szErrMsg);
        sqlite3_close(db);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

TEST(sqlite, delete)
{
    try
    {
        sqlite3 *db = NULL;
        int rc = sqlite3_open("test.db", &db);
        THROW_IF_FAILED(rc, sqlite3_errmsg(db));

        char *szErrMsg = 0;
        const char *data = "Callback function called";
        const char *sql = "DELETE from COMPANY where ID=2; "
                          "SELECT * from COMPANY";
        rc = sqlite3_exec(db, sql, callback, (void *)data, &szErrMsg);
        THROW_IF_FAILED(rc, szErrMsg);
        sqlite3_close(db);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

TEST(sqlite, api)
{
    int safe = sqlite3_threadsafe();
    printf("threadsfe is %d\n", safe);

    auto ver = sqlite3_libversion();
    printf("version is %s\n", ver);
}
