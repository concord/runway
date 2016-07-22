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
DEFINE_uint64(max_async_inserts, 10,
              "Maximum number of asynchronous inserts to Cassandra cluster");

int main(int argc, char *argv[]) {
  google::SetUsageMessage(
      "CassandraSink Operator:\n"
      "\t--keyspace Cassandra keyspace name\n"
      "\t--table Cassandra table name\n"
      "\t--contact_points Cassandra cluster public IPs\n"
      "\t--computation_name Name of operator to use in topology\n"
      "\t--input_streams Streams to listen onto\n"
      "\t--max_async_inserts Maximum Asynchronous Inserts to cluster\n"
      "\n");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  bolt::logging::glog_init(argv[0]);
  bolt::client::serveComputation(
      std::make_shared<concord::CassandraSink>(
          FLAGS_keyspace, FLAGS_table, FLAGS_contact_points,
          FLAGS_computation_name, FLAGS_input_streams, FLAGS_max_async_inserts),
      argc, argv);
  return 0;
}
