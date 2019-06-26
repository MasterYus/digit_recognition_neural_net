#include "pch.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <time.h>
#include <string>
#include <random>
using namespace std;

//Abstract single neuron interpretation
class Neuron
{
public:
	//Variable access functions
	void setOutputWeights(vector<double> weights) {
		output_weights = weights;
	}
	void setWeight(double weight, int n) {
		output_weights[n] = weight;
	}
	void initWeight(double weight) {
		output_weights.push_back(weight);
		prev_weight_delta.push_back(0);
	}
	void setPrevWeight(double weight, int n) {
		prev_weight_delta[n] = weight;
	}
	vector<double> getOutputWeights() { return output_weights; }
	vector<double> getPrevWeights() { return prev_weight_delta; }
	void setValue(double init) { value = init; };
	double getValue() { return value; }
	double getDelta() { return delta; }
	void setDelta(double delta) {
		this->delta = delta;
	}

private:
	double value = 0;
	double delta = 0;
	vector<double> output_weights;
	vector<double> prev_weight_delta;
};

//function prototypes
void initInput();
void initRandomWeights();
double round_to(double value, int digits);
vector<double> getRandomWeights(int size);
void feedForward();
double computeMiddleDelta(vector<Neuron> &next_layer, Neuron &curent_neuron);
void backPropagation();
void testInput();
void outputValue();
void backupData();
void restoreData();
void initLearningMode();
double computeError();

double sig(double x);
double sigd(double x);

//Net topology
const int output_neurons = 10;
const int middle_neurons = 16;
const int input_neurons = 784;

//Global params
double eps = 0.3; //epsilon, speed of learning
double a = 0.25; //momentum
double exd_err = 15; //Acceptable error(%)

bool debug = true; //output extra values
int learning_set = 10000; //amount of samples

bool learning_mode = false;
vector<double> input, ideal;
vector<Neuron> lay_in(input_neurons), lay_out(output_neurons), lay_m1(middle_neurons), lay_m2(middle_neurons);

//sigmoid activation
double sig(double x) { return 1 / (1 + exp(-x)); }
double sigd(double x) { return (1 - x) * x; }

int main()
{
	int option;
	restoreData();
	while(true){

	cout << "YUS NeuronNet Launcher v1.0.0" << endl;
	cout << "----------------------------" << endl;
	cout << "1. First init" << endl;
	cout << "2. Continue recognition" << endl;
	cout << "3. Learning mode" << endl;
	cout << "4. Exit saving data" <<endl;
	cout << "   Choose option: ";

	cin >> option;
	switch (option) {
	case 1: 
		initRandomWeights();
		cout << "Random weights set! Ready to go..." << endl;
		break;
	case 2: 
		testInput();
		feedForward();
		outputValue();
		break;
	case 3: 
		initLearningMode();
		break;
	case 4: 
		backupData();
		return 0;
		break;
	default: break;
	}
	system("pause");
	system("cls");
	}
}

//Initialize input layer
void initInput() {
	for (int i = 0 ; i < input.size(); i++) {
		lay_in[i].setValue(input[i]);
	}
}

//Set random weights
void initRandomWeights() {
	for (auto &v : lay_in) {
		v.setOutputWeights(getRandomWeights(middle_neurons));
	}
	for (auto &v : lay_m1) {
		v.setOutputWeights(getRandomWeights(middle_neurons));
	}
	for (auto &v : lay_m2) {
		v.setOutputWeights(getRandomWeights(output_neurons));
	}
}

//Round function
double round_to(double value, int digits) {
	if (value == 0.0)
		return 0.0;
	double factor = pow(10.0, digits - ceil(log10(fabs(value))));
	return round(value * factor) / factor;
}

//Random weights generator
vector<double> getRandomWeights(int size) {
	vector<double> weights;
	double max = 0.5, min = -0.5;
	srand(time(NULL));
	for (int i = 0; i < size; i++) {
		weights.push_back(round_to((max - min) * ((double)rand() / (double)RAND_MAX) + min, 2));
		//cout << round_to(rand() / double(RAND_MAX), 2) << endl;
	}
	return weights;
}

