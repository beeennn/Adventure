#include "Slide.h"
#include <string>
using namespace std;
#include "SimpleAudioEngine.h"

using namespace CocosDenshion;

//test
//Slide * Slide::slide = NULL;

Slide::Slide() : m_world(NULL)
{

}

Slide::~Slide()
{
	//Slide::slide = NULL;
}

Scene* Slide::createScene()
{
	Scene* scene = Scene::createWithPhysics();
	scene->getPhysicsWorld()->setGravity(Point(0, 0));
	Layer* layer = Slide::create(scene->getPhysicsWorld());
	scene->addChild(layer);
	return scene;
}

Slide* Slide::create(PhysicsWorld* world)
{
	Slide* pRet = new(std::nothrow) Slide();
	//Slide * pRet = Slide::init();
	if (pRet && pRet->init(world))
	{
		pRet->autorelease();
		return pRet;
	}
	delete pRet;
	pRet = NULL;
	return NULL;
}

bool Slide::init(PhysicsWorld* world)
{
	if (!Layer::init())
	{
		return false;
	}
	defence = false;
	difficalty = CCRANDOM_0_1() / 2 + 0.2; 
	moveTimes = 3;
	bleed = 300;
	timesNow = 0.0;
	timeLeft = 30;
	bomb = 0;
	shel = 0;
	SimpleAudioEngine::getInstance()->playBackgroundMusic("game.wav", true);
	visibleSize = Director::getInstance()->getVisibleSize();

	m_world = world;
	dispatcher = Director::getInstance()->getEventDispatcher();
	isCut = false;
	oldPos = Point(Director::getInstance()->getVisibleSize().width / 2, 0);

	
	leftTimeLayer = LeftTimeLayer::create();
	leftTimeLayer->setCascadeOpacityEnabled(true);
	leftTimeLayer->setOpacity(255 * 0.6);
	this->addChild(leftTimeLayer, 5);

	char bgpath[20];
	int r = (int)(CCRANDOM_0_1() * 100) % 5;
	sprintf(bgpath, "black_hole_bg%d.jpg", r);
	auto bgsprite = Sprite::create(bgpath);
	bgsprite->setPosition(visibleSize / 2);
	bgsprite->setScale(visibleSize.width / bgsprite->getContentSize().width, visibleSize.height / bgsprite->getContentSize().height);
	this->addChild(bgsprite, 0);

	auto ps = ParticleSystemQuad::create("black_hole.plist");
	ps->setPosition(visibleSize / 2);
	this->addChild(ps);
	playEffect();
	//ps->release();

	player = Sprite::create("player2.png");
	player->setAnchorPoint(Vec2(0.5, 0.5));
	player->setPhysicsBody(PhysicsBody::createCircle(35));
	player->getPhysicsBody()->setCategoryBitmask(0x00001111);
	player->getPhysicsBody()->setContactTestBitmask(0x00001111);
	player->setPosition(visibleSize / 2);
	player->getPhysicsBody()->setTag(1);
	player->getPhysicsBody()->setAngularVelocityLimit(0);
	addChild(player);

	auto ground = Node::create();
	ground->setPhysicsBody(PhysicsBody::createEdgeSegment(Vec2(0, 0), Vec2(visibleSize.width, 0)));
	ground->getPhysicsBody()->setDynamic(false);
	addChild(ground);
	auto ground2 = Node::create();
	ground2->setPhysicsBody(PhysicsBody::createEdgeSegment(Vec2(0, 0), Vec2(0, visibleSize.height)));
	ground2->getPhysicsBody()->setDynamic(false);
	addChild(ground2);
	auto ground3 = Node::create();
	ground3->setPhysicsBody(PhysicsBody::createEdgeSegment(visibleSize, Vec2(visibleSize.width, 0)));
	ground3->getPhysicsBody()->setDynamic(false);
	addChild(ground3);
	auto ground4 = Node::create();
	ground4->setPhysicsBody(PhysicsBody::createEdgeSegment(visibleSize, Vec2(0, visibleSize.height)));
	ground4->getPhysicsBody()->setDynamic(false);
	addChild(ground4);

	this->schedule(schedule_selector(Slide::update), 0.2);

	preloadMusic();
	playBgm();

	TouchEvent();
	testKeyboardEvent();
	testOntouch();

	// 添加技能工具箱
	skillToolLayer = SkillToolLayer::create();
	skillToolLayer->setLabelInstanceMove(moveTimes);
	skillToolLayer->setCascadeOpacityEnabled(true);
	skillToolLayer->setOpacity(255 * 0.6);
	this->addChild(skillToolLayer, 5);

	// 添加生命层
	lifeLayer = LifeLayer::create();
	lifeLayer->setBloodPercentage(100.0f);
	this->addChild(lifeLayer, 5);
	// 添加时间条

	return true;
}

