#include <SFML/Graphics.hpp>
#include <math.h>
#include <list>
#include <iostream>

using namespace std;
using namespace sf;

const int windowHeight = 1000;
const int windowWidth = 600;



class Entity {
public:
    static int id_generator;
    float x, y, angle, speed, R;
    int id;
    Sprite sprite;

    Entity() {
        id  = id_generator++;
    }

    void settings(Sprite &s, int X, int Y, float a=0) {
        sprite = s;
        sprite.setOrigin(R,R);
        x = X;
        y = Y;
        angle = a;
        R = 1;

    }

    virtual void update() {};

    void draw(RenderWindow &window) {
        CircleShape circle(R);
        circle.setFillColor(Color(255,0,0,170));
        circle.setPosition(x,y);
        circle.setOrigin(R,R);
        window.draw(sprite);
    }

    virtual ~Entity() {};
};

class ParkingSpot: public Entity {
public:
    bool free;
    ParkingSpot(int X, int Y) {
        x = X;
        y = Y;
        free = true;
    }

    void changeFree(){
        if(this->free){
            this->free = false;
        } else {
            this->free = true;
        }
    }
};

class Parking: public Entity {
public:
    int spotsCounter = 2;
    std::vector<ParkingSpot*> spots;
    Parking() {
        y = 110;
        for(int i = 0; i < 10; i++) {
            spots.push_back(new ParkingSpot(100, y+=60));
        }
    }
};

class Car: public Entity {
public:
    int n;
    bool parked = 0;
    Parking * parking;
    Car(Parking * parking) {
        speed=5;
        angle=0;
        n=0;
        this->parking = parking;
    }

    ParkingSpot* findFreeSpot() {
        for(int i = 0; i < parking->spots.size(); i++){
            ParkingSpot * tmp = parking->spots[i];
            if(tmp->free == true) {
                ParkingSpot * parkingSpot = tmp;
                return parkingSpot;
            }
        }
        return new ParkingSpot(500, -200);
    }

    void park() {
        ParkingSpot * parkingSpot = findFreeSpot();
        float tx=parkingSpot->x;
        float ty=parkingSpot->y;

        float beta = angle-atan2(tx-x,-ty+y);
        if(y < ty+200) {
            if (sin(beta)<0) angle+=0.005*speed;
            else angle-=0.005*speed;
            if ((x-tx)*(x-tx)+(y-ty)*(y-ty)<40*30) {
                n=(n+1)%parking->spotsCounter;
                parked = 1;
                parkingSpot->free = 0;
            }
        }
    }

    void unPark(){
        speed = -4;
    }

    void move() {

        x += sin(angle) * speed;
        y -= cos(angle) * speed;
    }

    void update() {
            if(!parked){
                park();
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

bool isColide(Entity* car, ParkingSpot* parkingSpot){
    float carx = car->x;
    float cary = car->y;
    float px = parkingSpot->x - 20;
    float py = parkingSpot->y;
    //cout << "parkingSpot x: " << px << " y: " << py << endl;
    //cout << "car x: " << carx << " y: " << cary << endl;
    if(((carx>px)&&(carx<(px+100)))&&((cary>py)&&(cary<(py+60)))){
        //cout << "true" << endl;
        return true;
    }
    //cout << "false" << endl;
    return false;
}

int main() {
    Texture t1, t2, t3;
    t1.loadFromFile("resources/background.png");
    t2.loadFromFile("resources/car.png");
    t3.loadFromFile("resources/parking.png");
    Sprite background(t1);
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


    float speed=0,angle=0;
    float maxSpeed=3.0;
    float acc=0.4, dec=0.3;
    float turnSpeed=0.05;
    while(window.isOpen()) {


        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed)
                if (event.key.code == Keyboard::Space)
                {
                    Entity *sCar = new Car(parking);
                    entities.push_back(sCar);
                    sCar->settings(carSprite, 500, 800);
                }
        }
        if(Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
            break;
        }

        /*
            bool Up=0,Right=0,Down=0,Left=0;
            if (Keyboard::isKeyPressed(Keyboard::Up)) Up=1;
            if (Keyboard::isKeyPressed(Keyboard::Right)) Right=1;
            if (Keyboard::isKeyPressed(Keyboard::Down)) Down=1;
            if (Keyboard::isKeyPressed(Keyboard::Left)) Left=1;

            //car movement
            if (Up && speed<maxSpeed)
                if (speed < 0)  speed += dec;
                else  speed += acc;

            if (Down && speed>-maxSpeed)
                if (speed > 0) speed -= dec;
                else  speed -= acc;

            if (!Up && !Down)
                if (speed - dec > 0) speed -= dec;
                else if (speed + dec < 0) speed += dec;
                else speed = 0;

            if (Right && speed!=0)  angle += turnSpeed * speed/maxSpeed;
            if (Left && speed!=0)   angle -= turnSpeed * speed/maxSpeed;

            car->speed = speed;
            car->angle = angle;
        */
        for(auto a:entities){
            for(ParkingSpot* ps:parking->spots){
                if(isColide(a, ps)){
                  //  ps->free = false;

                }
            }

        }

        for(auto i:entities){
            i->update();
            i->draw(window);
        }


        window.display();
    }

}
