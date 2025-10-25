classDiagram
    %% Managers / Scenes
    class GameScene {
      <<abstract>>
      +string GetName()* 
      +void Update()* 
      +void Draw()* 
      +~GameScene()
    }

    class GameSceneManager {
      -vector<shared_ptr<GameScene>> m_scenes
      -shared_ptr<GameScene> m_active_scene
      +void Transition(string name)
      +void AddScene(shared_ptr<GameScene> newScene)
      +bool HasScenes()
      +shared_ptr<GameScene> GetScene(string name)
      +shared_ptr<GameScene> GetActiveScene()
      +string GetActiveSceneName()
      +void UpdateActiveScene()
      +void DrawActiveScene()
    }

    %% Aquarium core
    class Aquarium {
      -int m_maxPopulation
      -int m_width
      -int m_height
      -int currentLevel
      -vector<shared_ptr<Creature>> m_creatures
      -vector<shared_ptr<Creature>> m_next_creatures
      -vector<shared_ptr<AquariumLevel>> m_aquariumlevels
      -shared_ptr<AquariumSpriteManager> m_sprite_manager
      +Aquarium(int w,int h, shared_ptr<AquariumSpriteManager>)
      +void addCreature(shared_ptr<Creature>)
      +void removeCreature(shared_ptr<Creature>)
      +void clearCreatures()
      +void update()
      +void draw() const
      +void setBounds(int w,int h)
      +void setMaxPopulation(int n)
      +void Repopulate()
      +void SpawnCreature(AquariumCreatureType)
      +shared_ptr<Creature> getCreatureAt(int idx)
      +int getCreatureCount() const
      +int getWidth() const
      +int getHeight() const
    }

    class AquariumLevel {
      -vector<shared_ptr<AquariumLevelPopulationNode>> m_levelPopulation
      -int m_level_score
      -int m_targetScore
      +AquariumLevel(int levelNumber,int targetScore)
      +void ConsumePopulation(AquariumCreatureType,int power)
      +bool isCompleted()
      +void populationReset()
      +void levelReset()
      +vector<AquariumCreatureType> Repopulate()*  %% abstract in base GameLevel; here virtual=0 in header
    }

    class AquariumSpriteManager {
      -shared_ptr<GameSprite> m_npc_fish
      -shared_ptr<GameSprite> m_big_fish
      +AquariumSpriteManager()
      +~AquariumSpriteManager()
      +shared_ptr<GameSprite> GetSprite(AquariumCreatureType)
    }

    %% Graphics
    class GameSprite {
      -ofImage m_image
      -ofImage m_flippedImage
      -bool m_flipped=false
      +GameSprite(string imagePath,int width,int height)
      +void draw(float x,float y) const
      +void setFlipped(bool)
    }

    %% Creatures
    class Creature {
      <<abstract>>
      #float m_x
      #float m_y
      #float m_dx
      #float m_dy
      #int   m_speed
      #float m_width
      #float m_height
      #float m_collisionRadius
      #int   m_value
      #shared_ptr<GameSprite> m_sprite
      +Creature(float x,float y,int speed,float collisionRadius,int value,shared_ptr<GameSprite>)
      +~Creature()
      +void move()* 
      +void draw()* const
      +float getCollisionRadius() const
      +void setCollisionRadius(float)
      +float getX() const
      +float getY() const
      +int   getSpeed() const
      +void  setSpeed(int)
      +void  setFlipped(bool)
      +void  setSprite(shared_ptr<GameSprite>)
      +int   getValue() const
      +void  setBounds(int w,int h)
    }

    class PlayerCreature {
      -int m_score=0
      -int m_lives=3
      -int m_power=1
      -int m_damage_debounce=0
      +PlayerCreature(float x,float y,int speed, shared_ptr<GameSprite>)
      +void move() 
      +void draw() const
      +void update()
      +void changeSpeed(int)
      +void setLives(int)
      +void setDirection(float dx,float dy)
      +bool isXDirectionActive()
      +bool isYDirectionActive()
      +float getDx()
      +float getDy()
      +int getScore() const
      +int getLives() const
      +int getPower() const
      +void addToScore(int amount,int weight=1)
      +void loseLife(int debounce)
      +void increasePower(int value)
      +void reduceDamageDebounce()
    }

    class NPCreature {
      -AquariumCreatureType m_creatureType
      +NPCreature(float x,float y,int speed, shared_ptr<GameSprite>)
      +AquariumCreatureType GetType()
      +void move()
      +void draw() const
    }

    %% Inheritance
    Creature <|-- PlayerCreature
    Creature <|-- NPCreature

    %% Ownership / usage
    GameSceneManager --> GameScene : manages
    GameScene --> Aquarium : (Aquarium Scene) owns
    Aquarium --> "1" AquariumSpriteManager : uses
    Aquarium --> "0..*" Creature : contains
    Aquarium --> "0..*" AquariumLevel : contains
    Creature --> GameSprite : draws with
    AquariumLevel ..> NPCreature : defines population/types
