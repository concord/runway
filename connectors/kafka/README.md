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

The `kafka_consumer_group_id` and `kafka_topics` command line arguments must be provided
or the operator will log FATAL and exit.

### Usage

The Kafka source will push records downstream on a stream that is dynamically named depending
on your kafka topic names. There will be one output stream per kafka topic provided, and the
names of your output streams will match your topics. For example if you have provided
`--kafka_topics=words` then your word counting operator should expect words to be provided
on a stream named `words`.

### Deployment

To deploy your operator simply use the `concord runway` command. Here is an example manifest
that passes any flags onto the kafka sink:

```
{
  "executable_arguments" : [
	"--kafka_brokers=localhost:9092",
	"--kafka_topics=words",
	"--kafka_topics_consume_from_beginning=true",
	"--kafka_consumer_group_id=words_group"
  ],
  "computation_name" : "kafka-source-1"
}
```
