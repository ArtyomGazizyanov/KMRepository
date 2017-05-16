// lab2.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <vector>
#include <string>
#include <fstream>
#include <queue>
#include <set>   
#include <iostream>
#include <map> 
#include <sstream>

typedef std::vector<std::vector<size_t>> Matrix;
const size_t FIELD_SIZE = 4;
using namespace std;
size_t WAY_COUNTER = 0;
vector<string> wayString;

Matrix WinMatrix = {
	{ 0, 1 ,2, 3},
	{ 4, 5, 6, 7},
	{ 8, 9 , 10, 11},
	{ 12, 13, 14, 15 }
};

struct Position
{
	Position(size_t x, size_t y) :
		x(x),
		y(y)
	{
	}
	size_t x = 0;
	size_t y = 0;
};

struct Node
{
	size_t hash = 0;
	Matrix matrix;
	Position zeroPosition = Position(0, 0);
	Node *father = nullptr;
	size_t depth = 0;
	void CalculateZeroPosition()
	{
		for (size_t i = 0; i < matrix.size(); ++i)
		{
			for (size_t j = 0; j < matrix[i].size(); ++j)
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
		for (size_t i = 0; i < matrix.size(); i++)
		{
			for (size_t j = 0; j < matrix.front().size(); j++)
			{
				hash = hash * 10 + matrix[i][j];
			}
		}
	}
};

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

Matrix GetMatrix(std::ifstream & input)
{
	Matrix tmp;
	int count = 0;
	vector<std::size_t> tempVector;
	while (input )
	{		
		int number;
		input >> number;
		tempVector.push_back(number);

		count++;
		if (count == FIELD_SIZE)
		{
			count = 0;
			tmp.push_back(tempVector);
			tempVector.clear();
		}

	}

	return tmp;
}


bool IsHashValid(const std::set<size_t> &passedHashes, size_t hash)
{
	return (passedHashes.find(hash) == passedHashes.end());
}


Node *CreateNewNode(Node *currentNode, int directionX, int directionY)
{
	Node *newNode = new Node;
	newNode->depth = currentNode->depth + 1;
	newNode->zeroPosition = Position(currentNode->zeroPosition.x + directionX, currentNode->zeroPosition.y + directionY);
	newNode->matrix = currentNode->matrix;
	newNode->father = currentNode;
	std::swap(newNode->matrix[newNode->zeroPosition.y][newNode->zeroPosition.x], newNode->matrix[currentNode->zeroPosition.y][currentNode->zeroPosition.x]);
	newNode->GetHash();

	return newNode;
}



Node* ProcessSearch(std::queue<Node*> &searchQueue, std::set<size_t> &hashes)
{
	Node *currNode = searchQueue.front();
	searchQueue.pop();
	if (currNode->matrix == WinMatrix)
	{
		return currNode;
	}
	if (IsHashValid(hashes, currNode->hash))
	{
		if (currNode->zeroPosition.x > 0)
		{
			searchQueue.push(CreateNewNode(currNode, -1, 0));
		}
		if (currNode->zeroPosition.y > 0)
		{
			searchQueue.push(CreateNewNode(currNode, 0, -1));
		}
		if (currNode->zeroPosition.x < FIELD_SIZE - 1)
		{
			searchQueue.push(CreateNewNode(currNode, 1, 0));
		}
		if (currNode->zeroPosition.y < FIELD_SIZE - 1)
		{
			searchQueue.push(CreateNewNode(currNode, 0, 1));
		}
		hashes.emplace(currNode->hash);
	}
	return nullptr;
}

Node * FindNodeWithMinDepth(vector<Node*> results)
{
	size_t minDepth = results[0]->depth;
	size_t minPos = 0;
	for (size_t i = 0; i < results.size(); ++i)
	{
		if (results[i]->depth < minDepth)
		{
			minDepth = results[i]->depth;
			minPos = i;
		}
	}
	return results[minPos];
}

size_t GetResult(Node **root, vector<Node*> & results)
{
	std::set<size_t> hashes;
	std::queue<Node*> nodesQueue;

	(*root)->CalculateZeroPosition();
	(*root)->GetHash();
	nodesQueue.push(*root);
	Node* WinPos = CreateNewNode(*root, 0, 0);
	bool isFound = false;

	while (!nodesQueue.empty())
	{
		WinPos = ProcessSearch(nodesQueue, hashes);
		if (WinPos != nullptr)
		{
			results.insert(results.begin(), WinPos);
		}
	}
	if (results.size() > 0)
	{
		Node* answer = FindNodeWithMinDepth(results);
		RemakeWay(answer, answer->zeroPosition.x, answer->zeroPosition.y);
	}
	return hashes.size();
}

int main()
{
	std::ifstream input("input.txt");
	Node *root = new Node;
	root->matrix = GetMatrix(input);
	vector<Node*> results;
	std::cout << "Result = " << GetResult(&root, results) << std::endl;
	cout << "WAY : [ ";
	for each(auto el in wayString)
	{
		cout << el << " ";
	}
	cout << "]; " << endl;
	return EXIT_SUCCESS;
}

