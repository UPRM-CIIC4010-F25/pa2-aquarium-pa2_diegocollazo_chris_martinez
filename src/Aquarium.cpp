#include "Aquarium.h"
#include <cstdlib>


std::string AquariumCreatureTypeToString(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return "BiggerFish";
        case AquariumCreatureType::NPCreature:
            return "BaseFish";
        case AquariumCreatureType::BonnieFish:
            return "BonnieFish";
        case AquariumCreatureType::RandomFish:
            return "RandomFish";
        default:
            return "UknownFish";
    }
}

// PlayerCreature Implementation
PlayerCreature::PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 10.0f, 1, sprite) {m_dx = 1.0f; m_dy = 0.0f;}


void PlayerCreature::setDirection(float dx, float dy) {
    m_dx = dx;
    m_dy = dy;
    normalize();
}

void PlayerCreature::move() {
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    this->bounce();
}

void PlayerCreature::applyPowerUp(PowerUpType t) {
    switch(t) {
        case PowerUpType::SpeedBoost:
            m_speed += 2;
            break;

        case PowerUpType::PowerBoost:
            m_power += 1;
            break;

        case PowerUpType::Dash:
            m_dashAvailable = true;
            break;

        case PowerUpType::GlowUp:
            m_glowTimer = 600; // ~10 sec
            break;
    }
}

void PlayerCreature::dash() {
    if (!m_dashAvailable) return;
    m_x += m_dx * (m_speed * 6);
    m_y += m_dy * (m_speed * 6);
    m_dashAvailable = false;
}


void PlayerCreature::reduceDamageDebounce() {
    if (m_damage_debounce > 0) {
        --m_damage_debounce;
    }
}

void PlayerCreature::update() {
    this->reduceDamageDebounce();
    this->move();
    if (m_glowTimer > 0) m_glowTimer--;
    if (m_powerUpTimer > 0) m_powerUpTimer--;
}


void PlayerCreature::draw() const {
    
    ofLogVerbose() << "PlayerCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    if (m_glowTimer > 0){
        ofSetColor(ofColor::yellow);
    }
    else if (this->m_damage_debounce > 0) {
        ofSetColor(ofColor::red); // Flash red if in damage debounce
    }
    else{ofSetColor(ofColor::white);}
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
    ofSetColor(ofColor::white); // Reset color

}

void PlayerCreature::changeSpeed(int speed) {
    m_speed = speed;
}

void PlayerCreature::loseLife(int debounce) {
    if (m_damage_debounce <= 0) {
        if (m_lives > 0) this->m_lives -= 1;
        m_damage_debounce = debounce; // Set debounce frames
        ofLogNotice() << "Player lost a life! Lives remaining: " << m_lives << std::endl;
    }
    // If in debounce period, do nothing
    if (m_damage_debounce > 0) {
        ofLogVerbose() << "Player is in damage debounce period. Frames left: " << m_damage_debounce << std::endl;
    }
}

// NPCreature Implementation
NPCreature::NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 30, 1, sprite) {
    m_dx = (rand() % 3 - 1); // -1, 0, or 1
    m_dy = (rand() % 3 - 1); // -1, 0, or 1
    normalize();

    m_creatureType = AquariumCreatureType::NPCreature;
}

void NPCreature::move() {
    // Simple AI movement logic (random direction)
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);

    }else {
        this->m_sprite->setFlipped(false);
    }
    const bool hitLeft = (m_x-m_collisionRadius < 0.0f);
    const bool hitRight = (m_x-m_collisionRadius > m_width);
    const bool hitTop = (m_y-m_collisionRadius < 0.0f);
    const bool hitBottom = (m_y-m_collisionRadius > m_height);
    const bool Bounced = hitLeft || hitRight || hitTop || hitBottom;
    
    bounce();

    if(Bounced){
        auto rand01 = []() -> float{return (static_cast<float>(rand()%200)-100.0f)/100.0f;};
        
        float rx = rand01();
        float ry = rand01();

        if(hitLeft) {rx = abs(rx);}
        if(hitRight) {rx = -abs(rx);}
        if(hitTop) {ry = abs(ry);}
        if(hitBottom) {ry = -abs(ry);}
        
        m_dx = rx;
        m_dy = ry;
        normalize();
    }
}

void NPCreature::draw() const {
    ofLogVerbose() << "NPCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    ofSetColor(ofColor::white);
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
}


BiggerFish::BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();

    setCollisionRadius(60); // Bigger fish have a larger collision radius
    m_value = 5; // Bigger fish have a higher value
    m_creatureType = AquariumCreatureType::BiggerFish;
}

