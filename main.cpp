/*
 * main.cpp
 *
 *  Created on: Dec 7, 2011
 *      Author: daikit_dai
 */


#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/multi_array.hpp>

using namespace std;
namespace po = boost::program_options;

string input_filename;
string config_filename;

float VX, VY, VZ;;

int W, H, D;
int pixel_size = 32;  // float

boost::multi_array<float, 3> volume;


std::vector<std::string> split(std::string s, std::string c){
	std::vector<std::string> ret;
	for( int i=0, n; i <= s.length(); i=n+1 ){
			n = s.find_first_of( c, i );
			if( n == std::string::npos ) n = s.length();
			std::string tmp = s.substr( i, n-i );
			ret.push_back(tmp);
		}
	return ret;
}

void ParseCommandLine(int argc, char** argv){
	po::options_description desc("Allowd options");
	po::variables_map vm;
	desc.add_options()
					("help,h","produce help message")
					("input,i", po::value<string>(&input_filename), "input file name (sinogram)")
					("config,c", po::value<string>(&config_filename), "config file name")
					//("size,s", po::value<int>(&reconst_size), "reconst_size")
					("width,W", po::value<int>(&W), "input width")
					("height,H", po::value<int>(&H), "input height")
					("depth,D", po::value<int>(&D), "input depth")
					//("pixel_size,p", po::value<int>(&pixel_size), "pixel size")
					//("conv", po::value<string>(&conv_filename), "convolution file name")
					//("dstep,d", po::value<int>(&d_step)->default_value(1), "detector scan step (for reduce computation time)")
					//("pstep,p", po::value<int>(&p_step)->default_value(1), "projection scan step (for reduce computation time)")
					//("subN,s", po::value<int>(&subN)->default_value(30), "OS-EM method loop number")
					//("verbose,v", po::value<bool>(&verbose)->default_value(false), "set output verbose ")
					;

	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help")){
		cout << desc << endl;
		exit(-1);

	}

	cout << "input file name = " << input_filename << endl;
	cout << "config file name = " << config_filename << endl;
};

void ParseConfigFile(string config_file_name){
	po::options_description desc_config("config file option");
	po::variables_map vm;
	desc_config.add_options()
		("VX", po::value<float>(&VX), "voxel size in x")
		("VY", po::value<float>(&VY), "voxel size in y")
		("VZ", po::value<float>(&VZ), "voxel size in z")
	;

	ifstream ifs(config_file_name.c_str());
	store(parse_config_file(ifs, desc_config), vm);
	notify(vm);

	cout << "*** Configuration ***" << endl;
	cout << "VX : " << VX << endl;
	cout << "VY : " << VY << endl;
	cout << "VZ : " << VZ << endl;
};

int main(int argc, char** argv){

	ParseCommandLine(argc, argv);
	ParseConfigFile(config_filename);

	volume.resize(boost::extents[D][H][W]);

	FILE* in = fopen(input_filename.c_str(), "r+b");
	short *buff = new short[W];
	for(int k=0; k<D; k++){
		for(int j=0; j<H; j++){
			fread(buff, W, sizeof(short), in);
			for(int i=0; i<W; i++){
				volume[k][j][i] = (float)buff[i];// -(float)log((double)buff[i]);
			}
		}
	}
	fclose(in);


	string basename = split(input_filename, ".").at(0);

	std::ofstream ofs;
	ofs.open((basename + ".inr").c_str(), std::ios::binary);

	stringstream header;
	std::string headerEND = "##}\n";
	//Header
	header << "#INRIMAGE-4#{" << endl;
	header << "XDIM="<< W << endl;
	header << "YDIM="<< H << endl;
	header << "ZDIM="<< D << endl;
	header << "VDIM=" << 1 << endl;
	header << "VX=" << VX << endl;
	header << "VY=" << VY << endl;
	header << "VZ=" << VZ << endl;
	header << "TYPE=float" <<endl;
	header << "PIXSIZE=" << pixel_size << " bits" << endl;
	header << "CPU=decm" << endl;

	int hlen = 256 - header.str().length() - headerEND.length();
	for(int i=0; i<hlen; i++){
		header << "\n";
	}
	header << headerEND;

	ofs.write(header.str().c_str(), header.str().size());

	float *buf = new float[W];

	for(int i=0; i<D; i++){
		for(int j=0; j<H; j++){
			for(int k=0; k<W; k++){
				//ofs << volume[i][j][k];
				//buf[k] = volume[i][j][k];
				ofs.write((char*)&volume[i][j][k], sizeof(float));
			}

		}
	}

	ofs.close();
}
