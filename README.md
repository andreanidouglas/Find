# Find

Find (and act on) all the files in a given path

## Usage

```bash
Find.exe [-n | --name filename] [--delete] [-e | --exact] [-h | --help]

options:
    -n, --name <filename>      filter list for files that contains filename
    -e, --exact                --name will match filename exactly. if --name is not specified then becames NOP
    --delete                   delete found files. --name parameter is mandatory
    -h, --help                 show this help message
```
