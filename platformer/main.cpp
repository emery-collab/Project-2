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
    ECSEngine::SpriteID spriteID = 0;
};
struct mapOrign{

    int Toplx = 1;
    int Toply = 1;
    int tileW = 1;
    int tileH = 1;
    int gridW = 1;
    int gridH = 1;
};
void LoadMap(Engine& engine, const std::string& path, const std::string& file) {
    std::ifstream inf{ path+file };
    // If we couldn't open the output file stream for reading
    if (!inf)
    {
        // Print an error and exit
        std::cerr << "Uh oh, map could not be opened for reading!\n";
        return;
    }
    std::string strInput{};
    std::string state ="null";
    loadedSprites ls;
    //reading dictionary 2
    inf >> state >> ls.dictIdent;
    std::cout << "state: "<< state << '\n';
    std::unordered_map<char, loadedSprites> spriteMap;
    while (state == "dictionary"){
        inf >> strInput;
        std::cout << "current str in dict: "<< strInput << '\n';
        if(strInput.length()==1){
            ls.spriteName = strInput[0];
            inf >> ls.path >> ls.locToplx >>ls.locToply >> ls.locW >> ls.locH;
            if(ls.dictIdent == 2){
                //bounding box
                inf >> ls.bbToplx >>ls.bbToply >> ls.bbW >> ls.bbH;
            }
            ls.spriteID = engine.GetSpriteManager().RegisterTexture(path+ls.path,
                ECSEngine::Rect(ECSEngine::Point2D(ls.locToplx, ls.locToply), ls.locW, ls.locH));
            spriteMap[ls.spriteName] = ls;

        }else{
            //std::cout << "leaving dict: "<< strInput << '\n';
            state = strInput;
        }
    }
    mapOrign mp;
    //map origin 0 0 tile 64 64 size 64 14
    inf >> state >> mp.Toplx >> mp.Toply;
    //std::cout << " new state: " << state << '\n';
    inf >> state >> mp.tileW >> mp.tileH;
    //std::cout << " new state: " << state << '\n';
    inf >> state >> mp.gridW  >> mp.gridH;

    std::vector<std::string> map = {};
    std::getline(inf, strInput);
    for(int i = 0; i < mp.gridH; ++i){
        std::getline(inf, strInput);
        std::cout << "inside mpGrid: " << strInput << '\n';
        map.push_back(strInput);
    }
    auto& em = engine.GetEntityManager();
    ECSEngine::SpriteID starID;
    starID = engine.GetSpriteManager().RegisterTexture(path+"spritesheet-tiles-default.png",
                ECSEngine::Rect(ECSEngine::Point2D(640, 320), 64, 64));
    for(size_t row = 0; row < map.size(); ++row){
        for(size_t col = 0; col < map[row].size(); ++col){
            char ch = map[row][col];
            auto it = spriteMap.find(ch);
            if (it == spriteMap.end()) continue;
            const loadedSprites& entry = it->second;
            float x = mp.Toplx + col * mp.tileW;
            float y = mp.Toply + row * mp.tileH;

            ECSEngine::EntityID id = em.CreateEntity(std::string(1, ch));
            //location component
            ECSEngine::LocationComponent locComp{};
            locComp.loc = ECSEngine::Point2D(x,y);
            em.AddComponent(id, locComp);
            //Sprite component
            ECSEngine::SpriteComponent sprComp{};
            sprComp.textureID = entry.spriteID;
            sprComp.bounds = ECSEngine::Rect(ECSEngine::Point2D(0.0f, 0.0f), entry.locW, entry.locH);
            sprComp.worldSpace = true;
            em.AddComponent(id, sprComp);
            //collision component
            if(entry.dictIdent==2){
                ECSEngine::CollisionComponent collComp{};
                collComp.staticEntity = 1;
                collComp.collisionBox = ECSEngine::Rect(ECSEngine::Point2D(entry.bbToplx, entry.bbToply), 
                    entry.bbW, entry.bbH);
                collComp.previousCollisionBox = ECSEngine::Rect(ECSEngine::Point2D(entry.bbToplx, entry.bbToply), 
                    entry.bbW, entry.bbH);
                em.AddComponent(id, collComp);
            }
            if (ch == 'S') {
                ECSEngine::SpawnComponent spwComp{};
                spwComp.entityID = id;
                spwComp.spawnType = "star";
                spwComp.spawnTextureID = starID;
                spwComp.timeToNextSpawn = 80;
                spwComp.timeBetweenSpawns = 80;
                spwComp.totalSpawns = 10;

                em.AddComponent(id, spwComp);

            }
        }   
    }
}


