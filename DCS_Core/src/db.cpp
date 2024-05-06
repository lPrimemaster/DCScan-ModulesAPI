#include "../include/internal.h"
#include "../include/DCS_ModuleCore.h"
#include <memory>
#include <filesystem>
#include <unordered_map>
#include <sqlite3.h>
#include <cstring>

static std::string authenticated_username;
static std::unordered_map<std::string, std::string> db_var_cache;
static sqlite3* realtime_db;

void DCS::RDB::OpenDatabase()
{
#ifdef WIN32
    std::string appdata_env = std::getenv("LocalAppData");
#else
    std::string appdata_env = std::getenv("HOME");
#endif
    std::string database_path = appdata_env + "/DCScan";
    std::filesystem::create_directory(database_path);

    if(sqlite3_open((database_path + "/rdb.db").c_str(), &realtime_db))
    {
        LOG_ERROR("Can't open database: %s", sqlite3_errmsg(realtime_db));
    }
    else
    {
        LOG_DEBUG("Opening realtime database.");
    }
}

void DCS::RDB::CloseDatabase()
{
    sqlite3_close(realtime_db);
}

void DCS::RDB::QueryEntries(const std::string& table,
                            const std::string& column,
                            const std::string& statement,
                            EntryType type,
                            void* data)
{
    sqlite3_stmt* stmt;
    const char* tail;
    const std::string query = "SELECT " + column + " FROM " + table + " WHERE " + statement;
    if(sqlite3_prepare(realtime_db, query.c_str(), (int)query.size(), &stmt, &tail) != SQLITE_OK)
    {
        LOG_ERROR("sqlite3_prepare error: %s", sqlite3_errmsg(realtime_db));
        return;
    }

    int i = 0;
    switch(type)
    {
        case EntryType::TEXT:
        {
            auto vector = static_cast<std::vector<std::string>*>(data);
            vector->clear();
            while(sqlite3_step(stmt) != SQLITE_DONE)
            {
                vector->emplace_back((char*)sqlite3_column_text(stmt, i++));
            }
        } break;
        case EntryType::INT:
        {
            auto vector = static_cast<std::vector<int>*>(data);
            vector->clear();
            while(sqlite3_step(stmt) == SQLITE_ROW)
            {
                vector->emplace_back(sqlite3_column_int(stmt, i++));
            }
        } break;
        case EntryType::REAL:
        {
            auto vector = static_cast<std::vector<double>*>(data);
            vector->clear();
            while(sqlite3_step(stmt) == SQLITE_ROW)
            {
                vector->emplace_back(sqlite3_column_double(stmt, i++));
            }
        } break;
        case EntryType::DATE:
        {
            auto vector = static_cast<std::vector<std::string>*>(data);
            vector->clear();
            while(sqlite3_step(stmt) == SQLITE_ROW)
            {
                vector->emplace_back((char*)sqlite3_column_text(stmt, i++));
            }
        } break;
        case EntryType::BLOB:
        {
            auto vector = static_cast<std::vector<std::vector<u8>>*>(data);
            vector->clear();
            while(sqlite3_step(stmt) == SQLITE_ROW)
            {
                const u8* temp = static_cast<const u8*>(sqlite3_column_blob(stmt, i++));
                size_t blob_sz; memcpy(&blob_sz, temp, sizeof(size_t));
                vector->emplace_back(temp, temp + blob_sz);
            }
        } break;
    }
    
    sqlite3_finalize(stmt);
}

template<typename ...Args>
static void InsertEntry(const std::string& table,
                 const std::string& fields,
                 Args&&... args)
{
    sqlite3_stmt* stmt;
    const char* tail;
    std::string row = "INSERT OR REPLACE INTO " + table + " (" + fields + ") VALUES (";
    for(auto i = 0; i < sizeof...(Args) - 1; i++)
    {
        row += "?,";
    }
    row += "?)";

    if(sqlite3_prepare_v2(realtime_db, row.c_str(), (int)row.size(), &stmt, &tail) != SQLITE_OK)
    {
        LOG_ERROR("sqlite3_prepare error: %s", sqlite3_errmsg(realtime_db));
        return;
    }

    InsertEntryRecurse(stmt, row, 1, args...);
    
    int rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE)
    {
        LOG_ERROR("sqlite3_step error (%d): %s", rc, sqlite3_errmsg(realtime_db));
        return;
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
}

