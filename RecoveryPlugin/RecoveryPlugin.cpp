#include "RecoveryPlugin.h"
#include "utils\parser.h"

#include "bakkesmod\wrappers\tutorialwrapper.h"

BAKKESMOD_PLUGIN(RecoveryPlugin, "Recovery plugin", "0.1", PLUGINTYPE_FREEPLAY)


void RecoveryPlugin::onLoad()
{
	cvarManager->registerCvar("recovery_bumpspeed_angular", "(0, 6)", "How hard you will get thrown rotationally", true, true, 0.f, true, 10.f);
	cvarManager->registerCvar("recovery_bumpspeed_linear", "(800, 1100)", "How hard you will get thrown", true, true, 0.f, true, 1999.9);
	cvarManager->registerCvar("recovery_bumpspeed_linear_z", "(400, 700)", "How hard you will get thrown (height)", true, true, 0.f, true, 1999.9);
	cvarManager->registerCvar("recovery_cooldown", "(3000, 6000)", "Minimum time to wait after a bump", true, true, 0.f, true, 120000.f);

	cvarManager->registerNotifier("recovery_start", [this](std::vector<string> params) {
		if (!gameWrapper->IsInTutorial())
		{
			cvarManager->log("You need to be in freeplay to use this plugin.");
			return;
		}
		lastCooldownTime = cvarManager->getCvar("recovery_cooldown").getFloatValue() / 1000;
		this->recoveryEnabled = true;
		this->CheckForBump();
	});

	cvarManager->registerNotifier("recovery_stop", [this](std::vector<string> params) {
		this->recoveryEnabled = false;
	});

}

void RecoveryPlugin::onUnload()
{
	this->recoveryEnabled = false;
}

void RecoveryPlugin::CheckForBump()
{
	if (!recoveryEnabled || !gameWrapper->IsInTutorial())
		return; //Player stopped recovery training or left freeplay

	gameWrapper->SetTimeout([this](GameWrapper* gw) {
		this->CheckForBump();
	}, this->GetBumpTimeout());
}

float RecoveryPlugin::GetBumpTimeout()
{
	if (!gameWrapper->IsInTutorial() || !recoveryEnabled)
		return .5f;
	TutorialWrapper training = gameWrapper->GetGameEventAsTutorial();
	float lastBump = training.GetSecondsElapsed() - lastBumpTime;
	if (lastBump > lastCooldownTime)
	{
		auto gameCar = training.GetGameCar();
		
		if (gameCar.GetIsOnGround() || gameCar.GetIsOnWall()) //player has landed
		{
			ExecuteBump();
			return lastCooldownTime;
		}
		return random(.2f, 1.f);
	}
	return lastCooldownTime - lastBump + 0.1f;
}

void RecoveryPlugin::ExecuteBump()
{
	if (!gameWrapper->IsInTutorial() || !recoveryEnabled)
		return;
	auto tutorial = gameWrapper->GetGameEventAsTutorial();



	Vector angularBump = {
		cvarManager->getCvar("recovery_bumpspeed_angular").getFloatValue(),
		cvarManager->getCvar("recovery_bumpspeed_angular").getFloatValue(),
		cvarManager->getCvar("recovery_bumpspeed_angular").getFloatValue()
	};
	Vector linearBump = {
		cvarManager->getCvar("recovery_bumpspeed_linear").getFloatValue(),
		cvarManager->getCvar("recovery_bumpspeed_linear").getFloatValue(),
		cvarManager->getCvar("recovery_bumpspeed_linear_z").getFloatValue()
	};


	angularBump.X = random(0, 1) == 1  ? angularBump.X : -angularBump.X;
	angularBump.Y = random(0, 1) == 1 ? angularBump.Y : -angularBump.Y;

	linearBump.X = random(0, 1) == 1 ? linearBump.X : -linearBump.X;
	linearBump.Y = random(0, 1) == 1 ? linearBump.Y : -linearBump.Y;

	auto car = tutorial.GetGameCar();
	car.SetAngularVelocity(angularBump);
	car.SetVelocity(linearBump);


	lastBumpTime = tutorial.GetSecondsElapsed();
	lastCooldownTime = cvarManager->getCvar("recovery_cooldown").getFloatValue() / 1000;
}
