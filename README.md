# runway
Open source operators and templates for the Concord Stream Processor.
This project is a work in progress, information on completed connectors and templates
will be managed in this README.

### Connectors

#### Cassandra Sink

The Cassnadra sink accepts 6 command line arguments for configuration. You can pass
these flags onto the computation through your operator manifest. Create an array of
arguments and set that to the "executable_arguments" key. For more information on
the concord CLI and how to deploy operators check out
[our docs](http://concord.io/docs/tutorials/cli.html#computation-json-manifest).
As for what args you can pass and what they do, information can be found in the source:

```cpp
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
```

If no input streams are provided, the operator will construct a default by appending
the given value of 'keyspace' to 'table' with a period seperating them: 'keyspace'.'table'





