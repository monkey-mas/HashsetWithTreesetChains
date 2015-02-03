## Benchmark
Benchmarks were given as follows:

- Comparisons with stdard libraries of C++(set and unordered_set)
- Scalability of the library with different number of threads

## Environment

|                                   |               |
|-----------------------------------|---------------|
| CPU								|				|
| # Model name						| Intel(R) Core(TM) i7-2600 | 
| # CPU clock                       | 3.40GHz       |
| # Cache size                      | 8192 KB       |
|                                   |               |
| Memory size                       | 12 GB         |

## The benchmark methodology
- Elements used were exactly from 0 to 1 million.
- Number of trials to measure time was 100 for each operation
- Time measured is the execution time to finish task using clock_gettime() func.

As you can find from benchmark source code, time consumption by add/remove/find operations were measured using for-loop. In each loop, one element was dealt with by an operation starting from 0 to 1 million. Timer starts when for loop begins and stops when for loop ends.

To measure operation time for array, we first create an array with 1 million elements then execute corresponding set operation, such as hashset_add_set(...). Timer starts from the execution of set operation and stops when it finishes. As there is no set operations of remove/find provided by std::set and std::unordered_set, we can't compare these operations with our library. This is the reason why there is no performance comparison with them.

## Results

The tables below show results (in *ms*) of set operations.
Each row indicates:

- Average : the average time of 100 trials
- Median  : the meadian time of 100 trials
- Best	  : the shortest time to finish a task
- Worst	  : the longest time to finish a task

### add
Comaprison of execution time of HashsetWTC/add.c, std::set/add.cpp, and std::unordered_set/add.cpp

| 			 |   std::set  | std::unordered_set | HashsetWTC(p=1) |
|------------|:-----------:|:------------------:|:---------------:|
| Average    |   659.78003 |    227.60439     	|   441.68686 	  |
| Median     |   657.4305  |    221.38  		|   440.3315 	  |
| Best       |   648.281   |    216.894 		|	425.217 	  |
| Worst      |   693.903   |    324.515			|   501.205       |

### add_array
Comaprison of execution time of HashsetWTC/add_array.c, std::set/add_array.cpp, and std::unordered_set/add_array.cpp

| 			 |   std::set  | std::unordered_set | HashsetWTC(p=1) | HashsetWTC(p=8) |
|------------|:-----------:|:------------------:|:---------------:|:---------------:|
| Average    |   130.04844 |    189.86745     	|   361.69872 	  |	  109.07113 	|
| Median     |   130.1185  |    188.9875  		|   358.976 	  |   107.7745 		|
| Best       |   126.855   |    187.914 		|	349.309 	  |   104.866 		|
| Worst      |   132.744   |    198.949			|   474.439       |   128.627 		|

### remove
Comaprison of execution time of HashsetWTC/remove.c, std::set/remove.cpp, and std::unordered_set/remove.cpp

| 			 |   std::set  | std::unordered_set | HashsetWTC(p=1) |
|------------|:-----------:|:------------------:|:---------------:|
| Average    |   522.32246 |    69.979963     	|   162.11074 	  |
| Median     |   517.539   |    69.876  		|   160.221 	  |
| Best       |   511.752   |    69.5292 		|	154.554 	  |
| Worst      |   559.948   |    71.1786			|   220.401       |

### find
Comaprison of execution time of HashsetWTC/find.c, std::set/find.cpp, and std::unordered_set/find.cpp

| 			 |   std::set  | std::unordered_set | HashsetWTC(p=1) |
|------------|:-----------:|:------------------:|:---------------:|
| Average    |   339.04008 |    43.81494     	|   206.66656 	  |
| Median     |   337.5855  |    43.76315  		|   205.11 		  |
| Best       |   324.073   |    43.4627 		|	199.706 	  |
| Worst      |   379.543   |    45.1154			|   257.452       |
