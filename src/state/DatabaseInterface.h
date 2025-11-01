#pragma once

#include "db/Database.h"

namespace state {

class DatabaseInterface {
private:
  static db::Database* database;

protected:
  static db::Database* getDatabase();

public:
  static void setDatabase(db::Database* _database);
};

} // namespace state