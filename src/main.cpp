#include "Camera.h"
#include "Charlie2D.h"
#include "LDTKEntity.h"
#include "UISliceRenderer.h"
#include "ldtk.h"
#include <iostream>

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Ability {
public:
  Ability(std::string _name, std::string _texture = "img/Jump.png",
          std::string _description = "")
      : name(_name), texture(_texture), description(_description){};
  virtual void start(){};
  virtual void update(Entity *player, float deltaTime){};
  std::string name;
  std::string description;
  std::string texture;
};

class WallJump : public Ability {
public:
  WallJump()
      : Ability("WallJump", "img/WallJump.png",
                "Hit space when touching wall to redirect") {}
  void update(Entity *player, float deltaTime) override {
    if (InputManager::checkInput("jumpTrigger")) {
      rightBox = player->require<entityBox>()->getBox();
      rightBox.position.x += player->require<entityBox>()->getSize().x;
      bool checkground = false;
      for (Collider *col : GameManager::getComponents<Collider>()) {
        if (col->solid) {
          if (col->checkBoxCollision(rightBox)) {
            checkground = true;
            break;
          }
        }
      }

      if (checkground && InputManager::checkHorizontal() > 0) {
        player->get<physicsBody>()->velocity = {-force, -jump};
      }

      leftBox = player->require<entityBox>()->getBox();
      leftBox.position.x -= player->require<entityBox>()->getSize().x / 1;
      checkground = false;
      for (Collider *col : GameManager::getComponents<Collider>()) {
        if (col->solid) {
          if (col->checkBoxCollision(leftBox)) {
            checkground = true;
            break;
          }
        }
      }

      if (checkground && InputManager::checkHorizontal() < 0) {
        player->get<physicsBody>()->velocity = {force, -jump};
      }
    }
  }

  Box rightBox;
  Box leftBox;
  float force = 500;
  float jump = 500;
};

class Jump : public Ability {
public:
  Jump() : Ability("Jump", "img/Jump.png", "Space to use\nHold to go higher") {}
  void start() override {
    GameManager::getEntities("Player")[0]->get<JumpMan>()->allowJump = true;
  }
};

class DoubleJump : public Ability {
public:
  DoubleJump()
      : Ability("DoubleJump", "img/DoubleJump.png", "Hit Space in air") {}
  void start() override {
    GameManager::getEntities("Player")[0]->get<JumpMan>()->jumps = 2;
  }
};

class BlueKey : public Ability {
public:
  BlueKey() : Ability("BlueKey", "img/BlueKey.png", "Opens blue gates") {}
  void start() override {
    for (Entity *entity : GameManager::getEntities("BlueGate")) {
      entity->toDestroy = true;
    }
  }
};

class GreenKey : public Ability {
public:
  GreenKey() : Ability("GreenKey", "img/GreenKey.png", "Opens green gates") {}
  void start() override {
    for (Entity *entity : GameManager::getEntities("GreenGate")) {
      entity->toDestroy = true;
    }
  }
};

class RedKey : public Ability {
public:
  RedKey() : Ability("RedKey", "img/RedKey.png", "Opens red gates") {}
  void start() override {
    for (Entity *entity : GameManager::getEntities("RedGate")) {
      entity->toDestroy = true;
    }
  }
};

class Dash : public Ability {
public:
  Dash() : Ability("Dash", "img/Dash.png", "Press Shift") {}
  void update(Entity *player, float deltaTime) override {
    if (InputManager::checkInput("dash")) {
      if (abs(player->get<physicsBody>()->velocity.x) < 0.1)
        return;
      int dir = 0;
      if (InputManager::checkInput("right"))
        dir = 1;
      if (InputManager::checkInput("left"))
        dir = -1;
      if (timer + time < SDL_GetTicks()) {
        player->get<physicsBody>()->velocity.x = dir * dash;
        timer = SDL_GetTicks();
      }
    }
  }

  Uint32 time = 500;
  Uint32 timer = SDL_GetTicks();
  const float dash = 600.0f;
};