void Slide::preloadMusic()
{
	SimpleAudioEngine::getInstance()->preloadBackgroundMusic("music/bg_music0.mp3");
	SimpleAudioEngine::getInstance()->preloadBackgroundMusic("music/bg_music1.mp3");
	SimpleAudioEngine::getInstance()->preloadBackgroundMusic("music/bg_music2.mp3");
	SimpleAudioEngine::getInstance()->preloadBackgroundMusic("music/bg_music3.mp3");
	SimpleAudioEngine::getInstance()->preloadEffect("music/explore.wav");
	SimpleAudioEngine::getInstance()->preloadEffect("music/shel.wav");
}

void Slide::playBgm()
{
	if (difficalty > 0.6)
		SimpleAudioEngine::getInstance()->playBackgroundMusic("music/bg_music3.mp3", true);
	else if (difficalty > 0.5)
		SimpleAudioEngine::getInstance()->playBackgroundMusic("music/bg_music2.mp3", true);
	else if (difficalty > 0.4)
		SimpleAudioEngine::getInstance()->playBackgroundMusic("music/bg_music1.mp3", true);
	else
		SimpleAudioEngine::getInstance()->playBackgroundMusic("music/bg_music0.mp3", true);
}

void Slide::playEffect()
{
	SimpleAudioEngine::getInstance()->playEffect("music/explore.wav");
}

void Slide::TouchEvent()
{
	
	auto listner = EventListenerTouchOneByOne::create();
	listner->onTouchBegan = CC_CALLBACK_2(Slide::onTouchBegan, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listner, this);
}


