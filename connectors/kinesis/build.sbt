lazy val root = (project in file(".")).
settings(
  name := "concord_kinesis_consumer",
  version := "0.1.0",
  scalaVersion := "2.11.6",
  autoScalaLibrary := false,
  crossPaths := false,
  scalacOptions ++= Seq("-feature", "-language:higherKinds"),
  libraryDependencies ++= Seq(
    "io.concord" % "concord" % "0.1.0",
    "io.concord" % "rawapi" % "0.1.1",
    "com.amazonaws" % "amazon-kinesis-client" % "1.6.1",
    "com.beust" % "jcommander" % "1.48",
    "com.google.guava" % "guava" % "19.0-rc2",
    "ch.qos.logback" % "logback-classic" % "1.1.3",
    "ch.qos.logback" % "logback-core" % "1.1.3",
    "ch.qos.logback" % "logback-access" % "1.1.3"
    ),
  assemblyMergeStrategy in assembly := {
    case x if x.endsWith("project.clj") => MergeStrategy.discard // Leiningen build files
    case x if x.toLowerCase.startsWith("meta-inf") => MergeStrategy.discard // More bumf
    case _ => MergeStrategy.first
  },
  mainClass in Compile := Some("com.concord.kinesis.Consumer"),
  resolvers ++= Seq(
    Resolver.sonatypeRepo("public"),
    "clojars" at "https://clojars.org/repo",
    "conjars" at "http://conjars.org/repo"
  )
)
