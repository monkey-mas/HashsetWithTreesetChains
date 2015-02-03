HashsetWithTreesetChains
====
#### Version
0.1.0 

## Description
HashsetWithTreesetChains is a Multi-threaded Set(of type int) library in C

- treeset.c is implemented based on AVL tree.
- hashset_chain.c is implemented based on chain methodology with tree set.

## Background & hypothesis
There are two popular set data structures, Tree set and Hash set. Self-balanced binary search trees, such as Red-Black tree or AVL tree, are well used to represent set data structure. Hash set is quite well used as well.
Each of them has their own Pros and Cons, but… what I'd like to point out here is the **difficulty to achieve parallel or distributed computation for set**. Tree sets are not thread-safe and Hash sets with open addressing methdology is not eighther, thus it is hard to use them for concurrent tasks. In fact, **standard set libraries in C++ and Java are not thread-safe**.

I believe one of the simplest solutions for this issue is to implement Hash set with chain methdology. Each chain or slot of hash table points to **another data structure**, such as linked-list or tree, so that concurent computation is available iff multiple set operations are done for different chains.
As can be seen from the name of the library, Tree set, implemented based on AVL tree, is used for chains. The benchmark results will be given below.

## Installation
Please copy hashset_chain.h & treeset.h into your header directory and put hashset_chain.c & treeset.c into your source code directory.
Then compile your code adding option \-pthread.
For example, you might do as follows with proper setting of $(YOUR_LIBRARIES) and $(OPTIONS) to make an executable file *main*
```sh
$ gcc -o main $(YOUR_LIBRARIES) hashset_chain.c treeset.c $(OPTIONS) -pthread
```
Also, examples are found in [example/](./example). After typing *make* in the top directory, you can execute *example* to try some set operations.
```sh
$ make
$ ./example
```
## Features
- Performance good enough with multi-threaded execution.
 -  As you can see from the graphs below, HashsetWithTreesetChains does good job compared to C++ std library, set and unordered_set, especially in the multithreaded environment. This indicates the scalability of the library with regard to thread-safety, which is not guaranteed for set and unsorted_set in C++.
 - The result shows add_array by HashsetWithTreesetChains with 8 threads is about ***15%*** and ***45%*** **faster** than  C++ std::set and std::unordered_set respectively. Also, HashsetWithTreesetChains boosts its performance (less time consumption to finish a task) as the number of threads increases.
 - You can find the actual benchmark code and detialed explanation in [benchmark/](./benchmark).
![Operation time with 1 million elements](https://docs.google.com/spreadsheets/d/1U6Lx-AYqO6-mzLi-DypT9RslBxID7PzMBp1YCSc6Nm4/pubchart?oid=1132328749&format=image)

![Multithreaded operation with 1 million elements](https://docs.google.com/spreadsheets/d/1U6Lx-AYqO6-mzLi-DypT9RslBxID7PzMBp1YCSc6Nm4/pubchart?oid=2089236691&format=image)

- Supports for a lot of set operations, such as operations with array elements and set, which are not provided by standard libraries set and unordered_set in C++.
 - You can do immutable set operations, such as union, as well by passing a new set object to store data of the operation result.

- Multithreaded operation is not supported for operation with one element.


## Future work
Improvements may be made for algorithm stuff.

## Contribution
Any contribution is much appreciated! Please give your PR or feedback :)

## Licence
Released under the [MIT license](http://opensource.org/licenses/mit-license.php)

## Author
Masaru Nomura ([monkey-mas](https://github.com/monkey-mas))

