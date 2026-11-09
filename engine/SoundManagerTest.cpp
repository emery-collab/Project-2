#include <iostream>
#include <string>

#include "SoundManager.h"

static int gFailures = 0;

// Like assert, but prints the result and keeps going instead of stopping.
#define CHECK(cond)                                                          \
    do {                                                                     \
        if (cond) {                                                          \
            std::cout << "  pass: " #cond "\n";                              \
        } else {                                                             \
            std::cout << "  FAIL: " #cond " (line " << __LINE__ << ")\n";    \
            ++gFailures;                                                     \
        }                                                                    \
    } while (false)

const std::string kAssets = "assets/";

void TestRegisterThenGet() {
    std::cout << "RegisterThenGet\n";
    ECSEngine::SoundManager sm;
    sm.RegisterSound(kAssets + "sfx_jump.ogg", "jump");
    CHECK(sm.GetSound("jump").getBuffer().getSampleCount() > 0);  // real audio was loaded
}

void TestSameNameGivesSameSound() {
    std::cout << "SameNameGivesSameSound\n";
    ECSEngine::SoundManager sm;
    sm.RegisterSound(kAssets + "sfx_jump.ogg", "jump");
    CHECK(&sm.GetSound("jump") == &sm.GetSound("jump"));
}

void TestDifferentNamesGiveDifferentSounds() {
    std::cout << "DifferentNamesGiveDifferentSounds\n";
    ECSEngine::SoundManager sm;
    sm.RegisterSound(kAssets + "sfx_jump.ogg", "jump");
    sm.RegisterSound(kAssets + "sfx_gem.ogg", "gem");
    CHECK(&sm.GetSound("jump") != &sm.GetSound("gem"));
    CHECK(&sm.GetSound("jump").getBuffer() != &sm.GetSound("gem").getBuffer());
}

void TestSoundStaysPutAfterMoreRegistrations() {
    std::cout << "SoundStaysPutAfterMoreRegistrations\n";
    ECSEngine::SoundManager sm;
    sm.RegisterSound(kAssets + "sfx_jump.ogg", "jump");
    sf::Sound* before = &sm.GetSound("jump");
    sm.RegisterSound(kAssets + "sfx_gem.ogg", "gem");
    sm.RegisterSound(kAssets + "error_008.ogg", "wall");
    sm.RegisterSound(kAssets + "laserLarge_000.ogg", "laser");
    sm.RegisterSound(kAssets + "footstep_grass_003.ogg", "land");
    CHECK(&sm.GetSound("jump") == before);  // other code may still hold this reference
}

int main() {
    TestRegisterThenGet();
    TestSameNameGivesSameSound();
    TestDifferentNamesGiveDifferentSounds();
    TestSoundStaysPutAfterMoreRegistrations();

    std::cout << (gFailures == 0 ? "All tests passed\n" : "Some tests FAILED\n");
    return gFailures == 0 ? 0 : 1;
}