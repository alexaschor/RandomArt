#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>


#include "MERSENNE_TWISTER.h"
#include "vec3.h"


using namespace std;

MERSENNE_TWISTER twister;
hash<string> stringHasher;


void writePPM(string filename, int xRes, int yRes, double* values)
{
	int totalCells = xRes * yRes;
	unsigned char* pixels = new unsigned char[3 * totalCells];
	for (int i = 0; i < 3 * totalCells; i++)
		pixels[i] = values[i];

	FILE *fp;
	fp = fopen(filename.c_str(), "wb");
	if (fp == NULL)
	{
		cout << " Could not open file \"" << filename.c_str() << "\" for writing." << endl;
		cout << " Make sure you're not trying to write from a weird location or with a " << endl;
		cout << " strange filename. Bailing ... " << endl;
		exit(0);
	}

	fprintf(fp, "P6\n%d %d\n255\n", xRes, yRes);
	fwrite(pixels, 1, totalCells * 3, fp);
	fclose(fp);
	delete[] pixels;
}

class Operation {
	public:
		virtual int getArity() = 0;		
		virtual vec3 eval(double x, double y) = 0;

		void setInputs(vector<Operation*> in) {
			inputs = in; 
		} 

		~Operation() {
			for (int i = 0; i < inputs.size(); ++i) {
				delete inputs[i];
			}
		}

	protected:
		vector<Operation *> inputs;
		vector<double> constants;
};

class Sum : public Operation {
	public:
		int getArity() {
			return 2;
		}
		 
		vec3 eval(double x, double y) {
			return(inputs[0]->eval(x, y) + inputs[1]->eval(x, y));
		}

};

class Product : public Operation {
	public:
		int getArity() {
			return 2;
		}
		 
		vec3 eval(double x, double y) {
			return(inputs[0]->eval(x, y) * inputs[1]->eval(x, y));
		}
};

class Mod : public Operation {
	public:
		int getArity() {
			return 2;
		}
		 
		vec3 eval(double x, double y) {
			vec3 a = inputs[0]->eval(x, y);
			vec3 b = inputs[1]->eval(x, y);
			return vec3(
					fmod(a[0], b[0]),
					fmod(a[1], b[1]),
					fmod(a[2], b[2])
					);

		}
};

class Constant: public Operation {
	public:
		int getArity() {
			return 0;
		}
		
		Constant(){
			constants = vector<double>();
			constants.push_back(twister.rand());
		}

		vec3 eval(double x, double y) {
			return vec3(constants[0], constants[0], constants[0]);
		}
};

class VarX: public Operation {
	public:
		int getArity() {
			return 0;
		}
		
		VarX(){}

		vec3 eval(double x, double y) {
			return vec3(x, x, x);
		}
};

class VarY: public Operation {
	public:
		int getArity() {
			return 0;
		}
		
		VarY(){}

		vec3 eval(double x, double y) {
			return vec3(y, y, y);
		}
};

class Circle: public Operation {
	public:
		int getArity() {
			return 0;
		}

		Circle(){
			constants.push_back(twister.rand()); // Center X
			constants.push_back(twister.rand()); // Center Y
		}

		vec3 eval(double x, double y) {
			double val = hypot(x - constants[0], y - constants[1]);
			return vec3(val, val, val);
		}
};

class Inverse: public Operation {
	public: 
		int getArity() {
			return 1;
		}

		Inverse(){}

		vec3 eval(double x, double y) {
			return vec3(1,1,1) - inputs[0]->eval(x, y);
		}
};

class PerChannelMask: public Operation {
	public:
		int getArity() {
			return 3;
		}

		PerChannelMask(){
			constants.push_back(twister.rand()); // Threshold to binarize mask
		}

		vec3 eval(double x, double y) {
			vec3 m = inputs[0]->eval(x,y);
			vec3 a = inputs[1]->eval(x,y);
			vec3 b = inputs[2]->eval(x,y);

			double t = constants[0];

			return vec3(
					(m[0] > t) ? a[0] : b[0],
					(m[1] > t) ? a[1] : b[1],
					(m[2] > t) ? a[2] : b[2]);
		}
};

class ColorMix: public Operation {
	public: 
		int getArity() {
			return 3;
		}

		ColorMix() {}

		vec3 eval(double x, double y) {
			double r = inputs[0]->eval(x, y)[0];
			double g = inputs[1]->eval(x, y)[1];
			double b = inputs[2]->eval(x, y)[2];

			return vec3(r,g,b);
		}
};

