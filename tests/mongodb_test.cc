/* Gearman server and library
 * Copyright (C) 2008 Brian Aker, Eric Day
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 */

#include <config.h>
#include <libtest/test.hpp>

using namespace libtest;

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libgearman/gearman.h>

#include <tests/basic.h>
#include <tests/context.h>

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

static test_return_t gearmand_basic_option_test(void *)
{
  const char *args[]= { "--check-args",
    "--queue-type=mongodb",
    "--mongodb-server=localhost",
    0 };

  test_compare(EXIT_SUCCESS, exec_cmdline(gearmand_binary(), args, true));

  return TEST_SUCCESS;
}

static test_return_t collection_init(void *object)
{
  const char *argv[]= {
    "--queue-type=mongodb",
    "--mongodb-server=localhost",
    0 };


  Context *test= (Context *)object;
  assert(test);

  test_truth(test->initialize(2, argv));

  return TEST_SUCCESS;
}

static test_return_t collection_cleanup(void *object)
{
  Context *test= (Context *)object;
  test->reset();

  return TEST_SUCCESS;
}


static void *world_create(server_startup_st& servers, test_return_t& error)
{
  if (HAVE_MONGOCLIENT == 0)
  {
    error= TEST_SKIPPED;
    return NULL;
  }

  return new Context(libtest::default_port(), servers);
}

static bool world_destroy(void *object)
{
  Context *test= (Context *)object;

  delete test;

  return TEST_SUCCESS;
}

test_st gearmand_basic_option_tests[] ={
  {"--queue-type=mongodb --mongodb-server=", 0, gearmand_basic_option_test },
  {0, 0, 0}
};


test_st tests[] ={
  {"gearman_client_echo()", 0, client_echo_test },
  {"gearman_client_echo() fail", 0, client_echo_fail_test },
  {"gearman_worker_echo()", 0, worker_echo_test },
  {"clean", 0, queue_clean },
  {"add", 0, queue_add },
  {"worker", 0, queue_worker },
  {0, 0, 0}
};

collection_st collection[] ={
  {"gearmand options", 0, 0, gearmand_basic_option_tests},
  {"mongodb queue", collection_init, collection_cleanup, tests},
  {0, 0, 0, 0}
};

void get_world(Framework *world)
{
  world->collections= collection;
  world->_create= world_create;
  world->_destroy= world_destroy;
}
