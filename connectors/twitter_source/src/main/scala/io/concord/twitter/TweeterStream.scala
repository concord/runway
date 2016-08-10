package io.concord.twitter
import java.util.{ HashSet => MutableHashSet}
import io.concord._
import io.concord.swift._

class SentenceGenerator extends Computation {
  override def init(ctx: ComputationContext): Unit = {
    println(s"${this.getClass.getSimpleName} initialized")
    ctx.setTimer("loop", System.currentTimeMillis())
  }

  override def destroy(): Unit = {
    println(s"${this.getClass.getSimpleName} destructing")
  }

  override def processRecord(ctx: ComputationContext, record: Record): Unit = ???

  override def processTimer(ctx: ComputationContext, key: String, time: Long): Unit = {
    // Stream, key, value. Empty value, no need for val
    Range(0, 10000).foreach {
      i => ctx.produceRecord("sentences".getBytes, sample().getBytes, "-".getBytes)
    }

    ctx.setTimer(key, System.currentTimeMillis())
  }

  override def metadata(): Metadata = {
    val ostreams = new MutableHashSet[String](java.util.Arrays.asList("sentences"))
    new Metadata("sentence-generator", new MutableHashSet[StreamTuple](), ostreams)
  }

}


object SentenceGenerator {
  def main(args: Array[String]): Unit = {
    ServeComputation.serve(new SentenceGenerator())
  }
}