class Jet : public Ability {
public:
  Jet() : Ability("Jet", "img/Jet.png", "Removes dash cooldown") {}
  void update(Entity *player, float deltaTime) override {
    if (InputManager::checkInput("dash")) {
      if (abs(player->get<physicsBody>()->velocity.x) < 0.1)
        return;
      int dir = 0;
      if (InputManager::checkInput("right"))
        dir = 1;
      if (InputManager::checkInput("left"))
        dir = -1;
      player->get<physicsBody>()->velocity.x = dir * dash;
    }
  }

  const float dash = 600.0f;
};

class Player : public Component {
public:
  void start() override {
    Sprite *sprite = entity->add<Sprite>();
    sprite->loadTexture("img/Player.png");
    sprite->layer = 10;
    entity->require<entityBox>()->setScale({32, 32});

    entity->add<Collider>();
    entity->add<physicsBody>();
    // entity->get<physicsBody>()->maxVelocity = {32, 32};

    // entity->add<JumpMan>();
    // entity->get<JumpMan>()->allowJump = false;

    // abilities.push_back(new Dash());
    // abilities.push_back(new WallJump());
    // abilities.push_back(new Jet());

    Camera::setPosition(entity->require<entityBox>()->getPosition());
  }

  std::string l;
  void update(float deltaTime) override {
    if (LDTK::checkOutsideBounds(entity)) {
      l = LDTK::findTraveledLevel(entity);
      LDTK::loadLevel(l);
    }

    if (entity->get<Collider>()->getCollisions("Elevator").size() > 0) {
      entity->get<physicsBody>()->velocity.y = -340;
    }

    for (Ability *ability : abilities) {
      ability->update(entity, deltaTime);
    }

    entity->get<Sprite>()->angle.rotate(entity->get<physicsBody>()->velocity.x /
                                        1000 * deltaTime);

    Camera::setPosition(entity->require<entityBox>()->getBox().getCenter());
    background->require<entityBox>()->setWithCenter(Camera::getPosition());
  }

  ~Player() {
    for (Ability *ability : abilities) {
      delete ability;
    }
  }

  Entity *background;
  std::vector<Ability *> abilities;
};

class ItemPanel : public Component {
public:
  void start() override {
    Entity *canvas = GameManager::createEntity("panel-canvas");
    canvas->add<UICanvas>();

    entity->setParent(canvas);
    entity->add<UISliceRenderer>();
    entity->get<UISliceRenderer>()->setColor({156, 39, 176});
    entity->require<entityBox>()->setScale({700, 300});
    entity->require<entityBox>()->anchor = 4;
    entity->require<entityBox>()->setLocalWithCenter({0, -900});
    entity->add<Text>();
    entity->get<Text>()->changeFont("img/fonts/prstart.ttf", 30);
  }

  void update(float deltaTime) override {
    if (closed && !hover) {
      entity->require<entityBox>()->setLocalPosition({-1000, -1000});
      return;
    }

    if (opening) {
      entity->require<entityBox>()->changeLocalPosition({0, deltaTime * 1300});
      if (entity->require<entityBox>()->getLocalBox().getCenter().y >= 0) {
        opening = false;
      }
    } else {
      if (InputManager::checkInput("jump")) {
        closing = true;
      }

      if (closing) {
        entity->require<entityBox>()->changeLocalPosition(
            {0, -deltaTime * 1300});
        if (entity->require<entityBox>()->getLocalPosition().y <= -900) {
          // entity->getParent()->toDestroy = true;
          closed = true;
        }
      }
    }
  }

  void onDestroy() override {}

  bool opening = true;
  bool closing = false;
  bool hover = false;
  bool closed = false;
};

class ItemIcon : public Component {
public:
  void start() override {
    entity->setParent(GameManager::getEntities("canvas")[0]);
    entity->require<entityBox>()->setScale({48 * 2, 48 * 2});
    entity->require<entityBox>()->setLocalPosition(
        {53 * 2 *
             static_cast<float>(
                 GameManager::GameManager::getComponents<Player>()[0]->abilities.size()),
         40});
    Sprite *sprite = entity->add<Sprite>();
    sprite->layer = 50;
    sprite->renderAsUI = true;

    entity->add<Button>();
    entity->get<Button>()->onHover = [this]() {
      if (panel->get<ItemPanel>()->closed) {
        panel->require<entityBox>()->setPosition(
            InputManager::getMouseUIPosition());
        panel->get<ItemPanel>()->hover = true;
      }
    };

    entity->get<Button>()->offHover = [this]() {
      panel->get<ItemPanel>()->hover = false;
    };
  }

