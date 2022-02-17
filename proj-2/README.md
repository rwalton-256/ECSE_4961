# Project

The aim of the project is to create a multithreaded application to compress
files using zstd algorithm.

## Building and Running

```
make
./test.sh
```

## Performance Results

Intel® Core™ i7-9750H CPU @ 2.60GHz × 12 

10.2GB text file, Charles Dickens books (10MB file) copied 1000 times.

### Custom zstd

Using 6 threads

#### Time

51.447s

#### Compression ratio

43.13%

### Provided zstd command

#### Time

1m13.556s

#### Compression ratio

36.27%

## Code structure

The compressing function uses a very simple and portable API, using C++
iostreams to read in and write out data. main.cpp is where the function
is actually called from.

The function uses a thread safe queue data type that allows communication
between a single master, task distributing and stream io thread, and an
arbitrary number of worker threads. The distrubution of work comes is
done by handing around pointers to "Compression_Blocks", a data type that
stores pointers to buffers for uncompressed and compressed data, as well
as associated metatdata. The process allocates a block of memory necessary
for num_threads * 2 pairs of uncompressed and compressed buffers, as well
as memory for num_threads * 2 "Compression_Blocks". This is enough to store
compression tasks for 2 times the number of threads (to serve as buffering
in case the main thread isn't able to provide tasks fast enough).
