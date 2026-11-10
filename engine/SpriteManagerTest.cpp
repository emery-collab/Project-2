// SpriteManager tests.
// Run from a desktop session (loading a texture needs a graphics context).
//
// How to read the output:
//   Each test prints its NUMBER + NAME, then WHY it exists (the spec sentence it checks),
//   then one line per check:  [pass] / [FAIL]  what was checked  -> expected vs. got
#include <iostream>
#include <string>

#include "SpriteManager.h"

using ECSEngine::Point2D;
using ECSEngine::Rect;
using ECSEngine::SpriteID;
using ECSEngine::SpriteManager;

// ======================= test helpers (you can skip reading these) =======================

static int gFailures = 0;

void StartTest(const std::string& name, const std::string& why) {
    std::cout << "\n" << name << "\n";
    std::cout << "  why: " << why << "\n";
}

// For numbers: prints what we expected and what we actually got.
void ExpectEqual(const std::string& what, long long got, long long expected) {
    if (got == expected) {
        std::cout << "  [pass] " << what << "  -> " << got << "\n";
    } else {
        std::cout << "  [FAIL] " << what << "  -> expected " << expected << ", got " << got << "\n";
        ++gFailures;
    }
}

// For yes/no questions (like "are these the same object?").
void ExpectTrue(const std::string& what, bool ok) {
    if (ok) {
        std::cout << "  [pass] " << what << "\n";
    } else {
        std::cout << "  [FAIL] " << what << "\n";
        ++gFailures;
    }
}

// ======================= test data =======================

// Both sheets are 1024 x 1024 pixel images.
const std::string kCharacterSheet = "assets/spritesheet-characters-default.png";
const std::string kEnemySheet = "assets/spritesheet-enemies-default.png";

//   (0,0)     (128,0)
//     +---------+---------+----
//     | LEFT    | RIGHT   |
//     | square  | square  |   ...rest of the 1024x1024 sheet
//     +---------+---------+----
const Rect kLeftSquare{Point2D(0.0f, 0.0f), 128.0f, 128.0f};
const Rect kRightSquare{Point2D(128.0f, 0.0f), 128.0f, 128.0f};

// ======================= tests =======================

void Test1_IdsCountUpFromZero() {
    StartTest("1. IDs count up from zero",
              "spec: \"The sprite id is just the index into the vector\"");

    SpriteManager sm;
    SpriteID first = sm.RegisterTexture(kCharacterSheet, kLeftSquare);
    SpriteID second = sm.RegisterTexture(kCharacterSheet, kRightSquare);
    SpriteID third = sm.RegisterTexture(kEnemySheet, kLeftSquare);

    ExpectEqual("1st registered sprite's id", first, 0);
    ExpectEqual("2nd registered sprite's id", second, 1);
    ExpectEqual("3rd registered sprite's id", third, 2);
}

void Test2_TextureIsLoaded() {
    StartTest("2. The image file actually gets loaded",
              "spec: \"loads ... a texture\" -- an empty texture would be 0 x 0");

    SpriteManager sm;
    SpriteID id = sm.RegisterTexture(kCharacterSheet, kLeftSquare);
    const sf::Texture& texture = sm.GetSprite(id).getTexture();

    ExpectEqual("texture width (sheet is 1024)", texture.getSize().x, 1024);
    ExpectEqual("texture height (sheet is 1024)", texture.getSize().y, 1024);
}

void Test3_SpriteShowsOnlyItsRect() {
    StartTest("3. The sprite shows only the rect you asked for, not the whole sheet",
              "spec: \"loads the SOURCE PORTION of a texture into a sprite\"");

    SpriteManager sm;
    SpriteID id = sm.RegisterTexture(kCharacterSheet, kRightSquare);  // asked for (128,0), 128x128
    sf::IntRect shown = sm.GetSprite(id).getTextureRect();          // what the sprite will draw

    ExpectEqual("left edge of what the sprite shows", shown.position.x, 128);
    ExpectEqual("top edge of what the sprite shows", shown.position.y, 0);
    ExpectEqual("width of what the sprite shows", shown.size.x, 128);
    ExpectEqual("height of what the sprite shows", shown.size.y, 128);
}