void Slide::testKeyboardEvent()
{
	auto listener = EventListenerKeyboard::create();

	listener->onKeyPressed = CC_CALLBACK_2(Slide::onKeyPressed, this);
	listener->onKeyReleased = CC_CALLBACK_2(Slide::onKeyReleased, this);

	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void Slide::testOntouch() {
	touchListener = EventListenerPhysicsContact::create();
	touchListener->onContactBegin = CC_CALLBACK_1(Slide::onConcactBegan, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(touchListener, 1);
}

bool Slide::onConcactBegan(PhysicsContact& contact) {
	if (contact.getShapeA()->getBody()->getTag() % 10 == 0) {
		auto ps = ParticleSystemQuad::create("explo.plist");
		ps->setPosition(contact.getShapeA()->getBody()->getPosition());
		this->addChild(ps);
		playEffect();
		//ps->release();

		if (contact.getShapeB()->getBody()->getTag() % 10 == 0) {
			contact.getShapeB()->getBody()->removeFromWorld();
			if (contact.getShapeB()->getBody()->getNode()) {
				((Sprite*)(contact.getShapeB()->getBody()->getNode()))->removeFromParentAndCleanup(true);
				//contact.getShapeB()->getBody()->getNode()->release();
			}
			enemys.eraseObject(contact.getShapeB()->getBody());
		}
		if (contact.getShapeB()->getBody()->getTag() == 1) {
			bleedNew(contact.getShapeA()->getBody()->getTag());
		}
		enemys.eraseObject(contact.getShapeA()->getBody());
		contact.getShapeA()->getBody()->removeFromWorld();
		if (contact.getShapeA()->getBody()->getNode()) {
			((Sprite*)(contact.getShapeA()->getBody()->getNode()))->removeFromParentAndCleanup(true);
			//contact.getShapeA()->getBody()->getNode()->release();
		}
	}
	else if (contact.getShapeA()->getBody()->getTag() == 1 && contact.getShapeB()->getBody()->getTag() % 10 == 0) {
		auto ps = ParticleSystemQuad::create("explo.plist");
		ps->setPosition(contact.getShapeB()->getBody()->getPosition());
		this->addChild(ps);
		playEffect();
		//ps->release();

		bleedNew(contact.getShapeB()->getBody()->getTag());
		enemys.eraseObject(contact.getShapeB()->getBody());
		contact.getShapeB()->getBody()->removeFromWorld();
		if (contact.getShapeB()->getBody()->getNode()) {
			((Sprite*)(contact.getShapeB()->getBody()->getNode()))->removeFromParentAndCleanup(true);
			//contact.getShapeB()->getBody()->getNode()->release();
		}
	}
	else if (contact.getShapeA()->getBody()->getTag() % 10 == 5 && contact.getShapeB()->getBody()->getTag() == 1) {
		switch (contact.getShapeA()->getBody()->getTag())
		{
		case 5:
			shel++;
			skillToolLayer->setLabelDefence(shel);
			break;
		case 15:
			bomb++;
			skillToolLayer->setLabelAttack(bomb);
			break;
		case 25:
			moveTimes += 1;
			skillToolLayer->setLabelInstanceMove(moveTimes);
			break;
		default:
			break;
		}
		contact.getShapeA()->getBody()->removeFromWorld();
		if (contact.getShapeA()->getBody()->getNode()) {
			((Sprite*)(contact.getShapeA()->getBody()->getNode()))->removeFromParentAndCleanup(true);
		}
	}
	else if (contact.getShapeB()->getBody()->getTag() % 10 == 5 && contact.getShapeA()->getBody()->getTag() == 1) {
		switch (contact.getShapeB()->getBody()->getTag())
		{
		case 5:
			shel++;
			skillToolLayer->setLabelDefence(shel);
			break;
		case 15:
			bomb++;
			skillToolLayer->setLabelAttack(bomb);
			break;
		case 25:
			moveTimes += 1;
			skillToolLayer->setLabelInstanceMove(moveTimes);
			break;
		default:
			break;
		}
		contact.getShapeB()->getBody()->removeFromWorld();
		if (contact.getShapeB()->getBody()->getNode()) {
			((Sprite*)(contact.getShapeB()->getBody()->getNode()))->removeFromParentAndCleanup(true);
		}
	}
	return true;
}

void Slide::update(float f) {
	if (CCRANDOM_0_1() > 0.2) newEnemys(difficalty);
	if (CCRANDOM_0_1() < 0.15) addBonus();
	timesNow += f;
	if (timesNow >= 1.0) {
		timeLeft--;
		timesNow = 0.0;
		leftTimeLayer->setLabelTime(timeLeft);
	}
	if (timeLeft == 0) escape();
}

void Slide::Schedule_(Ref *ref)
{
	schedule(schedule_selector(Slide::update), 0.5f);
}

bool Slide::onTouchBegan(Touch *touch, Event *unused_event){
	if (moveTimes-- > 0) {
		Point location = touch->getLocation();
		if (location.x >= visibleSize.width || location.x <= 0) {
			location.x = location.x > 0 ? visibleSize.width : 0;
		}
		if (location.y >= visibleSize.height || location.y <= 0) {
			location.y = location.y > 0 ? visibleSize.height : 0;
		}

		SimpleAudioEngine::getInstance()->playEffect("music/instance_fly.wav");

		instanceMoveTarget = location;
		
		// 设置瞬移点
		if (instance_move_target != NULL) {
			instance_move_target->removeFromParentAndCleanup(true);
			instance_move_target = NULL;
		}
		instance_move_target = Sprite::create("instance_move_target.png");
		instance_move_target->setPosition(instanceMoveTarget);
		this->addChild(instance_move_target);

		ScaleTo * scaleToSmall = ScaleTo::create(0.1f, 0.1f);
		ScaleTo * scaleToHold = ScaleTo::create(0.1f, 0.1f);
		ScaleTo * scaleToBig = ScaleTo::create(0.1f, 1.0f);
		CallFunc *instanceMove = CallFunc::create(this, CC_CALLFUNC_SELECTOR(Slide::InstanceMove));
		Sequence * sequence = Sequence::create(scaleToSmall->clone(), instanceMove, scaleToBig->clone(), NULL);
		player->runAction(sequence);

		skillToolLayer->setLabelInstanceMove(moveTimes);
		return true;
	}
}

void Slide::newEnemys(float f) {
	int newNum = 1;
	if (f > 0.3) newNum = 2;
	if (f > 0.7) newNum = 3;
	while (newNum--) {
		int type = 0;
		if (CCRANDOM_0_1() > 0.85) { type = 2; }
		else if (CCRANDOM_0_1() > 0.6) { type = 1; }

		Point location = Vec2(0, 0);
		switch (rand() % 4)
		{
		case 0:
			location.y = visibleSize.height;
			location.x = rand() % (int)(visibleSize.width);
			break;
		case 1:
			location.x = visibleSize.width;
			location.y = rand() % (int)(visibleSize.height);
			break;
		case 2:
			location.y = 0;
			location.x = rand() % (int)(visibleSize.width);
			break;
		case 3:
			location.x = 0;
			location.y = rand() % (int)(visibleSize.height);
			break;
		default:
			break;
		}
		addEnemy(type, location);
	}
}

void Slide::addEnemy(int type, Point p) {
	char path[100];
	int tag;
	switch (type)
	{
	case 0:
		sprintf(path, "stone1.png");
		tag = 10;
		break;
	case 1:
		sprintf(path, "stone2.png");
		tag = 20;
		break;
	case 2:
		sprintf(path, "stone3.png");
		tag = 30;
		break;
	default:
		sprintf(path, "stone1.png");
		tag = 10;
		break;
	}
	auto re = Sprite::create(path);
	re->setPhysicsBody(PhysicsBody::createCircle(re->getContentSize().height / 2));
	re->setAnchorPoint(Vec2(0.5, 0.5));
	re->setScale(0.5, 0.5);
	re->setPosition(p);
	re->getPhysicsBody()->setCategoryBitmask(0x00000001);
	re->getPhysicsBody()->setContactTestBitmask(0x00000001);
	re->getPhysicsBody()->setTag(tag);
	if (rand() % 100 < (100 * difficalty)) {
		re->getPhysicsBody()->setVelocity((player->getPosition() - p) * (difficalty - 0.22));
	}
	else {
		re->getPhysicsBody()->setVelocity((Point(rand() % (int)(visibleSize.width - 100) + 50, rand() % (int)(visibleSize.height - 100) + 50) - p) * (difficalty - 0.22));
	}
	re->getPhysicsBody()->setAngularVelocity(CCRANDOM_0_1() * 10);
	enemys.pushBack(re->getPhysicsBody());
	addChild(re);
}

void Slide::onKeyPressed(EventKeyboard::KeyCode code, Event* event) {
	switch (code)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
	case cocos2d::EventKeyboard::KeyCode::KEY_A:
		player->getPhysicsBody()->setVelocity(Point(-200, player->getPhysicsBody()->getVelocity().y));
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
	case cocos2d::EventKeyboard::KeyCode::KEY_D:
		player->getPhysicsBody()->setVelocity(Point(200, player->getPhysicsBody()->getVelocity().y));
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
	case cocos2d::EventKeyboard::KeyCode::KEY_W:
		player->getPhysicsBody()->setVelocity(Point(player->getPhysicsBody()->getVelocity().x, 200));
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
	case cocos2d::EventKeyboard::KeyCode::KEY_S:
		player->getPhysicsBody()->setVelocity(Point(player->getPhysicsBody()->getVelocity().x, -200));
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_Q:
		if (bomb > 0) {
			auto ps = ParticleSystemQuad::create("ring.plist");
			ps->setPosition(player->getPosition());
			this->addChild(ps);
			playEffect();
			//ps->release();

			// test
			SkillLayer::showSkillLayerRandom(this);

			for each (PhysicsBody* s in enemys)
			{
				if (s == nullptr) continue;
				auto ps = ParticleSystemQuad::create("explo.plist");
				ps->setPosition(s->getPosition());
				this->addChild(ps);
				playEffect();
				if (s != nullptr) {
					s->removeFromWorld();
					((Sprite*)s->getNode())->removeFromParentAndCleanup(true);
				}
			}
			enemys.clear();

			bomb--;
			skillToolLayer->setLabelAttack(bomb);
		}
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_E:
		if (shel > 0) {
			this->unschedule(schedule_selector(Slide::nodefence));
			defence = true;
			auto texture = CCTextureCache::sharedTextureCache()->addImage("player2_shel.png");
			player->setTexture(texture);
			this->scheduleOnce(schedule_selector(Slide::nodefence), 5.0f);

			shel--;
			skillToolLayer->setLabelDefence(shel);
			SimpleAudioEngine::getInstance()->playEffect("music/shel.wav");
		}
		break;
	default:
		break;
	}
}
void Slide::onKeyReleased(EventKeyboard::KeyCode code, Event* event) {
	switch (code)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
	case cocos2d::EventKeyboard::KeyCode::KEY_A:
		player->getPhysicsBody()->setVelocity(player->getPhysicsBody()->getVelocity() - Point(-200, 0));
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
	case cocos2d::EventKeyboard::KeyCode::KEY_D:
		player->getPhysicsBody()->setVelocity(player->getPhysicsBody()->getVelocity() - Point(200, 0));
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
	case cocos2d::EventKeyboard::KeyCode::KEY_W:
		player->getPhysicsBody()->setVelocity(player->getPhysicsBody()->getVelocity() - Point(0, 200));
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
	case cocos2d::EventKeyboard::KeyCode::KEY_S:
		player->getPhysicsBody()->setVelocity(player->getPhysicsBody()->getVelocity() - Point(0, -200));
		break;
	default:
		break;
	}
}

