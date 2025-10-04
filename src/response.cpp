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

std::vector<std::string> response :: tokenizer (const std::string &sentence){
    std::istringstream iss(sentence);
    std::string temp;
    std::vector<std::string> tokens;

    while (iss >> temp) {
        for (char& c : temp) { c = std::tolower(c); } // convert to lowercase
        if (!temp.empty()) {
            tokens.push_back(temp);
        }
    }
    return tokens;
}


std::vector<std::string> response :: preprocessing (const std::string &sentence)
{
    std::vector<std::string> words = tokenizer(sentence);
    std::vector<std::string> result;

    for (const auto& word : words) {
        if (stopwords.find(word) == stopwords.end()) {
            
            result.push_back(word);
        }
    }
    return result;
}

void response :: makepair(const std::vector<response :: QA>& training_set){
    
    for(auto& data : training_set){
        std::vector<std::string> words = preprocessing(data.input);
        for( int i = 0 ; i< words.size() ; i++ ){
            std::string target = words[i];
            int left = std::max(0 , i - window);
            int right = std::min((int)words.size() - 1 , i + window);
            for( int j = left ; j <= right ; j++){
                if (i == j) continue;
                training_pairs.push_back({target , words[j]});
            }
         }
    }
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

    std::unordered_map<std::string, int> word_to_index;
    for (int i = 0; i < vocablist.size(); i++){
         word_to_index[vocablist[i]] = i;
    }


    for(auto& data : training_set){
        std::vector<std::string> words = preprocessing(data.input);
        
        for(auto &word : words ){
            if(one_hot.find(word) == one_hot.end()){
                std::vector<float>temp_vec(vocablist.size() , 0);
                int index = word_to_index[word];
                temp_vec[index] = 1;
                one_hot[word] = temp_vec;
        }
    }
    }
    makepair(training_set);

}

std::vector<std::vector<float>> initialize_matrix( int rows , int columns) {
    std::vector<std::vector<float>> mat(rows , std::vector<float>(columns));
    float limit = 1.0 / std::sqrt(columns);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-limit , limit);

    for(int i = 0 ; i< rows ; i++){
        for( int j = 0 ;  j < columns ; j++){
            mat[i][j] = dist(gen);
        }
    }
    return mat;
}

int word_id(std::vector<std::string>& vocablist ,std::string& target){
    auto it = std::find(vocablist.begin(), vocablist.end(), target);
    return (it != vocablist.end()) ? std::distance(vocablist.begin(), it) : -1;
}

std::vector<float> multiply (const std::vector<float>& h , const std::vector<std::vector<float>>& W2 ){
    int V = W2[0].size();
    int D = W2.size();
    std::vector<float> u(V , 0.0);
    for(int i = 0 ; i< V ; i++){
        for( int j = 0 ; j< D  ; j++){
            u[i] +=   h[j] * W2[j][i]; //taking W2 as a flat vector
        }
    }
    return u;
}

std::vector<std::vector<float>> response :: forward_pass(int V , int D , std::vector<std::vector<float>>& W1 , std::vector<std::vector<float>>& W2){

        for(int epoch = 0 ; epoch <1000 ; epoch++){
            total_loss = 0.0;
            for(auto& word : training_pairs){
                
                std::string target = word.first;
                std::string context = word.second;
                int wordindex = word_id(vocablist , target);
                auto h = W1[wordindex];
                auto u = multiply(h , W2 );
                expo.clear();
                float sum = 0;
                float max_u = *std::max_element(u.begin() , u.end());
                for(int i = 0 ; i < V ; i++){
                    float ex = exp(u[i] - max_u);
                    expo.push_back(ex);
                    sum += ex;
                }
                prob.clear();
                for(int i = 0 ; i < V ; i++){
                    float p = expo[i]/ sum;
                    prob.push_back(p);
                }

                // Calculating loss 
                
                int context_index = word_id(vocablist , context);
                float loss = -log(prob[context_index] + 1e-10);
                total_loss += loss ;
                
                // Backward pass 
                backward_pass(h , W1 , W2 , target , context);
                
                
            }
            // Loss for each epoch
            // std::cout << "Epoch " << epoch  << " - Avg Loss: " << total_loss / training_pairs.size()  << std::endl;  
        } 

        return W1;
        

    
}

void response :: backward_pass(std::vector<float>& h ,std::vector<std::vector<float>>& W1 , std::vector<std::vector<float>>& W2, std::string& target , std::string& context){
    int D = h.size();
    int V = prob.size();
    std::vector<float>error(V , 0.0   ) ;
    std::vector<float> tempvec = one_hot[context];
    for(int i = 0 ; i < prob.size() ; i++){
        error[i] = prob[i] - tempvec[i];
    }
    

    // Gradient for W2
    std::vector<std::vector<float>>del_W2(D , std::vector<float>(V , 0.0));
    for( int i = 0 ; i < D ; i++){
        for(int j = 0 ; j < V ; j++){
            del_W2[i][j] = h[i] * error[j];
            W2[i][j] -= lr * del_W2[i][j];
        }
    }

    // Gradient for W1
    std::vector<float>del_h(D , 0.0);

    for(int i = 0 ; i< D ; i++){
        for(int j = 0 ; j< V ; j++){
            del_h[i] += error[j] * W2[i][j];

        }
       
    }
    for (auto &val : del_h) {
        if (val > 5.0) val = 5.0;
        if (val < -5.0) val = -5.0;
    }

    int target_index = word_id(vocablist , target);
    if(target_index != -1 ){
        for(int i = 0 ; i < D ; i++){
            W1[target_index][i] -= lr * del_h[i];
        }
    }



}


void response :: prediction( ){
    int V = vocablist.size();
    int D = embedding_size;
    auto W1 = initialize_matrix(V , D);
    auto W2 = initialize_matrix(D , V );
    
    // Forward propagation using SKIP_Gram method   
   std::vector<std::vector<float>> Word2vec =  forward_pass(V , D , W1 , W2);

    for(int j = 0 ; j <Word2vec.size() ; j++){
        wordsvec[vocablist[j]] = Word2vec[j];
    }   
}