int main(int argc, char* argv[]) {
    if (argc >= 3 && argv[1] == std::string("-path")) {
        gResourcePath = argv[2];
    }
    std::cout << "Usage: " << argv[0] << " [-path resource_path]\n";
    std::cout << "Using resource path: " << gResourcePath << "\n";

    // Engine test code
    Engine engine(1024, 768, "Test Engine");

    // Load sounds

    engine.GetSoundManager().RegisterSound(gResourcePath + "error_008.ogg", "error");
    engine.GetSoundManager().RegisterSound(gResourcePath + "footstep_grass_003.ogg", "grassFootsteps");
    engine.GetSoundManager().RegisterSound(gResourcePath + "footstep_snow_001.ogg", "snowFootsteps");
    engine.GetSoundManager().RegisterSound(gResourcePath + "laserLarge_000.ogg", "laser");
    engine.GetSoundManager().RegisterSound(gResourcePath + "sfx_gem.ogg", "gem");
    engine.GetSoundManager().RegisterSound(gResourcePath + "sfx_jump-high.ogg", "highJump");
    engine.GetSoundManager().RegisterSound(gResourcePath + "sfx_jump.ogg", "jump");

    //load world map
    LoadMap(engine, gResourcePath, "sky.map");
    LoadMap(engine, gResourcePath, "world.map");

    // Place player
    auto& em = engine.GetEntityManager();
    ECSEngine::SpriteID playerSpriteID;
    //purple guy
    playerSpriteID = engine.GetSpriteManager().RegisterTexture(gResourcePath+"spritesheet-characters-default.png",
                ECSEngine::Rect(ECSEngine::Point2D(140, 140), 116, 116));
    ECSEngine::EntityID id = em.CreateEntity("player");
    ECSEngine::Point2D spawn(13 * 64.0f, 6 * 64.0f);
    //location component
    ECSEngine::LocationComponent locComp{};
    locComp.loc = ECSEngine::Point2D(spawn);
    em.AddComponent(id, locComp);
    //Sprite component
    ECSEngine::SpriteComponent sprComp{};
    sprComp.textureID = playerSpriteID;
    sprComp.bounds = ECSEngine::Rect(ECSEngine::Point2D(0.0f, 0.0f), 128, 128);
    sprComp.worldSpace = true;
    em.AddComponent(id, sprComp);
    //collision
    ECSEngine::CollisionComponent collComp{};
    collComp.collisionBox = ECSEngine::Rect(ECSEngine::Point2D(14,16), 90,110);
    collComp.previousCollisionBox = ECSEngine::Rect(ECSEngine::Point2D(14,16), 90,110);
    em.AddComponent(id, collComp);
    //playermovement
    ECSEngine::MovementComponent movComp{};
    movComp.velocity = ECSEngine::Point2D(0,0);
    em.AddComponent(id, movComp);
    //gravity in pixels
    float gravity = 1500;
    ECSEngine::GravityComponent grvComp{ECSEngine::Point2D(0.0f,gravity)};
    em.AddComponent(id, grvComp);
    //inputComponent
    ECSEngine::InputComponent inptComp{id};
    em.AddComponent(id, inptComp);
    //respawn 
    ECSEngine::RespawnComponent respwComp{};
    respwComp.loc = spawn;
    em.AddComponent(id, respwComp);
    
    // Setup score component
    ECSEngine::ScoreComponent scoreComp{};
    for(int i = 0; i < 10; ++i){
        float x = 832;
        float y = (13-i) * 64;
        scoreComp.digitTextureID[i] = engine.GetSpriteManager().RegisterTexture(gResourcePath+"spritesheet-tiles-default.png",
                ECSEngine::Rect (ECSEngine::Point2D(x, y), 64, 64));
    }
    for(int j = 0; j<3; ++j){
        scoreComp.digitID[j] = em.CreateEntity(std::string("digit"+std::to_string(j)));
        //location component
        ECSEngine::LocationComponent locCompScore{};
        locCompScore.loc = ECSEngine::Point2D(16+j*64,16);
        em.AddComponent(scoreComp.digitID[j], locCompScore);
        //Sprite component
        ECSEngine::SpriteComponent sprCompScore{};
        sprCompScore.textureID = scoreComp.digitTextureID[0];
        sprCompScore.bounds = ECSEngine::Rect(ECSEngine::Point2D(0.0f, 0.0f), 64, 64);
        sprCompScore.worldSpace = false;
        em.AddComponent(scoreComp.digitID[j], sprCompScore);
    }
    ECSEngine::EntityID scoreEntity = em.CreateEntity("score");
    em.AddComponent(scoreEntity, scoreComp);

    // Setup Camera
    ECSEngine::EntityID camID = em.CreateEntity("camera");
    ECSEngine::CameraComponent camComp{};
    //fixedy is middle of screen. 
    float fixedY = 896.0f / 2;
    ECSEngine::Rect window = engine.GetWindowManager().GetWindowBounds();
    camComp.position = ECSEngine::Point2D(spawn.x +1000,fixedY);
    //14 rows
    camComp.worldUnitsPerPixel = (14.0f*64.0f)/768.0f;
    camComp.hardExclusion = ECSEngine::Rect(ECSEngine::Point2D(0.1* window.width, 0), 0.8 * window.width,window.height);
    camComp.softExclusion = ECSEngine::Rect(ECSEngine::Point2D(0.3* window.width, 0), 0.4 * window.width,window.height);
    //speed of camera movement
    camComp.speed = 1.0f;
    em.AddComponent(camID, camComp);
    //camera follow component
    ECSEngine::CameraFollower camFollComp{};
    camFollComp.entityID = id;
    em.AddComponent(camID, camFollComp);
    //camera shake component
    ECSEngine::CameraShake camShkeComp{};
    camShkeComp.frameLimit = 0;
    camShkeComp.offset = ECSEngine::Point2D(0,19);
    em.AddComponent(camID, camShkeComp);



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
