
				


		function Vertex(position, vertex_id)
		{
			this.position = position;			//vec3
			this.vertex_id = vertex_id;			
			this.adjacent_vertices = [];
			this.adjacent_triangles = [];
	
			this.collapse_vertex_id = -1; 		//-1 means no collapse_vertex
			this.collapse_cost = 1000000;		

		}

		Vertex.prototype.updateCollapseData = function(collapse_vertex_id,collapse_cost)
		{
				this.collapse_vertex_id = collapse_vertex_id;
				this.collapse_cost = collapse_cost; 
		}

		Vertex.prototype.addAdjacentVertex = function(vertex_id)
		{
			if(this.adjacent_vertices.indexOf(vertex_id) == -1 && this.vertex_id != vertex_id) this.adjacent_vertices.push(vertex_id);
		}

		Vertex.prototype.addAdjacentTriangle = function(triangle_id)
		{
			if(this.adjacent_triangles.indexOf(triangle_id) == -1) this.adjacent_triangles.push(triangle_id);
		}

		Vertex.prototype.deleteAdjacentVertex = function(vertex_id)
		{
			delete_vertex_index = this.adjacent_vertices.indexOf(vertex_id.toString());
			if(delete_vertex_index>=0) this.adjacent_vertices.splice(delete_vertex_index,1);
		}

		Vertex.prototype.deleteAdjacentTriangle = function(triangle_id)
		{
			delete_triangle_index = this.adjacent_triangles.indexOf(triangle_id);
			if(delete_triangle_index>=0) this.adjacent_triangles.splice(delete_triangle_index,1);
		}

		function Triangle(triangle_id,triangle_vertices_ids,triangle_texture_ids,triangle_normal_ids)
		{
			this.triangle_id = triangle_id;
			this.triangle_vertices_ids = triangle_vertices_ids;
			this.triangle_texture_ids = triangle_texture_ids;
			this.triangle_normal_ids = triangle_normal_ids;
			this.triangle_normal = 0;
			this.collapsed = false;

		}
		
		Triangle.prototype.computeNormal = function(vertex_u, vertex_v, vertex_w)
		{
			this.triangle_normal = vec3.normalize([0,0,0],vec3.cross([0,0,0],vec3.subtract([0,0,0],vertex_v,vertex_u),vec3.subtract([0,0,0],vertex_w,vertex_u)));
		}

		Triangle.prototype.hasVertex = function(vertex_id)
		{
			hasIt = false;
			for(t=0;t<this.triangle_vertices_ids.length;++t)
			{
				if(this.triangle_vertices_ids[t]==vertex_id)
				{
					hasIt = true;
				}
			}
			return hasIt;
		}

		Triangle.prototype.replaceVertex = function(old_id, new_id)
		{
			if(this.hasVertex(new_id))
			{
				this.collapsed = true;
			}

			replaced = false;
			for(t=0;t<3;++t)
			{
				if(this.triangle_vertices_ids[t]==old_id)
				{
					this.triangle_vertices_ids[t] = new_id;
					replaced = true;
				}
			}
			//Centinela en el caso de que el vertice no este en el triangulo
			if(!replaced) console.log("Tried to replace a non existing vertex");

		}

		function Decimate(raw_data,iterations_number)
		{
			this.raw_data = raw_data;						//We save the original content of the .obj file
			this.iterations_number = iterations_number;		//How many triangles we want to erase

			this.geometric_vertices = [];					//We save the original geometric vertices
			this.texture_vertices = [];						//We save the original texture vertices 
			this.vertex_normals = [];						//We save the original vetices normals
			this.faces = [];								//We save the original faces (triangles)

			//I don't want to use length each time (I set them in extractData function when all previous arrays are full)
			this.original_number_v  = 0;
			this.original_number_vt = 0;
			this.original_number_vn = 0;
			this.origianl_number_f  = 0;

			//Containers for Triangles and Vertex information
			this.vertices_container = [];
			this.triangles_container = [];
		}


		Decimate.prototype.computeEdgeCollapseCost = function(vertex_u, vertex_v)
		{	
			edgelength = Math.sqrt(vec3.squaredDistance(vertex_u, vertex_v));
			curvature = 1; //set to 0

			//Compute curvature

			return edgelength*curvature;
		}

		//Compute collapse cost for all edges and chose the lower one
		Decimate.prototype.computeEdgeCollapseCostAtVertex = function(vertex_id)
		{

			vertex_index = this.getVertexIndexFromId(vertex_id);

			candidate = -1;
			actual_cost = 1000000;

			for(j=1;j<this.vertices_container[vertex_index].adjacent_vertices.length;++j)
			{
				new_cost = this.computeEdgeCollapseCost( this.vertices_container[vertex_index].position , this.getVertexPosFromId(this.vertices_container[vertex_index].adjacent_vertices[j]));
				if(new_cost < actual_cost) 
				{
					candidate = this.vertices_container[vertex_index].adjacent_vertices[j];
					actual_cost = new_cost;
				}
			}
			this.vertices_container[vertex_index].updateCollapseData(candidate,actual_cost);
		}

		Decimate.prototype.collapseVertex = function(vertex_id)
		{
			collapse_vertex_index = this.getVertexIndexFromId(vertex_id);
			collapse_vertex_id = this.vertices_container[collapse_vertex_index].collapse_vertex_id
			
			vertex_index = this.getVertexIndexFromId(vertex_id);



			//Modify the vertex in all triangles
			for(h=0;h<this.vertices_container[vertex_index].adjacent_triangles.length;++h)
			{
					triangle_index = this.getTriangleIndexFromId(this.vertices_container[vertex_index].adjacent_triangles[h]);
					this.triangles_container[triangle_index].replaceVertex(vertex_id,collapse_vertex_id);
					if(this.triangles_container[triangle_index].collapsed) 
					{
						fv = this.getVertexIndexFromId(this.triangles_container[triangle_index].triangle_vertices_ids[0]);
						sv = this.getVertexIndexFromId(this.triangles_container[triangle_index].triangle_vertices_ids[1]);
						tv = this.getVertexIndexFromId(this.triangles_container[triangle_index].triangle_vertices_ids[2]);

						t_id = this.triangles_container[triangle_index].triangle_id; 

						this.vertices_container[fv].deleteAdjacentTriangle(t_id);
						this.vertices_container[sv].deleteAdjacentTriangle(t_id);

						this.vertices_container[tv].deleteAdjacentTriangle(t_id);

						this.triangles_container.splice(triangle_index,1);
					}
			}
			//Modify the vertex in all vertex adjacent_vertex array
			for(h=0;h<this.vertices_container[vertex_index].adjacent_vertices.length;++h)
			{
					adjacent_vertex_index = this.getVertexIndexFromId(this.vertices_container[vertex_index].adjacent_vertices[h]);

					this.vertices_container[adjacent_vertex_index].deleteAdjacentVertex(vertex_id);
					this.vertices_container[adjacent_vertex_index].addAdjacentVertex(collapse_vertex_id);

			}

			this.vertices_container.splice(vertex_index,1);

			for(h=0;h<this.vertices_container.length;++h)
			{
				this.computeEdgeCollapseCostAtVertex(this.vertices_container[h].vertex_id);
			}

			this.vertices_container.sort(function(a,b){return a.collapse_cost-b.collapse_cost});

		}

		Decimate.prototype.getTriangleIndexFromId = function(triangle_id)
		{
			for(t=0;t<this.triangles_container.length;++t)
			{
				if(this.triangles_container[t].triangle_id == triangle_id) return t;
			}
			return -1;
		}

		Decimate.prototype.getVertexPosFromId = function(vertex_id)
		{
			for(v=0;v<this.vertices_container.length;++v)
			{
				if(this.vertices_container[v].vertex_id == vertex_id) return this.vertices_container[v].position;
			}
			return -1;
		}

		Decimate.prototype.getVertexIndexFromId = function(vertex_id)
		{
			for(v=0;v<this.vertices_container.length;++v)
			{
				if(this.vertices_container[v].vertex_id == vertex_id) return v;
			}
			return -1;
		}

		Decimate.prototype.extractData = function()
		{
			parsedArray = this.raw_data.split(/\n|  | /);
			//Each position of this array will contain a obj data type keyword or a data type value
			//This implementation focuses on:
			// - Geometric vertices (v)
			// - Texture vertices (vt)
			// - Vertex normals (vn)
			// - Faces (f)
			//This data will be preserved, anything else will be lost
			//Consult http://www.martinreddy.net/gfx/3d/OBJ.spec for other references of obj format


			for(i=0;i<parsedArray.length;++i)
			{
				switch (parsedArray[i])
				{
					case 'v':
						this.geometric_vertices.push(vec3.fromValues(parsedArray[i+1],parsedArray[i+2],parsedArray[i+3]));
						i = i + 3;
						break;
					case 'vt':
						this.texture_vertices.push(vec3.fromValues(parsedArray[i+1],parsedArray[i+2],parsedArray[i+3]));
						i = i + 3;
						break;
					case 'vn':
						this.vertex_normals.push(vec3.fromValues(parsedArray[i+1],parsedArray[i+2],parsedArray[i+3]));
						i = i + 3;
						break;
					case 'f':
						this.faces.push([parsedArray[i+1],parsedArray[i+2],parsedArray[i+3]]);
						break;
				}
			}
			this.original_number_v  = this.geometric_vertices.length;
			this.original_number_vt = this.texture_vertices.length;
			this.original_number_vn = this.vertex_normals.length;
			this.origianl_number_f  = this.faces.length;

			//Save position and id (i value  +1 because obj format)
			for(i=0;i<this.original_number_v;++i)
			{
				this.vertices_container.push(new Vertex(this.geometric_vertices[i],i+1));
			}

			for(i=0;i<this.origianl_number_f;++i)
			{
				//The following arrays will have this contents
				//
				// - At pos 0 the vertex id
				// - At pos 1 the texture id
				// - At pos 2 the normal id

				first_point = this.faces[i][0].split("/");
				second_point = this.faces[i][1].split("/");
				third_point = this.faces[i][2].split("/");

				//The following arrays will have this contents
				//
				// - triangle_vertices_id will be the 3 identities of the vertices that are part of the triangle
				// - triangle_texture_id will have the texture id of the vertices of triangle_vertices_id 
				// - triangle_normal_id will have the normal id of the vertices of triangle_vertices_id
				//
				// If we want the texture id or the normal id we access the same pos of the vertex
				// that is the vertex of pos 0 will have his texture and normal at pos 0 of the other arrays

				triangle_vertices_ids = [first_point[0],second_point[0],third_point[0]];
				triangle_texture_ids = [first_point[1],second_point[1],third_point[1]];
				triangle_normal_ids = [first_point[2],second_point[2],third_point[2]];

				//Create and add the new Triangle to the container
				//i+1 will be the triangle_id 
				auxTriangle = new Triangle(i+1,triangle_vertices_ids,triangle_texture_ids,triangle_normal_ids);

				auxTriangle.computeNormal( this.getVertexPosFromId(auxTriangle.triangle_vertices_ids[0]) , this.getVertexPosFromId(auxTriangle.triangle_vertices_ids[1]),  this.getVertexPosFromId(auxTriangle.triangle_vertices_ids[2]));

				this.triangles_container.push(auxTriangle);

				//Now set the vertices and triangles adjacents for each vertex
				//I will use Vertex functions to ensure that no repetition ocurs
				
				index_fp = this.getVertexIndexFromId(first_point[0]);
				index_sp = this.getVertexIndexFromId(second_point[0]);
				index_tp = this.getVertexIndexFromId(third_point[0]);

				this.vertices_container[index_fp].addAdjacentVertex(second_point[0]);
				this.vertices_container[index_fp].addAdjacentVertex(third_point[0]);

				this.vertices_container[index_sp].addAdjacentVertex(first_point[0]);
				this.vertices_container[index_sp].addAdjacentVertex(third_point[0]);
				
				this.vertices_container[index_tp].addAdjacentVertex(second_point[0]);
				this.vertices_container[index_tp].addAdjacentVertex(first_point[0]);

				this.vertices_container[index_fp].addAdjacentTriangle(auxTriangle.triangle_id);
				this.vertices_container[index_sp].addAdjacentTriangle(auxTriangle.triangle_id);
				this.vertices_container[index_tp].addAdjacentTriangle(auxTriangle.triangle_id);
			}

			for(i=0;i<this.vertices_container.length;++i)
			{
				this.computeEdgeCollapseCostAtVertex(this.vertices_container[i].vertex_id);
			}

			this.vertices_container.sort(function(a,b){return a.collapse_cost-b.collapse_cost});

			//Data extracted
		}

		Decimate.prototype.decimateStep = function()
		{
			this.collapseVertex(this.vertices_container[0].vertex_id);
			this.vertices_container.sort(function(a,b){return a.collapse_cost-b.collapse_cost});
		}

		//This function will be used to create a file and download it
		Decimate.prototype.downloadFile = function(filename, text)
		{
			var element = document.createElement('a');
		  	element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));
		  	element.setAttribute('download', filename);

		  	element.style.display = 'none';
		  	document.body.appendChild(element);

		  	element.click();

		  	document.body.removeChild(element);
		}

		Decimate.prototype.downloadOriginalData = function()
		{
			file_content = "";
			for(i=0;i<this.original_number_v;++i)
			{
				file_content = file_content + "v " + this.geometric_vertices[i][0] + " " + this.geometric_vertices[i][1] + " " + this.geometric_vertices[i][2] + "\n";
			}
			for(i=0;i<this.original_number_vt;++i)
			{
				file_content = file_content + "vt " + this.texture_vertices[i][0] + " " + this.texture_vertices[i][1] + " " + this.texture_vertices[i][2] + "\n";
			}
			
			for(i=0;i<this.original_number_vn;++i)
			{
				file_content = file_content + "vn " + this.vertex_normals[i][0] + " " + this.vertex_normals[i][1] + " " + this.vertex_normals[i][2] + "\n";
			}

			for(i=0;i<this.origianl_number_f;++i)
			{
				file_content = file_content + "f " + this.faces[i][0] + " " + this.faces[i][1] + " " + this.faces[i][2] + "\n";
			}
			
			this.downloadFile("original_data.obj",file_content);
		}

		Decimate.prototype.downloadDecimated = function()
		{
			file_content = "";
			for(i=0;i<this.vertices_container.length;++i)
			{
				pos = this.vertices_container[i].position;
				file_content = file_content + "v " + pos[0] + " " + pos[1] + " " + pos[2] + "\n";
			}
			for(i=0;i<this.triangles_container.length;++i)
			{
				pos = this.triangles_container[i].triangle_vertices_ids;
				file_content = file_content + "f " +  (this.getVertexIndexFromId(pos[0])+1) + " " + (this.getVertexIndexFromId(pos[1])+1) + " " + (this.getVertexIndexFromId(pos[2])+1) + "\n";
			}
			this.downloadFile("decimated.obj",file_content);

		}