class BinaryMask: public Operation {
	public:
		int getArity() {
			return 3;
		}

		BinaryMask(){
			constants.push_back(twister.rand());
		}

		vec3 eval(double x, double y) {
			vec3 m = inputs[0]->eval(x, y);
			vec3 a = inputs[1]->eval(x, y);
			vec3 b = inputs[1]->eval(x, y);

			double thresh = constants[0];

			return (m.length() > thresh)? a : b;
		}
};

class SmoothMix: public Operation {
	public:
		int getArity() {
			return 3;
		}

		SmoothMix(){}

		vec3 eval(double x, double y) {
			double weight = inputs[0]->eval(x, y).length();
			vec3 a = inputs[1]->eval(x, y);
			vec3 b = inputs[1]->eval(x, y);

			return (weight * a) + ((1 - weight) * b);
		}

};

class Well: public Operation {
	public:
		int getArity() {
			return 1;
		}

		Well() {}

		vec3 eval(double x, double y) {
			vec3 in = inputs[0]->eval(x, y);
			return vec3(
					well(in[0]),
					well(in[1]),
					well(in[2])
					);
		}

	private:
		double well(double x) {
			return pow(1 - 2 / (1 + x*x),  8);
		}
};

class Tent: public Operation {
	public:
		int getArity() {
			return 1;
		}

		Tent() {}

		vec3 eval(double x, double y) {
			vec3 in = inputs[0]->eval(x, y);
			return vec3(
					tent(in[0]),
					tent(in[1]),
					tent(in[2])
					);
		}

	private:
		double tent(double x) {
			return 1 - 2 * abs(x);
		}
};

template<typename T>
Operation *Create() { return new T(); }

typedef Operation * (*OpMaker)();

OpMaker internalOps[] = {
	&Create<Sum>, 
	&Create<PerChannelMask>,
	&Create<ColorMix>,
	&Create<BinaryMask>,
	&Create<SmoothMix>,
	&Create<Well>,
	&Create<Tent>,
	&Create<Product>,
	&Create<Inverse>,
	&Create<Mod>, 
}; 

OpMaker leaves[] = {
	&Create<Constant>,
	&Create<VarX>,
	&Create<VarY>,
	&Create<Circle>
};

const size_t numInternal = sizeof(internalOps)/sizeof(internalOps[0]);
const size_t numLeaves = sizeof(leaves)/sizeof(leaves[0]);

Operation *randOp() {
   return internalOps[twister.randInt(numInternal - 1)]();
}

Operation *randLeaf() {
   return leaves[twister.randInt(numLeaves - 1)]();
}

void setColor(double* values, int x, int y, int xRes, vec3 color) {
	int index = x + (y * xRes);
	values[3 * index] = (int) (color[0] * 255) % 255;
	values[3 * index + 1] = (int) (color[1] * 255) % 255;
	values[3 * index + 2] = (int) (color[2] * 255) % 255;
}

vector<Operation *> populate(int arity, int maxDepth) {
	if (arity == 0) {
		return vector<Operation *>();
	} else {
		vector<Operation *> args;
		for (int i = 0; i < arity; ++i) {
			Operation *o = (maxDepth > 1)? randOp() : randLeaf();
			o->setInputs(populate(o->getArity(), maxDepth - 1));
			args.push_back(o);
		}
		return args;
	}
}


int main(int argc, char** argv) {

	if (argc == 2) {
		string s(argv[1]);
		size_t h = stringHasher(s);
		twister.seed(h);
	} else if (argc == 1) {
		auto now = chrono::high_resolution_clock::now();
		string s = to_string(now.time_since_epoch().count());
		cout << "Using seed " << s << "\n" << endl;
		size_t h = stringHasher(s);
		twister.seed(h);
	} else {
		printf("Usage: %s or %s <seed>.", argv[0], argv[0]);
	}


	int xRes = 1000;
	int yRes = 1000;

	double minX = 0;
	double maxX = 1;
	double minY = 0;
	double maxY = 1;

	double* values = new double[xRes * yRes * 3];


	ColorMix *root = new ColorMix();
	root->setInputs(populate(root->getArity(), 5));

	for (int x = 0; x < xRes; ++x) {
		for (int y = 0; y < yRes; ++y) {
			double fx = minX + ((double) x/xRes) * maxX;
			double fy = minY + ((double) y/yRes) * maxY;
				
			vec3 color = root->eval(fx, fy);
			setColor(values, x, y, xRes, color);
		}
	}

	writePPM("random.ppm", xRes, yRes, values);
}
