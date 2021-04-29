#include "../include/internal.h"

static FILE* database = nullptr;
static DCS::DB::User* users = nullptr;
static DCS::u64 num_users = 0;

// TODO : users.db name / directory could vary (not really important but...)
void DCS::DB::LoadDefaultDB()
{
    LOG_DEBUG("Opening user database.");

    database = fopen("users.db", "rb+");

    if(database == nullptr)
    {
        LOG_WARNING("Failed to find users.db. Creating new database file.");

        database = fopen("users.db", "wb+");

        if(database == nullptr)
        {
            LOG_CRITICAL("Failed to open user database file. Maybe is already in use?");
        }
    }
}

void DCS::DB::CloseDB()
{
    if(database != nullptr)
    {
        LOG_DEBUG("Closing users.db file.");
        fclose(database);
        database = nullptr;
    }
}

void DCS::DB::LoadUsers()
{
    if(database != nullptr)
    {
        LOG_DEBUG("Loading users from users.db");

        fseek(database, 0, SEEK_END);
        u64 fsize = ftell(database);
        rewind(database);

        if(fsize == 0)
        {
            LOG_WARNING("User accounts database is empty. Skipping...");
            return;
        }

        u64 db_size;
        fread(&db_size, sizeof(u64), 1, database);

        num_users = db_size;

        if(users != nullptr) free(users);

        users = (User*)malloc(db_size);

        if(users == nullptr)
        {
            LOG_ERROR("Failed to allocate users database pointer.");
            return;
        }

        for(u64 i = 0; i < db_size; i++)
        {
            // Read here, avoid more heap buffers
            User u;
            fread(&u, sizeof(User), 1, database);
            memcpy(users + i, &u, sizeof(User));
        }
    }
}

void DCS::DB::AddUser(User usr)
{
    std::string ps = std::string(usr.p);

    // Guard vs spaces in username and password
    if(std::string(usr.u).find(' ') != std::string::npos || ps.find(' ') != std::string::npos)
    {
        LOG_ERROR("Failed adding new user. Username or password cannot contain spaces.");
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

    if(num_users == 0)
    {
        users = (User*)malloc(sizeof(User));

        if(users == nullptr)
        {
            LOG_ERROR("Failed to allocate users database pointer.");
            return;
        }
    }
    else
    {
        u64 offset = FindUserByUsername(usr.u);

        if(offset != num_users)
        {
            LOG_ERROR("Cannot create user with username %s. Reason: duplicate username.", usr.u);
            return;
        }

        User* ob = users;
        users = (User*)realloc(users, (num_users + 1) * sizeof(User));

        if(users == nullptr)
        {
            LOG_ERROR("Failed to reallocate users database pointer.");
            free(ob);
            return;
        }
    }
    *(users + num_users++) = usr;

    if(database != nullptr)
    {
        LOG_DEBUG("Adding new user to database.");

        fseek(database, 0, SEEK_SET);
        fwrite(&num_users, sizeof(u64), 1, database);

        fseek(database, 0, SEEK_END);
        fwrite(&usr, sizeof(User), 1, database);
    }
}

void DCS::DB::RemoveUserByUsername(const char* username)
{
    u64 offset = FindUserByUsername(username);
    if(offset != num_users)
    {
        u64 next = offset + sizeof(User);
        memmove(users + offset, users + next, num_users * sizeof(User) - next);

        if(num_users > 1)
        {
            User* ob = users;
            users = (User*)realloc(users, (num_users-- - 1) * sizeof(User));
            if(users == nullptr)
            {
                LOG_ERROR("Failed to reallocate users database pointer.");
                free(ob);
                return;
            }
        }
        else
        {
            num_users = 0;
            free(users);
        }

        if(database != nullptr)
        {
            LOG_DEBUG("Removing user from database.");

            // This is a nasty trick to clear file contents
            fclose(database);
            database = fopen("users.db", "wb");
            
            if(database != nullptr && num_users > 0)
            {
                fseek(database, 0, SEEK_SET);
                fwrite(&num_users, sizeof(u64), 1, database);
                fwrite(users, sizeof(User), num_users, database);
                fclose(database);
            }

            database = fopen("users.db", "rb+");
        }
    }
}

DCS::u64 DCS::DB::FindUserByUsername(const char* username)
{
    for(u64 i = 0; i < num_users; i++)
    {
        if(std::string((*(users + i)).u) == username)
            return i;
    }
    return num_users;
}

DCS::DB::User DCS::DB::GetUser(const char* username)
{
    User u; 
    strcpy(u.u, "INVALID USER");

    u64 offset = FindUserByUsername(username);
    if(offset != num_users)
        return *(users + offset);
    return u;
}
