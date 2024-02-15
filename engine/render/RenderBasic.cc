// 
// Copyright 2023 Alexander Marklund (Allkams02@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand associated
// documentation files(the “Software”), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and /or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN 
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "config.h"
#include "RenderBasic.h"

namespace Render
{
	Mesh::Mesh(std::vector<Vertice> vertices, std::vector<GLuint> indices, std::vector<Primitive> primitives)
	{
		this->primitives = primitives;

		processMesh(vertices, indices);
	}

	void Mesh::processMesh(std::vector<Vertice> vertices, std::vector<GLuint> indices)
	{
		// Generating Buffers
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		// Binding Buffers
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		// Filling Buffers
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertice), &vertices[0], GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

		// Passing vertice position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)0);

		// Passing vertice color
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)offsetof(Vertice, Color));

		// Passing vertice texture cords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)offsetof(Vertice, TexCoord));

		// unbinding VAO
		glBindVertexArray(0);

	}

	void Mesh::renderMesh(GLuint primitiveIndex)
	{
		if (primitiveIndex <= primitives.size()) {

			//Drawes each primitive
			glDrawElements(GL_TRIANGLES, 1 + primitives[primitiveIndex].numVertices - primitives[primitiveIndex].startIndex, GL_UNSIGNED_INT, (void*)(primitives[primitiveIndex].startIndex * sizeof(GLint)));
		}
	}

	void Mesh::clearBuffers() {
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

	void Mesh::bindVAO() {
		glBindVertexArray(VAO);
	}

	void Mesh::unBindVAO() {
		glBindVertexArray(0);
	}

	Mesh CreateTriangle(float32 width, float32 height)
	{
		float32 widthPoint = width / 2.0f;
		float32 heightPoint = width / 2.0f;

		Vertice Point1 = Vertice(glm::vec3(-widthPoint, -heightPoint, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
		Vertice Point2 = Vertice(glm::vec3(widthPoint, -heightPoint, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
		Vertice Point3 = Vertice(glm::vec3(-0.0f, heightPoint, 0.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 1.0f));

		Primitive Triangle;
		Triangle.startIndex = 0;
		Triangle.numVertices = 3;

		return Mesh(
			// Vertice Points
			{
				Point1, Point2, Point3
			},
			// Indice order (What Vertice connects to what)
			{
				0, 1, 2
			},
			// Primitives
			{
				Triangle
			}
		);
	}
	
	Mesh CreatePlane(float32 width, float32 height);

	Mesh CreateCube(float32 width, float32 height, float32 depth)
	{
		float32 widthPos = width / 2.0f;
		float32 heightPos = height / 2.0f;
		float32 depthPos = depth / 2.0f;

		//Face one
		Vertice Point1 = Vertice(glm::vec3(widthPos, -heightPos, depthPos), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
		Vertice Point2 = Vertice(glm::vec3(widthPos, heightPos, depthPos), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
		Vertice Point3 = Vertice(glm::vec3(-widthPos, heightPos, depthPos), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 1.0f));
		Vertice Point4 = Vertice(glm::vec3(-widthPos, -heightPos, depthPos), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 1.0f));

		//Face two
		Vertice Point5 = Vertice(glm::vec3(widthPos, -heightPos, -depthPos), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
		Vertice Point6 = Vertice(glm::vec3(widthPos, heightPos, -depthPos), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
		Vertice Point7 = Vertice(glm::vec3(widthPos, heightPos, depthPos), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 1.0f));
		Vertice Point8 = Vertice(glm::vec3(widthPos, -heightPos, depthPos), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 1.0f));

		//Face three
		Vertice Point9 = Vertice(glm::vec3(-widthPos, -heightPos, -depthPos), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f));
		Vertice Point10 = Vertice(glm::vec3(-widthPos, heightPos, -depthPos), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
		Vertice Point11 = Vertice(glm::vec3(widthPos, heightPos, -depthPos), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
		Vertice Point12 = Vertice(glm::vec3(widthPos, -heightPos, -depthPos), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f));

		//Face four
		Vertice Point13 = Vertice(glm::vec3(-widthPos, -heightPos, depthPos), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f));
		Vertice Point14 = Vertice(glm::vec3(-widthPos, heightPos, depthPos), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
		Vertice Point15 = Vertice(glm::vec3(-widthPos, heightPos, -depthPos), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
		Vertice Point16 = Vertice(glm::vec3(-widthPos, -heightPos, -depthPos), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f));

		//Face top
		Vertice Point17 = Vertice(glm::vec3(widthPos, heightPos, depthPos), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f));
		Vertice Point18 = Vertice(glm::vec3(widthPos, heightPos, -depthPos), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
		Vertice Point19 = Vertice(glm::vec3(-widthPos, heightPos, -depthPos), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
		Vertice Point20 = Vertice(glm::vec3(-widthPos, heightPos, depthPos), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f));

		//Face bottom
		Vertice Point21 = Vertice(glm::vec3(widthPos, -heightPos, depthPos), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f));
		Vertice Point22 = Vertice(glm::vec3(widthPos, -heightPos, -depthPos), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
		Vertice Point23 = Vertice(glm::vec3(-widthPos, -heightPos, -depthPos), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
		Vertice Point24 = Vertice(glm::vec3(-widthPos, -heightPos, depthPos), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f));

		Primitive Cube;
		Cube.startIndex = 0;
		Cube.numVertices = 35;

		return Mesh(
			{
				Point1, Point2, Point3, Point4,
				Point5, Point6, Point7, Point8,
				Point9, Point10, Point11, Point12,
				Point13, Point14, Point15, Point16,
				Point17, Point18, Point19, Point20,
				Point21, Point22, Point23, Point24
			}, 
			{
				0,1,3,
				1,2,3,
				4,5,7,
				5,6,7,
				8,9,11,
				9,10,11,
				12,13,15,
				13,14,15,
				16,17,19,
				17,18,19,
				20,21,23,
				21,22,23
			}, 
			{
				Cube
			});
	}
	
	Mesh CreateSphere(float32 radius);
	Mesh CreateCylinder(float32 radius, float32 height);
}