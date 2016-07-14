#include <gflags/gflags.h>
#include <concord/glog_init.hpp>
#include "CassandraSink.hpp"

DEFINE_string(keyspace, "", "Cassandra keyspace");
DEFINE_string(table, "", "Name of table");
DEFINE_string(contact_points, "127.0.0.1",
              "Comma seperated list of ips of nodes of cassandra cluster");
DEFINE_string(computation_name, "cassandra-sink",
              "Name of the Concord operator");
DEFINE_string(input_streams, "",
              "Comma seperated list incoming streams to listen on");

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  bolt::logging::glog_init(argv[0]);
  bolt::client::serveComputation(std::make_shared<concord::CassandraSink>(
      FLAGS_keyspace, FLAGS_table, FLAGS_contact_points, FLAGS_computation_name,
      FLAGS_input_streams, 10000), argc, argv);
  return 0;
}
