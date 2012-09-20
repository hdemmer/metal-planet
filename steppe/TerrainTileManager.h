

#include <stack>

struct TerrainTile
{
	ID3D11Buffer * vertexBuffer;

	XMFLOAT2 origin;
	
	UINT depth;
};

#define TILE_BASE_SIZE WORLD_SIZE

UINT TerrainExpectedBufferWidth();
void GenerateTerrainTile(TerrainTile*tile);

void TerrainTileManagerSetup();
void TerrainTileManagerUpdate();
void TerrainTileManagerTearDown();

void TerrainTileManagerAllLeafTiles(TerrainTile** allTiles[], UINT *numTiles);

