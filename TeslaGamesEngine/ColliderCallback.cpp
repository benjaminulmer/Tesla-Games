#include "ColliderCallback.h"
#include "Vehicle.h"
#include "LapMarker.h"
#include "PickupBox.h"
#include <iostream>

using namespace std;

void ColliderCallback::onContact(const PxContactPairHeader & pairHeader, const PxContactPair * pairs, PxU32 nbPairs)
{
}

/*
WARNING WARNING WARNING WARNING

Due to how PhysX is coded, onTrigger will be called as long as the 2 objects are overlapping and moving! This means a vehicle moving
through a trigger volume will call it MULTIPLE TIMES (once per frame if they overlap)! Please code logic in here to check if you have 
already done the check for your event!

*/
void ColliderCallback::onTrigger(PxTriggerPair * pairs, PxU32 count)
{

	//NOTE: The otherActor and triggerActor are the vehicle(otherActor) and object(triggerActor) that collided


	for (PxU32 i = 0; i < count; i++)
	{
		// ignore pairs when shapes have been deleted
		if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
			continue;

		//vehicle and pickup box 
		if (strcmp(pairs[i].otherActor->getName(), "vehicle") == 0 && strcmp(pairs[i].triggerActor->getName(), "pickup") == 0)
		{
			Vehicle* v = (Vehicle*)pairs[i].otherActor->userData;	//this holds a ptr to the actual Vehicle object
			PickupBox* p = (PickupBox*)pairs[i].triggerActor->userData;	//this holds a ptr to the actual PickupBox object

			//if the box hasnt been picked up yet
			if (!p->getIsPicked()) {	//this avoids hitting the same box multiple times
				cout << "\nTrigger Block: Pickup Box\n";
				p->setIsPicked();
				v->pickup();
			}
			
			//otherwise ignore the box
			
			//I don't know how to remove it from triggerActor once it is hit
			//and now every hit will pick up a bunch of items, I guess it's the same reason you mentioned :(

			//add logic here
		}
		//vehicle and lap marker/counter
		else if (strcmp(pairs[i].otherActor->getName(), "vehicle") == 0 && strcmp(pairs[i].triggerActor->getName(), "lapmarker") == 0) {
			//cout << "\nTrigger Block: Lap Marker\n";

			Vehicle* v = (Vehicle*)pairs[i].otherActor->userData;
			LapMarker* l = (LapMarker*)pairs[i].triggerActor->userData;

			// std::cout << "LAP MARKER VALUE: " << l->markerValue << std::endl;

			//hardcoded number of laps and markers
			v->hitLapMarker(l->markerValue, 3, 12);	//3 laps, 12 markers (0->11)
			
		}
		//wont do yet, unsure how we want to handle the traps classes
		else if (strcmp(pairs[i].otherActor->getName(), "vehicle") == 0 && strcmp(pairs[i].triggerActor->getName(), "caltrops") == 0) {
			cout << "\nTrigger Block: Caltrops\n";

			Vehicle* v = (Vehicle*)pairs[i].otherActor->userData;
			Caltrops* c = (Caltrops*)pairs[i].triggerActor->userData;

			//should do damage (1pt) and should not hit the player it was placed by
			if (v->ID != c->id) {
				//do damage
				v->getDamage(1);
			}
		}
	}
}

void ColliderCallback::onSleep(PxActor ** actors, PxU32 count)
{
}

void ColliderCallback::onWake(PxActor ** actors, PxU32 count)
{
}

void ColliderCallback::onConstraintBreak(PxConstraintInfo * constraints, PxU32 count)
{
}

void ColliderCallback::onAdvance(const PxRigidBody * const * bodyBuffer, const PxTransform * poseBuffer, const PxU32 count)
{
}
