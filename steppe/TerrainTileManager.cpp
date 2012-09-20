
#include "Globals.h"
#include "TerrainTileManager.h"

#define MAX_TILES 1000
#define MAX_VERTICES 1000

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
		}
	}

	node->isLeaf = true;
}

QuadTreeNode * NewQuadTreeNode()
{
	QuadTreeNode * newNode = new QuadTreeNode;
	newNode->isLeaf = true;

	TerrainTile * tile = gTerrainTileManager->idleTiles->top();
	gTerrainTileManager->idleTiles->pop();

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
		node->children[i]=NewQuadTreeNode();
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

	// Prepare root node
	gTerrainTileManager->rootNode = NewQuadTreeNode();
}

void TerrainTileManagerUpdate()
{
}

void TerrainTileManagerTearDown()
{
}


void TerrainTileManagerAllTiles(TerrainTile** outAllTiles, UINT *outNumTiles)
{
	TerrainTile *allTiles = (TerrainTile*)malloc(sizeof(TerrainTile)*MAX_TILES);

	UINT numTiles = 0;

	TraverseQuadTreeNode(gTerrainTileManager->rootNode, &allTiles, &numTiles);

	*outNumTiles = numTiles;
	outAllTiles = &allTiles;
}
