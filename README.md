# runway
Open source operators and templates for the Concord Stream Processor.

## Installation

This repository holds the source code and corresponding metadata for pre-built operators
that work with the Concord stream processor. To use any of the runway pre-built operators
you'll need to invoke the `runway` command via the concord CLI. You can install the 
concord CLI using pip:

```
$ sudo pip install concord
$ concord runway -h
usage: runway [-h] [-c config-file] [-p zookeeper-path] [-z zookeeper-hosts]
              [-r repo-url]

Deploys preconfigured Concord connectors. Either pass a standard deploy
manifest file; pass necessary zookeeper_hosts/zookeeper_path arguments through
command line flags ; or 'concord config' stored defaults will be used

optional arguments:
  -h, --help            show this help message and exit
  -c config-file, --config config-file
                        i.e: ./src/config.json
  -p zookeeper-path, --zk_path zookeeper-path
                        Path of concord topology on zk
  -z zookeeper-hosts, --zookeeper zookeeper-hosts
                        i.e: 1.2.3.4:2181,2.3.4.5:2181
  -r repo-url, --repository repo-url
                        URL to a runway repository
```

## Usage

The runway command will present you with a list of vetted operators to deploy:

```
$ concord runway
INFO:cmd.runway:Fetching concord runway metadata at: https://raw.githubusercontent.com/concord/runway/master/meta/repo_metadata.json
Select an operator to deploy: 
+-------+----------------+--------------------------------------------------+--------------+------------+------------+
| Index | Connector      | Description                                      | Last Updated | Pull Count | Star Count |
+-------+----------------+--------------------------------------------------+--------------+------------+------------+
| 1     | Cassandra Sink | Push incoming stream data to Cassandra... fast   | 2016/08/01   | 15         | 1          |
| 2     | Kafka Source   | Pulls records from Kafka into a Concord topology | 2016/08/01   | 25         | 1          |
| 3     | Kafka Sink     | Pushes concord records to a kafka cluster        | 2016/08/01   | 4          | 1          |
+-------+----------------+--------------------------------------------------+--------------+------------+------------+
Selection: 
```

After you've made your selection you may then be presented with another list of
concord runtime versions, if there are multiple for this operator. Be sure to select
the runtime that matches your scheduler.

If your operator needs additional configuration parameters to run, the runway command
will request you enter them in, showing you defaults if they are applicable. As an 
alternative to this you may  pass an operator manifest file that runway will forward
to the `deploy` command. For more information on this manifest file and the deploy
command check out our
[CLI docs](http://concord.io/docs/tutorials/cli.html#computation-json-manifest). 
However, even if your operator needs no special configuraion parameters you'll still need to
tell the CLI the location of your concord zookeeper quorum and the path it uses to
store concords metadata. Although you can include this information inside your manifest
file, runway makes this easy. Currently there are three ways of passing this
information:

1. Include the `zookeeper_hosts` and `zookeeper_path` key in the operator manifest
2. Use the -z and -p command line flags on the runway command
3. **Most convienent:** Use the `concord config` command to store your default zookeeper settings. Use it like so:

```
$ concord config init
(...setup defaults, follow prompt)
$ concord runway # All zookeeper metadata will be fetched from your config file
```

Since your zookeeper address won't change often it makes sense to keep your cluster
info in your config file, and any operator specific information in seperate files.

From here you can either deploy using a provided manifest file:
```
File: kafka_runway_manifest.json
{
  "cpus" : 2.5,
  "executable_arguments" : [
	"--kafka_brokers=localhost:9092",
	"--kafka_topics=words",
	"--kafka_topics_consume_from_beginning=true",
	"--kafka_consumer_group_id=words_group"
  ],
  "computation_name" : "my-kafka-source"
}

Terminal:
$ concord runway -c kafka_runway_manifest.json
```

Or without the `-c` flag and just follow the prompts presented.

## Publishing a Package

To publish a package, fork the runway repository and include any and all source in 
in a new folder under the `connectors/` directory. Ensure that others can easily build
run, and package your source code by including a way to replicate the steps you took to
build (docs, build scripts, etc.). Then Dockerize your connector. Create a Dockerfile
in your project folder, populate it with rules that copy your application and any
runtime dependencies into the container. Ensure that you inherit from the desired
concord runtime by including something like this at the top of your Dockerfile:

```
FROM concord/runtime_executor:<desired_version>
...
```

### Updating runway metadata

The `concord runway` command works by making an HTTP request to github to find the
file in named `repo_metadata.json` in the `meta/` folder situated at the repository
root. However, this information is not cannon, operators and versions defined here 
will only be presented to the user if they exist in their respective locations on
https://hub.docker.com. To test your deployment process, first build/tag/push your package
to dockerhub:

```
sudo docker build -t <repo_name>/<project_name>:<project_tag> .
sudo docker push <repo_name>/<project_name>:<project_tag>
```

Then modify the repository metadata struct in `meta/repo_metadata.json` to include
your package. At the moment you'll have to manually edit this JSON file, we are
currently investigating ways to make this a more automatic process. Change this
file by adding an object entry inside the `packages` array. 

```
{
  "packages":[
    {
      "operator_name" : "Cassandra Sink", # Name presented in runway CLI
      "executable_name" : "cassandra_pumper", # Binary name runner script will exec
      "docker_repository" : "concord", # Repo name where package exists
      "docker_container" : "runway_cassandra", # Container name
      "supported_versions" : ["0.4.3.2", "0.4.3.1", "0.3.16.4"], # Only show these tagged containers
      "documentation_link" : "https://raw.githubusercontent.com/concord/runway/master/connectors/cassandra/README.md", # Optional
      "default_cpus" : 1.0, # Optional
      "default_mem" : 1024 # Optional
    },
	...
```

The runway command will allow you to test against your changes by overriding the
default repository URL that it uses to fetch this metadata. For example:

```
$ git push # Push your metadata changes
$ sudo docker push ... # Ensure your package is on dockerhub too
$ concord runway -r https://github.com/RBlafford/runway/tree/feature/my_package/
...(your operator should appear here)
```

You can also store this url in your concord config file so you won't have to 
type this in each time you invoke the runway command.

```
$ concord config show
Concord configuration data:  /home/rob/.concord.cfg
zookeeper_hosts=localhost:2181
zookeeper_path=/concord
runway_repository=https://github.com/concord/runway/master/

$ concord config set runway_repository=https://github.com/RBlafford/runway/tree/feature/my_package/
$ concord runway # All set!
...
```

Finally, submit a PR and you're finished!
