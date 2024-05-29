/*
    This file produces an executable that reads each file in the folders seq1/ seq2/ adjacent to the executable,
    and maps each file (containing the Letters ACTG) in seq1/ to each file in seq2/ and creates a dotplot for that.
    A match is an identical repeat of K letters, where K can be defined below.
    Addionally, reverse complement matches are noted. This can be turned off by removing a define below.
    The Position and kind of matches are saved in "xyc.txt", which gets passed to gnuplot.
    Gnuplot is then instructed via the "gplot.txt" script to create a dotplot from the data,
    notating the identical matches in red, and reverse complement ones in green. The output
    will be saved as a combination of the names of the two compared files.

    gnuplot has to be found by the terminal:
        on Windows it needs to be manually downloaded and the path to its binaries put into the PATH System variable
        on Linux it should be possible to install it via the Distributions package manager

    To compile: Any modern C++ compiler will work. For example:
        g++ main.cpp -Ofast --std=c++20
    
    This programm uses Multithreading with n Threads. You can set the number of threads to be used below.

    Created by Simon Klüpfel, 2023, Mozilla Public License 2.0
*/

//When changeing any of these values, the programm must be recompiled!
#define num_threads 4 //set your number of threads
#define K 10 //set the length matches should be
#define rev_comp //comment this line (by adding // at the beginning) to not compute reverse complement matches

#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <thread>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <bitset>
#include <chrono>

using namespace std::chrono;
enum Letter : uint8_t
{
    A = 0b00,
    C = 0b01,
    T = 0b10,
    G = 0b11
};

std::vector<Letter> data1L;
std::vector<Letter> data1C;
std::vector<Letter> data2L;

void comp()
{
    auto s = data1L.size();
    data1C.resize(s);
    for (int i = s; i >= 0; i--)
    {
        Letter c = data1L[s-i];
        switch (c)
        {
        case Letter::A:
            data1C[i] = Letter::T;
            break;
        case Letter::T:
            data1C[i] = Letter::A;
            break;
        case Letter::G:
            data1C[i] = Letter::C;
            break;
        case Letter::C:
            data1C[i] = Letter::G;
            break;
        }
    }
}

std::vector<int> x[num_threads];
std::vector<int> y[num_threads];
std::vector<char> c[num_threads];

std::string data1;
std::string data2;

void work(long start, long end, int index)
{
    std::bitset<K * 2> substring;

    int ll = 2 * K - 1;
    for (long l = 0; l < K; l++)
    {
        substring[ll] = data1L[l + start] & 0b00000010;
        ll--;
        substring[ll] = data1L[l + start] & 0b00000001;
        ll--;
    }
    
    #ifdef rev_comp
    std::bitset<K * 2> substringC;

    int llc = 2 * K - 1;
    for (long l = 0; l < K; l++)
    {
        substringC[llc] = data1C[l + start] & 0b00000010;
        llc--;
        substringC[llc] = data1C[l + start] & 0b00000001;
        llc--;
    }
    #endif
    for (long i = start; i < end; i++)
    {

        std::bitset<K * 2> toCompare;

        int ll = 2 * K - 1;
        for (long l = 0; l < K; l++)
        {
            toCompare[ll] = data2L[l] & 0b00000010;
            ll--;
            toCompare[ll] = data2L[l] & 0b00000001;
            ll--;
        }

        for (int j = 0; j < data2.length(); j++)
        {

            if (toCompare == substring)
            {
                x[index].push_back(i);
                y[index].push_back(j);
                c[index].push_back('r');
            }
            #ifdef rev_comp           
            else if(toCompare == substringC){
                x[index].push_back(i);
                y[index].push_back(j);
                c[index].push_back('g');
            }
            #endif
            toCompare = toCompare << 2;
            toCompare[0] = data2L[j + K] & 0b00000001;
            toCompare[1] = data2L[j + K] & 0b00000010;
        }

        substring = substring << 2;     
        substring[0] = data1L[i + K] & 0b00000001;
        substring[1] = data1L[i + K] & 0b00000010;
        
        #ifdef rev_comp
        substringC = substringC << 2;     
        substringC[0] = data1C[i + K] & 0b00000001;
        substringC[1] = data1C[i + K] & 0b00000010;
        #endif
        
    }
}

