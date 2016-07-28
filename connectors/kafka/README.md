# Kafka Source
Pulls records from Kafka into a Concord topology

### Initialization

The Kafka source accepts 4 command line arguments for configuration. You can pass
these flags onto the computation through your operator manifest. Create an array of
arguments and set that to the "executable_arguments" key. For more information on
the concord CLI and how to deploy operators check out
[our docs](http://concord.io/docs/tutorials/cli.html#computation-json-manifest).
As for what args you can pass and what they do, information can be found in the source:

```cpp
DEFINE_string(kafka_brokers, "localhost:9092", "seed kafka brokers");
DEFINE_string(kafka_topics, "", "coma delimited list of topics");
DEFINE_bool(kafka_topics_consume_from_beginning,
            false,
            "should the driver consume from the begining");
DEFINE_string(kafka_consumer_group_id, "", "name of the consumer group");
```

If `kafka_consumer_group_id` is unset, then the operator will default the Kafka driver
`group.id` to  ????

### Usage

The Kafka source will push records downstream on a stream that is dynamically named depending
on your kafka topic names. The output stream should look like this:
`kafka_source_<topic_name_1>.<topic_name_2>.<topic_name_n>`. Here is the relevent souce:

```cpp
  virtual bolt::Metadata metadata() override {
    bolt::Metadata m;
    m.name = "kafka_source_" + folly::join(".", ostreams_);
  ...
```

### Deployment

To deploy your operator simply use the `concord runway` command. This is still in development, please
check back soon!

