#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::cout << "=== Mapbox API Key Test ===" << std::endl;
    
    // Test 1: Check if .env file exists
    std::ifstream envFile(".env");
    if (!envFile.is_open()) {
        std::cout << "ERROR: .env file not found!" << std::endl;
        return 1;
    }
    
    // Test 2: Read and parse .env file
    std::string line;
    std::string mapboxKey;
    bool foundKey = false;
    
    while (std::getline(envFile, line)) {
        if (line.find("MAPBOX_API_KEY=") == 0) {
            mapboxKey = line.substr(15); // Remove "MAPBOX_API_KEY="
            foundKey = true;
            break;
        }
    }
    envFile.close();
    
    if (!foundKey) {
        std::cout << "ERROR: MAPBOX_API_KEY not found in .env file!" << std::endl;
        return 1;
    }
    
    // Test 3: Validate API key format
    if (mapboxKey.empty()) {
        std::cout << "ERROR: MAPBOX_API_KEY is empty!" << std::endl;
        return 1;
    }
    
    if (mapboxKey.length() < 10) {
        std::cout << "ERROR: MAPBOX_API_KEY too short (length: " << mapboxKey.length() << ")" << std::endl;
        return 1;
    }
    
    if (mapboxKey.substr(0, 3) != "pk.") {
        std::cout << "ERROR: MAPBOX_API_KEY should start with 'pk.'" << std::endl;
        return 1;
    }
    
    // Test 4: Display results
    std::cout << "SUCCESS: Found valid Mapbox API key!" << std::endl;
    std::cout << "Key prefix: " << mapboxKey.substr(0, 10) << "..." << std::endl;
    std::cout << "Key length: " << mapboxKey.length() << " characters" << std::endl;
    
    return 0;
}
