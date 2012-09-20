

#include <stack>

struct TerrainTile
{
	ID3D11Buffer * vertexBuffer;

	D3DXVECTOR2 origin;
	
	UINT depth;
};

#define TILE_BASE_SIZE 10

void GenerateTerrainTile(TerrainTile*tile);

void TerrainTileManagerSetup();
void TerrainTileManagerUpdate();
void TerrainTileManagerTearDown();

void TerrainTileManagerAllLeafTiles(TerrainTile** allTiles[], UINT *numTiles);