void Test4_SameFileLoadedOnce() {
    StartTest("4. Registering the same file twice reuses ONE texture",
              "textures are heavy; two sprites from one sheet should share it");

    SpriteManager sm;
    SpriteID a = sm.RegisterTexture(kCharacterSheet, kLeftSquare);
    SpriteID b = sm.RegisterTexture(kCharacterSheet, kRightSquare);

    const sf::Texture* textureOfA = &sm.GetSprite(a).getTexture();
    const sf::Texture* textureOfB = &sm.GetSprite(b).getTexture();
    ExpectTrue("both sprites point at the same texture object", textureOfA == textureOfB);
}

void Test5_DifferentFilesDifferentTextures() {
    StartTest("5. Two different files give two different textures",
              "the map key is the file path, so each path gets its own texture");

    SpriteManager sm;
    SpriteID hero = sm.RegisterTexture(kCharacterSheet, kLeftSquare);
    SpriteID enemy = sm.RegisterTexture(kEnemySheet, kLeftSquare);

    const sf::Texture* heroTexture = &sm.GetSprite(hero).getTexture();
    const sf::Texture* enemyTexture = &sm.GetSprite(enemy).getTexture();
    ExpectTrue("the two sprites point at different texture objects", heroTexture != enemyTexture);
}

void Test6_SameFileTwoRectsAreTwoSprites() {
    StartTest("6. Same file + two different rects = two separate sprites",
              "each RegisterTexture call makes a new sprite, even if the file repeats");

    SpriteManager sm;
    SpriteID left = sm.RegisterTexture(kCharacterSheet, kLeftSquare);
    SpriteID right = sm.RegisterTexture(kCharacterSheet, kRightSquare);

    ExpectTrue("they are different sprite objects", &sm.GetSprite(left) != &sm.GetSprite(right));
    ExpectEqual("left sprite's left edge", sm.GetSprite(left).getTextureRect().position.x, 0);
    ExpectEqual("right sprite's left edge", sm.GetSprite(right).getTextureRect().position.x, 128);
}

void Test7_OldSpritesSurviveNewRegistrations() {
    StartTest("7. The first sprite still works after 100 more registrations",
              "spec: \"unordered_map ... ensures stable memory references\" -- "
              "the sprite vector grows and moves, but textures must stay put");

    SpriteManager sm;
    SpriteID first = sm.RegisterTexture(kCharacterSheet, kLeftSquare);
    const sf::Texture* textureBefore = &sm.GetSprite(first).getTexture();

    for (int i = 0; i < 100; ++i) {
        (void)sm.RegisterTexture(i % 2 == 0 ? kCharacterSheet : kEnemySheet, kRightSquare);
    }

    const sf::Sprite& firstAfter = sm.GetSprite(first);
    ExpectTrue("first sprite still points at the same texture object",
               &firstAfter.getTexture() == textureBefore);
    ExpectEqual("that texture still holds the image (width)", firstAfter.getTexture().getSize().x, 1024);
    ExpectEqual("first sprite's left edge is unchanged", firstAfter.getTextureRect().position.x, 0);
}

int main() {
    Test1_IdsCountUpFromZero();
    Test2_TextureIsLoaded();
    Test3_SpriteShowsOnlyItsRect();
    Test4_SameFileLoadedOnce();
    Test5_DifferentFilesDifferentTextures();
    Test6_SameFileTwoRectsAreTwoSprites();
    Test7_OldSpritesSurviveNewRegistrations();

    std::cout << "\n" << (gFailures == 0 ? "ALL TESTS PASSED" : std::to_string(gFailures) + " CHECK(S) FAILED")
              << "\n";
    return gFailures == 0 ? 0 : 1;
}