void BiggerFish::move() {
    // Bigger fish might move slower or have different logic
    m_x += m_dx * (m_speed * 0.5); // Moves at half speed
    m_y += m_dy * (m_speed * 0.5);
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }

    bounce();
}

void BiggerFish::draw() const {
    ofLogVerbose() << "BiggerFish at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    this->m_sprite->draw(this->m_x, this->m_y);
}


// AquariumSpriteManager
AquariumSpriteManager::AquariumSpriteManager(){
    this->m_npc_fish = std::make_shared<GameSprite>("base-fish.png", 70,70);
    this->m_big_fish = std::make_shared<GameSprite>("bigger-fish.png", 120, 120);
    this->m_Bonniefish = std::make_shared<GameSprite>("Bonniefishie.png", 90, 90);
    this->m_Randomfish = std::make_shared<GameSprite>("Randomfishie.png", 90, 90);


    m_speedBoost = std::make_shared<GameSprite>("speed-boost.png", 50, 50);
    m_powerBoost = std::make_shared<GameSprite>("power-boost.png", 50, 50);
    m_dash       = std::make_shared<GameSprite>("dash.png", 50, 50);
    m_glowUp     = std::make_shared<GameSprite>("glow-up.png", 50, 50);
}

std::shared_ptr<GameSprite> AquariumSpriteManager::GetSprite(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return std::make_shared<GameSprite>(*this->m_big_fish);
            
        case AquariumCreatureType::NPCreature:
            return std::make_shared<GameSprite>(*this->m_npc_fish);

         case AquariumCreatureType::BonnieFish:
            return std::make_shared<GameSprite>(*this->m_Bonniefish);

        case AquariumCreatureType::RandomFish:
            return std::make_shared<GameSprite>(*this->m_Randomfish);
        default:
            return nullptr;
    }
}
std::shared_ptr<GameSprite> AquariumSpriteManager::GetPowerUpSprite(PowerUpType t){
    switch (t) {
        case PowerUpType::SpeedBoost: return m_speedBoost;
        case PowerUpType::PowerBoost: return m_powerBoost;
        case PowerUpType::Dash:       return m_dash;
        case PowerUpType::GlowUp:     return m_glowUp;
        default: return nullptr;
    }
}


// Aquarium Implementation
Aquarium::Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager)
    : m_width(width), m_height(height) {
        m_sprite_manager =  spriteManager;
    }



void Aquarium::addCreature(std::shared_ptr<Creature> creature) {
    creature->setBounds(m_width - 20, m_height - 20);
    m_creatures.push_back(creature);
}

void Aquarium::addAquariumLevel(std::shared_ptr<AquariumLevel> level){
    if(level == nullptr){return;} // guard to not add noise
    this->m_aquariumlevels.push_back(level);
}

void Aquarium::addPowerUp(std::shared_ptr<PowerUp> p) {
    m_powerups.push_back(p);
}

void Aquarium::update() {
    for (auto& creature : m_creatures) {
        creature->move();
    }
    this->Repopulate();
      if (rand() % 60 == 0) { 
        SpawnRandomPowerUp();
    }
}

void Aquarium::draw() const {
    for (const auto& creature : m_creatures) {
        creature->draw();
    }
    for (const auto& p : m_powerups) {
        p->draw();
    }
}

void Aquarium::SpawnRandomPowerUp() {
    int x = rand() % m_width;
    int y = rand() % m_height;

    PowerUpType type = static_cast<PowerUpType>(rand() % 4);
    auto sprite = m_sprite_manager->GetPowerUpSprite(type);

    addPowerUp(std::make_shared<PowerUp>(x, y, type, sprite));
}


void Aquarium::removeCreature(std::shared_ptr<Creature> creature) {
    auto it = std::find(m_creatures.begin(), m_creatures.end(), creature);
    if (it != m_creatures.end()) {
        ofLogVerbose() << "removing creature " << endl;
        int selectLvl = this->currentLevel % this->m_aquariumlevels.size();
        auto npcCreature = std::static_pointer_cast<NPCreature>(creature);
        this->m_aquariumlevels.at(selectLvl)->ConsumePopulation(npcCreature->GetType(), npcCreature->getValue());
        m_creatures.erase(it);
    }
}

void Aquarium::clearCreatures() {
    m_creatures.clear();
}

std::shared_ptr<Creature> Aquarium::getCreatureAt(int index) {
    if (index < 0 || size_t(index) >= m_creatures.size()) {
        return nullptr;
    }
    return m_creatures[index];
}
BonnieFish::BonnieFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
    : NPCreature(x, y, speed, sprite)
{
    m_creatureType = AquariumCreatureType::BonnieFish;
}

