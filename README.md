# SeqOmb
A DNA sequence comparator with a focus on speed. It computes the  location of matches of K-length of 2 DNA (ACTG..) sequences, and plotts them in an image. It uses multi-threading, and can be used to automatically run on multiple comparisons after another.

## Dependencies
gnuplot: Installation with apt: `sudo apt-get install gnuplot`
g++, or any c++ compiler. This package includes g++: `sudo apt-get install build-essential`

## Usage
As a reference, two sequences and a resulting plot are included. Delete them to avoid confusion with your data!

1. Put the sequences that shall be compared to another in the folders `seq1` and `seq2`. Every Sequence will be compared to a sequence in the other folder, the Sequence in `seq1` will be called "Reference", the other "Read" in the final output. Letters other than ACTG, or whitespace, will be ignored.

2.  Edit the programs parameters. For speed, this is done as definitions. Open `main.cpp` and adjust the values of these parameters to your needs:
```cpp
#define num_threads 4   //set your number of threads the program should use (idealy the number of hardware threads your machine has)
#define K 10            //set the length matches should be
#define rev_comp        //comment this line (by adding // at the beginning) to not compute reverse complement matches
```

3. Compile the program by opening a terminal in the folder of `main.cpp` and calling the command:
```
g++ main.cpp -Ofast --std=c++20
```

4. Run the resulting executable (usually called `a.out`) in your terminal:
```
./a.out
```

5. In the outputs folder, plots will appear. Green dots represent matches in the forward direction, red ones are reverse-complement matches. Tadaa!
During computaion, a file called xyc.txt is created. Once the program has run, it is safe to delete.
