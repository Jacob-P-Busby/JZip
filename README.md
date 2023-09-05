# jzip
Some mid compression software

Currently, the file is stored as bytes, so the compressed file is 8 times larger than should be.

## Usage
```jzip <file>```

## TODO
- [ ] Move from bytes to bits
- [ ] Add support for directories (archiving like tar)
- [ ] Multithreading
- [ ] Commands and flags to make the program actually usable

## Directory Structure
```
|           - The root directory holding the README, cmakelist, gitignore as well as the other directories
├── src     - The directory holding the source files
├── headers - The directory holding the header files
└── working - The directory where the binary should be executed from
```

Feel free to fork and whatnot :happy: