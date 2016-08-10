package io.concord.twitter
import java.util.{ HashSet => MutableHashSet}
import io.concord._
import io.concord.swift._

// todo(agallego) - fix and scope this a little bit more
import twitter4j._

class ConcordTwitterStream extends Computation with LazyLogging {
  override def init(ctx: ComputationContext): Unit = {
    logger.info(s"${this.getClass.getSimpleName} initialized")
    ctx.setTimer("loop", System.currentTimeMillis())
  }
  override def destroy(): Unit = {
    logger.info(s"${this.getClass.getSimpleName} destructing")
  }
  override def processRecord(ctx: ComputationContext, record: Record) = ???

  override def processTimer(ctx: ComputationContext,
                            key: String, time: Long): Unit = {


    ctx.setTimer(key, System.currentTimeMillis())
  }

  override def metadata(): Metadata = {
    val ostreams = new MutableHashSet[String](java.util.Arrays.asList("tweets"))
    new Metadata("concord-twitter-stream",
        new MutableHashSet[StreamTuple](),
        ostreams)
  }

}

object ConcordTwitterStream {
  def main(args: Array[String]): Unit = {
    ServeComputation.serve(new ConcordTwitterStream())
  }
}
