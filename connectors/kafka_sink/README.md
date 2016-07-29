# Kafka Sink
Pushes concord records to a kafka cluster

### Initialization

The Kafka source accepts 3 command line arguments for configuration. You can pass
these flags onto the computation through your operator manifest. Create an array of
arguments and set that to the "executable_arguments" key. For more information on
the concord CLI and how to deploy operators check out
[our docs](http://concord.io/docs/tutorials/cli.html#computation-json-manifest).
As for what args you can pass and what they do, information can be found in the source:

```cpp
DEFINE_string(kafka_brokers, "127.0.0.1:9092", "seed kafka brokers");
DEFINE_string(kafka_topics, "", "coma delimited list of topics to produce to");
DEFINE_string(istreams, "", "coma delimed list of streams to listen on");
```
As this operator is a sink it does not contain any ostreams. You must provide istreams
as a command line argument or this operator will log FATAL and exit. A list of kafka topics
will also be neccessary in order for this operator to be properly initialized. 

### Usage

Because this operator is stateless is adheres to the ROUND_ROBIN stream grouping strategy.
All incoming records will be pushed to Kafka onto all given `kafka_topics`. 

```cpp
for (const auto &topic : topics_) {
  producer_.produce(topic, r.key, r.value);
}
```

When sending records to this operator remember that the second parameter to produceRecord 
is for a key. The kafka sink will use this as the key for the data passed in via 
`binaryValue`.

```cpp
  /** Emit a record downstream.
   *
   * @param[in] streamName  The name of the stream on which the record should be
   *                        emitted.
   * @param[in] binaryKey   The key associated with the record. Mostly relevant
   *                        when routing method is `GROUP_BY`.
   * @param[in] binaryValue The binary blob to send downstream.
   */
  virtual void produceRecord(const std::string &streamName,
                             const std::string &binaryKey,
                             const std::string &binValue) = 0;
```

### Deployment

To deploy this operataor simply use the `concord runway` command. You'll need to provide
a value for `zookeeper_hosts` and `zookeeper_path`, the easiest way to do this is to set
this via `concord config`. To pass specific operator arguments such as the ones discussed
above, create a manifest and pass it to `concord runway` using the `-c` option:

```
{
  "computation_name": "kafka-sink",
  "executable_arguments": [
    "--istreams=words",
    "--kafka_topics=new_words",
    "--kafka_brokers=127.0.0.1:9092"
  ]
}
```
