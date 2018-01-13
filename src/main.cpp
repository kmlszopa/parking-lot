#include <SFML/Graphics.hpp>
#include <math.h>
#include <list>
#include <iostream>

using namespace std;
using namespace sf;

const int windowHeight = 1000;
const int windowWidth = 600;

struct Target {
    float x;
    float y;
};

class Entity {
public:
    static int id_generator;
    float x, y, angle, speed, R;
    int id;
    Sprite sprite;

    Entity() {
        id  = id_generator++;
    }

    void settings(Sprite &s, float X, float Y, float a=0) {
        sprite = s;
        sprite.setOrigin(R,R);
        x = X;
        y = Y;
        angle = a;


    }

    virtual void update() {};

    void draw(RenderWindow &window) {
        window.draw(sprite);
    }

    virtual ~Entity() {};
};

class ParkingSpot: public Entity {
public:
    bool free;
    ParkingSpot(float X, float Y) {
        x = X;
        y = Y;
        free = true;
    }

    void changeFree() {
        if(this->free) {
            this->free = false;
        } else {
            this->free = true;
        }
    }
};

class Parking: public Entity {
public:
    Target exit = {250, 1200};
    int spotsCounter = 10;
    std::vector<ParkingSpot*> spots;
    Parking() {
        y = 696;
        for(int i = 0; i < spotsCounter; i++) {
            spots.push_back(new ParkingSpot(100, y-=58));
        }
    }
};

class Car: public Entity {
public:
    Clock clock;
    float parkingTime;
    ParkingSpot * parkingSpot;
    int n;
    float turnSpeed;
    bool parked = 0, toExit = 0, left = 0;
    Target target;
    Parking * parking;

    Car(Parking * parking) {
        srand(time(NULL));
        parkingTime = ((rand() % 10) + 5);
        turnSpeed = 0.0075;
        speed=3.5;
        angle=0;
        n=0;
        this->parking = parking;
        this->parkingSpot = findFreeSpot();

    }

    ParkingSpot* findFreeSpot() {
        for(int i = 0; i < parking->spots.size(); i++) {
            ParkingSpot * tmp = parking->spots[i];
            if(tmp->free == true) {
                tmp->free = 0;
                ParkingSpot * parkingSpot = tmp;
                return parkingSpot;
            }
        }
        return new ParkingSpot(500, -10000000);
    }

    void park() {
        float tx=parkingSpot->x;
        float ty=parkingSpot->y;

        float beta = angle-atan2(tx-x,-ty+y);
        if(y < ty+127) {
            if (sin(beta)<0) angle+=turnSpeed*speed;
            else angle-=turnSpeed*speed;
            if ((x-tx)*(x-tx)+(y-ty)*(y-ty)<40) {
                n=(n+1)%parking->spotsCounter;
                parked = 1;
                cout << parkingSpot->free << endl;
                target.x = parking->exit.x+172;
                target.y = ty;
                clock.restart();
            }
        }
    }

    void moveToExit() {
        float tx=parking->exit.x;
        float ty=parking->exit.y;
        float beta = angle-atan2(tx-x,-ty+y);
        if(y < ty+124) {
            if (sin(beta)<0) angle+=turnSpeed*speed;
            else angle-=turnSpeed*speed;
        }

        if(!left){
            this->parkingSpot->free = 1;
            left = true;
        }

    }

    void unPark() {
        Time time = clock.getElapsedTime();
        speed = 0;
        if(time.asSeconds() > parkingTime) {
            speed = -3.5;
        }
        if(target.x-this->x <  30) {
            this->toExit = 1;
            speed = 3.5;
        }
    }

    void move() {
        x += sin(angle) * speed;
        y -= cos(angle) * speed;
    }

    void update() {
        if(!parked) {
            park();
            move();
        } else if(toExit) {
            moveToExit();
            move();
        } else {
            unPark();
            move();
        }


        sprite.setPosition(x, y);
        sprite.setRotation(angle*180/3.141593);
    }

};

int Entity::id_generator = 0;

int main() {
    Texture  t2, t3;

    t2.loadFromFile("resources/car.png");
    t3.loadFromFile("resources/parking.png");
    t2.setSmooth(true);

    Sprite carSprite(t2);
    Sprite parkingSprite(t3);
    bool Up=0,Right=0,Down=0,Left=0;

    RenderWindow window(VideoMode(windowWidth, windowHeight), "Parking");
    window.setFramerateLimit(60);
    std::list<Entity*> entities;

    Parking *parking = new Parking();
    entities.push_back(parking);

    Car *car = new Car(parking);
    entities.push_back(car);
    parking->settings(parkingSprite, 0, 0);
    car->settings(carSprite, 500, 800);



    while(window.isOpen()) {


        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed)
                if (event.key.code == Keyboard::Space) {
                    Entity *sCar = new Car(parking);
                    entities.push_back(sCar);
                    sCar->settings(carSprite, 500, 800);
                }
        }
        if(Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
            break;
        }


        for(auto i:entities) {
            i->update();
            i->draw(window);
        }


        window.display();
    }

}
