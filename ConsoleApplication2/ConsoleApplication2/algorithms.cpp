// ConsoleApplication3.cpp: define el punto de entrada de la aplicación de consola.
//



#include "stdafx.h"
#include <stdexcept>
#include <limits>
#include <stdlib.h>
#include <crtdbg.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "decimation_01.h"

//function to handle the face format
std::vector<int> faceArray(std::string extract)
{
	std::vector<int> data(3);

	if (extract.find('/') == std::string::npos)
	{
		data[0] = (std::stoi(extract)) - 1; //Substract -1 to adquire the true position of the data. Arrays go from 0 to length-1, data comes from 1 to length.
		data[1] = -1;
		data[2] = -1;
		return data;
	}
	else
	{
		int firstpos = extract.find("/");
		if (extract.find("/", firstpos + 1) == std::string::npos)
		{
			data[0] = stoi(extract.substr(0, firstpos)) - 1;
			data[1] = stoi(extract.substr(firstpos + 1)) - 1;
			data[2] = -1;
		}
		else
		{
			int secondpos = extract.find("/", firstpos + 1);
			data[0] = stoi(extract.substr(0, firstpos)) - 1;
			if (secondpos - firstpos < 2) data[1] = -1;
			else data[1] = stoi(extract.substr(firstpos + 1, secondpos)) - 1;
			data[2] = stoi(extract.substr(secondpos + 1)) - 1;
		}
		return data;
	}

}


int _tmain(int argc, _TCHAR* argv[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	std::vector<float> raw_v_data;				//this vector will contain the original vertex data
	std::vector<float> raw_vt_data;				//this vector will contain the original texture data
	std::vector<float> raw_vn_data;				//this vector will contain the original normal data
	std::vector<std::vector<int>> raw_f_data;	//this vector will contain the original faces data

	std::string file_name;	//name of the file with extension will be required
	std::string file_path;	//variable to store the complete path, in this case will add obj/ directory
	std::cin >> file_name;	//insert file name

	file_path = "obj/" + file_name; //complete path

	std::ifstream obj_file(file_path); //open file

	//Read and Parse File
	std::string line; 
	if (obj_file.is_open())
	{
		while (getline(obj_file, line))
		{
			std::string header = line.substr(0, 2);
			if (header == "v ")
			{
				std::string body = line.substr(2, std::string::npos);
				bool number = false;
				std::string extract = "";
				for (int i = 0; i < body.length(); ++i)
				{
					if (!number && body[i] != ' ')
					{
						number = true;
						extract.push_back(body[i]);
					}
					else if (number && body[i] != ' ')
					{
						extract.push_back(body[i]);
					}
					else if (number && body[i] == ' ')
					{
						number = false;
						raw_v_data.push_back(std::stof(extract));
						extract = "";
					}

					if (number && (i + 1 == body.length()))
					{
						raw_v_data.push_back(std::stof(extract));
					}
				}
			}
			else if (header == "vt")
			{
				std::string body = line.substr(2, std::string::npos);
				bool number = false;
				std::string extract = "";
				for (int i = 0; i < body.length(); ++i)
				{
					if (!number && body[i] != ' ')
					{
						number = true;
						extract.push_back(body[i]);
					}
					else if (number && body[i] != ' ')
					{
						extract.push_back(body[i]);
					}
					else if (number && body[i] == ' ')
					{
						number = false;
						raw_vt_data.push_back(std::stof(extract));
						extract = "";
					}

					if (number && (i + 1 == body.length()))
					{
						raw_vt_data.push_back(std::stof(extract));
					}
				}
			}
			if (header == "vn")
			{
				std::string body = line.substr(2, std::string::npos);
				bool number = false;
				std::string extract = "";
				for (int i = 0; i < body.length(); ++i)
				{
					if (!number && body[i] != ' ')
					{
						number = true;
						extract.push_back(body[i]);
					}
					else if (number && body[i] != ' ')
					{
						extract.push_back(body[i]);
					}
					else if (number && body[i] == ' ')
					{
						number = false;
						raw_vn_data.push_back(std::stof(extract));
						extract = "";
					}

					if (number && (i + 1 == body.length()))
					{
						raw_vn_data.push_back(std::stof(extract));
					}
				}
			}
			else if (header == "f ")
			{
				std::string body = line.substr(2, std::string::npos);
				bool face = false;
				std::string extract = "";
				for (int i = 0; i < body.length(); ++i)
				{
					if (!face && body[i] != ' ')
					{
						face = true;
						extract.push_back(body[i]);
					}
					else if (face && body[i] != ' ')
					{
						extract.push_back(body[i]);
					}
					else if (face && body[i] == ' ')
					{
						face = false;
						raw_f_data.push_back(faceArray(extract));
						extract = "";
					}

					if (face && (i + 1 == body.length()))
					{
						raw_f_data.push_back(faceArray(extract));
					}
				}
			}


		}
	}
	//End of Read and Parse File

	//Simple menu for selection of algorithm
	std::cout << "Loaded Data" << std::endl;

	std::cout << "Select Algorithm:" << std::endl;
	std::cout << "1- Polygon Reduction" << std::endl;
	std::cout << "2- Not Implemented" << std::endl;
	std::cout << "3- Not Implemented" << std::endl;
	std::cout << "Select Algorithm: " ;
	int algorithm;

	std::cin >> algorithm; //insert number 

	switch (algorithm)
	{
	case 1:
		polygonReduction(raw_v_data,raw_vt_data,raw_vn_data,raw_f_data);
		break;
	case 2:
		break;
	case 3:
		break;
	defalut:
		break;
	}

	//this code create a .obj file from the original data.
/*
	std::cin >> file_name;

	std::string objformat = "";
	int i = 0;
	for (std::vector<float>::iterator it = raw_v_data.begin(); it != raw_v_data.end(); ++it)
	{
		if (i % 3 == 0) objformat += "\nv ";
		objformat += " ";
		objformat += std::to_string(*it);

		++i;
	}
	for (std::vector<float>::iterator it = raw_vn_data.begin(); it != raw_vn_data.end(); ++it)
	{
		if (i % 3 == 0) objformat += "\nvn";
		objformat += " ";
		objformat += std::to_string(*it);

		++i;
	}
	for (std::vector<float>::iterator it = raw_vt_data.begin(); it != raw_vt_data.end(); ++it)
	{
		if (i % 3 == 0) objformat += "\nvt";
		objformat += " ";
		objformat += std::to_string(*it);

		++i;
	}
	for (std::vector<std::vector<int>>::iterator it = raw_f_data.begin(); it != raw_f_data.end(); ++it)
	{
		if (i % 3 == 0) objformat += "\nf ";
		objformat += " ";
		if ((*it)[1] == -1 && (*it)[2] == -1) objformat += std::to_string((*it)[0] + 1);
		else if ((*it)[1] != -1 && (*it)[2] == -1) objformat += std::to_string((*it)[0] + 1) + "/" + std::to_string((*it)[1] + 1);
		else if ((*it)[1] != -1 && (*it)[2] != -1) objformat += std::to_string((*it)[0] + 1) + "/" + std::to_string((*it)[1] + 1) + "/" + std::to_string((*it)[2] + 1);
		else if ((*it)[1] == -1 && (*it)[2] != -1) objformat += std::to_string((*it)[0] + 1) + "/" + "/" + std::to_string((*it)[2] + 1);

		++i;
	}


	std::ofstream outputfile;
	outputfile.open(file_name);
	outputfile << objformat;
	outputfile.close();

	std::cin >> file_name;
	*/
	return 0;
}