template<typename T>
static void ProccessBindArg(sqlite3_stmt* stmt, const std::string& row, const int index, T&& t)
{
    if constexpr(std::is_same<std::remove_reference_t<T>, const char*>::value)
    {
        if(t == nullptr)
        {
            sqlite3_bind_null(stmt, index);
        }
        else
        {
            sqlite3_bind_text(stmt, index, t, -1, SQLITE_STATIC);
        }
    }
    else if constexpr(std::is_same<std::remove_reference_t<T>, int>::value)
    {
        sqlite3_bind_int(stmt, index, t);
    }
    else if constexpr(std::is_same<std::remove_reference_t<T>, double>::value)
    {
        sqlite3_bind_double(stmt, index, t);
    }
    else if constexpr(std::is_same<std::remove_reference_t<T>, std::vector<unsigned char>>::value)
    {
        sqlite3_bind_blob(stmt, index, t.data(), (int)t.size(), SQLITE_STATIC);
    }
    else if constexpr(std::is_same<std::remove_reference_t<T>, std::nullptr_t>::value)
    {
        sqlite3_bind_null(stmt, index);
    }
}

static void InsertEntryRecurse(sqlite3_stmt* stmt, const std::string& row, const int index) {  }

template<typename T, typename ...Args>
static void InsertEntryRecurse(sqlite3_stmt* stmt,
                        const std::string& row,
                        const int index,
                        T&& t,
                        Args&&... args)
{
    ProccessBindArg(stmt, row, index, t);
    InsertEntryRecurse(stmt, row, index + 1, args...);
}

void DCS::RDB::WriteVariable(const char* name, const char* value, const char* descriptor)
{
    std::string what = std::string(name) + "=" + value;
    if(DCS::RDB::ReadVariable(name).empty())
    {
        LogEventSystem(LogOperation::VAR_CREATE, what.c_str());
    }
    else
    {
        LogEventSystem(LogOperation::VAR_UPDATE, what.c_str());
    }
    InsertEntry("VARIABLES", "NAME,VALUE,DESCRIPTOR", name, value, descriptor);
}

void DCS::RDB::WriteVariable(const char* name, f64 value, const char* descriptor)
{
    std::string what = std::string(name) + "=" + std::to_string(value);
    if(DCS::RDB::ReadVariable(name).empty())
    {
        LogEventSystem(LogOperation::VAR_CREATE, what.c_str());
    }
    else
    {
        LogEventSystem(LogOperation::VAR_UPDATE, what.c_str());
    }
    InsertEntry("VARIABLES", "NAME,VALUE,DESCRIPTOR", name, std::to_string(value).c_str(), descriptor);
}

void DCS::RDB::WriteVariableSys(const char* name, const char* value, const char* descriptor)
{
    std::string what = std::string(name) + "=" + value;
    // if(DCS::RDB::ReadVariable(name).empty())
    // {
    //     LogEventSystem(LogOperation::VAR_CREATE, what.c_str());
    // }
    // else
    // {
    //     LogEventSystem(LogOperation::VAR_UPDATE, what.c_str());
    // }
    InsertEntry("VARIABLES", "NAME,VALUE,DESCRIPTOR", name, value, descriptor);
}

void DCS::RDB::WriteVariableSys(const char* name, f64 value, const char* descriptor)
{
    std::string what = std::string(name) + "=" + std::to_string(value);
    // if(DCS::RDB::ReadVariable(name).empty())
    // {
    //     LogEventSystem(LogOperation::VAR_CREATE, what.c_str());
    // }
    // else
    // {
    //     LogEventSystem(LogOperation::VAR_UPDATE, what.c_str());
    // }
    InsertEntry("VARIABLES", "NAME,VALUE,DESCRIPTOR", name, std::to_string(value).c_str(), descriptor);
}


