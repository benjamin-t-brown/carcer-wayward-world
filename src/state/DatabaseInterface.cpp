#include "DatabaseInterface.h"

namespace state {

db::Database* DatabaseInterface::database = nullptr;

void DatabaseInterface::setDatabase(db::Database* _database) {
  database = _database;
}

db::Database* DatabaseInterface::getDatabase() {
  return database;
}

} // namespace state