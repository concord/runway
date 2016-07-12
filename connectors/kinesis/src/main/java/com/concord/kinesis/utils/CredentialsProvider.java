package com.concord.kinesis.utils;

import com.amazonaws.auth.AWSCredentials;
import com.amazonaws.auth.AWSCredentialsProvider;
import com.google.common.base.Preconditions;

public class CredentialsProvider implements AWSCredentialsProvider {
  private final Credentials credentials;

  public CredentialsProvider(String key, String secret) {
    Preconditions.checkNotNull(key);
    Preconditions.checkNotNull(secret);
    credentials = new Credentials(key, secret);
  }

  @Override
  public AWSCredentials getCredentials() {
    return credentials;
  }

  @Override
  public void refresh() {}
}