std::string DCS::RDB::ReadVariable(const char* name)
{
    std::vector<std::string> rows;
    QueryEntries("VARIABLES", "VALUE", "NAME = \"" + std::string(name) + "\"", EntryType::TEXT, &rows);
    if(rows.empty())
    {
        // NOTE: Ommit this since we are using the function to check if a variable exists
        //       Like so, we remove unnecessary output verbosity
        // LOG_ERROR("Could not find database variable named \'%s\'.", name);
        return std::string();
    }
    return rows[0];
}

std::string DCS::RDB::GetVariableDescriptor(const char* name)
{
    std::vector<std::string> rows;
    QueryEntries("VARIABLES", "DESCRIPTOR", "NAME = \"" + std::string(name) + "\"", EntryType::TEXT, &rows);
    if(rows.empty())
    {
        LOG_ERROR("Could not find database variable named \'%s\'.", name);
        return std::string();
    }
    return rows[0];
}

static bool DeleteVariableInternal(const char* name)
{
    if(DCS::RDB::ReadVariable(name).empty())
    {
        return false; // Variable does not exist
    }

    sqlite3_stmt* stmt;
    const char* tail;
    const std::string query = "DELETE FROM VARIABLES WHERE NAME = \"" + std::string(name) + "\"";
    if(sqlite3_prepare(realtime_db, query.c_str(), (int)query.size(), &stmt, &tail) != SQLITE_OK)
    {
        LOG_ERROR("sqlite3_prepare error: %s", sqlite3_errmsg(realtime_db));
        return false;
    }

    int rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE)
    {
        LOG_ERROR("sqlite3_step error (%d): %s", rc, sqlite3_errmsg(realtime_db));
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

void DCS::RDB::DeleteVariable(const char* name)
{
    if(DeleteVariableInternal(name))
    {
        LogEventUser(LogOperation::VAR_DELETE, name);
    }
}

void DCS::RDB::DeleteVariableSys(const char* name)
{
    if(DeleteVariableInternal(name))
    {
        LogEventSystem(LogOperation::VAR_DELETE, name);
    }
}

void DCS::RDB::LogEventUser(LogOperation op, const char* what)
{
    InsertEntry(
        "EVENTS",
        "USERNAME,OPERATION,OP_DESCRIPTION",
        authenticated_username.c_str(),
        LogOperationNames[std::underlying_type_t<LogOperation>(op)],
        what
    );
}

void DCS::RDB::LogEventSystem(LogOperation op, const char* what)
{
    const char* name = "System";
    InsertEntry(
        "EVENTS",
        "USERNAME,OPERATION,OP_DESCRIPTION",
        name,
        LogOperationNames[std::underlying_type_t<LogOperation>(op)],
        what
    );
}

void DCS::RDB::CreateTables()
{
    const char* events_table =
        "CREATE TABLE IF NOT EXISTS EVENTS ("
        "EVENT_ID        INTEGER     PRIMARY KEY AUTOINCREMENT         ,"
        "EVENT_TIME      DATETIME    DEFAULT CURRENT_TIMESTAMP NOT NULL,"
        "USERNAME        TEXT        NOT NULL                          ,"
        "OPERATION       TEXT        NOT NULL                          ,"
        "OP_DESCRIPTION  TEXT        NOT NULL                          );"
    ;

    const char* variables_table =
        "CREATE TABLE IF NOT EXISTS VARIABLES ("
        "NAME       TEXT             PRIMARY KEY NOT NULL,"
        "VALUE      TEXT             NOT NULL            ,"
        "DESCRIPTOR TEXT                                );"
    ;

    const char* users_table =
        "CREATE TABLE IF NOT EXISTS USERS ("
        "ID         INTEGER          PRIMARY KEY AUTOINCREMENT,"
        "USERNAME   TEXT             UNIQUE NOT NULL          ,"
        "PASS_HASH  BLOB             NOT NULL                 ,"
        "ACCESS_LVL INT              NOT NULL                );"
    ;

    char* err_msg = nullptr;
    if(sqlite3_exec(realtime_db, events_table, nullptr, 0, &err_msg) != SQLITE_OK)
    {
        LOG_ERROR("SQL error: %s", err_msg);
        sqlite3_free(err_msg);
    }
    if(sqlite3_exec(realtime_db, variables_table, nullptr, 0, &err_msg) != SQLITE_OK)
    {
        LOG_ERROR("SQL error: %s", err_msg);
        sqlite3_free(err_msg);
    }
    if(sqlite3_exec(realtime_db, users_table, nullptr, 0, &err_msg) != SQLITE_OK)
    {
        LOG_ERROR("SQL error: %s", err_msg);
        sqlite3_free(err_msg);
    }
}

void DCS::RDB::AddUser(const char* username, const char* password)
{
    std::string ps = std::string(password);

    // Guard vs username length
    if(strlen(username) >= 32)
    {
        LOG_ERROR("Failed adding new user. Username must be at most 32 characters.");
        return;
    }

    // Guard vs spaces in username and password
    if(std::string(username).find(' ') != std::string::npos || ps.find(' ') != std::string::npos)
    {
        LOG_ERROR("Failed adding new user. Username or password cannot contain spaces.");
        return;
    }

    // Guard vs existing username
    if(std::string(DCS::RDB::GetUser(username).u) != "INVALID_USER")
    {
        LOG_ERROR("Failed adding new user. Username already exists.");
        return;
    }

    if(ps.size() < 5)
    {
        LOG_ERROR("Failed adding new user. Password must be have at least 5 chars.");
        return;
    }

    if(std::find_if(ps.begin(), ps.end(), [](char c) { return std::isdigit(c); }) == ps.end())
    {
        LOG_ERROR("Failed adding new user. Password must contain at least one digit.");
        return;
    }

    // Save password hash
    u8 hash[40];
    Auth::SHA256Str(password, &hash[8]);
    size_t blob_size = 40;
    memcpy(hash, &blob_size, sizeof(size_t));
    std::vector<u8> blob_data(hash, hash + 40);
    InsertEntry("USERS", "USERNAME,PASS_HASH,ACCESS_LVL", username, blob_data, 0);
    LogEventSystem(LogOperation::USR_CREATE, username);
}


void DCS::RDB::RemoveUser(const char* username)
{
    if(std::string(DCS::RDB::GetUser(username).u) == "INVALID_USER")
    {
        return; // User does not exist
    }

    sqlite3_stmt* stmt;
    const char* tail;
    const std::string query = "DELETE FROM USERS WHERE USERNAME = \"" + std::string(username) + "\"";
    if(sqlite3_prepare(realtime_db, query.c_str(), (int)query.size(), &stmt, &tail) != SQLITE_OK)
    {
        LOG_ERROR("sqlite3_prepare error: %s", sqlite3_errmsg(realtime_db));
        return;
    }

    int rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE)
    {
        LOG_ERROR("sqlite3_step error (%d): %s", rc, sqlite3_errmsg(realtime_db));
        return;
    }

    sqlite3_finalize(stmt);
    LogEventSystem(LogOperation::USR_DELETE, username);
}

