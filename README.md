# jzip
Some mid compression software

Currently, the file is stored as bytes, so the compressed file is 8 times larger than should be.

Once the files is stored as bits, the file should be relatively close to entropy, 
the theoretical minimum size of the file while being lossless.

## Usage
```
jzip compress <file>
jzip inflate <file>
```

## TODO
- [ ] Move from bytes to bits
- [ ] Add support for directories (archiving like tar)
- [ ] Multithreading
- [X] Commands and flags to make the program actually usable

## Directory Structure
```
|           - The root directory holding the README, cmakelist, gitignore as well as the other directories
├── scripts - The directory holding maintenance scripts, such as the working directory cleaner
├── src     - The directory holding the source files
├── headers - The directory holding the header files
└── working - The directory where the binary should be executed from
```

Feel free to fork and whatnot :)

Made by me, Jacob Busby, will probably worry about licensing later
