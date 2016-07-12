package com.concord.kinesis;

import com.amazonaws.services.kinesis.clientlibrary.interfaces.IRecordProcessorFactory;
import java.util.concurrent.ArrayBlockingQueue;
import com.amazonaws.services.kinesis.model.Record;

public class RecordProcessorFactory implements IRecordProcessorFactory {
  private ArrayBlockingQueue<Record> queue;

  public RecordProcessorFactory(ArrayBlockingQueue<Record> q) { queue = q; }

  @Override
  public RecordProcessor createProcessor() {
    return new RecordProcessor(queue);
  }
}