void BonnieFish::move() {
    zigTimer++;

    if (zigTimer >= maxZigTimer) {
        m_dy = -m_dy;
        zigTimer = 0;
    }

    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;

    if (m_dx < 0) m_sprite->setFlipped(true);
    else m_sprite->setFlipped(false);

    bounce();
}

void Aquarium::SpawnCreature(AquariumCreatureType type) {
    int x = rand() % this->getWidth();
    int y = rand() % this->getHeight();
    int speed = 1 + rand() % 25; // Speed between 1 and 25
    ofLogNotice() << "Spawned " << AquariumCreatureTypeToString(type)
              << " at (" << x << "," << y << ")";


        std::shared_ptr<Creature> creature = nullptr;

    switch (type) {
        case AquariumCreatureType::NPCreature:
            creature = std::make_shared<NPCreature>(x, y, speed, m_sprite_manager->GetSprite(AquariumCreatureType::NPCreature));
            break;
        case AquariumCreatureType::BiggerFish:
            creature = std::make_shared<BiggerFish>(x, y, speed, m_sprite_manager->GetSprite(AquariumCreatureType::BiggerFish));
            break;
        case AquariumCreatureType::BonnieFish:
            creature = std::make_shared<BonnieFish>(x, y, speed, m_sprite_manager->GetSprite(AquariumCreatureType::BonnieFish));
            break;
        case AquariumCreatureType::RandomFish:
            creature = std::make_shared<RandomFish>(x, y, speed, m_sprite_manager->GetSprite(AquariumCreatureType::RandomFish));
            break;
        default:
            ofLogError() << "Unknown creature type to spawn!";
            return;
    }

    if (creature) {
        ofLogNotice() << "Spawning creature: " << AquariumCreatureTypeToString(type)
                      << " at (" << x << "," << y << ")";
        this->addCreature(creature);
    }
};


// repopulation will be called from the levl class
// it will compose into aquarium so eating eats frm the pool of NPCs in the lvl class
// once lvl criteria met, we move to new lvl through inner signal asking for new lvl
// which will mean incrementing the buffer and pointing to a new lvl index
void Aquarium::Repopulate() {
    ofLogVerbose("entering phase repopulation");
    // lets make the levels circular
    int selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
    ofLogVerbose() << "the current index: " << selectedLevelIdx << endl;
    std::shared_ptr<AquariumLevel> level = this->m_aquariumlevels.at(selectedLevelIdx);


    if(level->isCompleted()){
        level->levelReset();
        this->currentLevel += 1;
        selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
        ofLogNotice()<<"new level reached : " << selectedLevelIdx << std::endl;
        level = this->m_aquariumlevels.at(selectedLevelIdx);
        this->clearCreatures();
    }

    
    // now lets find how many to respawn if needed 
    std::vector<AquariumCreatureType> toRespawn = level->Repopulate();
    ofLogVerbose() << "amount to repopulate : " << toRespawn.size() << endl;
    if(toRespawn.size() <= 0 ){return;} // there is nothing for me to do here
    for(AquariumCreatureType newCreatureType : toRespawn){
        this->SpawnCreature(newCreatureType);
    }
}


// Aquarium collision detection
std::shared_ptr<GameEvent> DetectAquariumCollisions(std::shared_ptr<Aquarium> aquarium, std::shared_ptr<PlayerCreature> player) {
    if (!aquarium || !player) return nullptr;
    
    for (int i = 0; i < aquarium->getCreatureCount(); ++i) {
        std::shared_ptr<Creature> npc = aquarium->getCreatureAt(i);
        if (npc && checkCollision(player, npc)) {
            return std::make_shared<GameEvent>(GameEventType::COLLISION, player, npc);
        }
    }
    return nullptr;
};

std::shared_ptr<PowerUp> DetectPowerUpCollision(
    std::shared_ptr<Aquarium> aquarium,
    std::shared_ptr<PlayerCreature> player)
{
    if (!aquarium || !player) 
        return nullptr;

    for (int i = 0; i < aquarium->getPowerUpCount(); i++) 
    {
        auto p = aquarium->getPowerUpAt(i);
        if (!p) 
            continue;

        if (checkCollision(player, p->getX(), p->getY(), 20.0f)) 
        {
            return p;  
        }
    }
    return nullptr;   
}

