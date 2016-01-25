#ifndef DECIMATION_01
#define DECIMATION_01
//A Simple, Fast, and Effective Polygon Reduction Algorithm (Stan Melax)
//with vertex normal && texture perservation

#include <string>
#include <algorithm>
#include <vector>
#include <stdlib.h>
#include <assert.h>
#include "stdafx.h"
//Data Structure

class Triangle;
class Vertex;
class Vertex
{
	public:
		float position[3];	//location of this vertex
		int id;							//identity (place of the vertex in original list)
		std::vector<Vertex*> neighbor;	//adjacent vertex
		std::vector<Triangle*> face;	//adjacent triangles
		float cost;						//cost of collapsing edge
		Vertex* collapse;				//candidate vertex for collapse

		Vertex(float posX, float posY, float posZ, int identity)	//Class Constructor
		{
			position[0] = posX;
			position[1] = posY;
			position[2] = posZ;

			id = identity;
		}
		~Vertex()													//Class Destructor
		{

		}

		void removeFace(Triangle* t)
		{
			std::remove(face.begin(), face.end(), t);
		}

		void removeIfNonNeighbor(Vertex* v)	//Remove Neighbor
		{
			std::remove(neighbor.begin(), neighbor.end(), v);
		}
};

class Triangle
{
	public:
		Vertex* vertices[3];	//the (3) points that make this triangle
		float normal[3];	//orthogonal unit vector for triangle normal

		Triangle(Vertex* v1, Vertex* v2, Vertex* v3)	//Class Constructor
		{
			assert(v1!=v2 && v2!=v3 && v3!=v1); //check every vertex is diferent
			vertices[0] = v1;
			vertices[1] = v2;
			vertices[2] = v3;
			computeNormal();
			for (int i = 0; i < 3; ++i) 
			{
				vertices[i]->face.push_back(this); //assing this triangle to each vertex
				for (int j = 0; j < 3; ++j)
				{
					if (i != j) //we do not want to assing as a neigbour a vertex itself
					{
						vertices[i]->neighbor.push_back(vertices[j]);
					}
				}
			}
		}
		~Triangle()									//Class Destructor
		{
		
		}
		int hasVertex(Vertex *v)
		{
			
			for (int i = 0; i < 3; ++i)
			{
				if (vertices[i] == v) return i;
			}
			
			return -1;
		}
		void computeNormal()
		{
			float v[3];
			float w[3];

			v[0] = vertices[1]->position[0] - vertices[0]->position[0];
			v[0] = vertices[1]->position[1] - vertices[0]->position[1];
			v[0] = vertices[1]->position[2] - vertices[0]->position[2];

			w[0] = vertices[2]->position[0] - vertices[1]->position[0];
			w[0] = vertices[2]->position[1] - vertices[1]->position[1];
			w[0] = vertices[2]->position[2] - vertices[1]->position[2];

			normal[0] = (v[1] * w[2]) - (v[2] * w[1]);
			normal[1] = (v[2] * w[0]) - (v[0] * w[2]);
			normal[2] = (v[0] * w[1]) - (v[1] * w[2]);

			float norm = std::abs(normal[0]) + std::abs(normal[1]) + std::abs(normal[2]);

			normal[0] = normal[0] / norm;
			normal[1] = normal[1] / norm;
			normal[2] = normal[2] / norm;

		}
		void replaceVertex(Vertex * new_vertex, Vertex * old_vertex)
		{
			assert(new_vertex && old_vertex); //check they exist
			assert(old_vertex==vertices[0] || old_vertex == vertices[1] || old_vertex == vertices[2]); //check old_vertex is realy part of this triangle
			assert(new_vertex != vertices[0] || new_vertex != vertices[1] || new_vertex != vertices[2]); //check new_vertex is NOT part of this triangle
			
			if (vertices[0] == old_vertex)
			{
				vertices[0] = new_vertex;
			}
			else if (vertices[1] == old_vertex)
			{
				vertices[1] = new_vertex;
			}
			else if (vertices[2] == old_vertex)
			{
				vertices[2] = new_vertex;
			}
			old_vertex->removeFace(this);		//remove face from old vertex
			new_vertex->face.push_back(this);	//add face to new vertex

			//Remove old neighbors
			for (int i = 0; i < 3; ++i)
			{
				old_vertex->removeIfNonNeighbor(vertices[i]);
				vertices[i]->removeIfNonNeighbor(old_vertex);
			}
			//Add new neighbors
			for (int i = 0; i < 3; ++i)
			{
				if (vertices[i] != new_vertex)
				{
					vertices[i]->neighbor.push_back(new_vertex);
					new_vertex->neighbor.push_back(vertices[i]);
				}
			}

			computeNormal();
		}
};

