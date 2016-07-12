package com.concord.kinesis.utils;

import java.util.List;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.UUID;
import com.beust.jcommander.Parameter;
import com.beust.jcommander.JCommander;
import com.amazonaws.services.kinesis.clientlibrary.lib.worker.KinesisClientLibConfiguration;

public class Options {
  @Parameter private List<String> parameters;

  @Parameter(names = "-topic",
             description = "Name of the topic to consume from Kinesis")
  private String topic;
  public String getTopic() { return topic; }

  @Parameter(names = "-ostreams",
             description = "Comma separated list of streams to emit records on")
  private String ostreams;
  public ArrayList<String> getOstreams() {
    return new ArrayList<String>(Arrays.asList(ostreams.split(",")));
  }

  @Parameter(names = "-awskey", description = "AWS Access Key ID")
  private String awsAccessKeyId;
  public String getAwsAccessKeyId() { return awsAccessKeyId; }

  @Parameter(names = "-awssecret", description = "AWS Secret Key")
  private String awsSecretKey;
  public String getAwsSecretKey() { return awsSecretKey; }

  @Parameter(names = "-name", description = "The name of the computation")
  private String name;
  public String getName() { return name; }

  @Parameter(names = "-queuesize",
             description = "Maximum number of messages to buffer in memory")
  private int queueSize = 16384;
  public int getQueueSize() { return queueSize; }

  public CredentialsProvider getCredentialsProvider() {
    return new CredentialsProvider(awsAccessKeyId, awsSecretKey);
  }

  public KinesisClientLibConfiguration getKinesisConfiguration() {
    return new KinesisClientLibConfiguration(
      name, topic, getCredentialsProvider(),
      name + "-" + UUID.randomUUID().toString());
  }

  public static Options parse(String[] argv) {
    Options opts = new Options();
    new JCommander(opts, argv);
    return opts;
  }
}
