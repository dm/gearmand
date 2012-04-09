/* Gearman sserver MongoDB queue adapter
 * Copyright (C) 2012 Marc Easen
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 */

#include <config.h>
#include <libgearman-server/common.h>

#include <libgearman-server/plugins/queue/base.h>
#include <libgearman-server/plugins/queue/mongodb/queue.h>


// Save gearmand preprocessor values and remove
#define _VERSION VERSION                // VERSION collision will be removed MongoDB 2.1.0
#undef VERSION
#define _INVALID_SOCKET INVALID_SOCKET
#undef INVALID_SOCKET

#include <mongo/client/dbclient.h>

// Remove MongoDB preprocessor values
#undef VERSION
#undef INVALID_SOCKET

// Add the Gearman ones back in
#define VERSION _VERSION
#undef _VERSION
#define INVALID_SOCKET _INVALID_SOCKET
#undef INVALID_SOCKET


#pragma GCC diagnostic ignored "-Wold-style-cast"

using namespace gearmand;
using namespace mongo;


/**
 * Default values.
 */
#define GEARMAN_QUEUE_MONGODB_DEFAULT_COLLECTION "gearmand.queue"

namespace gearmand { namespace plugins { namespace queue { class Mongodb;  }}}

static gearmand_error_t
_initialize(plugins::queue::Mongodb *queue_obj,
            gearman_server_st *server);

namespace gearmand {
namespace plugins {
namespace queue {

class Mongodb : public gearmand::plugins::Queue {
public:
  Mongodb ();
  ~Mongodb ();

  gearmand_error_t initialize();

  mongo::DBClientConnection dbclient;
  std::string server;
private:

};

Mongodb::Mongodb() :
  Queue("mongodb")
{
  command_line_options().add_options()
    ("mongodb-server", boost::program_options::value(&server), "Location of the mongodb server (=localhost)");
}

Mongodb::~Mongodb()
{
}

gearmand_error_t Mongodb::initialize()
{
  return _initialize(this, &Gearmand()->server);
}

void initialize_mongodb()
{
  static Mongodb local_instance;
}

} // namespace queue
} // namespace plugins
} // namespace gearmand

/* Queue callback functions. */
static gearmand_error_t _mongodb_add(gearman_server_st *server,
                                          void *context,
                                          const char *unique, size_t unique_size,
                                          const char *function_name, size_t function_name_size,
                                          const void *data, size_t data_size,
                                          gearmand_job_priority_t priority,
					  int64_t when);

static gearmand_error_t _mongodb_flush(gearman_server_st *server,
                                            void *context);

static gearmand_error_t _mongodb_done(gearman_server_st *server,
                                           void *context,
                                           const char *unique, size_t unique_size,
                                           const char *function_name, size_t function_name_size);

static gearmand_error_t _mongodb_replay(gearman_server_st *server,
                                             void *context,
                                             gearman_queue_add_fn *add_fn,
                                             void *add_context);

/** @} */

/*
 * Public definitions
 */

gearmand_error_t _initialize(plugins::queue::Mongodb *queue,
                             gearman_server_st *server)
{
  gearmand_info("Initializing mongodb module");
  try {
    queue->dbclient.connect(queue->server.c_str());
  } catch (mongo::DBException &e) {
    gearmand_log_error(GEARMAN_DEFAULT_LOG_PARAM, "unable to connect to mongodb server %s", queue->server.c_str());

    return GEARMAN_QUEUE_ERROR;
  }

  gearman_server_set_queue(server, queue, _mongodb_add, _mongodb_flush, _mongodb_done, _mongodb_replay);

  return GEARMAN_SUCCESS;
}

/*
 * Static definitions
 */

static gearmand_error_t _mongodb_add(gearman_server_st *server,
                                          void *context,
                                          const char *unique,
                                          size_t unique_size,
                                          const char *function_name,
                                          size_t function_name_size,
                                          const void *data, size_t data_size,
                                          gearmand_job_priority_t priority,
					  int64_t when)
{

  gearmand_error_t ret= GEARMAN_SUCCESS;

  gearmand_log_debug(GEARMAN_DEFAULT_LOG_PARAM, "mongodb add: %.*s", (uint32_t)unique_size, (char *)unique);

  gearmand::plugins::queue::Mongodb *queue= (gearmand::plugins::queue::Mongodb *)context;

  // Copy the data to the correct size
  void *data_storage = malloc(data_size);
  memcpy(data_storage, data, data_size);

  BSONObjBuilder b;
  b.append("function_name", function_name);
  b.appendNumber("function_name_size", (int)function_name_size);
  b.append("priority", priority);
  b.append("unique", unique);
  b.appendNumber("unique_size", (int)unique_size);
  b.appendBinData("data", data_size, BinDataGeneral, (const char *)data_storage);
  b.appendNumber("data_size", (int)data_size);
  b.appendNumber("when", (int)when);
  BSONObj job = b.obj();

  try {
    queue->dbclient.insert(GEARMAN_QUEUE_MONGODB_DEFAULT_COLLECTION, job);
    queue->dbclient.ensureIndex(GEARMAN_QUEUE_MONGODB_DEFAULT_COLLECTION, BSON("function_name" << 1 << "priority" << 1 ));
  } catch (mongo::DBException &e) {
      gearmand_log_error(GEARMAN_DEFAULT_LOG_PARAM, "unable to add job to MongoDB %s", e.what());
      ret = GEARMAN_QUEUE_ERROR;
  }

  free(data_storage);
  return ret;
}

static gearmand_error_t _mongodb_flush(gearman_server_st *server,
                                            void *context)
{
  gearmand_debug("mongodb flush");
  (void)server;
  (void)context;

  return GEARMAN_SUCCESS;
}

static gearmand_error_t _mongodb_done(gearman_server_st *server,
                                           void *context,
                                           const char *unique, size_t unique_size,
                                           const char *function_name, size_t function_name_size)
{
  gearmand_info("mongodb done start");
  gearmand::plugins::queue::Mongodb *queue= (gearmand::plugins::queue::Mongodb *)context;

  queue->dbclient.remove(GEARMAN_QUEUE_MONGODB_DEFAULT_COLLECTION, QUERY( "unique" << unique << "function_name" << function_name), true );

  return GEARMAN_SUCCESS;
}

/*
  If we have any failures for loading values back into replay we just ignore them.
*/
static gearmand_error_t _mongodb_replay(gearman_server_st *server, void *context,
                                             gearman_queue_add_fn *add_fn,
                                             void *add_context)
{
  gearmand_info("mongodb replay start");
  gearmand::plugins::queue::Mongodb *queue= (gearmand::plugins::queue::Mongodb *)context;

  auto_ptr<DBClientCursor> cursor = queue->dbclient.query(GEARMAN_QUEUE_MONGODB_DEFAULT_COLLECTION, BSONObj());
  while(cursor->more()) {
    BSONObj job = cursor->next();

    int data_size;
    char *data = (char *)job.getField("data").binData(data_size);
    void *correct_data = malloc(data_size);
    memcpy(correct_data, data, data_size + 1);

    gearmand_error_t gret = (*add_fn)(
      server,
      add_context,
      job.getStringField("unique"),
      job.getIntField("unique_size"),
      job.getStringField("function_name"),
      job.getIntField("function_name_size"),
      (void *)correct_data,
      data_size,
      static_cast<gearmand_job_priority_t>(job.getIntField("priority"), 0),
      job.getIntField("when")
    );

    if (gearmand_failed(gret))
    {
      return gret;
    }
    cout << "added" << endl;
  }

  gearmand_info("mongodb replay finished successfully");

  return GEARMAN_SUCCESS;
}
