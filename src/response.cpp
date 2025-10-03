#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cmath>
#include <unordered_set>
#include <iterator>
#include <fstream>
#include <random>
#include <cmath>
#include <map>
#include "response.h"



std::vector<response:: QA> response::load_training_data(const std::string &trainingdata) {
    std::ifstream data(trainingdata);
    std::vector<response :: QA> training_set;
    std::string line;

    if (!data.is_open()) {
        std::cerr << "Training data not loaded" << std::endl;
        return training_set;
    }

    while (std::getline(data, line)) {
        if (!line.empty()) {
            std::istringstream iss(line);
            std::string input, output;
            if (std::getline(iss, input, '|') && std::getline(iss, output)) {
                training_set.push_back({input , output});
            }
        }
    }
    data.close();
    return training_set;
}

void response :: training(){
    std::vector<QA> training_set = load_training_data("trainingdata.txt");
    

    // Step 1: Build vocabulary from all training sentences
    for (auto &data : training_set) {
        std::vector<std::string> words = preprocessing(data.input);
        for (auto &word : words) {
            vocabulary.insert(word);
        }
    }

    // Step 2: Convert vocabulary (set) to a list for indexing
    vocablist.assign(vocabulary.begin(), vocabulary.end());

    for(auto& data : training_set){
        std::vector<std::string> words = preprocessing(data.input);
        
        for(auto &word : words ){
            if(wordsvec.find(word) == wordsvec.end()){
                std::vector<float>temp_vec(vocablist.size() , 0);
                auto it = std::find(vocablist.begin() , vocablist.end() , word );
                if( it != vocablist.end()){
                    int index = std::distance(vocablist.begin() , it);
                    temp_vec[index] = 1;
                }
                wordsvec[word] = temp_vec;
        }
    }
    }
    // makepair(training_set);

}