void Slide::bleedNew(int lose) {
	if (defence) return;
	bleed -= lose;
	if (bleed < 0) bleed = 0;
	lifeLayer->setBloodPercentage((float)bleed / (float)3);
	lifeLayer->setLabelBlood(bleed);
	if (bleed <= 30) lifeLayer->setBloodColor(Color3B::RED);
	if (bleed <= 0) gameOver();
}

void Slide::addBonus() {
	auto location = Vec2(0, 0);
	switch (rand() % 4)
	{
	case 0:
		location.y = visibleSize.height;
		location.x = rand() % (int)(visibleSize.width);
		break;
	case 1:
		location.x = visibleSize.width;
		location.y = rand() % (int)(visibleSize.height);
		break;
	case 2:
		location.y = 0;
		location.x = rand() % (int)(visibleSize.width);
		break;
	case 3:
		location.x = 0;
		location.y = rand() % (int)(visibleSize.height);
		break;
	default:
		break;
	}

	char path[30];
	int tag;
	int cbit;
	switch (rand() % 3)
	{
	case 0:
		sprintf(path, "defence.png");
		tag = 5;
		cbit = 0x00000010;
		break;
	case 1:
		sprintf(path, "bomb1.png");
		tag = 15;
		cbit = 0x00000100;
		break;
	default:
		sprintf(path, "defend.png");
		tag = 25;
		cbit = 0x00001000;
		break;
	}
	auto re = Sprite::create(path);
	re->setPhysicsBody(PhysicsBody::createCircle(30));
	re->getPhysicsBody()->setTag(tag);
	re->getPhysicsBody()->setCategoryBitmask(cbit);
	re->getPhysicsBody()->setContactTestBitmask(cbit);
	re->setAnchorPoint(Vec2(0.5, 0.5));
	re->setPosition(location);
	re->getPhysicsBody()->setVelocity((Point(rand() % (int)(visibleSize.width - 100) + 50, rand() % (int)(visibleSize.height - 100) + 50) - location) * difficalty);
	addChild(re, 2);
}

