# runway
Open source operators and templates for the Concord Stream Processor.


## Creating / Publishing a Package

0. Fork the runway repository
1. Create your custom connector, place the code in the `connectors/` folder
2. Dockerize the connector
3. Edit the `meta/repo_metadata.json` JSON structure to include your package

To test to see if runway can properly deploy your operator you can pass a repository URL to the
runway command using the `-r` or `--repository` flag like so:

```
$ concord runway -r https://raw.githubusercontent.com/concord/runway/<github_branch>
```

4. Submit a PR for submission!







