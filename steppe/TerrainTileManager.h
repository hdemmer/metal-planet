

#include <stack>

struct TerrainTile
{
	ID3D11Buffer * vertexBuffer;
};


void TerrainTileManagerSetup();
void TerrainTileManagerUpdate();
void TerrainTileManagerTearDown();

void TerrainTileManagerAllTiles(TerrainTile** allTiles, UINT *numTiles);

