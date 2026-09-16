#include "DatabaseInterface.h"
#include "in3/QuestProgress.h"

namespace state {

db::Database* DatabaseInterface::database = nullptr;

void DatabaseInterface::setDatabase(db::Database* _database) {
  database = _database;
  in3::setQuestTemplates(_database ? &_database->getQuestTemplates() : nullptr);
}

db::Database* DatabaseInterface::getDatabase() { return database; }

} // namespace state