void Slide::nodefence(float f) {
	defence = false;
	auto texture = CCTextureCache::sharedTextureCache()->addImage("player2.png");
	player->setTexture(texture);
}

void Slide::escape() {
	for each (PhysicsBody* s in enemys)
	{
		if (s == nullptr) continue;
		s->removeFromWorld();
		((Sprite*)s->getNode())->removeFromParentAndCleanup(true);
	}
	enemys.clear();
	this->unschedule(schedule_selector(Slide::update));
	MoveTo * moveTo = MoveTo::create(1.0f, visibleSize / 2);
	ScaleTo * scaleTo = ScaleTo::create(1.0f, 0);
	FadeOut * fadeOut = FadeOut::create(1.0f);
	Spawn * spawn = Spawn::create(moveTo->clone(), scaleTo->clone(), fadeOut->clone(), NULL);
	player->runAction(spawn);
	player->getPhysicsBody()->setVelocity(Vec2(0, 0));

	if (difficalty > 0.6)
		SimpleAudioEngine::getInstance()->stopBackgroundMusic("music/bg_music3.mp3");
	else if (difficalty > 0.5)
		SimpleAudioEngine::getInstance()->stopBackgroundMusic("music/bg_music2.mp3");
	else if (difficalty > 0.4)
		SimpleAudioEngine::getInstance()->stopBackgroundMusic("music/bg_music1.mp3");
	else
		SimpleAudioEngine::getInstance()->stopBackgroundMusic("music/bg_music0.mp3");

	SimpleAudioEngine::getInstance()->playEffect("music/win.mp3");
	Director::getInstance()->getEventDispatcher()->removeEventListener(this->getTouchListener());
	DialogLayer::showDialogLayer(this, "ESCAPE from another black hole!!", "", NULL,"Escape", menu_selector(Slide::Return), DialogLayer::STATUS::WIN);

}