std::vector<Letter> string_to_vec(std::string s)
{
    std::vector<Letter> tmp;
    tmp.resize(s.length());

    for (int i = 0; i < s.length(); i++)
    {
        char c = s[i];
        switch (c)
        {
        case 'A':
            tmp[i] = Letter::A;
            break;
        case 'T':
            tmp[i] = Letter::T;
            break;
        case 'G':
            tmp[i] = Letter::G;
            break;
        case 'C':
            tmp[i] = Letter::C;
            break;
        }
    }
    return tmp;
}
int main()
{
    namespace fs = std::filesystem;
    std::string path1 = "seq1/";
    std::string path2 = "seq2/";

    //Gather and parse all seqence files
    for (const auto &entry1 : fs::directory_iterator(path1))
    {

        std::ifstream inFile;
        inFile.open(entry1.path()); // open the input file

        std::stringstream strStream;
        strStream << inFile.rdbuf(); // read the file
        data1 = strStream.str();     // data1 holds the content of the file
        std::erase_if(data1, [](char const &c)
                      { return !std::isalnum(c); });

        for (const auto &entry2 : fs::directory_iterator(path2))
        {

            std::ifstream inFile;
            inFile.open(entry2.path()); // open the input file

            std::stringstream strStream;
            strStream << inFile.rdbuf(); // read the file
            data2 = strStream.str();     // data2 holds the content of the file
            std::erase_if(data2, [](char const &c)
                          { return !std::isalnum(c); });
                          
            std::cout << "starting" << std::endl;
            auto startT = steady_clock::now();
            
            //Convert strings to bin Enums
            data1L = string_to_vec(data1);
            #ifdef rev_comp
            comp();
            #endif
            data2L = string_to_vec(data2);
            
	    //Multithread the actual work
            std::vector<std::thread> trs;
            double step = data1L.size() / num_threads;
            for(int i = 0; i < num_threads; i++){
                int lower = i * step;
                int higher = (i+1)*step;
                std::thread t(work, lower, higher, i);
                trs.push_back(std::move(t));
            }
            
            std::vector<int> xg;
            std::vector<int> yg;
            std::vector<char> cg;
            for (int i = 0; i < num_threads; i++)
            {
                trs[i].join(); //Wait for threads to finish
            }
            
            std::cout<<duration_cast<milliseconds>(steady_clock::now()-startT).count()<<" ms"<<std::endl;
            
            //Join results
            for (int i = 0; i < num_threads; i++)
            {
                xg.insert(xg.end(), x[i].begin(), x[i].end());
                yg.insert(yg.end(), y[i].begin(), y[i].end());
                cg.insert(cg.end(), c[i].begin(), c[i].end());
            }
            
            //Squareify by copying data
            int i = 0;
            int orgLen = data1.length();
            int plotSize = std::max(data1.length(), data2.length());
            while (i < xg.size())
            {
                if (xg[i] + orgLen < plotSize)
                {
                    xg.push_back(xg[i] + orgLen);
                    yg.push_back(yg[i]);
                    cg.push_back(cg[i]);
                }
                i += 1;
            }
            
            //Write into file
            std::ofstream ofs("xyc.txt", std::ofstream::trunc);
            for (int i = 0; i < xg.size(); i++)
            {
                if (cg[i] == 'r')
                    ofs << std::to_string(xg[i]) + " " + std::to_string(yg[i]) + " 0\n";
                else
                    ofs << std::to_string(xg[i]) + " " + std::to_string(yg[i]) + " 1\n";

            }
            ofs.close();
            
            // Call gnuplot to save the plot
            std::string name = "output/" + entry1.path().string().substr(5, 10) + "+" + entry2.path().string().substr(5, 10) + ".png";
            std::ostringstream cmd;
            cmd << "gnuplot -e \"fname='" << name << "'\" \"gplot.txt\"";
            std::cout << name << std::endl;
            system(cmd.str().c_str());
        }
    }

    return 0;
}
