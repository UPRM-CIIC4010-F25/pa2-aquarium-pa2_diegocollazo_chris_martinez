#define NOMINMAX // To avoid min/max macro conflict on Windows

#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include "Core.h"


enum class AquariumCreatureType {
    NPCreature,
    BiggerFish,
    BonnieFish,
    RandomFish
};

string AquariumCreatureTypeToString(AquariumCreatureType t);

class AquariumLevelPopulationNode{
    public:
        AquariumLevelPopulationNode() = default;
        AquariumLevelPopulationNode(AquariumCreatureType creature_type, int population) {
            this->creatureType = creature_type;
            this->population = population;
            this->currentPopulation = 0;
        };
        AquariumCreatureType creatureType;
        int population;
        int currentPopulation;
};

class AquariumLevel : public GameLevel {
    public:
        AquariumLevel(int levelNumber, int targetScore)
        : GameLevel(levelNumber), m_level_score(0), m_targetScore(targetScore){};
        void ConsumePopulation(AquariumCreatureType creature, int power);
        bool isCompleted() override;
        void populationReset();
        void levelReset(){m_level_score=0;this->populationReset();}
        virtual std::vector<AquariumCreatureType> Repopulate() = 0;
    protected:
        std::vector<std::shared_ptr<AquariumLevelPopulationNode>> m_levelPopulation;
        int m_level_score;
        int m_targetScore;

};


enum class PowerUpType {
    SpeedBoost,
    PowerBoost,
    Dash,
    GlowUp
};
class PlayerCreature : public Creature {
public:

    PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move();
    void draw() const;
    void update();
    void changeSpeed(int speed);
    void setLives(int lives) { m_lives = lives; }
    void setDirection(float dx, float dy);
    void applyPowerUp(PowerUpType t);
    void dash();
    float isXDirectionActive() { return m_dx != 0; }
    float isYDirectionActive() {return m_dy != 0; }
    float getDx() { return m_dx; }
    float getDy() { return m_dy; }

    int getScore()const { return m_score; }
    int getLives() const { return m_lives; }
    int getPower() const { return m_power; }
    
    void addToScore(int amount, int weight=1) { m_score += amount * weight; }
    void loseLife(int debounce);
    void increasePower(int value) { m_power += value; }
    void reduceDamageDebounce();
    
private:
    int m_score = 0;
    int m_lives = 3;
    int m_power = 1; // mark current power lvl
    int m_damage_debounce = 0; // frames to wait after eating
    bool m_dashAvailable = false;
    int m_glowTimer = 0; 
    int m_powerUpTimer = 0;
    float m_speed = 5;
};

class PowerUp {
public:
    PowerUp(float x, float y, PowerUpType type, std::shared_ptr<GameSprite> sprite)
        : m_x(x), m_y(y), m_type(type), m_sprite(sprite) {}

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    PowerUpType getType() const { return m_type; }

    void draw() const {
        if (m_sprite) {
            m_sprite->draw(m_x, m_y);
        }
    }

private:
    float m_x, m_y;
    PowerUpType m_type;
    std::shared_ptr<GameSprite> m_sprite;
};

class NPCreature : public Creature {
public:
    NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    AquariumCreatureType GetType() {return this->m_creatureType;}
    void move() override;
    void draw() const override;
protected:
    AquariumCreatureType m_creatureType;


};

class BonnieFish : public NPCreature {
public:
    BonnieFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move() override;
private:
    int zigTimer = 0;
    int maxZigTimer = 50;
};

class RandomFish : public NPCreature {
public:
 RandomFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
        : NPCreature(x, y, speed, sprite) 
    {
        m_creatureType = AquariumCreatureType::RandomFish;
    }

   void move(){
    
        if(rand() % 50 == 0) {
            float angle = (rand() % 360) * 3.14159f / 180.0f;
            m_dx = cos(angle);
            m_dy = sin(angle);
        }

        m_x += m_dx * m_speed;
        m_y += m_dy * m_speed;

        if(m_x < 0) { m_x = 0; m_dx = -m_dx; }
        if(m_y < 0) { m_y = 0; m_dy = -m_dy; }
        if(m_x > m_aquariumWidth) { m_x = m_aquariumWidth; m_dx = -m_dx; }
        if(m_y > m_aquariumHeight) { m_y = m_aquariumHeight; m_dy = -m_dy; }

        if(m_dx < 0) m_sprite->setFlipped(true);
        else m_sprite->setFlipped(false);
    }

private:
 float m_x, m_y;
    float m_speed;
    int m_aquariumWidth = 800;  
    int m_aquariumHeight = 600; 
};

class BiggerFish : public NPCreature {
public:
    BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move() override;
    void draw() const override;
};


