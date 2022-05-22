#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <cstring>
#include <iomanip>
#include <cmath>

using namespace std;

vector <string> TNT = { "SN", "WN", "WT", "ST" }; //2-bit counter
map <string, int> BINtoDEC = { { "000", 0 }, { "001", 1 }, { "010", 2 }, { "011", 3 }
							, { "100", 4 }	, { "101", 5 } 	, { "110", 6 } , { "111", 7 } }; //�G�i����Q�i��

class counter { //counter
public:
	int index; //0 ~ 3
	string tnt; //SN or WN or WT or ST
	counter() { //��l��
		index = 0;
		tnt = TNT[index];
	}
	void operator++(int) { //SN >> WN >> WT >> ST
		if (index == 3)
			return;
		tnt = TNT[++index];
	}
	void operator--(int) { //SN << WN << WT << ST
		if (index == 0)
			return;
		tnt = TNT[--index];
	}
};

class predictor { //3-bit predictor
public:
	string index; //3-bit history
	vector <counter> counters; //2-bit counters
	predictor() { //��l��
		index = "000";
		counters.resize(8);
	}
};

stringstream ss; //�r��y�A�ΨӤ��Φr��
vector <string> PC, inst, operand; //ex: PC: 0x110, inst: li, operand: R2,0
map <string, int> label, reg; //�ΨӦslabel�n���h���@��Mregister����
vector <predictor> P; //�w����
vector <int> misprediction; //�smisrediction
int N = 0; //entry

void input() {
	while (N <= 0 || (N & (N - 1))) { //���ϥΪ̿�Jentry�A�P�ɤ]�קK���p�󵥩�s�M�D2������
		cout << "Please input entry (entry > 0): ";
		cin >> N;
	}
}

void init() {
	P.resize(N); //N��enttry�N��N�ӹw����
	misprediction.resize(N); //�ΨӦsN�ӹw������misprediction
}

void read() {
	ifstream inFile("input.txt", ios::in); //�}��
	if (!inFile) { //�T�w�ɮצ����}
		cout << "File could not be opened!\n";
		system("pause");
		exit(1);
	}

	while (!inFile.eof()) { //�@��Ū���ɮ׵���
		ss.str("");
		ss.clear(); //�M�Ŧr��y
		string buffer;
		getline(inFile, buffer); //Ū�@���ibuffer
		if (buffer.size() == 0) //�קK���Ŧ�
			continue;
		ss << buffer; //�Nbuffer�e�i�r��y
		string temp; //�ΨӼȦs���Φn���r��
		if (!isdigit(buffer[0])) { //�P�_�O�_��label
			ss >> temp; //�N���Φn���r��s�Jtemp��
			temp.erase(temp.end() - 1); //��label�̫᪺�_���h��
			label[temp] = PC.size(); //�Nlabel���U�@��s�Jlabel��map��
			continue;
		}
		ss >> temp;//�N���Φn���r��s�Jtemp��
		PC.push_back(temp); //�s�JPC
		ss >> temp;//�N���Φn���r��s�Jtemp��
		inst.push_back(temp); //�s�Jinst
		ss >> temp;//�N���Φn���r��s�Jtemp��
		operand.push_back(temp); //�s�Joperand
	}
}

bool _pre(int i) {
	if (i) {
		i %= N; //�ΨӧP�_�έ���entry
	}

	if (P[i].counters[BINtoDEC[P[i].index]].index < 2) { //�P�_�O�_taken
		return false; //SN or WN
	}
	else
		return true; // WT or ST
}