//Given two vertex (u and v) compute the cost of colapse u to v;
float computeEdgeCollapseCost(Vertex* u, Vertex* v)
{
	//I will skip the sqrt because each distance sould be squared, 
	//so for comparison is irrelevant to do the operation
	float edgelength =	abs(
							(v->position[0] - u->position[0])*(v->position[0] - u->position[0])
						+	(v->position[1] - u->position[1])*(v->position[1] - u->position[1])
						+	(v->position[2] - u->position[2])*(v->position[2] - u->position[2])
						);


	float curvature = 0;
	
	//Find the triangles that share the edge uv
	std::vector<Triangle*> sides;
	//For each face at vertex u check if that face also has vertex v
	for (int i = 0; i < u->face.size(); ++i)
	{
		if (u->face[i]->hasVertex(v))
		{
			sides.push_back(u->face[i]);
		}
	}
	//Now we have the triangles that share edge uv
	//We use them to compute the curvature term
	for (int i = 0; i < u->face.size(); ++i)
	{
		float mincurv = 1;
		
		for (int j = 0; j < sides.size(); ++j)
		{
			float dotprod = u->face[i]->normal[0] * sides[j]->normal[0] + u->face[i]->normal[1] * sides[j]->normal[1] + u->face[i]->normal[2] * sides[j]->normal[2];
			mincurv = std::min(mincurv,(1-dotprod)/2.0f);
		}
		
		curvature = std::max(curvature, mincurv);
		
	}
	
	return edgelength * curvature;
}

//Edge cost for all edges that start with vertex v
void computeEdgeCostAtVertex(Vertex *v)
{
	if (v->neighbor.size() == 0) //If v does not have neighbors collapse cost will be -0.01f;
	{
		v->collapse = NULL; //No vertex to who collapse
		v->cost = -0.01f;
		return;
	}
	v->cost = 10000000; //high enough value
	v->collapse = NULL; //for now we do not have a vertex to who collapse
	//search for "least cost" edge

	for (int i = 0; i < v->neighbor.size(); ++i)
	{
		float c;
		c = computeEdgeCollapseCost(v, v->neighbor[i]);
		if (c < v->cost)
		{
			v->collapse = v->neighbor[i];
			v->cost = c;
			
		}
	}
	
}

void collapse(Vertex* u, Vertex* v) //Collapse u onto v
{
	
	if (!v) //This means u does not have another vertex onto which collapse so we delete it
	{	
		//delete u;
		return;
	}
	
	//store neigbours of u
	std::vector<Vertex*> tmp;
	for (int i = 0; i < u->neighbor.size(); ++i)
	{
		tmp.push_back(u->neighbor[i]);
	}
	
	//Now we delete triangles with the edge uv
	for (int i = u->face.size()-1;i>=0; --i)
	{
		if (u->face[i]->hasVertex(v))
		{
			//delete(u->face[i]);
		}
	}
	
	//Replace u with v for the other triangles
	
	for (int i = u->face.size()-1;i>=0; --i)
	{
		u->face[i]->replaceVertex(u,v);
	}
	/*
	//delete u;
	//Compute new collapse cost for neigbors
	for (int i = 0; i < tmp.size(); ++i)
	{
		computeEdgeCostAtVertex(tmp[i]);
	}
	*/
}

bool mySortFunction(Vertex* v, Vertex* u)
{
	return (v->cost > u->cost);
}

//We call this function to start the algorithm
void polygonReduction(	std::vector<float> mod_v_data, 
						std::vector<float> mod_vt_data, 
						std::vector<float> mod_vn_data, 
						std::vector<std::vector<int>> mod_f_data)
{
	std::vector<Vertex*> vertices;
	std::vector<Triangle*> triangles;

	int vId = 0;
	
	//vId will be vertex identity
	//vector iterator will be incremented in 3 each loop because we need 3 values (x,y,z) for vertex position
	//mod_v_data must have the format x1,y1,z1,x2,y2,z2...xn,yn,zn

	for (std::vector<float>::iterator it = mod_v_data.begin(); it != mod_v_data.end(); ++it)
	{
		Vertex* v = new Vertex(*it,*(it+1),*(it+2),vId);
		vertices.push_back(v);
		++vId;
		++it;
		++it;
	}
	
	for (std::vector<std::vector<int>>::iterator it = mod_f_data.begin(); it != mod_f_data.end(); ++it)
	{

		Triangle* t = new Triangle(vertices[(*it)[0]],vertices[(*(it+1))[0]],vertices[(*(it+2))[0]]);
		triangles.push_back(t);
		++it;
		++it;
	}

	for (int i = 0; i < vertices.size(); ++i)
	{
		computeEdgeCostAtVertex(vertices[i]);
	}

	std::sort(vertices.begin(), vertices.end(), mySortFunction); //sort references in order to speed "search" for delete

	collapse(vertices[vertices.size() - 1], vertices[vertices.size() - 1]->collapse);




	std::string stop;
	std::cin >> stop;


}

#endif // DECIMATION_01
