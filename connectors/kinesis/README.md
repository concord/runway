Concord Kinesis Consumer
========================
The Concord Kinesis consumer allows Concord users to quickly and easily ingest
messages from Kinesis queues into their Concord topologies. The project is,
itself, open source and buildable, but can be used by simply fetching a prebuilt
megajar and configuring a task deployment.

Downloading
-----------
The latest release can be found at the following URL:
https://storage.googleapis.com/concord-release/concord_kinesis_consumer-0.1.0.jar

Running
-------
To launch a Kinesis consumer, you'll need two things:

- A jar of a recent build of the Concord Kinesis consumer
- A run script / "shim"
- A json deployment manifest

### Shim
The run script is responsible for defining the coniguration options. These
include the following command line switches:

- `-topic` (_required_): The name of the topic to consume from Kinesis
- `-ostreams` (_required_): A comma separated list of Concord streams on which
  we will emit every record pulled off Kinesis.
- `-awskey` (_required_): Your AWS Access Key ID
- `-awssecret` (_required_): Your AWS Secret Key
- `-name` (_required_): The name of the computation (to register with Concord)
- `-queuesize` (_default: 16384_): The maximum number of messages to buffer in
  memory

An example shim, `runner.bash` might look like this:

```bash
#!/bin/bash

JAR="/path/to/concord-kinesis.jar"
AWSKEY="..."
AWSSECRET="..."

exec java -jar $JAR -topic my-topic -ostreams foo,bar\
  -awskey $AWSKEY -awssecret $AWSSECRET -name kinesis-consumer
```

### JSON Manifest
An example JSON manifest for the above configuration might look like this:

```json
{
  "zookeeper_hosts": "localhost:2181",
  "zookeeper_path": "/concord",
  "executable_name": "runner.bash",
  "compress_files": ["concord_kinesis_consumer.jar", "runner.bash"],
  "computation_name": "kinesis-consumer",
  "mem": 1024,
  "cpus": 1.0,
  "instances": 1
}
```

### Directory structure
For this example, a simple flat directory structure will suffice:

```
kinesis-example/
  deploy.json
  runner.bash
  concord_kinesis_consumer.jar
```

### Launching

To launch, simply run:

```
$ concord deploy deploy.json
```

and look at the Mesos dashboard. You should see the computation running!

