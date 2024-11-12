# Pi BBP
Implementsπ BBP-Type Formulas For Calculating the nth-digit of Pi Concurrently. Still Working on the full relatory, soon will be added to this repo.

Code Only Tested in Linux!

## 📘 Usage
Can be called With using only the program name `./pi-bbp`. Or can be called directly using `./pi-bbp [algorithm] [offset] [threads]`, with algorithm beign `bellard` or `original`.

## 🧮 Formulas
* [Original BBP-Formula (4-Term)](https://en.wikipedia.org/wiki/Bailey%E2%80%93Borwein%E2%80%93Plouffe_formula)

$$\pi \ = \ \sum_{k=0}^\infty \dfrac{1}{16^k} \left(\dfrac{4}{8k + 1} - \dfrac{2}{8k + 4} - \dfrac{1}{8k+5} - \dfrac{1}{8k+6}\right)$$

* [Bellard's Formula (7-Term)](https://bellard.org/pi/)

$$\pi \ = \ \dfrac{1}{2^6} \sum_{k=0}^\infty \dfrac{(-1)^k}{2^{10k}} \left(-\dfrac{2^5}{4k + 1} - \dfrac{1}{4k + 3} + \dfrac{2^8}{10k + 1} - \dfrac{2^6}{10k + 3} - \dfrac{2^2}{10k + 5} - \dfrac{2^2}{10k + 7} + \dfrac{1}{10k + 9}\right)$$

## 🧰 Build
Just use `make` and the project will build.

## ⚡ Performance
One of the Fastest Full Open Source Implementation. Notice that y-cruncher is 1000x faster but the majority of it's code is **not** open-source.
AlL tests where made in my personal computer which have the following specs:

* **Machine**: Lenovo Ideapad 3
* **OS**: Linux Mint Victoria 21.2
* **CPU**: AMD Ryzen 5 5500U
* **Ram**: 12Gb

### ✅ Results
The results are a median of 3 executions, we using the worst and best results for each algorithm and calculating it's improvement based on number of threads and in the batch size.

| Algorithm | Offset | Best Threads | Best Batch Size | Best Median Time (s) | Worst to Best Improvement (%) |
|-----------|--------|--------------|-----------------|-----------------------|-------------------------------|
| BBP       | 1      | 1            | 1               | 0.000227              | 67.24                         |
| Bellard   | 1      | 2            | 1               | 0.000227              | 72.42                         |
| BBP       | 10     | 2            | 10              | 0.000253              | 66.27                         |
| Bellard   | 10     | 1            | 10              | 0.000290              | 63.61                         |
| BBP       | 10^4   | 12           | 100             | 0.000723              | 92.53                         |
| Bellard   | 10^4   | 12           | 100             | 0.000667              | 88.98                         |
| BBP       | 10^6   | 12           | 100             | 0.048973              | 91.24                         |
| Bellard   | 10^6   | 12           | 1000            | 0.036590              | 87.49                         |
| BBP       | 10^7   | 12           | 1000            | 0.542547              | 90.59                         |
| Bellard   | 10^8   | 12           | 1000            | 0.406323              | 87.16                         |



### 📊 Offset, Threads and Batches
In the image below, we can see a graph between the two algorithms and it's permofance as more threads are added, and when more work is given to then.

![offset_10000000](https://github.com/user-attachments/assets/49e56f2d-d6be-471a-afd8-0047f868b695)

# 💡 Inspirations
* [y-cruncher](http://www.numberworld.org/y-cruncher/)
* [Bellard's Formula Implementation](https://link.springer.com/article/10.1007/s11139-021-00475-y)

# 📝 To-do
* [x] Interative Menu
* [ ] Digits Verification
* [ ] Checkpoints
* [ ] Output To File
* [ ] Arbritary Precision
* [ ] SIMD Instructions
* [ ] Montgomery Multiplication


