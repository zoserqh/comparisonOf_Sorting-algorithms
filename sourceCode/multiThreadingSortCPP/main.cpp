//g++ main.cpp -pthread -o ../../bin/mtCPP
// reading file: 5.65481sec
// filling buckets: 5.09247sec
// sorting buckets with multi-threading: 9.71529sec
// grouping buckets: 1.0708sec
// total time taken sorting: 15.8786sec

#include<string>
#include<algorithm>
#include<iostream>
#include<fstream>
#include<vector>
#include <iomanip>
#include <chrono>
#include<thread>
#include <filesystem>
#include <cstdint>
#include <cstring>

void readList(std::vector<double>& v);
void sortList(std::vector<double>& v);
void fillBuckets(std::vector<double>& v,std::vector<std::vector<double>>& buckets);
void mtSorting(std::vector<std::vector<double>>& buckets);
void sortingBuckts(std::vector<double>* b);
void grouping(std::vector<double>& v,std::vector<std::vector<double>>& buckets);
void mtRead(char* ptrb,double* ptra,int lengthLine,int arrSize, int nroThreads);
void printTimeTaken(std::chrono::_V2::high_resolution_clock::time_point start,
                    std::chrono::_V2::high_resolution_clock::time_point end,std::string text);

int main(){

    std::vector<double> v;

    readList(v);
    sortList(v);

    //test
    for(int i=1;i<v.size();i++){
        if(v[i-1]>v[i]){ std::cout<<"unsorted\n"; }
    }

    // std::cout << std::fixed << std::setprecision(20);
    // for(int i=0;i<v.size();i++){
    //     std::cout << v[i] << " ";
    // } std::cout << "\n";

    return 0;
}

void readList(std::vector<double>& v){
    auto t1 = std::chrono::high_resolution_clock::now();
    
    std::string pathFile = "../data/list.txt";
    std::ifstream myFile (pathFile);
    unsigned long int bufferSize = std::filesystem::file_size(pathFile);
    std::cout << "File size is: "<< bufferSize << " bytes" << std::endl;
    std::unique_ptr<char[]> buffer(new char[bufferSize]);
    
    if (myFile.is_open()){
        myFile.read(buffer.get(), bufferSize);

        //each line have one number with 22 digits + 1 \n = 23
        int lengthLine = 23;
        int arrSize = bufferSize/lengthLine;
        std::unique_ptr<double[]> arr(new double[arrSize]);
        
        //multiThreading using 10 threads
        int nroThreads = 10;
        char* ptrB[nroThreads]; // pointer to buffer
        double* ptrA[nroThreads]; // pointer to array
        std::thread myThreads[nroThreads];
        for(int i=0;i<nroThreads;i++){
            ptrB[i]=&buffer[i*(bufferSize/nroThreads)];
            ptrA[i]=&arr[i*(arrSize/nroThreads)];
            myThreads[i] = std::thread(mtRead,ptrB[i],ptrA[i],lengthLine,arrSize,nroThreads);
        }

        for(int i=0;i<nroThreads;i++){
            myThreads[i].join();
        }

        v.insert(v.begin(),ptrA[0],ptrA[0]+arrSize);
    }
    else {  std::cout << "Unable to open file"; }

    auto t2 = std::chrono::high_resolution_clock::now();
    printTimeTaken(t1,t2,"reading file: ");
}

void sortList(std::vector<double>& v){
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::vector<double>> buckets(10);
    fillBuckets(v,buckets);
    mtSorting(buckets);
    grouping(v,buckets);
    
    auto end = std::chrono::high_resolution_clock::now();
    printTimeTaken(start,end,"total time taken sorting: ");

    // std::cout << std::fixed << std::setprecision(20);
    // for(int i=0;i<v.size();i++){
    //     std::cout << v[i] << " "; 
    // } std::cout << "\n";

}

void fillBuckets(std::vector<double>& v,std::vector<std::vector<double>>& buckets){
    auto start = std::chrono::high_resolution_clock::now();

    for(int i=0;i<v.size();i++){
        int nroBucket=0;
        if(v[i]<0.1){ nroBucket = 0; }
        else if(v[i]<0.2){ nroBucket = 1;}
        else if(v[i]<0.3){ nroBucket = 2;}
        else if(v[i]<0.4){ nroBucket = 3;}
        else if(v[i]<0.5){ nroBucket = 4;}
        else if(v[i]<0.6){ nroBucket = 5;}
        else if(v[i]<0.7){ nroBucket = 6;}
        else if(v[i]<0.8){ nroBucket = 7;}
        else if(v[i]<0.9){ nroBucket = 8;}
        else if(v[i]<=1.0){ nroBucket = 9;}

        buckets[nroBucket].push_back(v[i]);
    }

    auto end = std::chrono::high_resolution_clock::now();
    printTimeTaken(start,end,"filling buckets: ");

    // for(int i=0;i<buckets.size();i++){
    //     std::cout << buckets[i].size()<<" ";
    // } std::cout << "\n";
}

void mtSorting(std::vector<std::vector<double>>& buckets){
    auto start = std::chrono::high_resolution_clock::now();

    std::thread myThreads[10];
    for(int i=0;i<10;i++){
        myThreads[i] = std::thread(sortingBuckts,&buckets[i]);
    }

    for(int i=0;i<10;i++){
        myThreads[i].join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    printTimeTaken(start,end,"sorting buckets with multi-threading: ");
}
void sortingBuckts(std::vector<double>* b){

    std::sort (b->begin(), b->end());
    //std::cout<< "id: "<<std::this_thread::get_id()<<"\n";
}

void grouping(std::vector<double>& v,std::vector<std::vector<double>>& buckets){
    auto start = std::chrono::high_resolution_clock::now();

    int offset = 0;
    for(int i=0;i<buckets.size();i++){
        for(int j=0;j<buckets[i].size();j++){
            v[offset+j]=buckets[i][j];
        }
        offset +=buckets[i].size();
    }

    auto end = std::chrono::high_resolution_clock::now();
    printTimeTaken(start,end,"grouping buckets: ");
}

void mtRead(char* ptrb, double* ptra, int lengthLine, int arrSize, int nroThreads){
    for(int i=0;i<arrSize/nroThreads;i++){
        char number[lengthLine-1];
        std::memcpy(number,ptrb,lengthLine-1);
        *ptra=std::atof(number);

        ptrb += lengthLine;
        ptra += 1;
    }
    // std::cout<< "id: "<<std::this_thread::get_id()<< " "<< *(ptrb+2) <<"\n";
}

void printTimeTaken(std::chrono::_V2::high_resolution_clock::time_point start,
                    std::chrono::_V2::high_resolution_clock::time_point end,std::string text){
    double time_taken = ( std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() ) * 1e-9;
    std::cout << text << time_taken << " sec\n";

}