DCS::RDB::User DCS::RDB::GetUser(const char* username)
{
    std::vector<std::string> rows;
    QueryEntries("USERS", "USERNAME", "USERNAME = \"" + std::string(username) + "\"", EntryType::TEXT, &rows);
    if(rows.empty())
    {
        LOG_ERROR("Could not find user named \'%s\'.", username);
        User user;
        strcpy(user.u, "INVALID_USER");
        return user;
    }

    User user;
    strcpy(user.u, rows[0].c_str());

    std::vector<std::vector<u8>> hash;
    QueryEntries("USERS", "PASS_HASH", "USERNAME = \"" + std::string(username) + "\"", EntryType::BLOB, &hash);
    memcpy(user.p, hash[0].data() + 8, 32);
    return user;
}

const std::vector<DCS::RDB::User> DCS::RDB::GetAllUsers()
{
    std::vector<std::string> usernames;
    std::vector<DCS::RDB::User> users;
    QueryEntries("USERS", "USERNAME", "1==1", EntryType::TEXT, &usernames);
    if(usernames.empty())
    {
        LOG_ERROR("Could not find any user in the database.");
        return users;
    }
    LOG_DEBUG("%llu", usernames.size());
    for (auto r : usernames) LOG_DEBUG("%s", r.c_str());

    users.reserve(usernames.size());
    for(auto& u : usernames)
    {
        User user;
        strcpy(user.u, u.c_str());
        users.push_back(user);
    }

    std::vector<std::vector<u8>> hash;
    QueryEntries("USERS", "PASS_HASH", "1==1", EntryType::BLOB, &hash);
    for(u64 i = 0; i < users.size(); i++)
    {
        memcpy(users[i].p, hash[i].data() + 8, 32);
    }
    
    return users;
}

