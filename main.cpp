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
							, { "100", 4 }	, { "101", 5 } 	, { "110", 6 } , { "111", 7 } }; //二進制轉十進制

class counter { //counter
public:
	int index; //0 ~ 3
	string tnt; //SN or WN or WT or ST
	counter() { //初始化
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
	predictor() { //初始化
		index = "000";
		counters.resize(8);
	}
};

stringstream ss; //字串流，用來切割字串
vector <string> PC, inst, operand; //ex: PC: 0x110, inst: li, operand: R2,0
map <string, int> label, reg; //用來存label要跳去哪一行和register的值
vector <predictor> P; //預測器
vector <int> misprediction; //存misrediction
int N = 0; //entry

void input() {
	while (N <= 0 || (N & (N - 1))) { //給使用者輸入entry，同時也避免掉小於等於零和非2的次方
		cout << "Please input entry (entry > 0): ";
		cin >> N;
	}
}

void init() {
	P.resize(N); //N個enttry就給N個預測器
	misprediction.resize(N); //用來存N個預測器的misprediction
}

void read() {
	ifstream inFile("input.txt", ios::in); //開檔
	if (!inFile) { //確定檔案有打開
		cout << "File could not be opened!\n";
		system("pause");
		exit(1);
	}

	while (!inFile.eof()) { //一直讀到檔案結尾
		ss.str("");
		ss.clear(); //清空字串流
		string buffer;
		getline(inFile, buffer); //讀一整行進buffer
		if (buffer.size() == 0) //避免掉空行
			continue;
		ss << buffer; //將buffer送進字串流
		string temp; //用來暫存切割好的字串
		if (!isdigit(buffer[0])) { //判斷是否為label
			ss >> temp; //將切割好的字串存入temp裡
			temp.erase(temp.end() - 1); //把label最後的冒號去掉
			label[temp] = PC.size(); //將label的下一行存入label的map裡
			continue;
		}
		ss >> temp;//將切割好的字串存入temp裡
		PC.push_back(temp); //存入PC
		ss >> temp;//將切割好的字串存入temp裡
		inst.push_back(temp); //存入inst
		ss >> temp;//將切割好的字串存入temp裡
		operand.push_back(temp); //存入operand
	}
}

bool _pre(int i) {
	if (i) {
		i %= N; //用來判斷用哪個entry
	}

	if (P[i].counters[BINtoDEC[P[i].index]].index < 2) { //判斷是否taken
		return false; //SN or WN
	}
	else
		return true; // WT or ST
}

int ALU(int i) {
	int begin = 0;
	int end = operand[i].find(",");
	string temp = operand[i].substr(begin, end); //切割第一個運算元

	begin = end + 1;
	end = operand[i].find(",", begin);

	string temp2 = operand[i].substr(begin, end - begin); //切割第二個運算元

	if (inst[i] == "li") {
		reg[temp] = stoi(temp2);
		return 0;
	}
	else {
		begin = end + 1;
		end = operand[i].find(",", begin);
		string temp3 = operand[i].substr(begin, end - begin); //切割第三個運算元

		if (inst[i] == "add") { //ADD
			reg[temp] = reg[temp2] + reg[temp3]; //運算並更新register
			return 0;
		}
		else if (inst[i] == "addi") { //ADDI
			reg[temp] = reg[temp2] + stoi(temp3); //運算並更新register
			return 0;
		}
		else if (inst[i] == "andi") { //ANDI
			reg[temp] = reg[temp2] % (stoi(temp3) + 1); //運算並更新register
			return 0;
		}
		else if (inst[i] == "beq") { //BEQ
			if (reg[temp] == reg[temp2]) //如果成立的話代表會跳
				return label[temp3]; //回傳要跳去哪一行
			else
				return 0; //不會跳就return 0
		}
		else if (inst[i] == "bne") { //BNQ
			if (reg[temp] != reg[temp2]) //如果成立的話代表會跳
				return label[temp3]; //回傳要跳去哪一行
			else
				return 0; //不會跳就return 0
		}
	}
}

void output(ostream& outFile, bool pre, bool out, int i) {
	outFile << "entry: " << i % N << endl; //輸出第幾個entry
	outFile << left << setw(10) << PC[i] << left << setw(7) << inst[i] << left << setw(20) << operand[i] << endl; //輸出PC、instruction、operand
	outFile << left << setw(10) << P[i % N].index; //輸出3-bit history
	outFile << "(";
	for (int j = 0; j < P[i % N].counters.size(); ++j) { //輸出2-bit counters
		if (j)
			outFile << ", ";
		outFile << P[i % N].counters[j].tnt;
	}
	outFile << left << setw(6) << ")";
	outFile << "prediction / outcome: ";
	if (pre) //輸出預測器的結果
		outFile << right << setw(3) << "T";
	else
		outFile << right << setw(3) << "NT";
	if (out) //輸出實際的結果
		outFile << right << setw(3) << "T";
	else
		outFile << right << setw(3) << "NT";
	outFile << right << setw(20) << "misprediction: " << misprediction[i % N] << endl << endl; //輸出misprediction
}

void update(bool pre, int out, int& i) {
	if (out) { //實際的結果會跳
		P[i % N].counters[BINtoDEC[P[i % N].index]]++; //SN >> WN >> WT >> ST
		P[i % N].index[0] = P[i % N].index[1];
		P[i % N].index[1] = P[i % N].index[2];
		P[i % N].index[2] = '1'; //將3-bit history往左移並更新最右邊的bit
		i = out - 1; //將i設為out - 1，也就是要跳去的那一行，這裡減一是因為for完之後都會++i，所以要減一來平衡
	}
	else { //實際的結果不會跳
		P[i % N].counters[BINtoDEC[P[i % N].index]]--; // SN << WN << WT << ST
		P[i % N].index[0] = P[i % N].index[1];
		P[i % N].index[1] = P[i % N].index[2];
		P[i % N].index[2] = '0'; //將3-bit history往左移並更新最右邊的bit
	}
}

void exec() {
	ofstream outFile("output.txt", ios::out); //寫檔要用到的東西
	if (!outFile) { //確定檔案有打開
		cout << "File could not be opened!\n";
		system("pause");
		exit(1);
	}
	bool pre;
	int out;
	for (int i = 0; i < PC.size(); ++i) {
		pre = _pre(i); //return預測的結果
		out = ALU(i); //return要跳去哪一行，如果不跳的話return 0
		if (pre != (bool)out) { //如果prediction和outcome不符合的話就把misprediction加一;
			++misprediction[i % N];
		}
		output(cout, pre, out, i); //輸出結果
		output(outFile, pre, out, i); //寫入output
		update(pre, out, i); //更新預測器
	}
}

int main() {
	input(); //輸入entry
	init(); //初始化資料結構
	read(); //讀檔
	exec(); //開始執行
}
