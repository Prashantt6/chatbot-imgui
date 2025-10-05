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

std::vector<std::string> response::tokenizer(const std::string &sentence) {
    std::istringstream iss(sentence);
    std::string temp;
    std::vector<std::string> tokens;

    while (iss >> temp) {
        // remove punctuation
        temp.erase(std::remove_if(temp.begin(), temp.end(),
                                  [](unsigned char c){ return std::ispunct(c); }),
                   temp.end());
        for (char &c : temp) c = std::tolower(c);
        if (!temp.empty()) tokens.push_back(temp);
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
            size_t left = (i > window) ? i - window : 0;
            size_t right = std::min(words.size() - 1, static_cast<size_t>(i + window));
            for( size_t j = left ; j <= right ; ++j){
                if (i == j) continue;
                training_pairs.push_back({target , words[j]});
            }
         }
    }
}


void response :: training(){
    std::vector<QA> training_set = load_training_data("data/trainingdata.txt");

    // Build quick response map for exact matches
    for (auto &qa : training_set) {
        std::vector<std::string> processed = preprocessing(qa.input);
        if (!processed.empty()) {
            response[processed.back()] = qa.output; // map last word to output
        }
    }

    

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
    float limit = 1.0f / std::sqrt(static_cast<float>(columns));


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

int word_id(const std::vector<std::string>& vocablist, const std::string& target) {
    auto it = std::find(vocablist.begin(), vocablist.end(), target);
    if (it == vocablist.end()) return -1;
    return static_cast<int>(std::distance(vocablist.begin(), it));
}


std::vector<float> multiply (const std::vector<float>& h , const std::vector<std::vector<float>>& W2 ){
    size_t V = W2[0].size();
    size_t D = W2.size();
    std::vector<float> u(V , 0.0);
    for (size_t i = 0; i < V; ++i){
        for( int j = 0 ; j< D  ; j++){
            u[i] +=   h[j] * W2[j][i]; //taking W2 as a flat vector
        }
    }
    return u;
}

std::vector<std::vector<float>> response :: forward_pass(int V , int D , std::vector<std::vector<float>>& W1 , std::vector<std::vector<float>>& W2){

        for(int epoch = 0 ; epoch <1000 ; epoch++){
            total_loss = 0.0f   ;
            for(auto& word : training_pairs){
                
                std::string target = word.first;
                std::string context = word.second;
                size_t wordindex = static_cast<size_t>(word_id(vocablist, target));
                auto& h = W1[wordindex];
                auto u = multiply(h , W2 );
                expo.clear();
                float sum = 0;
                float max_u = *std::max_element(u.begin() , u.end());
                for(size_t i = 0 ; i < V ; ++i){
                    float ex = static_cast<float>(std:: exp(u[i] - max_u));
                    expo.push_back(ex);
                    sum += ex;
                }
                prob.clear();
                for(size_t i = 0 ; i < V ; ++i){
                    float p = expo[i]/ sum;
                    prob.push_back(p);
                }

                // Calculating loss 
                
                int context_index = word_id(vocablist , context);
                float loss = static_cast<float>(-log(prob[context_index] + 1e-10));
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
    size_t D = h.size();
    size_t V = prob.size();
    std::vector<float>error(V , 0.0   ) ;
    std::vector<float> tempvec = one_hot[context];
    for(int i = 0 ; i < prob.size() ; i++){
        error[i] = prob[i] - tempvec[i];
    }
    

    // Gradient for W2
    std::vector<std::vector<float>>del_W2(D , std::vector<float>(V , 0.0));
    for( size_t i = 0 ; i < D ; ++i){
        for(size_t j = 0 ; j < V ; ++j){
            del_W2[i][j] = h[i] * error[j];
            W2[i][j] -= static_cast<float>(lr * del_W2[i][j]);

        }
    }

    // Gradient for W1
    std::vector<float>del_h(D , 0.0);

    for(size_t i = 0 ; i< D ; ++i){
        for(size_t j = 0 ; j< V ; ++j){
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
    size_t V = vocablist.size();
    size_t  D = embedding_size;
    auto W1 = initialize_matrix(V , D);
    auto W2 = initialize_matrix(D , V );
    
    // Forward propagation using SKIP_Gram method   
   std::vector<std::vector<float>> Word2vec =  forward_pass(V , D , W1 , W2);

    for(int j = 0 ; j <Word2vec.size() ; j++){
        wordsvec[vocablist[j]] = Word2vec[j];
    }   
}std::string response::get_response(const std::string &user_input) {
    std::vector<std::string> words = preprocessing(user_input);
    if (words.empty()) return "I didn't understand it.";

    // Check exact match shortcut
    for (auto &w : words) {
        if (response.find(w) != response.end()) {
            return response[w];
        }
    }

    // Otherwise, use embeddings + cosine similarity
    int count = 0;
    std::vector<float> input_vec(embedding_size, 0.0f);
    for (auto &w : words) {
        if (wordsvec.find(w) != wordsvec.end()) {
            for (int i = 0; i < embedding_size; i++) {
                input_vec[i] += wordsvec[w][i];
            }
            count++;
        }
    }
    if (count > 0) {
        for (int i = 0; i < embedding_size; i++) input_vec[i] /= count;
    }

    float best_sim = -1.0f;
    std::string best_response = "I didn't understand it.";
    std::vector<response::QA> training_data = load_training_data("data/trainingdata.txt");

    for (auto &qa : training_data) {
        std::vector<std::string> train_words = preprocessing(qa.input);
        int n = 0;
        std::vector<float> train_vec(embedding_size, 0.0f);
        for (auto &tw : train_words) {
            if (wordsvec.find(tw) != wordsvec.end()) {
                for (int i = 0; i < embedding_size; i++) train_vec[i] += wordsvec[tw][i];
                n++;
            }
        }
        if (n > 0) for (int i = 0; i < embedding_size; i++) train_vec[i] /= n;

        float dot = 0.0f, a = 0.0f, b = 0.0f;
        for (int i = 0; i < embedding_size; i++) {
            dot += train_vec[i] * input_vec[i];
            a += input_vec[i] * input_vec[i];
            b += train_vec[i] * train_vec[i];
        }

        float sim = dot / (std::sqrt(a) * std::sqrt(b) + 1e-10f);
        if (sim > best_sim) {
            best_sim = sim;
            best_response = qa.output;
        }
    }

    return best_response;
}



