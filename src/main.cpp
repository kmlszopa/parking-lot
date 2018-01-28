#include <SFML/Graphics.hpp>
#include <math.h>
#include <list>
#include <iostream>
#include <string>
#include <sstream>
using namespace std;
using namespace sf;

const int windowHeight = 1000;
const int windowWidth = 1040;

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

    void settings(Sprite &s, float X, float Y, float a=0,int radius=1) {
        sprite = s;
        R = radius;
        sprite.setOrigin(R,R);
        x = X;
        y = Y;
        angle = a;


    }

    virtual void update() {};

    void draw(RenderWindow &window) {
        window.draw(sprite);
    }

    void setSprite(Sprite s) {
        s.setPosition(x,y);
        this->sprite = s;
    }

    virtual ~Entity() {};
};

class ParkingSpot: public Entity {
public:
    bool free;
    bool isTop;
    ParkingSpot() {
        free = true;

    }

    ParkingSpot(float X, float Y, float angle = 90, bool isTop = false) {
        x = X;
        y = Y;
        free = true;
        this->angle = angle;
        this->isTop = isTop;
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
    Target exit = {250, 1400000};
    int spotsCounterSide = 9;
    int spotsCounterTop  = 5;
    std::vector<ParkingSpot*> spots;
    int carsLeftCounter = 0;
    Parking() {
        y = 696;
        //draw left spots
        for(int i = 0; i < spotsCounterSide; i++) {

            spots.push_back(new ParkingSpot(100, y-=58, 0));
        }
        //draw top spots
        y = 0;
        x = 300;
        for(int i = 0; i < spotsCounterTop; i++) {

            spots.push_back(new ParkingSpot(x+=60, y, 90, true));
        }
        //draw right spots
        y = 660;
        for(int i = 0; i < spotsCounterSide; i++) {

            spots.push_back(new ParkingSpot(x+330, y-=58, 180));
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
        parkingTime = ((rand() % 40) + 10);
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
        return  NULL;
    }

    void park() {
        if(this->parkingSpot == NULL) {
            speed = 0;
            this->parkingSpot = findFreeSpot();
        } else {
            float tx=parkingSpot->x;
            float ty=parkingSpot->y;

            float beta = angle-atan2(tx-x,-ty+y);
            if(!this->parkingSpot->isTop) {
                if(y < ty+127) {
                    if (sin(beta)<0) angle+=turnSpeed*speed;
                    else angle-=turnSpeed*speed;
                    if ((x-tx)*(x-tx)+(y-ty)*(y-ty)<40) {
                        n=(n+1)%parking->spotsCounterSide;
                        parked = 1;
                        parking->carsLeftCounter++;
                        cout << parkingSpot->free << endl;
                        target.x = parking->exit.x+172;
                        target.y = ty;
                        clock.restart();
                    }
                }
            } else {
                if (sin(beta)<0)
                    angle+=turnSpeed*speed;
                else angle-=turnSpeed*speed;
                if ((x-tx)*(x-tx)+(y-ty)*(y-ty)<40) {
                    n=(n+1)%parking->spotsCounterSide;
                    parked = 1;
                    parking->carsLeftCounter++;
                    cout << parkingSpot->free << endl;
                    target.x = parking->exit.x+172;
                    target.y = ty;
                    clock.restart();
                }
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

        if(!left) {
            if(y > windowHeight) {
                this->parkingSpot->free = 1;
                left = true;

            }
        }

    }

    void unPark() {
        Time time = clock.getElapsedTime();
        speed = 0;
        if(time.asSeconds() > parkingTime) {
            speed = -3.5;
            if(time.asSeconds() > parkingTime+0.5) {
                if(this->parkingSpot->isTop) {
                    angle+=0.01*speed;
                }
            }
        }
        if(time.asSeconds() > parkingTime+1.4) {
            speed = 3.5;
            this->toExit = 1;
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

class DigitalClock {
public:
    Clock timer;
    float x, y;
    Text time;
    Text carCounter;
    Text parkingCapacity;
    Font font;
    DigitalClock( float x, float y) {
        this->x = x;
        this->y = y;
        timer.restart();
        font.loadFromFile("resources/arial.ttf");
        time.setFont(font);
        time.setCharacterSize(30);
        time.setColor(sf::Color::Black);
        time.setPosition(x,y);
        carCounter.setFont(font);
        carCounter.setCharacterSize(30);
        carCounter.setColor(sf::Color::Black);
        carCounter.setPosition(x,y+100);
        parkingCapacity.setFont(font);
        parkingCapacity.setCharacterSize(30);
        parkingCapacity.setColor(sf::Color::Black);
        parkingCapacity.setPosition(x,y+200);

    }

    void draw(RenderWindow &window) {

        window.draw(carCounter);
        window.draw(time);
        window.draw(parkingCapacity);
    }

    void updateInformations(float carCount) {
        ostringstream ss1;
        ostringstream ss2;
        ostringstream ss3;
        ss1 << "Czas w minutach: " << timer.getElapsedTime().asSeconds();
        time.setString(ss1.str());
        ss2 << "Ilosc samochodow: " << carCount;
        carCounter.setString(ss2.str());
        ss3 << "Przepustowosc parkingu: " << (carCount / timer.getElapsedTime().asSeconds()) * 60 << "/h";
        parkingCapacity.setString(ss3.str());
    }

};


int Entity::id_generator = 0;

int main() {
    int carCounter;

    Texture t1, t2, t3;
    t1.loadFromFile("resources/parking-spot.png");
    t2.loadFromFile("resources/car.png");
    t3.loadFromFile("resources/clear-parking.png");
    t2.setSmooth(true);


    Sprite spotSprite(t1);
    Sprite carSprite(t2);
    Sprite parkingSprite(t3);

    DigitalClock * digitalClock = new DigitalClock(windowHeight/4,windowWidth/7);

    RenderWindow window(VideoMode(windowWidth, windowHeight), "Parking");
    window.setFramerateLimit(60);

    std::list<Entity*> entities;
    Parking *parking = new Parking();
    entities.push_back(parking);

    Car *car = new Car(parking);
    entities.push_back(car);

    for(auto i: parking->spots) {
        i->setSprite(spotSprite);
        i->sprite.setRotation(i->angle);
        i->sprite.setOrigin(0,50);
    }

    parking->settings(parkingSprite, 0, 0);
    car->settings(carSprite, 400, 1200);



    while(window.isOpen()) {


        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();


           if (event.type == Event::KeyPressed)
                if (event.key.code == Keyboard::Space) {
                    Entity *sCar = new Car(parking);
                    entities.push_back(sCar);
                    carSprite.setColor(sf::Color(rand() % 255, rand() % 255, rand() % 255));
                    sCar->settings(carSprite, 500, 1200);

            }
        }


        if(Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
            break;
        }

        entities.reverse();
        for(auto a:entities)
            for(auto b:entities) {

                if(a->id != b->id)

                    if ( a->sprite.getGlobalBounds().intersects(b->sprite.getGlobalBounds()) ) {
//            cout << "a:asdas " << a->id << endl;
//            cout << "b: asdas" << b->id << endl;
                        b->speed = 0;

                    }
                a->speed = 3.5;
            }
        entities.reverse();

        for(ParkingSpot * i:parking->spots) {
            i->R = 20;
            i->update();
            i->draw(window);

        }

        digitalClock->updateInformations(parking->carsLeftCounter);
        digitalClock->draw(window);

        for(auto i:entities) {
            i->update();
            i->draw(window);

        }



        window.display();
        window.clear(sf::Color::White);
    }

}
