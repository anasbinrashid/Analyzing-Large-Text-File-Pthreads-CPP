#define _GNU_SOURCE
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <pthread.h>
#include <algorithm>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <ctime>
#include <sched.h>
#include <unistd.h>
using namespace std;

#define numberofthreads 8  // Define the number of threads that will be created
#define sizeofchunk 4096  // Define the size of each chunk of text to be processed

// Mutex and condition variable for thread synchronization

mutex mtx;
condition_variable cv;
bool done = false;  // Flag to indicate when all chunks have been added to the queue
queue<string> tasks;  // Queue to store text chunks for processing

// Structure to hold data processed by each thread

struct dataforthread 
{
    size_t threadswords = 0;  // Count of words processed by the thread
    size_t threadsvowels = 0;  // Count of words starting with a vowel
    unordered_map<string, int> threadswordscount;  // Word frequency map for each thread
    int threadid;  // ID of the thread for CPU affinity
};

vector<dataforthread> threadsvector(numberofthreads);  // Vector to hold thread-specific data
pthread_t threads[numberofthreads];  // Array to store thread identifiers

// Function to check if a character is a vowel

bool vowelcheck(char ch) 
{    
    if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' || ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U')
    {
    	return true;
    }
    else
    {
    	return false;
    }
}

// Thread function to process text chunks from the queue
void* chunking(void* arg) 
{
    dataforthread* data = (dataforthread*)arg;
    
    // Set CPU affinity to maximize parallelism
    
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(data->threadid % sysconf(_SC_NPROCESSORS_ONLN), &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    while (true) 
    {
        string chunk;
        {
            unique_lock<mutex> lock(mtx);  // Lock the mutex to safely access the queue
            cv.wait(lock, [] 
            { 
                return !tasks.empty() || done; 
            });  // Wait until a chunk is available
            
            if (tasks.empty() && done)
            {
            	break;  // Exit if all tasks are done
            }
            
            chunk = move(tasks.front());  // Retrieve the next chunk
            tasks.pop();
        }

        size_t words = 0;
        size_t vowels = 0;
        unordered_map<string, int> wordscount;
        string word;
        
        // Process each character in the chunk
        
        for (char ch : chunk) 
        {
            if (isalpha(ch)) 
            {
                word += tolower(ch);  // Convert character to lowercase and add to current word
            } 
            else if (!word.empty())  // If a word has been formed
            {
                words++;  // Increment word count
                
                if (vowelcheck(word[0]))
                {
                	vowels++;  // Check if the word starts with a vowel
                } 
                
                wordscount[word]++;  // Update word frequency
                word.clear();  // Reset word for next occurrence
            }
        }
        
        if (!word.empty())  // If there is an unfinished word at the end of chunk
        {
            words++;
            
            if (vowelcheck(word[0]))
            {
            	vowels++;
            }
            
            wordscount[word]++;
        }

        // Store results in thread-specific data structure
        
        data->threadswords = data->threadswords + words;
        data->threadsvowels = data->threadsvowels + vowels;
        
        for (const auto& pair : wordscount) 
        {
            data->threadswordscount[pair.first] += pair.second;
        }
    }
    
    return nullptr;
}

int main() 
{    
    clock_t thestart = clock();  // Start measuring execution time
    
    cout << "Opening Text File\n";
    
    ifstream file("text.txt");  // Open the input file
    
    if (!file) 
    {
        cerr << "Error Opening File" << endl;
        return 1;
    }

    cout << "Creating Threads\n";
    
    // Create worker threads
    
    for (int i = 0; i < numberofthreads; i++) 
    {
        threadsvector[i].threadid = i;  // Assign thread ID
        pthread_create(&threads[i], nullptr, chunking, &threadsvector[i]);
    }

    string buffer;
    buffer.reserve(sizeofchunk);  // Reserve memory for buffer to store text chunks
    
    // Read file contents and divide into chunks
    
    while (file) 
    {
        char ch;
        while (buffer.size() < sizeofchunk && file.get(ch)) 
        {
            buffer = buffer + ch;
        }
        
        {
            lock_guard<mutex> lock(mtx);  // Lock the queue before adding a new task
            tasks.push(move(buffer));  // Move the buffer into the queue
        }
        
        cv.notify_one();  // Notify one waiting thread
        buffer.clear();
    }

    {
        lock_guard<mutex> lock(mtx);
        done = true;  // Indicate that no more chunks will be added
    }
    
    cv.notify_all();  // Wake up all threads to finish processing

    cout << "Threads Joining\n";
    
    // Wait for all threads to finish execution
    
    for (int i = 0; i < numberofthreads; i++) 
    {
        pthread_join(threads[i], nullptr);
    }

    cout << "Finalising Calculations\n";
    
    size_t totalwordcount = 0;
    size_t vowelcount = 0;
    unordered_map<string, int> overallfrequency;
    
    // Combine word counts from all threads
    
    for (const auto& data : threadsvector) 
    {
        totalwordcount = totalwordcount + data.threadswords;
        vowelcount = vowelcount + data.threadsvowels;
        
        for (const auto& entry : data.threadswordscount) 
        {
            overallfrequency[entry.first] = overallfrequency[entry.first] + entry.second;
        }
    }

    vector<pair<string, int>> finalwords(overallfrequency.begin(), overallfrequency.end());
    
    // Sort words based on frequency
   
    sort(finalwords.begin(), finalwords.end(), [](const auto& a, const auto& b) 
    {
        if (a.second > b.second)
        {
        	return true;
        }
        
        return false;
    });

    // Display results
    
    cout << "\nTotal Words: " << totalwordcount;
    cout << "\nTotal Words Starting with Vowel: " << vowelcount;
    cout << "\nTop 10 Most Frequent Words:\n";
    
    for (size_t i = 0; i < min(finalwords.size(), size_t(10)); i++) 
    {
        cout << finalwords[i].first << " --> " << finalwords[i].second << "\n";
    }
    
    clock_t theend = clock();
    
    cout << "\nExecution Time: " << (double(theend - thestart) / CLOCKS_PER_SEC) << " seconds\n";
    
    return 0;
}

