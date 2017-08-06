#include "ui/CocosGUI.h"
#include "GameOverScene.h"
#include "MainGame.h"
#include "AudioEngine.h"
#include "Definitions.h"
#include "TextureBlur.h"
#include "2d/CCRenderTexture.h"




//using namespace ui;
USING_NS_CC;


Scene* MainGame::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = MainGame::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

// on "init" you need to initialize your instance
bool MainGame::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();                                //! Для расстановки изображений
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	Size screenSize = Director::getInstance()->getWinSize();

	background = Sprite::create("bg.png");																//! Добавляем фон
	meat = MenuItemImage::create("meat.png", "meat.png", CC_CALLBACK_1(MainGame::Points, this));   //! Это должен быть кусок мяса ;
	men2 = Sprite::create("menturning.png");
	plate = Sprite::create("plate.png");
	table = Sprite::create("table.png");
	
	


	men2->setSpriteFrame(SpriteFrame::create("menturning.png", Rect(0, 0, 300.0f, 800.0f)));

	menu = Menu::create(meat, NULL);                      //? Переделать потом - пока так
	menu->setEnabled(false);								  //

	score = 0;   //! Устанавливаем счет

	__String *tempScore = __String::createWithFormat("TAP TO PLAY!");   //! Создаём надпись "Tap To Play!"

	scoreLabel = Label::createWithTTF(tempScore->getCString(), "Archive.otf", visibleSize.height * SCORE_FONT_SIZE);

	MenRed = SpriteFrame::create("men2.png", Rect(0, 0, 300.0f, 800.0f));


	cocos2d::experimental::AudioEngine::play2d("BirdSingingSFX.mp3", true, 1.0f);


	//!!!!! Создаём анимацию надписи "Tap To Play!"
	auto TapLabelFadeOut = FadeOut::create(2.0f);
	auto TapLabelFadeIn = FadeIn::create(1.5f);
	auto DelayTapToPlay = DelayTime::create(2.0f);
	auto TapLabelSequence = Sequence::create(TapLabelFadeOut, DelayTapToPlay, TapLabelFadeIn, nullptr);    //! Создаём сиквенцию с нашими действиями, "анимацию"

	repeatTapLabel = RepeatForever::create(TapLabelSequence);     //! Зацикливаем "анимацию" надписи

	TapLabelAnimation();  //! запускаем анимацию (Вызываем функцию 1 раз)
						  //!!!!! Создаём анимацию надписи "Tap To Play!"



	//!!!!! Создаем TouchEvent - при нажатии по экрану - игра запускается
	contactListener = EventListenerTouchOneByOne::create();
	contactListener->setSwallowTouches(true);
	contactListener->setEnabled(true);
	contactListener->onTouchBegan = CC_CALLBACK_1(MainGame::onCantactBegin, this);   //! Должен отключить возможность нажатия после запуска функции "onCantactBegin"
	contactListener->onTouchMoved = CC_CALLBACK_1(MainGame::Points, this);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(contactListener, this);
	//!!!!! Создаем TouchEvent - при нажатии по экрану - игра запускается



	//!!!!!!! Создаем TouchEvent - при MoveTo ( в любой части экрана ) убираем старую тарелку, спавним мясо 
	platemoveevent = EventListenerTouchOneByOne::create();
	platemoveevent->setSwallowTouches(true);
	platemoveevent->setEnabled(false);
	platemoveevent->onTouchBegan = [](Touch* touch, Event* event) 
	{
		return true; 
	};
	platemoveevent->onTouchMoved = CC_CALLBACK_1(MainGame::spawnmeat, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(platemoveevent, plate);
	//!!!!!!! Создаем TouchEvent - при MoveTo ( в любой части экрана ) убираем старую тарелку, спавним мясо 



	scoreLabel->setColor(Color3B::BLACK);

	scoreLabel->setPosition(Point(visibleSize.width / 2 + origin.x, visibleSize.height * 0.75f + origin.y));
	background->setPosition(Point(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
	menu->setPosition(Point(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y - 270));
	table->setPosition(Point(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y  + 50));
	men2->setPosition(Point(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
	plate->setScale(0.85f);
	plate->setPosition(Point(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y - 290));

	//meat->setScale(0.75f);
	//table->setScale(0.75f);
	//background->setScale(0.75f);

	MainGame::addChild(scoreLabel, 5);
	MainGame::addChild(background, 1);
	MainGame::addChild(table, 4);
	MainGame::addChild(menu, 6);
	MainGame::addChild(men2, 3);
	MainGame::addChild(plate, 5);

	MainGame::scheduleUpdate();

	return true;
}
//! Разoрбраться с -> ::  .

void MainGame::TapLabelAnimation()            //! Запускаем зацикленную анимацию надписи "Tap To Play!"
{
	scoreLabel->runAction(repeatTapLabel);
}


bool MainGame::onCantactBegin(cocos2d::Touch *touch)      //! Запускаем игру после нажатия по экрану
{
	stopAction(repeatTapLabel);								//! Выключаем зацикленную анимацию надписи "Tap to play!"
	scoreLabel->setOpacity(255);							  // Устанавливаем прозрачность счетчика очков на 255
	scoreLabel->setString("0");								  // Устанавливаем счет на 0

	contactListener->setEnabled(false);					//! Запрещаем принимать "все" нажатия по экрану
	menu->setEnabled(true);       //? Включаем стол

	MainGame::PauseButtonCreate();				// Создание кнопки паузы
	MainGame::schedule(schedule_selector(MainGame::MenWatch), 6);				// Запускает врага

	return true;
}


void MainGame::PauseButtonCreate()        //! Создаём и добавляем на экран кнопку паузы
{

	auto visibleSize = Director::getInstance()->getVisibleSize();        //? Мб добавить их в хедер раз они везде юзаются????
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	auto button = cocos2d::ui::Button::create("GUI_button_pause_orange.png", "GUI_button_pause_orange-dark.png", "GUI_button_pause_orange-dark.png");  ////
	button->setPosition(Point(visibleSize.width / 2 * 1.75f, visibleSize.height / 2 * 1.75f));									  //  Создаём кнопку паузы и задаём ей параметры
	button->setScale(0.75f);																															 ////


	auto &buttonpause = button;			// Создаём ссылку для лямбда функции


	button->addTouchEventListener([&, buttonpause](Ref* sender, cocos2d::ui::Widget::TouchEventType type)                //! Функционал кнопки паузы
	{
		switch (type)
		{
		case cocos2d::ui::Widget::TouchEventType::BEGAN:
			log("Tap Start");

			menu->setEnabled(false);		// Выкчлючаем возможность есть мясо

			break;
		case cocos2d::ui::Widget::TouchEventType::MOVED:
			break;
		case cocos2d::ui::Widget::TouchEventType::ENDED:
			men2->pause();

			if (sfxOn)
			{
				cocos2d::experimental::AudioEngine::play2d("Tap.mp3", false, 1.0f);
			}

			scoreLabel->setOpacity(0);

			log("TAP ENDED BUTTON PAUSE");
			MainGame::PauseGame();
			MainGame::PauseMenuCreate();    //? Тут ошибка - создаю одно и тоже дохера раз похоже      - но так как функция локальная всё ок
											//? Игра должна ставить на паузу

			MainGame::removeChild(buttonpause, true);   // Удаляем с экрана кнопку паузы  ( только с экрана в пямити всё осталось )
			break;

		case cocos2d::ui::Widget::TouchEventType::CANCELED:
			break;

		default:
			break;
		}

	});

	MainGame::addChild(button, 12);		// Добавляем кнопку паузы на экран
}

void MainGame::CreditMenuCreate()
{
	auto visibleSize = Director::getInstance()->getVisibleSize();        //? Мб добавить их в хедер раз они везде юзаются????
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	auto credit = cocos2d::ui::Button::create("t-1.png", "t-1.png", "t-1.png");  ////
	credit->setPosition(Point(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));									  //  Создаём кнопку меню титров
	credit->setScale(0.75);																															 ////


	auto &creditlink = credit;			// Создаём ссылку для лямбда функции

	MainGame::addChild(credit, 6);		// Добавляем кнопку паузы на экран

	credit->addTouchEventListener([&, creditlink](Ref* sender, cocos2d::ui::Widget::TouchEventType type)                //! Функционал меню титров
	{
		switch (type)
		{
		case cocos2d::ui::Widget::TouchEventType::BEGAN:
			log("Credit Tap Statred");
			break;
		case cocos2d::ui::Widget::TouchEventType::MOVED:
			break;

		case cocos2d::ui::Widget::TouchEventType::ENDED:
			log("Credit Tap Ended");
			MainGame::removeChild(creditlink, true);   // Удаляем с экрана кнопку паузы  ( только с экрана в пямити всё осталось )
			MainGame::PauseMenuCreate();    //? Тут ошибка - создаю одно и тоже дохера раз похоже      - но так как функция локальная всё ок
											//? Игра должна ставить на паузу

			break;

		case cocos2d::ui::Widget::TouchEventType::CANCELED:
			break;

		default:
			break;
		}

	});




}


void MainGame::PauseMenuCreate()					//! Создание меню паузы
{
	auto visibleSize = Director::getInstance()->getVisibleSize();				//? Мб добавить их в хедер раз они везде юзаются????
	Vec2 origin = Director::getInstance()->getVisibleOrigin();


	// Создаём все кнопки
	auto resume = cocos2d::ui::Button::create("GUI_button_play_orange.png", "GUI_button_play_orange-dark.png", "GUI_button_play_orange.png");
	auto sfx = cocos2d::ui::CheckBox::create("SFXON.png",
		"SFXTAP.png",
		"SFXOFF.png",
		"GUI_button_play_orange.png",
		"GUI_button_play_orange.png");


	// Создаём ссылки для лямбдо функции
	auto &sfxlink = sfx;
	auto &resumelink = resume;

	// Устанавливаем позиции и размер
	sfx->setPosition(Point(visibleSize.width / 2 * 1.75, visibleSize.height / 2 * 1.75));
	sfx->setScale(1.00);

	resume->setPosition(Point(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
	resume->setScale(0.75);

	// Добавляем на экран все кнопки
	this->addChild(sfx, 12);
	this->addChild(resume, 12);



	resume->addTouchEventListener([&, resumelink, sfxlink](Ref* sender, cocos2d::ui::Widget::TouchEventType type)		//! Функционал кнопки "продолжить"
	{
		switch (type)
		{
		case cocos2d::ui::Widget::TouchEventType::BEGAN:
			log("TAP BEGAN BUTTON PLAY");
			break;
		case cocos2d::ui::Widget::TouchEventType::MOVED:
			break;
		case cocos2d::ui::Widget::TouchEventType::ENDED:
			log("TAP ENDED BUTTON PLAY");
			MainGame::removeChild(resumelink, true);
			MainGame::removeChild(sfxlink, true);
			//   MainGame::removeChild(pausebg, true);           ОШИБКА БАГ ТЫ ПИДОР ПЕТЯ
			//? Продолжить игру здесь надо
			menu->setEnabled(true);						// Включаем возможность "есть" мясо
			scoreLabel->setOpacity(255);
			PauseButtonCreate();					//  Создаём кнопку паузы
			men2->resume();
			MainGame::PauseGame();
			break;
		case cocos2d::ui::Widget::TouchEventType::CANCELED:
			break;
		default:
			break;
		}

	});




	if (sfxOn == false)				///Графически настраиваем вид кнопки
	{
		sfx->setSelected(true);
	}

	sfx->addTouchEventListener([&, sfxlink](Ref* sender, cocos2d::ui::Widget::TouchEventType type) //! Функционал регулятора музыки
	{
		switch (type)
		{
		case ui::Widget::TouchEventType::BEGAN:
			break;
		case ui::Widget::TouchEventType::ENDED:
			log("AUDIO CLICED");

			if (sfxOn)
			{
				sfxOn = false;
				cocos2d::experimental::AudioEngine::pauseAll();					//? Тут чет все криво
				log("SFX OFF");

			}
			else
			{
				sfxOn = true;
				cocos2d::experimental::AudioEngine::resumeAll();				//? Тут тоже не очень
				log("SFX ONNN");
			}


			break;
		default:
			break;
		}
	});


}

void MainGame::PauseGame()   //? Размытие по Гауссу и пауза ( но мб это объеденю в одну функцию )
{
	if (gameplaying)
	{
		// Director::getInstance()->pause();
		gameplaying = false;

		auto visibleSize = Director::getInstance()->getVisibleSize();                                //! Для расстановки изображений
		Vec2 origin = Director::getInstance()->getVisibleOrigin();

		Size screenSize = Director::getInstance()->getWinSize();

		//men2->retain();
		//menu->retain();
		//table->retain();
		//plate->retain();

		auto rendert = RenderTexture::create(screenSize.width, screenSize.height);
		rendert->retain();
		rendert->begin();
		background->visit();
		men2->visit();
		table->visit();
		plate->visit();
		menu->visit();
		rendert->end();

		TextureBlur::initShader(screenSize / 1.0f);
		auto texture = TextureBlur::create(rendert->getSprite()->getTexture() , 1.1f, 1);
		auto bluredbg = Sprite::createWithTexture(texture);
		bluredbg->getTexture()->setAntiAliasTexParameters();
		bluredbg->setPosition(Point(visibleSize.width / 2, visibleSize.height / 2));
		bluredbg->setScale(1.0f);
		bluredbg->setFlippedY(true);
		pausebg = bluredbg;
		addChild(pausebg, 7);

	}
	else
	{
		// Director::getInstance()->resume();
		gameplaying = true;
		removeChild(pausebg, true);
	}

}

void MainGame::Points(cocos2d::Ref *sender) //! Должен считать результат, уменьшать мясо, и проверять не проиграл ли игрок
{

	auto t = ScaleTo::create(0.5, meat->getScale() * 0.85);			// Анимация уменьшения мяса

	auto d = ScaleTo::create(0.5, meat->getScale() * 0.0);


	if (men2->getSpriteFrame() != MenRed)		// Проверяет является ли текущий кадр, кадром при котором игрок проигрывает, если нет - добовляет очков
	{
		if (meat->getScale() > 0.4)
		{
			meat->runAction(t);
		}
		else
		{
			platemoveevent->setEnabled(true);
			meat->runAction(d);
			//meat->setScale(1.0);
		}
		score++;

		__String *tempScore = __String::createWithFormat("%i", score);

		scoreLabel->setString(tempScore->getCString());

	}

	else
	{

		auto scene = GameOver::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(TRANSITION_TIME, scene));
	}

}


bool MainGame::spawnmeat(cocos2d::Touch *touch)
{
	auto visibleSize = Director::getInstance()->getVisibleSize();                                //! Для расстановки изображений
	Vec2 origin = Director::getInstance()->getVisibleOrigin();



	auto RemovePlate = MoveTo::create(1.0f, Vec2(plate->getPositionX() * -1.25f, plate->getPositionY()));
	auto TeleoprtPlate = MoveTo::create(0.0f, Point((visibleSize.width / 2 + origin.x) * 2.5f, visibleSize.height / 2 + origin.y));
	auto SetLocationPlate = MoveTo::create(0.5f, Point(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y - 290));
	auto SetLocationMeat = MoveTo::create(0.5f, Point(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y - 270));
	auto delay = DelayTime::create(1.0f);
	menu->setPosition(Point((visibleSize.width / 2 + origin.x) * 2.5f, visibleSize.height / 2 + origin.y));
	meat->setScale(1.0f);
	plate->runAction(Sequence::create(RemovePlate, TeleoprtPlate, SetLocationPlate, nullptr));
	menu->runAction(Sequence::create(delay, SetLocationMeat, nullptr));

	platemoveevent->setEnabled(false);
	return true;

}
void MainGame::MenWatch(float dt)  //! Должен генерировать поведение противника
{
	Vector<SpriteFrame*> turningAnimation;

	MenRed = SpriteFrame::create("men2.png", Rect(0, 0, 300, 800));		// Положение мужика , когда он смотрит за игроком

	turningAnimation.pushBack(SpriteFrame::create("men1.png", Rect(0, 0, 300, 800)));
	turningAnimation.pushBack(SpriteFrame::create("menturned.png", Rect(0, 0, 300, 800)));
	turningAnimation.pushBack(MenRed);
	Animation* animation = Animation::createWithSpriteFrames(turningAnimation, 1.5f);
	Animate* animateTurning = Animate::create(animation);


	Vector<SpriteFrame*> fastTurning;

	fastTurning.pushBack(MenRed);
	fastTurning.pushBack(SpriteFrame::create("menturned.png", Rect(0, 0, 300, 800)));
	Animation* animation2 = Animation::createWithSpriteFrames(fastTurning, 1.5f);
	Animate* animateFTurning = Animate::create(animation2);


	Vector<SpriteFrame*> Relax;
	Relax.pushBack(SpriteFrame::create("menturning.png", Rect(0, 0, 300, 800)));
	Relax.pushBack(SpriteFrame::create("men1.png", Rect(0, 0, 300, 800)));
	Animation* animation3 = Animation::createWithSpriteFrames(Relax, 1.5f);
	Animate* animateRelax = Animate::create(animation3);

	int z = random(0, 2);
	switch (z)
	{
	case 0:
		log("Turning");
		men2->runAction(animateTurning);
		break;
	case 1:
		men2->runAction(animateRelax);
		log("Relax");
		break;
	case 2:
		men2->runAction(animateFTurning);
		log("FTunring");
		break;
	}
	log("%i", z);

}