  void update(float deltaTime) override {
    // if (panel != nullptr)
    // entity->require<entityBox>()->setPosition(InputManager::getMouseUIPosition());
  }

  Entity *panel = nullptr;
};

class ItemPickup : public Component {
public:
  void start() override {
    entity->add<Sprite>();
    entity->add<Collider>();
    entity->skipUpdate = true;
  }

  void update(float deltaTime) override {
    for (Ability *a : GameManager::GameManager::getComponents<Player>()[0]->abilities) {
      if (ability->name == a->name)
        entity->toDestroy = true;
    }

    if (entity->get<Collider>()->checkBoxCollision(
            GameManager::GameManager::getEntities("Player")[0]->require<entityBox>()->getBox())) {
      GameManager::GameManager::getComponents<Player>()[0]->abilities.push_back(ability);
      ability->start();
      Entity *panel = GameManager::createEntity("panel");
      panel->add<ItemPanel>();
      panel->get<Text>()->text = ability->name + "\n\n" + ability->description +
                                 "\n\nPress Space to close";

      Entity *icon = GameManager::createEntity("icon");
      icon->add<ItemIcon>();
      icon->get<Sprite>()->loadTexture(ability->texture, false);
      icon->get<ItemIcon>()->panel = panel;

      entity->toDestroy = true;
    }
  }

  Ability *ability = nullptr;
};

class StartPanel : public Component {
public:
  void start() override {
    entity->setParent(GameManager::GameManager::getEntities("canvas")[0]);
    entity->require<entityBox>()->setScale({600, 200});
    entity->require<entityBox>()->anchor = 4;
    entity->require<entityBox>()->setLocalWithCenter({0, 0});

    Entity *playButton = GameManager::createEntity("");
    playButton->add<UISliceRenderer>();
    playButton->setParent(entity);
    playButton->require<entityBox>()->setScale({600, 200});
    playButton->require<entityBox>()->setLocalWithCenter({0, 100});
    playButton->require<entityBox>()->anchor = 4;
    playButton->add<Text>();
    playButton->get<Text>()->changeFont("img/fonts/ThaleahFat_TTF.ttf", 100);
    playButton->get<Text>()->text = "Play";
    playButton->add<Button>();
    playButton->get<Button>()->onClick = [this]() {
      // GameManager::GameManager::getEntities("Player")[0]->add<JumpMan>();
      // GameManager::GameManager::getEntities("Player")[0]->get<JumpMan>()->allowJump = false;
      clicked = true;
    };

    Entity *title = GameManager::createEntity("");
    title->add<Text>();
    title->get<Text>()->changeFont("img/fonts/ThaleahFat_TTF.ttf", 200);
    title->get<Text>()->text = "ICNTBALL";
    title->get<Text>()->text_color = {255, 255, 255};
    title->require<entityBox>()->setScale({800, 200});
    title->require<entityBox>()->setLocalWithCenter({0, -200});
    title->require<entityBox>()->anchor = 4;
    title->setParent(entity);

    Entity *remastered = GameManager::createEntity("");
    remastered->add<Text>();
    remastered->get<Text>()->changeFont("img/fonts/ThaleahFat_TTF.ttf", 100);
    remastered->get<Text>()->text = "Remastered";
    remastered->get<Text>()->text_color = {255, 255, 255};
    remastered->require<entityBox>()->setScale({600, 200});
    remastered->require<entityBox>()->setLocalWithCenter({0, -100});
    remastered->require<entityBox>()->anchor = 4;
    remastered->setParent(entity);

    Entity *ethanscharlie = GameManager::createEntity("");
    Sprite *sprite = ethanscharlie->add<Sprite>();
    sprite->loadTexture("img/PixilEthan.png");
    sprite->renderAsUI = true;
    ethanscharlie->require<entityBox>()->setScale({141 * 5, 26 * 5});
    ethanscharlie->require<entityBox>()->setLocalWithCenter({0, -400});
    ethanscharlie->require<entityBox>()->anchor = 4;
    ethanscharlie->setParent(entity);
  }

  void update(float deltaTime) override {
    if (!clicked)
      return;
    if (entity->require<entityBox>()->getLocalPosition().y < -1300)
      entity->toDestroy = true;

    entity->require<entityBox>()->changeLocalPosition({0, -deltaTime * 1300});
  }