//every neuron a(1)= a(0).1*w1 + a(0).2*w2...
void feedForward() {
	int current_neuron = 0;
	for (auto &v : lay_m1) {
		for (int j = 0; j < lay_in.size(); j++) {
			v.setValue(v.getValue() + lay_in[j].getValue() * lay_in[j].getOutputWeights()[current_neuron]);
		}
		//v.setValue(v.getValue() + vector_bias[i];
		v.setValue(sig(v.getValue()));
		current_neuron++;
		
	}
	current_neuron = 0;
	for (auto &v : lay_m2) {
		for (int j = 0; j < lay_m1.size(); j++) {
			v.setValue(v.getValue() + lay_m1[j].getValue() * lay_m1[j].getOutputWeights()[current_neuron]);
		}
		//v.setValue(v.getValue() + vector_bias[i];
		v.setValue(sig(v.getValue()));
		current_neuron++;
	}
	current_neuron = 0;
	for (auto &v : lay_out) {
		for (int j = 0; j < lay_m2.size(); j++) {
			v.setValue(v.getValue() + lay_m2[j].getValue() * lay_m2[j].getOutputWeights()[current_neuron]);
		}
		//v.setValue(v.getValue() + vector_bias[i];
		v.setValue(sig(v.getValue()));
		current_neuron++;
	}
}

void outputValue() {
	if (debug) cout << "---- Begin output stream ----" << endl;
	double max = (double)INT_MIN;
	double av_error = 0;
	int label, iterator = 0;
	for (auto &v : lay_out) {
		if (debug) cout << iterator << ") "<< v.getValue() << endl;
		if (v.getValue() > max){
			max = v.getValue();
		label = iterator;
		}
		iterator++;
		}
	if(learning_mode) cout << "Average error: " << computeError() << "%" << endl;
	cout << "Recognized: " << label << endl;
	if (debug) cout << "---- End output stream ----" << endl;
}

double computeError() {
	double av_error = 0;
	int iterator = 0;
	for (auto &v : ideal) {
		 av_error += pow(v - lay_out[iterator].getValue(), 2);
		 iterator++;
	}
	av_error = (av_error / 2) * 100;
	return av_error;
}

//Teach it!
void initLearningMode() {
	//init randomizer
	random_device rd;
	mt19937 rng(rd());
	uniform_int_distribution<int> uni(0, 9);
	int random;
	ifstream finput;
	learning_mode = true;
	
	vector<string> database = { "data0","data1", "data2", "data3", "data4", "data5", "data6","data7","data8","data9" };
	vector<int> database_iterator = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	//Initialize database
	vector<vector<unsigned char>> database_img;
	int iterator = 0;
	cout << "---- Begin datatbase initatialization ----" << endl;
	for (auto &v : database) {
		finput.open("mnist\\" + v, std::ios::binary);
		if (!finput.is_open()) {
			cout << endl << "fuck! No file!" << endl;
		}
		vector<unsigned char> img((
			istreambuf_iterator<char>(finput)),
			(istreambuf_iterator<char>()));
		database_img.push_back(img);
		finput.close();
		iterator++;
	}
	//Start training
	for (int i = 0; i < learning_set; i++) {
		random = uni(rng);
		//random = 3;
		if (database_iterator[random]<1000) {
			cout << "---- Current learning iteration: " << i << endl;
			cout << "Needed value: " << random << endl;
			input.clear();
			double value;
			for (int j = database_iterator[random] * 784; j < 784 * (database_iterator[random] + 1); j++) {
				value = round_to(abs((double)(database_img[random].at(j))) / 255, 1);
				//(value == 0.0) ? cout << "0.0" << " " : cout << value << " ";
				input.push_back(value);
				//if ((j + 1) % 28 == 0) cout << endl;
			}
			database_iterator[random]++;
			initInput();
			feedForward();
			ideal.clear();
			for (int f = 0; f < 10; f++) {
				(f == random) ? ideal.push_back(1.0) : ideal.push_back(0.0);
			}
			outputValue();
			if (computeError()>=exd_err){
			backPropagation();
			}
			else {
				//cout << "Backpropagation status: ";
				//cout << "Acceptable error! No backprop!" << endl;
			}
			cout << endl;
		}
		else {
			i--;
			bool stop = true;
			for (auto &v : database_iterator) {
				stop *= (v == 1000);
			}
			if (stop) break;
		}
	}
	learning_mode = false;
}

void testInput() {
	cout << "---- Begin recognition input stream ----" << endl;
	ifstream finput;
	finput.open("input.yaf");
	if (!finput.is_open()) {
		cout << endl << "fuck! No file!" << endl;
	}
	input.clear();
	double value;
	string buf;
	for (int i=0; i < 784;i++) {
		if (!finput.eof()) {
			finput >> buf;
		}
		value = stod(buf, NULL);
		cout << value << " ";
		input.push_back(value);
		if ((i + 1) % 28 == 0) cout << endl;
	}
	initInput();
	cout << "---- End recognition input stream ----" << endl;
}

