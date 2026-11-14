#include "EntityManager.h"
struct P { int x = 0; };
struct H { int hp = 0; };
//int argc, char* argv[]e
int main() {     
    // ECSEngine::EntityManager<P, H> em;
    // auto a = em.CreateEntity("a");
    // auto b = em.CreateEntity("b");
    // std::cout << a << " " << b << "\n";
    // em.RemoveEntity(a);
    // auto c = em.CreateEntity("c");
    // std::cout << "c valid? " << em.validEntity(c) << "\n";
    // std::cout << c << "\n";
    // em.AddComponent(b, H{50});

    // std::cout << em.HasComponent<H>(b) <<" and b:" << em.HasComponent<P>(b) << ". 1 \n";   // line 1
    // std::cout << em.GetComponent<H>(b).hp << ". 2\n";                         // line 2

    // em.RemoveComponent<H>(b);
    // std::cout << em.HasComponent<H>(b) << ". 3\n";                            // line 3

    // auto& hs = em.GetComponentStorage<H>();
    // std::cout << hs.mStorage.size() << " " << hs.mFreeList.size() << ". 4 \n"; // line 4

    // em.AddComponent(c, H{70});
    // std::cout << hs.mStorage.size() << " " << hs.mFreeList.size() << ". 5\n"; // line 5

    // em.RemoveEntity(c);
    // std::cout << hs.mStorage.size() << " " << hs.mFreeList.size() << ". 6\n"; // line 6
    // for (const auto& e : em) {
    // std::cout << e.GetID() << " valid=" << em.validEntity(e.GetID()) << "\n";
    // }
    return 0;
}

