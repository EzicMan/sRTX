//                          _
//  _._ _..._ .-',     _.._(`))
// '-. `     '  /-._.-'    ',/
//    )         \            '.
//   / _    _    |             \
//  |  a    a    /              |
//  \   .-.                     ;
//   '-('' ).-'       ,'       ;
//      '-;           |      .'
//         \           \    /
//         | 7  .__  _.-\   \
//         | |  |  ``/  /`  /
//        /,_|  |   /,_/   /
//           /,_/      '`-'
//
// Safety Pig Igor warns you: this code is unreadable and full of kostils.
// Listen to Safety Pig Igor and close the source code file right now.
// If you really want to continue, please consider taking Igor with yourself in case
// you need immediate medical help.
//

#pragma once
#include <vector>
#include "Vector3.hpp"
#include <memory>
#include "Object.hpp"

class Object;

constexpr double SIZE_OF_NO_RETURN = 0.5;

class kdTree{
    std::unique_ptr<kdTree> sides[2];
    Vector3 n;
    double dist;
    Vector3 bbox[2];
    std::vector<Object*> container;
public:
    kdTree(Vector3 lowerPoint, Vector3 upperPoint, Vector3 n, const std::vector<Object*>& objects){
        this->bbox[0] = lowerPoint;
        this->bbox[1] = upperPoint;
        this->n = n;
        for(Object* obj : objects){
            bool no = false;
            for (int i = 0; i < 3; i++) {
                if(bbox[1][i] < obj->bbox[0][i] || bbox[0][i] > obj->bbox[1][i]){
                    no = true;
                }
            }
            if(no) continue;
            container.push_back(obj);
        }
        double debug = (upperPoint - lowerPoint).length();
        if((upperPoint - lowerPoint).length() < SIZE_OF_NO_RETURN) {
            sides[0] = nullptr;
            sides[1] = nullptr;
            return;
        }

        dist = (lowerPoint + n * ((upperPoint - lowerPoint) * n * 0.5)) * n;

        sides[0] = std::make_unique<kdTree>(lowerPoint,
                                             Vector3(n[0] == 1 ? dist : upperPoint[0],
                                                     n[1] == 1 ? dist : upperPoint[1],
                                                     n[2] == 1 ? dist : upperPoint[2]),
                                             Vector3(n[2],n[0],n[1]),
                                             container);
        sides[1] = std::make_unique<kdTree>(Vector3(n[0] == 1 ? dist : lowerPoint[0],
                                                     n[1] == 1 ? dist : lowerPoint[1],
                                                     n[2] == 1 ? dist : lowerPoint[2]),
                                             upperPoint,
                                             Vector3(n[2],n[0],n[1]),
                                             container);
    }

    void getObj(Vector3 ray, std::set<Object *>& set){
        if(sides[0] == nullptr || sides[1] == nullptr){
            for (auto k : container) {
                set.insert(k);
            }
            return;
        }
        if(ray*n == 0 && dist >= 0) {
            sides[0]->getObj(ray,set);
            return;
        }else if(ray*n == 0 && dist < 0){
            sides[1]->getObj(ray, set);
            return;
        }
        Vector3 a = ray * (dist / (ray*n));
        bool no1 = false;
        bool no2 = false;
        for(int i = 0; i < 3; i++){
            if(a[i] < sides[0]->bbox[0][i] || a[i] > sides[0]->bbox[1][i]){
                no1 = true;
            }
            if(a[i] < sides[1]->bbox[0][i] || a[i] > sides[1]->bbox[1][i]){
                no2 = true;
            }
        }
        if (no1 && no2) {
            return;
        }
        if(no1){
            sides[1]->getObj(ray, set);
            return;
        }
        if(no2){
            sides[0]->getObj(ray, set);
            return;
        }
        sides[0]->getObj(ray,set);
        sides[1]->getObj(ray,set);
    }
};