//Delta function
double computeMiddleDelta(vector<Neuron> &next_layer, Neuron &curent_neuron) {
	double delta = 0;
	int iterator = 0;
	for (auto &v : next_layer) {
		delta += v.getDelta() * (curent_neuron.getOutputWeights()[iterator]);
		iterator++;
	}
	return delta;
}

//Learning function
void backPropagation() {
	double dw = 0; //store delta weight
	/* ideal output vector - ideal
	create delta for output neurons */
	int iterator = 0;
	// 10 output neurons in my case
	for (auto &v : lay_out) {
		v.setDelta((ideal[iterator]-v.getValue())*sigd(v.getValue()));
		if(debug) cout << iterator<< ": " <<v.getDelta() << endl;
		iterator++;
	}
	if (debug)cout << "-----\n";
	// 16 midle-level neurons
	for (auto &v : lay_m2) {
		v.setDelta(computeMiddleDelta(lay_out, v)*sigd(v.getValue()));
		if (debug) cout << iterator << ": " << v.getDelta() << endl;
	}
	if (debug)cout << "-----\n";
	for (auto &v : lay_m1) {
		v.setDelta(computeMiddleDelta(lay_m2, v)*sigd(v.getValue()));
		if (debug) cout << iterator << ": " << v.getDelta() << endl;
	}
	if (debug)cout << "-----\n";
	// After computing delta (input neurons has no delta) do finding GRADIENT and update weights
	for (auto &neuron : lay_in) {
		iterator = 0;
		for (auto &v : neuron.getOutputWeights()) {
			dw = eps * (neuron.getValue()*lay_m1[iterator].getDelta()) + a * neuron.getPrevWeights()[iterator];
			//cout << "( " << dw /*<< " : " << neuron.getValue()*lay_m1[iterator].getDelta() << " : " << a * neuron.getPrevWeights()[iterator] */<< ") \n";
			neuron.setWeight(v + dw, iterator);
			neuron.setPrevWeight(dw, iterator);
			iterator++;
		}
	}
	for (auto &neuron : lay_m1) {
		iterator = 0;
		for (auto &v : neuron.getOutputWeights()) {
			dw = eps * (neuron.getValue()*lay_m2[iterator].getDelta()) + a * neuron.getPrevWeights()[iterator];
			neuron.setWeight(v + dw, iterator);
			neuron.setPrevWeight(dw, iterator);
			iterator++;
		}
	}
	for (auto &neuron : lay_m2) {
		iterator = 0;
		for (auto &v : neuron.getOutputWeights()) {
			dw = eps * (neuron.getValue()*lay_out[iterator].getDelta()) + a * neuron.getPrevWeights()[iterator];
			neuron.setWeight(v + dw, iterator);
			neuron.setPrevWeight(dw, iterator);
			iterator++;
		}
	}
}

//Backups all weights
void backupData() {
	ofstream out("data.bin");
	for (auto &neuron : lay_in) {
		for (auto &v : neuron.getOutputWeights()) {
			out << v << " ";
		}
	}
	for (auto &neuron : lay_m1) {
		for (auto &v : neuron.getOutputWeights()) {
			out << v << " ";
		}
	}
	for (auto &neuron : lay_m2) {
		for (auto &v : neuron.getOutputWeights()) {
			out << v << " ";
		}
		
	}
	out.close();
}

//Restores all weights
void restoreData() {
	ifstream fin("data.bin");
	string buf;
	int iterator = 0;
	if(fin.is_open()){
	for (auto &neuron : lay_in) {
		for (int i = 0; i < lay_m1.size();i++) {
			if (!fin.eof()) {
				fin >> buf;
				neuron.initWeight(stod(buf, NULL));
			}
		}
	}
	for (auto &neuron : lay_m1) {
		for (int i = 0; i < lay_m2.size(); i++) {
			if (!fin.eof()) {
				fin >> buf;
				neuron.initWeight(stod(buf, NULL));
			}
		}
	}
	for (auto &neuron : lay_m2) {
		for (int i = 0; i < lay_out.size(); i++) {
			if (!fin.eof()) {
				fin >> buf;
				neuron.initWeight(stod(buf, NULL));
			}
		}
	}
	fin.close();
	}
}
