package com.concord.kinesis;

import com.amazonaws.services.kinesis.clientlibrary.interfaces.IRecordProcessor;
import com.amazonaws.services.kinesis.clientlibrary.interfaces.IRecordProcessorFactory;
import com.amazonaws.services.kinesis.clientlibrary.interfaces.IRecordProcessorCheckpointer;
import com.amazonaws.services.kinesis.clientlibrary.types.ShutdownReason;
import com.amazonaws.services.kinesis.model.Record;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import com.amazonaws.services.kinesis.clientlibrary.exceptions.*;
import com.google.common.base.Preconditions;
import com.google.common.base.Throwables;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class RecordProcessor implements IRecordProcessor {
  private final BlockingQueue<Record> recordQueue;
  private String shardId;
  private final Logger logger = LoggerFactory.getLogger(RecordProcessor.class);

  public RecordProcessor(BlockingQueue<Record> rq) { recordQueue = rq; }

  @Override
  public void initialize(String shardId) {
    Preconditions.checkNotNull(shardId);
    logger.info("Initialized processor on shard id: {}", shardId);
    this.shardId = shardId;
  }

  @Override
  public void processRecords(List<Record> records,
                             IRecordProcessorCheckpointer checkpointer) {
    for(Record record : records) {
      try {
        recordQueue.put(record);
        checkpointer.checkpoint(record);
      } catch(InterruptedException | InvalidStateException
              | ShutdownException e) {
        Throwables.propagate(e);
      }
    }
  }

  @Override
  public void shutdown(IRecordProcessorCheckpointer checkpointer,
                       ShutdownReason reason) {
    logger.error("Shutting down Kinesis consumer for shard: {}", shardId);
    System.exit(1);
  }
}
