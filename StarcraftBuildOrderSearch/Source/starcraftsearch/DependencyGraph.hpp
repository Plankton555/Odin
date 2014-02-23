#ifndef DEPENDENCY_GRAPH
#define DEPENDENCY_GRAPH

#include <vector>
#include "ActionSet.hpp"

#define NO_STRICT_DEPENDENCY -1

namespace BuildOrderSearch
{
template <class T>
class DependencyGraph
{
	int size;
	
	// the graph
	// if G[i][j] is true, then I depends on J
	std::vector< std::vector<T> > G;
	
	// strict dependency vector
	std::vector< ActionSet > D;
	
	// returns an ActionSet of strict dependencies
	ActionSet getDependency(int row)
	{
		ActionSet deps;
	
		for (int c=0; c<size; ++c)
		{
			if (G[row][c])
			{
				deps.add(c);
			}
		}
		
		return deps;
	}
	
public:

	// default constructor
	DependencyGraph<T>() {}
	
	// constructor with size of the square graph
	DependencyGraph<T>(int s) : size(s) 
	{
		// initialize the graph
		G = std::vector< std::vector<T> >( size, std::vector<T>(size,false));
		
		// initialize the dependencies
		D = std::vector<ActionSet>(size);
	}
	
	std::vector<T> & operator [] (int & row)
	{
		return G[row];
	}
	
	void set(int row, int col, T val)
	{
		G[row][col] = val;
	}
	
	T get(int row, int col) const
	{
		return G[row][col];
	}
	
	// this function 
	void transitiveReduction()
	{	
		// the new graph
		std::vector< std::vector<T> > TG(G);
	
		// do transititve reduction
		for (int x=0; x<size; ++x)
		{
			for (int y=0; y<size; ++y)
			{
				for (int z=0; z<size; ++z)
				{
					// if the transitive relationship exists
					if (G[x][y] && G[y][z] && G[x][z])
					{
						//printf("Eliminating edge (%d,%d)\n", x, z);
					
						// eliminate this one
						TG[x][z] = false;
					}
				}
			}
		}
		
		// set the old graph equal to the new graph
		G = TG;
		
		// set up the vectors
		for (int a=0; a<size; ++a)
		{
			D[a] = getDependency(a);
		}
	}
	
	// returns an action set of strict dependencies
	ActionSet getStrictDependency(int action) const
	{
		return D[action];
	}
	
	void printGraph() const
	{
		for (int r=0; r<size; ++r)
		{
			printf("%d\t", r);
			
			for (int c=0; c<size; ++c)
			{
				printf("%d", G[r][c] ? 1 : 0);
			}
			
			printf("\n");
		}
	}
};
}
#endif
