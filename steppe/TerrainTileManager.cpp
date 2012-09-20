
#include "Globals.h"
#include "TerrainTileManager.h"

#define MAX_TILES 1000
#define MAX_VERTICES 10000

struct QuadTreeNode
{
	bool isLeaf;
	QuadTreeNode * children[4];

	TerrainTile * tile;
};

struct TerrainTileManager
{
	QuadTreeNode * rootNode;

	std::stack<TerrainTile*> *idleTiles;
};

TerrainTileManager * gTerrainTileManager;

//////
// QuadTree

void TraverseQuadTreeNode(QuadTreeNode * node, TerrainTile *allTiles[], UINT *numTiles)
{
	if (node->isLeaf)
	{
		allTiles[*numTiles] = node->tile;
		UINT i = *numTiles;
		*numTiles = i + 1;
		i = *numTiles;
	} else {
		for (int i=0; i<4; i++)
		{
			TraverseQuadTreeNode(node->children[i],allTiles,numTiles);
		}
	}
}

void CollapseQuadTreeNode(QuadTreeNode * node)
{
	if (node->isLeaf)
		return;

	for (int i=0;i<4;i++)
	{
		CollapseQuadTreeNode(node->children[i]);
		if (node->children[i]->tile)
		{
			gTerrainTileManager->idleTiles->push(node->children[i]->tile);
			node->children[i]->tile = NULL;
			free(node->children[i]);
		}
	}

	node->isLeaf = true;
}

QuadTreeNode * NewRootQuadTreeNode()
{
	// Prepare root node
	QuadTreeNode * newRootNode = (QuadTreeNode*)malloc(sizeof(QuadTreeNode));
	newRootNode->isLeaf = true;
	TerrainTile * tile = gTerrainTileManager->idleTiles->top();
	gTerrainTileManager->idleTiles->pop();
	tile->origin = XMFLOAT2(0,0);
	tile->depth=0;
	GenerateTerrainTile(tile);
	newRootNode->tile = tile;
	gTerrainTileManager->rootNode = newRootNode;

	return newRootNode;
}

QuadTreeNode * NewQuadTreeNode(QuadTreeNode *parent, int childNum)
{
	QuadTreeNode * newNode = (QuadTreeNode*)malloc(sizeof(QuadTreeNode));
	newNode->isLeaf = true;

	TerrainTile * tile = gTerrainTileManager->idleTiles->top();
	gTerrainTileManager->idleTiles->pop();

	int xOff = 0;
	int yOff = 0;
	switch (childNum)
	{
	case 1:
		xOff=1;
		break;
	case 2:
		yOff=1;
		break;
	case 3:
		xOff=1;
		yOff=1;
		break;
	}

	UINT depth = parent->tile->depth + 1;

	float scaledTileSize = TILE_BASE_SIZE / (float)(1<<depth);
	tile->origin = XMFLOAT2(parent->tile->origin.x + scaledTileSize*xOff, parent->tile->origin.y + scaledTileSize*yOff);
	tile->depth = depth;
	GenerateTerrainTile(tile);

	newNode->tile = tile;

	return newNode;
}

void SplitQuadTreeNode(QuadTreeNode *node)
{
	if (!node->isLeaf)
		return;

	if (gTerrainTileManager->idleTiles->size() < 4)
	{
		printf("Not enough free tiles left for split.");

		return;
	}

	for (int i=0;i<4;i++)
	{
		node->children[i]=NewQuadTreeNode(node, i);
		// TODO: if any of these fail, we have run out of memory
	}

	node->isLeaf = false;
}

////////
// Terrain Tile Manager

void TerrainTileManagerSetup()
{
	gTerrainTileManager = new TerrainTileManager();

	gTerrainTileManager->idleTiles = new std::stack<TerrainTile*>();

	D3D11_BUFFER_DESC streamOutBufferDesc;

	streamOutBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	streamOutBufferDesc.ByteWidth = sizeof(DeferredVertexType) * MAX_VERTICES;
	streamOutBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	streamOutBufferDesc.CPUAccessFlags = 0;
	streamOutBufferDesc.MiscFlags = 0;
	streamOutBufferDesc.StructureByteStride = 0;

	for (int i=0;i<MAX_TILES;i++)
	{
		ID3D11Buffer * vertexBuffer;
		dev->CreateBuffer(&streamOutBufferDesc, NULL, &vertexBuffer);
		TerrainTile * tile = new TerrainTile();
		tile->vertexBuffer = vertexBuffer;
		gTerrainTileManager->idleTiles->push(tile);
	}

	gTerrainTileManager->rootNode = NULL;
}

void TerrainTileManagerUpdate()
{
	if (!gTerrainTileManager->rootNode)
		gTerrainTileManager->rootNode = NewRootQuadTreeNode();

	if (gTerrainTileManager->rootNode->isLeaf)
	{
		SplitQuadTreeNode(gTerrainTileManager->rootNode);
	}else {
		CollapseQuadTreeNode(gTerrainTileManager->rootNode);
	}
}

void TerrainTileManagerTearDown()
{
}


void TerrainTileManagerAllLeafTiles(TerrainTile** outAllTiles[], UINT *outNumTiles)
{
	TerrainTile **allTiles = (TerrainTile**)malloc(sizeof(TerrainTile*)*MAX_TILES);

	UINT numTiles = 0;

	TraverseQuadTreeNode(gTerrainTileManager->rootNode, allTiles, &numTiles);

	*outNumTiles = numTiles;
	*outAllTiles = allTiles;
}