class AquariumSpriteManager {
    public:
        AquariumSpriteManager();
        ~AquariumSpriteManager() = default;
        std::shared_ptr<GameSprite>GetSprite(AquariumCreatureType t);
        std::shared_ptr<GameSprite> GetPowerUpSprite(PowerUpType t);
    private:
        std::shared_ptr<GameSprite> m_npc_fish;
        std::shared_ptr<GameSprite> m_big_fish;
        std::shared_ptr<GameSprite> m_Bonniefish;
        std::shared_ptr<GameSprite> m_Randomfish;
        std::shared_ptr<GameSprite> m_speedBoost;
        std::shared_ptr<GameSprite> m_powerBoost;
        std::shared_ptr<GameSprite> m_dash;
        std::shared_ptr<GameSprite> m_glowUp;
        
};


class Aquarium{
public:
    Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager);
    void addCreature(std::shared_ptr<Creature> creature);
    void addAquariumLevel(std::shared_ptr<AquariumLevel> level);
    void removeCreature(std::shared_ptr<Creature> creature);
    void clearCreatures();
    void update();
    void draw() const;
    void setBounds(int w, int h) { m_width = w; m_height = h; }
    void setMaxPopulation(int n) { m_maxPopulation = n; }
    void Repopulate();
    void SpawnCreature(AquariumCreatureType type);
    void addPowerUp(std::shared_ptr<PowerUp> p);
    void SpawnRandomPowerUp();
    int getPowerUpCount() const{ return m_powerups.size();}
    std::shared_ptr<PowerUp> getPowerUpAt(int index){
        if (index < 0 || index >= (int)m_powerups.size()) return nullptr;
        return m_powerups[index];
    }
    void removePowerUp(std::shared_ptr<PowerUp> p);
    
    std::shared_ptr<Creature> getCreatureAt(int index);
    int getCreatureCount() const { return m_creatures.size(); }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }


private:
    int m_maxPopulation = 0;
    int m_width;
    int m_height;
    int currentLevel = 0;
    std::vector<std::shared_ptr<Creature>> m_creatures;
    std::vector<std::shared_ptr<Creature>> m_next_creatures;
    std::vector<std::shared_ptr<PowerUp>> m_powerups;
    std::vector<std::shared_ptr<AquariumLevel>> m_aquariumlevels;
    std::shared_ptr<AquariumSpriteManager> m_sprite_manager;
};


std::shared_ptr<GameEvent> DetectAquariumCollisions(std::shared_ptr<Aquarium> aquarium, std::shared_ptr<PlayerCreature> player);
    std::shared_ptr<PowerUp> DetectPowerUpCollision(std::shared_ptr<Aquarium> aquarium,std::shared_ptr<PlayerCreature> player);


class AquariumGameScene : public GameScene {
    public:
        AquariumGameScene(std::shared_ptr<PlayerCreature> player, std::shared_ptr<Aquarium> aquarium, string name)
        : m_player(std::move(player)) , m_aquarium(std::move(aquarium)), m_name(name){}
        std::shared_ptr<GameEvent> GetLastEvent(){return m_lastEvent;}
        void SetLastEvent(std::shared_ptr<GameEvent> event){this->m_lastEvent = event;}
        std::shared_ptr<PlayerCreature> GetPlayer(){return this->m_player;}
        std::shared_ptr<Aquarium> GetAquarium(){return this->m_aquarium;}
        string GetName()override {return this->m_name;}
        void Update() override;
        void Draw() override;
    private:
        int m_powerUpTimer = 0;
        void paintAquariumHUD();
        std::shared_ptr<PlayerCreature> m_player;
        std::shared_ptr<Aquarium> m_aquarium;
        std::shared_ptr<GameEvent> m_lastEvent;
        string m_name;
        AwaitFrames updateControl{5};
};


class Level_0 : public AquariumLevel  {
    public:
        Level_0(int levelNumber, int targetScore): AquariumLevel(levelNumber, targetScore){
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::NPCreature, 10));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BiggerFish, 3));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BonnieFish, 4));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::RandomFish, 2));
        };
        std::vector<AquariumCreatureType> Repopulate() override;

};
class Level_1 : public AquariumLevel  {
    public:
        Level_1(int levelNumber, int targetScore): AquariumLevel(levelNumber, targetScore){
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::NPCreature, 20));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BiggerFish, 10));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BonnieFish, 7));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::RandomFish, 6));
        };
        std::vector<AquariumCreatureType> Repopulate() override;
        

};
class Level_2 : public AquariumLevel  {
    public:
        Level_2(int levelNumber, int targetScore): AquariumLevel(levelNumber, targetScore){
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::NPCreature, 30));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BiggerFish, 10));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BonnieFish, 7));
            this->m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::RandomFish, 14));

        };
        std::vector<AquariumCreatureType> Repopulate() override;

};

std::shared_ptr<PowerUp> DetectPowerUpCollision(
    std::shared_ptr<Aquarium> aquarium,
    std::shared_ptr<PlayerCreature> player);
bool checkCollision(std::shared_ptr<PlayerCreature> player, float x, float y, float radius);
