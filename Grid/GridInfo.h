#ifndef GRIDINFO_H
#define GRIDINFO_H

#include <vector>
#include <sparsehash/dense_hash_set>

namespace ssvsc
{
	class Body;
	class Grid;
	class Cell;

	class GridInfo
	{
		private:
			Grid& grid;
			Body& body;

			std::vector<Cell*> cells;
			std::vector<std::vector<Body*>*> queries; // Cell vector ptrs to query
			int startX{0}, startY{0}, endX{0}, endY{0}; // Edge cell positions
			bool invalid{false}; // IF TRUE CRASHES ON START - MUST FIX!!!

			void calcEdges(); // Sets startX, startY, endX, endY
			void checkEdges(); // Checks if startXY... is different from previousStartXY... - if so, recalculates
			void calcCells(); // Clears cells/queries and gets new ones
			void clear();

		public:
			GridInfo(Grid& mGrid, Body& mBody);
			~GridInfo();

			void invalidate();
			void preUpdate();
			void postUpdate();
			google::dense_hash_set<Body*> getBodiesToCheck();
	};
}

#endif // GRIDINFO_H
