#include <fstream>
#include <iostream>
#include <vector>

#include "CameraSystem.h"
#include "CollisionComponent.h"
#include "CollisionLateSystem.h"
#include "CollisionSystem.h"
#include "ECSEngine.h"
#include "EntityManager.h"
#include "GravitySystem.h"
#include "InputComponent.h"
#include "InputSystem.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "MovementSystem.h"
#include "ScoreComponent.h"
#include "ScoreSystem.h"
#include "SpawnComponent.h"
#include "SpawnSystem.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SpriteSystem.h"

std::string gResourcePath = "../assets/";
#define COMPONENTS                                                                             \
    ECSEngine::LocationComponent, ECSEngine::MovementComponent, ECSEngine::CollisionComponent, \
        ECSEngine::SpriteComponent, ECSEngine::SpawnComponent, ECSEngine::CameraComponent,     \
        ECSEngine::CameraFollower, ECSEngine::InputComponent, ECSEngine::GravityComponent,     \
        ECSEngine::CameraShake, ECSEngine::ScoreComponent, ECSEngine::RespawnComponent
using Engine = ECSEngine::ECSEngine<COMPONENTS>;

void LoadMap(Engine& engine, const std::string& path, const std::string& file) {}

struct loadedSprites{
    int dictIdent = 0;
    char spriteName = 'z';
    std::string path = "_";
    int locToplx = 1;
    int locToply = 1;
    int locW = 1;
    int locH = 1;
    int bbToplx = 0;
    int bbToply = 0;
    int bbW = 0;
    int bbH = 0;
};
struct mapOrign{

    int Toplx = 1;
    int Toply = 1;
    int tileW = 1;
    int tileH = 1;
    int gridW = 1;
    int gridH = 1;
};

int main(int argc, char* argv[]) {
    if (argc >= 3 && argv[1] == std::string("-path")) {
        gResourcePath = argv[2];
    }
    std::cout << "Usage: " << argv[0] << " [-path resource_path]\n";
    std::cout << "Using resource path: " << gResourcePath << "\n";

    // Engine test code
    Engine engine(1024, 768, "Test Engine");

    // Load sounds

    // Load maps it isn't pretty but it works
    //load world map
    std::ifstream inf{ gResourcePath+"world.map" };
    // If we couldn't open the output file stream for reading
    if (!inf)
    {
        // Print an error and exit
        std::cerr << "Uh oh, map could not be opened for reading!\n";
        return 1;
    }
    std::string strInput{};
    std::string state ="null";
    loadedSprites ls;
    //reading dictionary 2
    inf >> state >> ls.dictIdent;
    std::cout << "state: "<< state << '\n';
    std::vector<loadedSprites> spriteVector = {};
    //dictionary 1
    // h spritesheet-backgrounds-default.png 256 768 256 256
    while (state == "dictionary"){
        inf >> strInput;
        std::cout << "current str in dict: "<< strInput << '\n';
        if(strInput.length()==1){
            ls.spriteName = strInput[0];
            inf >> ls.path >> ls.locToplx >>ls.locToply >> ls.locW >> ls.locH 
                >> ls.bbToplx >>ls.bbToply >> ls.bbW >> ls.bbH;
                spriteVector.push_back(ls);
        }else{
            std::cout << "leaving dict: "<< strInput << '\n';
            state = strInput;
        }
    }
    for (auto i : spriteVector)
        std::cout << i.spriteName << "\n";

    mapOrign mp;
    //map origin 0 0 tile 64 64 size 64 14
    inf >> state >> mp.Toplx >> mp.Toply;
    std::cout << " new state: " << state << '\n';
    inf >> state >> mp.tileW >> mp.tileH;
    std::cout << " new state: " << state << '\n';
    inf >> state >> mp.gridW  >> mp.gridH;

    std::vector<std::string> map = {};
    for(int i = 0; i < mp.gridH; ++i){
        std::getline(inf, strInput);
        //skipping blank line
        if(i!= 0){
            std::cout << "inside mpGrid: " << strInput << '\n';
            map.push_back(strInput);
        }

    }

    //skymap
    std::ifstream inf2{ gResourcePath+"sky.map" };
    // If we couldn't open the output file stream for reading
    if (!inf2)
    {
        // Print an error and exit
        std::cerr << "Uh oh, map could not be opened for reading!\n";
        return 1;
    }
    loadedSprites ls2;
    //reading dictionary 2
    inf2 >> state >> ls2.dictIdent;
    std::cout << "state: "<< state << '\n';
    std::cout << "ls2.dictIdent: "<< ls2.dictIdent << '\n';
    std::vector<loadedSprites> spriteVector2 = {};
    std::cout << "before str in dict2: "<< strInput << '\n';
    //h spritesheet-backgrounds-default.png 256 768 256 256
    while (state == "dictionary"){
        inf2 >> strInput;
        std::cout << "current str in dict2: "<< strInput << '\n';
        if(strInput.length()==1){
            ls2.spriteName = strInput[0];
            inf2 >> ls2.path >> ls2.locToplx >>ls2.locToply >> ls2.locW >> ls2.locH;
            std::cout << "current ls2.path in dict2 part 2: "<< ls2.path << '\n';
            std::cout << "current ls2.locToplx in dict2 part 2: "<< ls2.locToplx << '\n';
            spriteVector.push_back(ls2);
        }else{
            std::cout << "leaving dict: "<< strInput << '\n';
            state = strInput;
        }
    }
    for (auto i : spriteVector2)
        std::cout << i.spriteName << "\n";

    mapOrign mp2;
    //map origin 0 0 tile 64 64 size 64 14
    inf2 >> state >> mp2.Toplx >> mp2.Toply;
    std::cout << " new state: " << state << '\n';
    inf2 >> state >> mp2.tileW >> mp2.tileH;
    std::cout << " new state: " << state << '\n';
    inf2 >> state >> mp2.gridW  >> mp2.gridH;

    std::vector<std::string> map2 = {};
    for(int i = 0; i < mp2.gridH; ++i){
        std::getline(inf2, strInput);
        
        if(i!=0){
            map2.push_back(strInput);
            std::cout << "inside mpGrid: " << strInput << '\n';
        }
        
    }
    


    // Place player

    // Setup score component

    // Setup Camera

    // Add all systems
    auto& sm = engine.GetSystemManager();
    sm.AddSystem(std::make_unique<ECSEngine::CollisionLateSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::InputSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::GravitySystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::MovementSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CollisionSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::ScoreSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CameraSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpriteSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpawnSystem<COMPONENTS>>());

    engine.Run();
    return 0;
}
