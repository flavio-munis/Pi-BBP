# Pi BBP
Implements BBP-Type Formulas For Calculating the nth-digit of Pi Concurrently. Still Working on the full relatory, soon will be added to this repo.

Code Only Tested in Linux!

## Formulas
* [Original BBP-Formula (4-Term)](https://en.wikipedia.org/wiki/Bailey%E2%80%93Borwein%E2%80%93Plouffe_formula)

$$\pi \ = \ \sum_{k=0}^\infty \dfrac{1}{16^k} \left(\dfrac{4}{8k + 1} - \dfrac{2}{8k + 4} - \dfrac{1}{8k+5} - \dfrac{1}{8k+6}\right)$$

* [Bellard's Formula (7-Term)](https://bellard.org/pi/)

$$\pi \ = \ \dfrac{1}{2^6} \sum_{k=0}^\infty \dfrac{(-1)^k}{2^{10k}} \left(-\dfrac{2^5}{4k + 1} - \dfrac{1}{4k + 3} + \dfrac{2^8}{10k + 1} - \dfrac{2^6}{10k + 3} - \dfrac{2^2}{10k + 5} - \dfrac{2^2}{10k + 7} + \dfrac{1}{10k + 9}\right)$$

# Build
Just use `make` and the project will build.

# Performance
Fastest Full Open Source Implementation. Notice that y-cruncher is 1000x faster but the majority of it's code is **not** open-source.
AlL tests where made in my personal computer which have the following specs:

* **Machine**: Lenovo Ideapad 3
* **OS**: Linux Mint Victoria 21.2
* **CPU**: AMD Ryzen 5 5500U
* **Ram**: 12Gb

## Results
The results are a median of 5 executions, each which the optimal number of threads available.

| Algorithm | Offset | Threads |   Time   |
|  -------  | ------ | ------- |  ------- |
|  Bellard  |   1    |    2    | 0.00016s |
| Original  |   1    |    2    | 0.00028s |
|  Bellard  |  10^4  |   12    | 0.00132s |
| Original  |  10^4  |   12    | 0.00090s |
|  Bellard  |  10^6  |   12    | 0.05631s |
| Original  |  10^6  |   12    | 0.00090s |


## Offset, Threads and Batches
In the image below, we can see a graph between the two algorithms and it's permofance as more threads are added, and when more work is given to then.

![offset5](https://github.com/user-attachments/assets/dce6e498-4d9a-490a-b880-cd0b0dad9295)

# Inspirations
* [y-cruncher](http://www.numberworld.org/y-cruncher/)
* [Bellard's Formula Implementation](https://link.springer.com/article/10.1007/s11139-021-00475-y)

# To-do
* [x] Interative Menu
* [ ] Verification of Digits
* [ ] Checkpoints
* [ ] Output To File
* [ ] Arbritary Precision
* [ ] SIMD Instructions
* [ ] Montgomery Multiplication


