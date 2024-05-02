# Kelpa
A Miniature Server Framework

[![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

Brief Introduction

    This is a leaning project. It is developed using the C++ 20 standard and running on the windows system platform.

## Content List

- [Futures](#Futures)
- [Install](#Install)
- [Usage](#Usage)
- [Example](#Example)
- [Maintainer](#Maintainer)

## Futures

`Kelpa` 

1. **Configurable**:
    Users can have the freedom to flesh out configurable params outside of the main code branch without impacting stable configurations, by editing a json format config file, and the program dynamically loads and parses this configuration file at run time.

2. **Concurrent log library**:
    The project contains an efficient log library that supports concurrency and formatting and can support multi-channel log output.
3. **Json interpreter**:
    The project contains an efficient and easy-to-use json format file interpreter module

4. **IO Scheduler**:
    The project contains a high concurrency IO scheduling module based on reactor model and IO multiplexing technology.

5. **Thread facilities**:
    The project encapsulates a number of user-friendly threading concurrency support facilities
6. **Serialize**:
    The project supports serialization and deserialization of data in stream mode
7. **Compress**:
    The project contains a series of compression algorithm modules to compress packets during data transmission



## Install

```sh
cd ./Build
cmake ..
make 
#make file
```

## Usage

In the `Tests` directory is `simple Http server` built with the framework, and in the `Examples` directory are some `sample usages` of each module in the project


```sh
./../Bin/x64/Debug/test
# Runs the simple http server
```
   ### [dependencies]
	Windows Socket libraries, Windows DbgHelp libraries, POSIX pthread
	
   ### [build arguments]
	Compile:	-ftemplate-backtrace-limit=0 
				-fconcepts-diagnostics-depth=2 
				-Wno-unused-parameter 
				-debug
	Link:		-lDbgHelp  
				-lWs2_32
				-lpthread
   ### [environment]
	Platform: Windows 
	Complie: MinGW GCC 13.2.0 64bit
	Language Standard: GNU Cpp 20
   ### [directories]    
```
├─Bin       
│  └─x64
│      └─Debug -- test  #Simple http server demo 
 |          
├─Build                     #Intermediate files produced by cmake
├─Config                    #Contains a json file for config
├─Examples              #Example programmes
├─Include               #head files
├─Lib                       #libraries(static dynamic)
├─Src                       #source code
│  ├─Compress           #Compressing module
│  ├─Configure          #Configure loading and parsing module
│  ├─Coroutine          #Async coroutine module
│  ├─CppJson            #Json file parsing and dumping module
│  ├─IOSchedule         #IO sheduling module
│  ├─Journal            #Logging module
│  ├─Serde              #Serializing module
│  ├─Thread             #Concurent module
│  │  └─Sync            #Concurent Containers
│  └─Utility            #Util facilities
└─Tests                 #tests programmes
```
## Example

```
See here: ./Examples/*.cc

Each program is an example of the use of a module of the project

```

## Maintainer

[@RFoe](https://github.com/RFoe).

