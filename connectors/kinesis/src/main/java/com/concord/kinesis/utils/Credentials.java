package com.concord.kinesis.utils;

import com.amazonaws.auth.AWSCredentials;
import com.google.common.base.Preconditions;

public final class Credentials implements AWSCredentials {
  private final String key;
  private final String secret;

  public Credentials(String key, String secret) {
    Preconditions.checkNotNull(key);
    Preconditions.checkNotNull(secret);
    this.key = key;
    this.secret = secret;
  }

  @Override
  public String getAWSAccessKeyId() {
    return key;
  }

  @Override
  public String getAWSSecretKey() {
    return secret;
  }
}