int ALU(int i) {
	int begin = 0;
	int end = operand[i].find(",");
	string temp = operand[i].substr(begin, end); //���βĤ@�ӹB�⤸

	begin = end + 1;
	end = operand[i].find(",", begin);

	string temp2 = operand[i].substr(begin, end - begin); //���βĤG�ӹB�⤸

	if (inst[i] == "li") {
		reg[temp] = stoi(temp2);
		return 0;
	}
	else {
		begin = end + 1;
		end = operand[i].find(",", begin);
		string temp3 = operand[i].substr(begin, end - begin); //���βĤT�ӹB�⤸

		if (inst[i] == "add") { //ADD
			reg[temp] = reg[temp2] + reg[temp3]; //�B��ç�sregister
			return 0;
		}
		else if (inst[i] == "addi") { //ADDI
			reg[temp] = reg[temp2] + stoi(temp3); //�B��ç�sregister
			return 0;
		}
		else if (inst[i] == "andi") { //ANDI
			reg[temp] = reg[temp2] % (stoi(temp3) + 1); //�B��ç�sregister
			return 0;
		}
		else if (inst[i] == "beq") { //BEQ
			if (reg[temp] == reg[temp2]) //�p�G���ߪ��ܥN��|��
				return label[temp3]; //�^�ǭn���h���@��
			else
				return 0; //���|���Nreturn 0
		}
		else if (inst[i] == "bne") { //BNQ
			if (reg[temp] != reg[temp2]) //�p�G���ߪ��ܥN��|��
				return label[temp3]; //�^�ǭn���h���@��
			else
				return 0; //���|���Nreturn 0
		}
	}
}

void output(ostream& outFile, bool pre, bool out, int i) {
	outFile << "entry: " << i % N << endl; //��X�ĴX��entry
	outFile << left << setw(10) << PC[i] << left << setw(7) << inst[i] << left << setw(20) << operand[i] << endl; //��XPC�Binstruction�Boperand
	outFile << left << setw(10) << P[i % N].index; //��X3-bit history
	outFile << "(";
	for (int j = 0; j < P[i % N].counters.size(); ++j) { //��X2-bit counters
		if (j)
			outFile << ", ";
		outFile << P[i % N].counters[j].tnt;
	}
	outFile << left << setw(6) << ")";
	outFile << "prediction / outcome: ";
	if (pre) //��X�w���������G
		outFile << right << setw(3) << "T";
	else
		outFile << right << setw(3) << "NT";
	if (out) //��X��ڪ����G
		outFile << right << setw(3) << "T";
	else
		outFile << right << setw(3) << "NT";
	outFile << right << setw(20) << "misprediction: " << misprediction[i % N] << endl << endl; //��Xmisprediction
}

void update(bool pre, int out, int& i) {
	if (out) { //��ڪ����G�|��
		P[i % N].counters[BINtoDEC[P[i % N].index]]++; //SN >> WN >> WT >> ST
		P[i % N].index[0] = P[i % N].index[1];
		P[i % N].index[1] = P[i % N].index[2];
		P[i % N].index[2] = '1'; //�N3-bit history�������ç�s�̥k�䪺bit
		i = out - 1; //�Ni�]��out - 1�A�]�N�O�n���h�����@��A�o�̴�@�O�]��for�����᳣�|++i�A�ҥH�n��@�ӥ���
	}
	else { //��ڪ����G���|��
		P[i % N].counters[BINtoDEC[P[i % N].index]]--; // SN << WN << WT << ST
		P[i % N].index[0] = P[i % N].index[1];
		P[i % N].index[1] = P[i % N].index[2];
		P[i % N].index[2] = '0'; //�N3-bit history�������ç�s�̥k�䪺bit
	}
}

void exec() {
	ofstream outFile("output.txt", ios::out); //�g�ɭn�Ψ쪺�F��
	if (!outFile) { //�T�w�ɮצ����}
		cout << "File could not be opened!\n";
		system("pause");
		exit(1);
	}
	bool pre;
	int out;
	for (int i = 0; i < PC.size(); ++i) {
		pre = _pre(i); //return�w�������G
		out = ALU(i); //return�n���h���@��A�p�G��������return 0
		if (pre != (bool)out) { //�p�Gprediction�Moutcome���ŦX���ܴN��misprediction�[�@;
			++misprediction[i % N];
		}
		output(cout, pre, out, i); //��X���G
		output(outFile, pre, out, i); //�g�Joutput
		update(pre, out, i); //��s�w����
	}
}

int main() {
	input(); //��Jentry
	init(); //��l�Ƹ�Ƶ��c
	read(); //Ū��
	exec(); //�}�l����
}
