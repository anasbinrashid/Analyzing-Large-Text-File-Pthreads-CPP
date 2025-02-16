#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <pthread.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <ctime>
#include <algorithm>
using namespace std;

#define numberofthreads 8 // Define the number of threads that will be created
#define sizeofchunk 4096 // Define the size of each chunk of text to be processed

// Global synchronization variables

mutex mtx;
condition_variable cv;
bool done = false; // Flag to indicate when all chunks have been added to the queue
queue<string> tasks; // Queue to store text chunks for processing

// Thread-local data structure to store word frequency for each thread

struct dataforthread 
{
    unordered_map<string, int> wordfrequency; // Local word frequency map
};

vector<dataforthread> threadsvector(numberofthreads); // Vector to hold thread-specific data
pthread_t threads[numberofthreads]; // Array to store thread identifiers

// Function executed by each thread to process text chunks from the queue

void* chunking(void* arg) 
{
    dataforthread* data = (dataforthread*)arg;
    
    while (true) 
    {
        string chunk;
        {
            unique_lock<mutex> lock(mtx); // Lock the mutex to safely access the queue
            cv.wait(lock, [] 
            { 
            	return !tasks.empty() || done; 
            }); // Wait until a chunk is available
            
            if (tasks.empty() && done) 
            {
            	break; // Exit if all tasks are done
            }
            
            chunk = move(tasks.front()); // Retrieve the next chunk
            tasks.pop();
        }

        string word;
        
        // Process each character in the chunk
        
        for (char ch : chunk) 
        {
            if (isalpha(ch)) 
            {
                word += tolower(ch); // Convert character to lowercase and add to current word
            } 
            else if (!word.empty()) // If a word has been formed
            {
                data->wordfrequency[word]++; // Update local word frequency
                word.clear(); // Reset word for next occurrence
            }
        }
        
        if (!word.empty()) // If there is an unfinished word at the end of chunk
        {
            data->wordfrequency[word]++;
        }
    }
    
    return nullptr;
}

// Function to write results to an output file

void filewriting(const unordered_map<string, int>& overallfrequency, int wordcount, int uniquewordcount) 
{
    ofstream outputFile("3output.txt"); // Open output file
    
    if (!outputFile) 
    {
        cerr << "Error creating output file!" << endl;
        return;
    }
    
    // Sort word counts in descending order of frequency
    
    vector<pair<string, int>> sortedwordcount(overallfrequency.begin(), overallfrequency.end());
    sort(sortedwordcount.begin(), sortedwordcount.end(), [](const auto& a, const auto& b) 
    {
        if (a.second > b.second)
        {
        	return true;
        }
        
        return false;
    });

    // Write results to the file
    
    outputFile << "Total word count: " << wordcount << "\n";
    outputFile << "Total number of unique words: " << uniquewordcount << "\n\n";
    outputFile << "Term frequency for each word (sorted by frequency):\n";
    
    for (const auto& pair : sortedwordcount) 
    {
        outputFile << pair.first << ": " << pair.second << "\n";
    }
    
    cout << "Results written to 3output.txt" << endl;
}

int main() 
{
    clock_t start_time = clock(); // Start measuring execution time
    cout << "Opening File" << endl;
    
    ifstream file("text.txt"); // Open the input file
    
    if (!file) 
    {
        cerr << "Error Opening File" << endl;
        return 1;
    }

    cout << "Creating Threads" << endl;
    
    // Create worker threads
    
    for (int i = 0; i < numberofthreads; i++) 
    {
        pthread_create(&threads[i], nullptr, chunking, &threadsvector[i]);
    }

    // Read the file in chunks and add them to the task queue
    
    string buffer;
    buffer.reserve(sizeofchunk); // Reserve memory for buffer to store text chunks
    
    while (file) 
    {
        char ch;
        while (buffer.size() < sizeofchunk && file.get(ch)) 
        {
            buffer += ch;
        }
        {
            lock_guard<mutex> lock(mtx); // Lock the queue before adding a new task
            tasks.push(move(buffer)); // Move the buffer into the queue
        }
        
        cv.notify_one(); // Notify one waiting thread
        buffer.clear();
    }

    // Signal that all chunks have been processed
    
    {
        lock_guard<mutex> lock(mtx);
        done = true;
    }
    cv.notify_all(); // Wake up all threads to finish processing

    cout << "Threads Joining" << endl;
    
    // Wait for all threads to finish execution
    
    for (int i = 0; i < numberofthreads; i++) 
    {
        pthread_join(threads[i], nullptr);
    }

    cout << "Finalising Calculations" << endl;
    
    unordered_map<string, int> overallfrequency;
    
    // Combine word frequencies from all threads
    
    for (const auto& data : threadsvector) 
    {
        for (const auto& entry : data.wordfrequency) 
        {
            overallfrequency[entry.first] += entry.second;
        }
    }

    // Calculate total words and unique words
    
    int wordcount = 0;
    
    for (const auto& pair : overallfrequency) 
    {
        wordcount += pair.second;
    }
    
    int uniquewordcount = overallfrequency.size();

    // Output results to console
    
    cout << "Total Words: " << wordcount << endl;
    cout << "Total Unique Words: " << uniquewordcount << endl;

    // Write results to a file
    
    filewriting(overallfrequency, wordcount, uniquewordcount);

    // Measure and print execution time
    
    clock_t end_time = clock();
    cout << "Execution Time: " << double(end_time - start_time) / CLOCKS_PER_SEC << " seconds" << endl;
    
    return 0;
}
