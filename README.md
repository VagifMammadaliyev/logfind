### Logfind project
This tool must get pathname patterns mentioned in `.logfind` file, search paths to logfiles using these patterns. Then search each logfile for other patterns provided as arguments to executable. Outputs paths to logfiles where patterns are found.

# Usage
Search for files containing both pattern1 and pattern2
```bash
./logfind pattern1 pattern2
```

Search for files containing pattern1 or pattern2
```bash
./logfind -or pattern1 pattern2
``` 
