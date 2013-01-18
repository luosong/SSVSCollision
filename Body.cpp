#include "Body.h"
#include "World.h"
#include "Cell.h"

using namespace std;
using namespace sf;

namespace ssvsc
{
	Body::Body(World& mWorld, bool mIsStatic, Vector2i mPosition, int mWidth, int mHeight) :
		world(mWorld), isStatic{mIsStatic}, position{mPosition}, previousPosition{position}, halfSize{mWidth / 2, mHeight / 2} { }

	void Body::addGroups(const vector<string>& mGroups)
	{
		for(auto& group : mGroups) groups.push_back(group);
		mustRecalculate = true;
	}
	void Body::addGroupsToCheck(const vector<string>& mGroups)
	{
		for(auto& group : mGroups) groupsToCheck.push_back(group);
		mustRecalculate = true;
	}
	void Body::addGroupsNoResolve(const vector<string>& mGroups) { for(auto& group : mGroups) groupsNoResolve.push_back(group); }

	bool Body::isOverlapping(Body* mBody) { return getRight() > mBody->getLeft() && getLeft() < mBody->getRight() && (getBottom() > mBody->getTop() && getTop() < mBody->getBottom()); }
	void Body::update(float mFrameTime)
	{
		if(mustRecalculate) { checkCells(); recalculateCells(); }
		if(isStatic) return;

		Vector2f tempVelocity{velocity.x * mFrameTime, velocity.y * mFrameTime};
		Vector2f tempPosition{position.x + tempVelocity.x, position.y + tempVelocity.y};
		previousPosition = position;
		position = Vector2i(tempPosition.x, tempPosition.y);

		unordered_set<Body*> checkedBodies{this};

		// auto comp = [](Body* mBodyA, Body* mBodyB) -> bool { return mBodyA->getVelocity().x > 0 ? mBodyA->getX() > mBodyB->getX() : -mBodyA->getX() > mBodyB->getX(); };
		// set<Body*, decltype(comp)> bodiesToCheck(comp);
		// for(Body* body : getBodiesToCheck()) bodiesToCheck.insert(body);
		//bodiesToCheck.OrderBy(x => Velocity.X > 0 ? x.X : -x.X)

		vector<Body*> bodiesToCheck;
		for(auto& query : queries) for(auto& body : *query) bodiesToCheck.push_back(body);

		for(auto& body : bodiesToCheck)
		{
			if(checkedBodies.find(body) != checkedBodies.end()) continue;
			checkedBodies.insert(body);

			if(!isOverlapping(body)) continue;

			onCollision({body, mFrameTime, body->getUserData()});
			body->onCollision({this, mFrameTime, userData}); // ? Y/N

			bool mustResolve{true};
			for(auto& group : groupsNoResolve)
				if(find(begin(body->getGroups()), end(body->getGroups()), group) != end(body->getGroups())) { mustResolve = false; break; }

			if(mustResolve) resolve(body);
		}

		checkCells();
	}

	void Body::resolve(Body* mBody)
	{
		int encrX{0}, encrY{0};

		if (getBottom() < mBody->getBottom() && getBottom() >= mBody->getTop()) encrY = mBody->getTop() - getBottom();
		else if (getTop() > mBody->getTop() && getTop() <= mBody->getBottom()) encrY = mBody->getBottom() - getTop();

		if (getLeft() < mBody->getLeft() && getRight() >= mBody->getLeft()) encrX = mBody->getLeft() - getRight();
		else if (getRight() > mBody->getRight() && getLeft() <= mBody->getRight()) encrX = mBody->getRight() - getLeft();

		int overlapX{getLeft() < mBody->getLeft() ? getRight() - mBody->getLeft() : mBody->getRight() - getLeft()};
		int overlapY{getTop() < mBody->getTop() ? getBottom() - mBody->getTop() : mBody->getBottom() - getTop()};

		if(overlapX > overlapY) position.y += encrY; else position.x += encrX;
	}

	void Body::checkCells()
	{
		startX = getLeft() / world.getCellSize();
		startY = getTop() / world.getCellSize();
		endX = getRight() / world.getCellSize();
		endY = getBottom() / world.getCellSize();

		previousStartX = (previousPosition.x - halfSize.x) / world.getCellSize();
		if(previousStartX != startX) { recalculateCells(); return; }

		previousStartY = (previousPosition.y - halfSize.y) / world.getCellSize();
		if(previousStartY != startY) { recalculateCells(); return; }

		previousEndX = (previousPosition.x + halfSize.x) / world.getCellSize();
		if(previousEndX != endX) { recalculateCells(); return; }

		previousEndY = (previousPosition.y + halfSize.y) / world.getCellSize();
		if(previousEndY != endY) { recalculateCells(); return; }
	}

	void Body::clearCells()
	{
		for(auto& cell : cells) cell->del(this);
		cells.clear(); queries.clear();
	}
	void Body::recalculateCells()
	{
		clearCells();
		
		if(startX < 0 || endX >= world.getColumns() || startY < 0 || endY >= world.getRows()) { onOutOfBounds(); }
		for(int iY{startY}; iY <= endY; iY++) for(int iX{startX}; iX <= endX; iX++) cells.push_back(world.getCell(iX + world.getOffset(), iY + world.getOffset()));

		for(auto& cell : cells)
		{
			cell->add(this);
			for(auto& group : groupsToCheck) queries.push_back(cell->getQuery(group));
		}
		
		mustRecalculate = false;
	}

	// Properties
	void Body::setPosition(Vector2i mPosition) 			{ position = mPosition; }
	void Body::setVelocity(Vector2i mVelocity) 			{ velocity = mVelocity; }
	const vector<string>& Body::getGroups()				{ return groups; }
	const vector<string>& Body::getGroupsToCheck()		{ return groupsToCheck; }
	Vector2i Body::getPosition() 						{ return position; }
	Vector2i Body::getVelocity() 						{ return velocity; }
	void* Body::getUserData()							{ return userData; }
	int Body::getX() 									{ return position.x; }
	int Body::getY() 									{ return position.y; }
	int Body::getLeft() 								{ return position.x - halfSize.x; }
	int Body::getRight() 								{ return position.x + halfSize.x; }
	int Body::getTop() 									{ return position.y - halfSize.y; }
	int Body::getBottom() 								{ return position.y + halfSize.y; }
	int Body::getHalfWidth() 							{ return halfSize.x; }
	int Body::getHalfHeight() 							{ return halfSize.y; }
	int Body::getWidth() 								{ return halfSize.x * 2; }
	int Body::getHeight() 								{ return halfSize.y * 2; }
}

