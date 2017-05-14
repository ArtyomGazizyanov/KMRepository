#include <vector>
#include <string>
#include <fstream>
#include <set>
#include <iostream>
#include <map>
#include <boost/functional/hash.hpp>
#include <boost/chrono.hpp>

using namespace std;

typedef std::vector<std::vector<std::size_t>> Matrix;
const std::size_t FIELD_SIZE = 4;

struct Position
{
	Position(std::size_t x, std::size_t y) :
		x(x),
		y(y)
	{
	}
	std::size_t x = 0;
	std::size_t y = 0;
};

struct Node
{
	std::size_t hash = 0;
	Matrix matrix;
	Position zeroPosition = Position(0, 0);
	Node *father = nullptr;
	size_t depth = 0;
	Matrix GetMatrix()
	{
		return matrix;
	}
	void CalculateZeroPosition()
	{
		for (std::size_t i = 0; i < matrix.size(); ++i)
		{
			for (std::size_t j = 0; j < matrix[i].size(); ++j)
			{
				if (matrix[i][j] == 0)
				{
					zeroPosition = Position(j, i);
					break;
				}
			}
		}
	}
	void GetHash()
	{
		size_t seed = 0;
		for (size_t i = 0; i < matrix.size(); ++i)
		{
			seed ^= boost::hash_range(matrix[i].begin(), matrix[i].end()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		hash = seed;
	}
};

map <size_t, vector<Node*>> manhattanMap;
vector<string> wayString;
int WAY_COUNTER= 0;
size_t openNodeCouner = 0;
size_t generatedNodeCouner = 0;


const Matrix WIN_MATRIX =
{
	{ 0, 1, 2, 3 },
	{ 4, 5, 6, 7 },
	{ 8, 9, 10, 11 },
	{ 12, 13, 14, 15 }
};

const std::map<size_t, std::pair<size_t, size_t>> WIN_POSITIONS =
{
	{ 0, { 0, 0 } },
	{ 1,{ 1, 0 } },
	{ 2,{ 2, 0 } },
	{ 3,{ 3, 0 } },
	{ 4,{ 0, 1 } },
	{ 5,{ 1, 1 } },
	{ 6,{ 2, 1 } },
	{ 7,{ 3, 1 } },
	{ 8,{ 0, 2 } },
	{ 9,{ 1, 2 } },
	{ 10,{ 2, 2 } },
	{ 11,{ 3, 2 } },
	{ 12,{ 0, 3 } },
	{ 13,{ 1, 3 } },
	{ 14,{ 2, 3 } },
	{ 15,{ 3, 3 } }
};

size_t GetManhattanDistance(const Matrix & matrix)
{
		size_t result = 0;
		for (size_t y = 0; y < matrix.size(); ++y)
		{
			for (size_t x = 0; x < matrix[y].size(); ++x)
			{
				if (WIN_POSITIONS.at(matrix[y][x]).first > x)
				{
					result += WIN_POSITIONS.at(matrix[y][x]).first - x;
				}
				else
				{
					result += x - WIN_POSITIONS.at(matrix[y][x]).first;
				}
				if (WIN_POSITIONS.at(matrix[y][x]).second > y)
				{
					result += WIN_POSITIONS.at(matrix[y][x]).second - y;
				}
				else
				{
					result += y - WIN_POSITIONS.at(matrix[y][x]).second;
				}
			}
		}
		return result;	
}

/*
получаем состояние из очереди
проверяем было ли оно уже рассмотренно
считаем манхэтонское расстоние до конечного сосотояния
заносим этот узел в map
запускаем самый верхний map.begin()*/


Matrix GetMatrix(std::ifstream & input)
{
	Matrix tmp;
	std::string str;
	getline(input, str);
	stringstream strStream;
	strStream << str;
	while (!strStream.eof())
	{
		int count = 0;
		std::vector<std::size_t> tempVector;
		while (count != FIELD_SIZE)
		{
			int number;
			strStream >> number;
			tempVector.push_back(number);
			count++;
		}
		tmp.push_back(tempVector);
	}
	
	return tmp;
}


bool IsHashValid(const std::set<std::size_t> &passedHashes, std::size_t hash)
{
	return (passedHashes.find(hash) == passedHashes.end());
}

Node *CreateNewNode(Node *currentNode, int directionX, int directionY)
{
	Node *newNode = new Node;
	newNode->depth = currentNode->depth++;
	newNode->father = currentNode;
	newNode->zeroPosition = Position(currentNode->zeroPosition.x + directionX, currentNode->zeroPosition.y + directionY);
	newNode->matrix = currentNode->matrix;
	std::swap(newNode->matrix[currentNode->zeroPosition.y + directionY][currentNode->zeroPosition.x + directionX], 
		newNode->matrix[currentNode->zeroPosition.y][currentNode->zeroPosition.x]);
	newNode->GetHash();

	return newNode;
}


void ProcessSearch(std::vector<Node*> &searchQueue, std::set<std::size_t> &hashes)
{
	Node *currNode = searchQueue.front();
	searchQueue.erase(searchQueue.begin());

	if (IsHashValid(hashes, currNode->hash))
	{
		if (currNode->zeroPosition.x > 0)
		{
			searchQueue.emplace(searchQueue.begin(), CreateNewNode(currNode, -1, 0));
		}
		if (currNode->zeroPosition.y > 0)
		{
			searchQueue.emplace(searchQueue.begin(), CreateNewNode(currNode, 0, -1));
		}
		if (currNode->zeroPosition.x < FIELD_SIZE - 1)
		{
			searchQueue.emplace(searchQueue.begin(), CreateNewNode(currNode, 1, 0));
		}
		if (currNode->zeroPosition.y < FIELD_SIZE - 1)
		{
			searchQueue.emplace(searchQueue.begin(), CreateNewNode(currNode, 0, 1));
		}
		hashes.emplace(currNode->hash);
	}
}


void ProcessAStar(std::set<std::size_t> &hashes)
{ 
	map <size_t, vector<Node*>>::iterator currIt = manhattanMap.begin();
	Node *currNode = currIt->second.front();
	
	if (IsHashValid(hashes, currNode->hash))
	{
		if (currNode->zeroPosition.x > 0)
		{ 
			Node * newNode = CreateNewNode(currNode, -1, 0);
			size_t currManhattenDistance = GetManhattanDistance(newNode->matrix) + newNode->depth;
			if (manhattanMap.find(currManhattenDistance) == manhattanMap.end())
			{
				vector<Node*> searchQueue;
				searchQueue.push_back(newNode);
				manhattanMap.insert(std::pair<size_t, vector<Node*>>(currManhattenDistance, searchQueue));
			}
			else
			{
				manhattanMap[currManhattenDistance].push_back(newNode);
			}
			generatedNodeCouner++;
		}
		if (currNode->zeroPosition.y > 0)
		{
			Node * newNode = CreateNewNode(currNode, 0, -1);
			size_t currManhattenDistance = GetManhattanDistance(newNode->matrix) + newNode->depth;
			if (manhattanMap.find(currManhattenDistance) == manhattanMap.end())
			{
				vector<Node*> searchQueue;
				searchQueue.push_back(newNode);
				manhattanMap.insert(std::pair<size_t, vector<Node*>>(currManhattenDistance, searchQueue));
			}
			else
			{
				manhattanMap[currManhattenDistance].push_back(newNode);
			}
			generatedNodeCouner++;
		}
		if (currNode->zeroPosition.x < FIELD_SIZE - 1)
		{		 
			Node * newNode = CreateNewNode(currNode, 1, 0);
			size_t currManhattenDistance = GetManhattanDistance(newNode->matrix) + newNode->depth;
			if (manhattanMap.find(currManhattenDistance) == manhattanMap.end())
			{
				vector<Node*> searchQueue;
				searchQueue.push_back(newNode);
				manhattanMap.insert(std::pair<size_t, vector<Node*>>(currManhattenDistance, searchQueue));
			}
			else
			{
				manhattanMap[currManhattenDistance].push_back(newNode);
			}
			generatedNodeCouner++;
		}
		if (currNode->zeroPosition.y < FIELD_SIZE - 1)
		{  
			Node * newNode = CreateNewNode(currNode, 0, 1);
			size_t currManhattenDistance = GetManhattanDistance(newNode->matrix) + newNode->depth;
			if (manhattanMap.find(currManhattenDistance) == manhattanMap.end())
			{
				vector<Node*> searchQueue;
				searchQueue.push_back(newNode);
				manhattanMap.insert(std::pair<size_t, vector<Node*>>(currManhattenDistance, searchQueue));
			}
			else
			{
				manhattanMap[currManhattenDistance].push_back(newNode);
			}
			generatedNodeCouner++;
		}
		hashes.emplace(currNode->hash);		
	}


	currIt->second.erase(currIt->second.begin());
	
	if (currIt->second.size() == 0)
	{
		manhattanMap.erase(currIt);
	}
}


std::size_t GetResult(Node **root)
{
	std::set<std::size_t> hashes;
	std::vector<Node*> nodesQueue;

	(*root)->CalculateZeroPosition();
	(*root)->GetHash();
	nodesQueue.push_back(*root);
	while (!nodesQueue.empty())
	{
		ProcessSearch(nodesQueue, hashes);
	}
	return hashes.size();
}

std::size_t GetManthattenResult(Node **root)
{
	std::set<std::size_t> hashes;
	std::vector<Node*> nodesQueue;

	(*root)->CalculateZeroPosition();
	(*root)->GetHash();
	nodesQueue.push_back(*root);
	manhattanMap.insert(std::pair<size_t, vector<Node*>>(GetManhattanDistance((*root)->matrix), nodesQueue));

	while (manhattanMap.begin()->second.front()->matrix != WIN_MATRIX)/*(manhattanMap.begin() ->first != 0)*/
	{
		ProcessAStar(hashes);
	}

	cout << "used Nodes = " << hashes.size() << endl;
	return manhattanMap.size() - 1;
}

void PrintWay(Node *currentNode, const Matrix & matrix, size_t & wayLength)
{
	if (currentNode->father != nullptr)
	{
		PrintWay(currentNode->father, currentNode->father->matrix, wayLength);
	}
	++wayLength;
	for (size_t y = 0; y < matrix.size(); ++y)
	{
		for (size_t x = 0; x < matrix[y].size(); ++x)
		{
			cout << matrix[y][x] << " ";
		} cout << endl;
	}
	cout << endl;
}

void PrintMatrix(Matrix matrix)
{
	for (auto row : matrix)
	{
		for (auto el : row)
		{
			cout << el << " ";
		}
		cout << endl;
	}
	cout << endl;
}

void RemakeWay(Node * currNode, size_t x, size_t y)
{
	PrintMatrix(currNode->matrix);
	if (currNode->father != nullptr)
	{
		RemakeWay(currNode->father, currNode->father->zeroPosition.x, currNode->father->zeroPosition.y);
		WAY_COUNTER++;
		if (currNode->zeroPosition.x > currNode->father->zeroPosition.x)
		{
			wayString.push_back("'left', ");
		}
		else if (currNode->zeroPosition.x < currNode->father->zeroPosition.x)
		{
			wayString.push_back("'right', ");
		}
		else if (currNode->zeroPosition.y > currNode->father->zeroPosition.y)
		{
			wayString.push_back("'up', ");
		}
		else if (currNode->zeroPosition.y < currNode->father->zeroPosition.y)
		{
			wayString.push_back("'down', ");
		}
	}
}


int main()
{
	setlocale(LC_ALL, "rus");
	std::ifstream input("input/input0.txt");
	Node *root = new Node;
	root->matrix = GetMatrix(input);

	GetManthattenResult(&root);
	RemakeWay(manhattanMap.begin()->second.front(), manhattanMap.begin()->second.front()->zeroPosition.x,
		manhattanMap.begin()->second.front()->zeroPosition.y);
	reverse(wayString.begin(), wayString.end());
	cout << "[";
	for each(auto el in wayString)
	{
		cout << el << " ";
	}
	cout << "]" << endl;
	cout << "Length = " << WAY_COUNTER << endl;
	cout << "generatedNodes = " << generatedNodeCouner <<endl;
	return EXIT_SUCCESS;
}


