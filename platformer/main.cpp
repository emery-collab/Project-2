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
    int bbToplx = 1;
    int bbToply = 1;
    int bbW = 1;
    int bbH = 1;
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

    // Load maps
    // ifstream is used for reading files
    // We'll read from a file called Sample.txt
    std::ifstream inf{ gResourcePath+"world.map" };

    // If we couldn't open the output file stream for reading
    if (!inf)
    {
        // Print an error and exit
        std::cerr << "Uh oh, map could not be opened for reading!\n";
        return 1;
    }

    // While there's still stuff left to read
    std::string strInput{};

    std::string state ="null";
    loadedSprites ls;
    //reading dictionary 2
    inf >> state >> ls.dictIdent;
    std::cout << "state: "<< state << '\n';
    std::vector<loadedSprites> spriteVector = {};
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
        std::cout << i.path << "\n";

    mapOrign mp;
    //map origin 0 0 tile 64 64 size 64 14
    inf >> state >> mp.Toplx >> mp.Toply;
     std::cout << " new state: " << state << '\n';
    inf >> state >> mp.tileW >> mp.tileH;
     std::cout << " new state: " << state << '\n';
    inf >> state >> mp.gridW  >> mp.gridH;
    std::cout << " new state: " << state << '\n';
    std::cout << " mp.Toplx : " << mp.Toplx  << '\n';
    std::cout << " mp.Toply: " << mp.Toply<< '\n';
    std::cout << " mp.W : " << mp.tileW << '\n';
    std::cout << " mp.H: " << mp.tileH<< '\n';
    std::cout << " mpGridy: " << mp.gridH << '\n';
    std::cout << " mpGridx: " << mp.gridW << '\n';
    std::cout << " mpGridy: " << mp.gridH << '\n';
    std::vector<std::string> map = {};
    for(int i = 0; i < mp.gridH; ++i){
        std::getline(inf, strInput);
        std::cout << "inside mpGrid: " << strInput << '\n';
        map.push_back(strInput);
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
