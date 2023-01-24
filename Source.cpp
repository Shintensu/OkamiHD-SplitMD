#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>

char header[4];

const char* scrString = "scr";
// 4 bytes padding
char subMeshCount[4];
// 4 bytes padding
char textureOffsets[16]; // Unclear if actually is textureOffsets

const char* mdbString = "mdb";
char meshType[4];
char meshId[2];
char meshDivisions[2];

// 4 bytes padding
// 16 bytes padding

unsigned int offsetPosition;
char meshDivisionOffsets[16];
unsigned int meshDivisionOffset;

// 8 bytes padding

// Bones?
char vertexPointer[4];
char tMapPointer[4];
char colorWeightPointer[4];
char uvPointer[4];

char numberOfIndices[2];

unsigned int numberOfIndicesUInt;


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Not enough arguments!" << std::endl;
		return -1;
	}
	if (argc > 2)
	{
		std::cout << "Too many arguments" << std::endl;
		return -1;
	}
	std::ifstream inFile(argv[1], std::ifstream::binary);

	//read scr
	inFile.read(header, 4);

	if (strcmp(header, scrString) != 0)
	{
		std::cout << "No scr header string at position 0" << std::endl;
		inFile.close();
		return -1;
	}
	inFile.ignore(4); // padding

	inFile.read(subMeshCount, 4);

	inFile.ignore(4); // padding

	inFile.read(textureOffsets, 16);

	for (int i = 0; i < ((int)*subMeshCount); i++)
	{
		//read mdb
		inFile.read(header, 4);
		if (strcmp(header, mdbString) != 0)
		{
			std::cout << "No mdb header string at subMesh position " + (i + 1) << std::endl;
			inFile.close();
			return -1;
		}
		inFile.read(meshType, 4);
		inFile.read(meshId, 2);
		inFile.read(meshDivisions, 2);

		inFile.ignore(4); // padding
		inFile.ignore(16); // padding

		offsetPosition = inFile.tellg();
		inFile.read(meshDivisionOffsets, 16);

		//Bones

		for (int j = 0; j < ((unsigned int)(*meshDivisions)); j++)
		{
			/*std::cout << std::hex << (offsetPosition) << std::endl;
			std::cout << std::hex << (int)*(meshDivisionOffsets + (j * 4)) << std::endl;
			std::cout << std::hex << (int)(meshDivisionOffsets[0 + (j * 4)] + meshDivisionOffsets[1 + (j * 4)] * 256) << std::endl;*/
			meshDivisionOffset = offsetPosition;
			meshDivisionOffset += (unsigned char)meshDivisionOffsets[0 + (j * 4)];
			meshDivisionOffset += ((unsigned char)meshDivisionOffsets[1 + (j * 4)] * 256);
			meshDivisionOffset -= 0x20;
			inFile.seekg(meshDivisionOffset);

			inFile.read(vertexPointer, 4);

			inFile.ignore(4); // padding

			inFile.read(tMapPointer, 4);
			inFile.read(colorWeightPointer, 4);
			inFile.read(uvPointer, 4);
			inFile.read(numberOfIndices, 2);

			inFile.ignore(2); // padding

			inFile.ignore(8); // padding

			numberOfIndicesUInt = (unsigned char)numberOfIndices[0];
			numberOfIndicesUInt += (unsigned char)numberOfIndices[1] * 256;

			char* indexBuffer = (char*)malloc(numberOfIndicesUInt * 16);
			char* textureMapBuffer = (char*)malloc(numberOfIndicesUInt * 4);
			char* colorWeightBuffer = (char*)malloc(numberOfIndicesUInt * 4);
			char* uvBuffer = (char*)malloc(numberOfIndicesUInt * 8);

			inFile.read(indexBuffer, (numberOfIndicesUInt * 16));
			inFile.read(textureMapBuffer, (numberOfIndicesUInt * 4));
			inFile.read(colorWeightBuffer, (numberOfIndicesUInt * 4));
			inFile.read(uvBuffer, (numberOfIndicesUInt * 8));

			std::ofstream indicesOut((std::string(argv[1]) + std::string("_") + std::to_string(i + 1) + std::string("_") + std::to_string(j + 1) + std::string(".ind")), std::ofstream::binary);
			std::ofstream textureMapOut((std::string(argv[1]) + std::string("_") + std::to_string(i + 1) + std::string("_") + std::to_string(j + 1) + std::string(".tmap")), std::ofstream::binary);
			std::ofstream colorWeightOut((std::string(argv[1]) + std::string("_") + std::to_string(i + 1) + std::string("_") + std::to_string(j + 1) + std::string(".colw")), std::ofstream::binary);
			std::ofstream uvOut((std::string(argv[1]) + std::string("_") + std::to_string(i + 1) + std::string("_") + std::to_string(j + 1) + std::string(".uv")), std::ofstream::binary);

			indicesOut.write(indexBuffer, (numberOfIndicesUInt * 16));
			textureMapOut.write(textureMapBuffer, (numberOfIndicesUInt * 4));
			colorWeightOut.write(colorWeightBuffer, (numberOfIndicesUInt * 4));
			uvOut.write(uvBuffer, (numberOfIndicesUInt * 8));

			indicesOut.close();
			textureMapOut.close();
			colorWeightOut.close();
			uvOut.close();

			free(indexBuffer);
			free(textureMapBuffer);
			free(colorWeightBuffer);
			free(uvBuffer);
			std::cout << "Extracted submesh " << i << ", Division " << j << std::endl;
			// failsafe to prevent invite file writes
			if (j > 10)
			{
				return -1;
			}
		}
		inFile.ignore(96); // padding
	}
	inFile.close();
	std::cout << "Finished Extraction" << std::endl;
}