  bool clicked = false;
};

void mainScene() {
  Camera::scale = (2);
  LDTK::loadJson("img/ldtk/icnt.ldtk");

  Entity *canvas = GameManager::createEntity("canvas");
  canvas->add<UICanvas>();

  Entity *startPanel = GameManager::createEntity("");
  startPanel->add<StartPanel>();

  LDTK::onLoadLevel = [=]() {
    for (Entity *entity : GameManager::getAllObjects()) {
      if (entity->tag == "PlayerSpawn") {
        if (GameManager::getEntities("Player").size() < 1) {
          Entity *player = GameManager::createEntity("Player");
          player->require<entityBox>()->setPosition(
              entity->require<entityBox>()->getPosition());
          LDTK::ldtkPlayer = player;
          player->add<Player>();
          GameManager::getEntities("Player")[0]->add<JumpMan>();
          GameManager::getEntities("Player")[0]->get<JumpMan>()->allowJump = false;

          Entity *background = GameManager::createEntity("Background");
          background->add<Sprite>();
          background->get<Sprite>()->loadTexture("img/Back.png");
          background->get<Sprite>()->layer = -1;
          background->require<entityBox>()->setScale(
              GameManager::gameWindowSize * 0.5);
          background->require<entityBox>()->setWithCenter({0, 0});
          player->get<Player>()->background = background;
        }
      } else if (entity->tag == "Ground") {
        entity->add<Collider>();
        entity->get<Collider>()->solid = true;

      } else if (entity->tag == "Elevator") {
        entity->add<Collider>();

      } else if (entity->tag == "BlueGate" || entity->tag == "GreenGate" ||
                 entity->tag == "RedGate") {
        entity->add<Collider>()->solid = true;
        if (GameManager::getEntities("Player").size() > 0) {
          for (Ability *a : GameManager::getComponents<Player>()[0]->abilities) {
            if ((a->name == "BlueKey" && entity->tag == "BlueGate") ||
                (a->name == "GreenKey" && entity->tag == "GreenGate") ||
                (a->name == "RedKey" && entity->tag == "RedGate")) {
              entity->toDestroy = true;
            }
          }
        }

      } else if (entity->tag == "Item") {
        std::string itemtype = entity->get<LDTKEntity>()
                                   ->entityJson["fieldInstances"][0]["__value"];
        entity->add<ItemPickup>();

        if (itemtype == "WallJump") {
          entity->get<ItemPickup>()->ability = new WallJump();
          entity->get<Sprite>()->loadTexture("img/WallJump.png");
        } else if (itemtype == "BlueKey") {
          entity->get<ItemPickup>()->ability = new BlueKey();
          entity->get<Sprite>()->loadTexture("img/BlueKey.png");
        } else if (itemtype == "GreenKey") {
          entity->get<ItemPickup>()->ability = new GreenKey();
          entity->get<Sprite>()->loadTexture("img/GreenKey.png");
        } else if (itemtype == "RedKey") {
          entity->get<ItemPickup>()->ability = new RedKey();
          entity->get<Sprite>()->loadTexture("img/RedKey.png");
        } else if (itemtype == "DoubleJump") {
          entity->get<ItemPickup>()->ability = new DoubleJump();
          entity->get<Sprite>()->loadTexture("img/DoubleJump.png");
        } else if (itemtype == "Dash") {
          entity->get<ItemPickup>()->ability = new Dash();
          entity->get<Sprite>()->loadTexture("img/Dash.png");
        } else if (itemtype == "Jet") {
          entity->get<ItemPickup>()->ability = new Jet();
          entity->get<Sprite>()->loadTexture("img/Jet.png");
        } else if (itemtype == "Jump") {
          entity->get<ItemPickup>()->ability = new Jump();
          entity->get<Sprite>()->loadTexture("img/Jump.png");
        }
      }
    }
  };

  LDTK::loadLevel("dffeb9a0-6280-11ee-bf15-ab978f05dce2");
}

int main() {
  GameManager::init();
  // GameManager::AddScene("game", new mainScene());
  // GameManager::LoadScene("game");
  mainScene();
  GameManager::doUpdateLoop();
  return 0;
}