void DCS::RDB::SetAuthenticatedUser(const User* user)
{
    if(user != nullptr)
    {
        authenticated_username = user->u;
    }
    else
    {
        authenticated_username.clear();
        authenticated_username.shrink_to_fit();
    }
}

const std::string DCS::RDB::GetAuthenticatedUsername()
{
    return authenticated_username;
}

void DCS::Database::Open()
{
    RDB::OpenDatabase();
    RDB::CreateTables();
}

void DCS::Database::Close()
{
    RDB::CloseDatabase();
}

void DCS::Database::InvalidateCache()
{
    db_var_cache.clear();
}

const DCS::f64 DCS::Database::ReadValuef64(Utils::BasicString variable)
{
    if(db_var_cache.find(variable.buffer) == db_var_cache.end())
    {
        std::string value = RDB::ReadVariable(variable.buffer);
        if(value.empty())
        {
            LOG_WARNING("Could not read database variable : %s", variable.buffer);
            return 0.0;
        }
        db_var_cache[variable.buffer] = value;
    }
    return std::atof(db_var_cache[variable.buffer].c_str());
}

const DCS::Utils::BasicString DCS::Database::ReadValueString(Utils::BasicString variable)
{
    if(db_var_cache.find(variable.buffer) == db_var_cache.end())
    {
        std::string value = RDB::ReadVariable(variable.buffer);
        if(value.empty())
        {
            LOG_WARNING("Could not read database variable : %s", variable.buffer);
            return { "\0" };
        }
        db_var_cache[variable.buffer] = value;
    }
    Utils::BasicString out;
    memcpy(out.buffer, db_var_cache[variable.buffer].c_str(), db_var_cache[variable.buffer].size() + 1);
    return out;
}

void DCS::Database::WriteValuef64(Authority auth, Utils::BasicString variable, f64 value, Utils::BasicString description)
{
    switch (auth)
    {
    case Authority::SYS:
        RDB::WriteVariableSys(variable.buffer, value, description.buffer);
        break;
    case Authority::USER:
        RDB::WriteVariable(variable.buffer, value, description.buffer);
        break;
    }

    // Invalidate the cache entry even if the write fails
    auto it = db_var_cache.find(variable.buffer);
    if(it != db_var_cache.end())
    {
        db_var_cache.erase(it);
    }
}

void DCS::Database::WriteValueString(Authority auth, Utils::BasicString variable, Utils::BasicString value, Utils::BasicString description)
{
    switch (auth)
    {
    case Authority::SYS:
        RDB::WriteVariableSys(variable.buffer, value.buffer, description.buffer);
        break;
    case Authority::USER:
        RDB::WriteVariable(variable.buffer, value.buffer, description.buffer);
        break;
    }

    // Invalidate the cache entry even if the write fails
    auto it = db_var_cache.find(variable.buffer);
    if(it != db_var_cache.end())
    {
        db_var_cache.erase(it);
    }
}

void DCS::Database::DeleteValue(Authority auth, Utils::BasicString variable)
{
    switch (auth)
    {
    case Authority::SYS:
        RDB::DeleteVariableSys(variable.buffer);
        break;
    case Authority::USER:
        RDB::DeleteVariable(variable.buffer);
        break;
    }

    // Invalidate the cache entry even if the delete fails
    auto it = db_var_cache.find(variable.buffer);
    if(it != db_var_cache.end())
    {
        db_var_cache.erase(it);
    }
}