bool checkCollision(std::shared_ptr<PlayerCreature> player, float x, float y, float radius) {
    if (!player) return false;
    float dx = player->getX() - x;
    float dy = player->getY() - y;
    float distanceSquared = dx*dx + dy*dy;
    return distanceSquared <= radius * radius;
}


void AquariumGameScene::Update(){
    std::shared_ptr<GameEvent> event;
    m_player->update();
    if (updateControl.tick()) {
        auto powerUp = DetectPowerUpCollision(m_aquarium, m_player);
        if (powerUp) {
            m_player->applyPowerUp(powerUp->getType());
            m_aquarium->removePowerUp(powerUp);
        }
        event = DetectAquariumCollisions(m_aquarium, m_player);
        if (event != nullptr && event->isCollisionEvent()) {

            if(event->creatureB != nullptr){
                event->print();
                if(m_player->getPower() < event->creatureB->getValue()){
                    m_player->loseLife(3*60);
                    if(m_player->getLives() <= 0){
                        m_lastEvent = std::make_shared<GameEvent>(GameEventType::GAME_OVER, m_player, nullptr);
                        return;
                    }
                } else {
                    m_aquarium->removeCreature(event->creatureB);
                    m_player->addToScore(1, event->creatureB->getValue());
                    if (m_player->getScore() % 25 == 0){
                        m_player->increasePower(1);
                    }
                }
            }
        }

        m_aquarium->update();
    }
}

void AquariumGameScene::Draw() {
    this->m_player->draw();
    this->m_aquarium->draw();
    this->paintAquariumHUD();

}


void AquariumGameScene::paintAquariumHUD(){
    float panelWidth = ofGetWindowWidth() - 150;
    ofDrawBitmapString("Score: " + std::to_string(this->m_player->getScore()), panelWidth, 20);
    ofDrawBitmapString("Power: " + std::to_string(this->m_player->getPower()), panelWidth, 30);
    ofDrawBitmapString("Lives: " + std::to_string(this->m_player->getLives()), panelWidth, 40);
    for (int i = 0; i < this->m_player->getLives(); ++i) {
        ofSetColor(ofColor::red);
        ofDrawCircle(panelWidth + i * 20, 50, 5);
    }
    ofSetColor(ofColor::white); // Reset color to white for other drawings
}

void AquariumLevel::populationReset(){
    for(auto node: this->m_levelPopulation){
        node->currentPopulation = 0; // need to reset the population to ensure they are made a new in the next level
    }
}

void AquariumLevel::ConsumePopulation(AquariumCreatureType creatureType, int power){
    for(std::shared_ptr<AquariumLevelPopulationNode> node: this->m_levelPopulation){
        ofLogVerbose() << "consuming from this level creatures" << endl;
        if(node->creatureType == creatureType){
            ofLogVerbose() << "-cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            if(node->currentPopulation == 0){
                return;
            } 
            node->currentPopulation -= 1;
            ofLogVerbose() << "+cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            this->m_level_score += power;
            return;
        }
    }
}

bool AquariumLevel::isCompleted(){
    return this->m_level_score >= this->m_targetScore;
}

void Aquarium::removePowerUp(std::shared_ptr<PowerUp> p) {
    auto it = std::find(m_powerups.begin(), m_powerups.end(), p);
    if (it != m_powerups.end())
        m_powerups.erase(it);
}


std::vector<AquariumCreatureType> Level_0::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for(std::shared_ptr<AquariumLevelPopulationNode> node : this->m_levelPopulation){
        int delta = node->population - node->currentPopulation;
        ofLogVerbose() << "to Repopulate :  " << delta << endl;
        if(delta >0){
            for(int i = 0; i<delta; i++){
                toRepopulate.push_back(node->creatureType);
            }
            node->currentPopulation += delta;
        }
    }
    return toRepopulate;

}

std::vector<AquariumCreatureType> Level_1::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for(std::shared_ptr<AquariumLevelPopulationNode> node : this->m_levelPopulation){
        int delta = node->population - node->currentPopulation;
        if(delta >0){
            for(int i=0; i<delta; i++){
                toRepopulate.push_back(node->creatureType);
            }
            node->currentPopulation += delta;
        }
    }
    return toRepopulate;
}

std::vector<AquariumCreatureType> Level_2::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for(std::shared_ptr<AquariumLevelPopulationNode> node : this->m_levelPopulation){
        int delta = node->population - node->currentPopulation;
        if(delta >0){
            for(int i=0; i<delta; i++){
                toRepopulate.push_back(node->creatureType);
            }
            node->currentPopulation += delta;


            
        }
    }
    return toRepopulate;
}
