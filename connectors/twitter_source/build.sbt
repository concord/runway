name := "twitter_source"

version := "0.0.1"

scalaVersion := "2.11.0"

libraryDependencies ++= Seq(
  "io.concord" % "concord" % "0.1.2",
  "io.concord" % "rawapi" % "0.2.5",
  "org.twitter4j" % "twitter4j-core" % "3.0.3",
  "org.twitter4j" % "twitter4j-stream" % "3.0.3",
  "ch.qos.logback" %  "logback-classic" % "1.1.7",
  "com.typesafe.scala-logging" %% "scala-logging" % "3.4.0"
)



assemblyMergeStrategy in assembly := {
  case x if x.endsWith("project.clj") => MergeStrategy.discard
  case x if x.toLowerCase.startsWith("meta-inf") => MergeStrategy.discard
  case _ => MergeStrategy.first
}


resolvers += Resolver.sonatypeRepo("public")

resolvers += "clojars" at "https://clojars.org/repo"

resolvers += "conjars" at "http://conjars.org/repo"
