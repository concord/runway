# Cassandra Sink
Save incoming stream data to Cassandra... fast

### Initiliazation 

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
the given value of 'keyspace' to 'table' with a period seperating them: 

```cpp
if (inputStreams.empty()) {
  inputStreams = folly::sformat("{}.{}", keyspace, table);
}
```

### Usage

This operator expects all incoming streaming data to be formatted as a JSON object. This object
representing the data it will push to Cassandra, format your JSON like so:

```json
{
  "values" : {
    "column1" : "value1",
    "column2" : "value2",
    ...
    "columnN" : "valueN"
  }
}
```

### Deployment

To deploy your operator simply use the `concord runway` command. This is still in development, please
check back soon!