void Slide::gameOver() {
	for each (PhysicsBody* s in enemys)
	{
		if (s->getNode() == nullptr) continue;
		s->removeFromWorld();
		((Sprite*)s->getNode())->removeFromParentAndCleanup(true);
	}
	enemys.clear();
	this->unschedule(schedule_selector(Slide::update));

	// 失败后飞船爆炸并且消失
	auto ps = ParticleSystemQuad::create("explo.plist");
	ps->setPosition(player->getPosition());
	this->addChild(ps);
	player->removeFromParentAndCleanup(true);

	// 停止音乐
	if (difficalty > 0.6)
		SimpleAudioEngine::getInstance()->stopBackgroundMusic("music/bg_music3.mp3");
	else if (difficalty > 0.5)
		SimpleAudioEngine::getInstance()->stopBackgroundMusic("music/bg_music2.mp3");
	else if (difficalty > 0.4)
		SimpleAudioEngine::getInstance()->stopBackgroundMusic("music/bg_music1.mp3");
	else
		SimpleAudioEngine::getInstance()->stopBackgroundMusic("music/bg_music0.mp3");

	// 失败后停止计时
	leftTimeLayer->stopAction();

	SimpleAudioEngine::getInstance()->playEffect("music/fail.mp3");

	Director::getInstance()->getEventDispatcher()->removeEventListener(this->getTouchListener());
	DialogLayer::showDialogLayer(this, "You died in the black hole!!", "Play again", menu_selector(Slide::Restart), "Exit", menu_selector(Slide::Exit), DialogLayer::STATUS::FAIL);
}

void Slide::Return(Ref *ref) {
	SimpleAudioEngine::getInstance()->playBackgroundMusic("music/map_music0.mp3");
	Director::getInstance()->popScene();
}

void Slide::Restart(Ref *ref) {
	Director::getInstance()->getTextureCache()->removeUnusedTextures();
	CCAnimationCache::purgeSharedAnimationCache();
	CCSpriteFrameCache::sharedSpriteFrameCache()->removeUnusedSpriteFrames();
	SpriteFrameCache::getInstance()->removeUnusedSpriteFrames();
	SpriteFrameCache::getInstance()->removeSpriteFrames();

	Scene * scene = Adventure::scene();
	Director::getInstance()->replaceScene(scene);
}

void Slide::Exit(Ref *ref) {
	Director::getInstance()->getTextureCache()->removeUnusedTextures();
	CCAnimationCache::purgeSharedAnimationCache();
	CCSpriteFrameCache::sharedSpriteFrameCache()->removeUnusedSpriteFrames();
	SpriteFrameCache::getInstance()->removeUnusedSpriteFrames();
	SpriteFrameCache::getInstance()->removeSpriteFrames();

	Director::getInstance()->end();
}

//test

EventListenerPhysicsContact * Slide::getTouchListener() {
	return touchListener;
}

void Slide::InstanceMove() {
	if (instance_move_target != NULL) {
		instance_move_target->removeFromParentAndCleanup(true);
		instance_move_target = NULL;
	}
	player->setPosition(instanceMoveTarget);
}