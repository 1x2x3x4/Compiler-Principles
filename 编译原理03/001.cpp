#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
using namespace std;

const int maxSize = 20;
const string errorstr = "你输入的语言该文法无法识别";
const int outWidth = 20;
int terminatorNum, nonterminatorNum, stateNum, num;
int Goto[maxSize][maxSize];
char action[maxSize][maxSize];
int terAction[maxSize][maxSize];
string chang[maxSize];
map<char, int> terminatorSet;
map<char, int> nonterminatorSet;
vector<char> signStack;
vector<int> stateStack;

void analysisProcess(string);
string intToString(int);

int main() {
    /*system("chcp 65001");*/
    int i, j; char str;
    cout << "请输入产生式的个数："; cin >> num;
    cout << "请按顺序输入产生式：\n";
    for (i = 1; i <= num; ++i) cin >> chang[i];
    cout << "请输入终结符的个数："; cin >> terminatorNum;
    cout << "请依次按照分析表输入终结符(包含结束符#)：" << endl;
    for (i = 1; i <= terminatorNum; ++i) { cin >> str; terminatorSet[str] = i; }
    cout << "请输入非终结符的个数："; cin >> nonterminatorNum;
    cout << "请依次按照分析表输入终结符：" << endl;
    for (i = 1; i <= nonterminatorNum; ++i) { cin >> str; nonterminatorSet[str] = i; }
    cout << "请输入最后一个状态数（从0开始）："; cin >> stateNum;
    cout << "请依次输入非终结符goto表的内容，对于出错错误的请输入-1:\n";
    for (i = 0; i <= stateNum; ++i) {
        for (j = 1; j <= nonterminatorNum; ++j) { cin >> Goto[i][j]; }
    }
    cout << "请依次输入终结符action的内容，对于出错错误的请输入-1:\n";
    cout << "输入格式按照 移进(S state) 规约(R state) 接受(A -1) 出错(E -1),state代表状态数\n";
    for (i = 0; i <= stateNum; ++i) {
        for (j = 1; j <= terminatorNum; ++j) { cin >> action[i][j] >> terAction[i][j]; }
    }
    cout << "请输入要分析语言: "; string L; cin >> L;
    analysisProcess(L);
    system("pause");
    return 0;
}

void analysisProcess(string str) {
    str += "#";
    cout << setw(outWidth) << setiosflags(ios::left) << "步骤";
    cout << setw(outWidth) << setiosflags(ios::left) << "状态栈";
    cout << setw(outWidth) << setiosflags(ios::left) << "符号栈";
    cout << setw(outWidth) << setiosflags(ios::left) << "产生式" << "输入串" << endl;
    signStack.clear(); stateStack.clear();
    signStack.push_back('#'); stateStack.push_back(0);
    int i = 0, ip = 0, Sm, t;
    int a = terminatorSet[str[ip]];
    cout << setw(outWidth) << setiosflags(ios::left) << (++i);
    cout << setw(outWidth) << setiosflags(ios::left) << "0";
    cout << setw(outWidth) << setiosflags(ios::left) << "#";
    cout << setw(outWidth) << setiosflags(ios::left) << " " << str << endl;
    string outStr;
    while (++i) {
        Sm = stateStack.back();
        if (action[Sm][a] == 'S') {
            stateStack.push_back(terAction[Sm][a]);
            signStack.push_back(str[ip]);
            a = terminatorSet[str[++ip]];
            cout << setw(outWidth) << setiosflags(ios::left) << i;
            outStr = "";
            for (t = 0; t < stateStack.size(); ++t) outStr += intToString(stateStack[t]);
            cout << setw(outWidth) << setiosflags(ios::left) << outStr;
            outStr = "";
            for (t = 0; t < signStack.size(); ++t) outStr += signStack[t];
            cout << setw(outWidth) << setiosflags(ios::left) << outStr;
            cout << setw(outWidth) << setiosflags(ios::left) << " " << str.substr(ip) << endl;
        }
        else if (action[Sm][a] == 'R') {
            int j = terAction[Sm][a];
            int size = chang[j].size() - 3;
            if (chang[j][3] == '@') size = 0;
            for (t = 0; t < size; ++t) stateStack.pop_back();
            for (t = 0; t < size; ++t) signStack.pop_back();
            Sm = stateStack.back();
            char A = chang[j][0]; signStack.push_back(A);
            stateStack.push_back(Goto[Sm][nonterminatorSet[A]]);
            cout << setw(outWidth) << setiosflags(ios::left) << i;
            outStr = "";
            for (t = 0; t < stateStack.size(); ++t) outStr += intToString(stateStack[t]);
            cout << setw(outWidth) << setiosflags(ios::left) << outStr;
            outStr = "";
            for (t = 0; t < signStack.size(); ++t) outStr += signStack[t];
            cout << setw(outWidth) << setiosflags(ios::left) << outStr;
            cout << setw(outWidth) << setiosflags(ios::left) << chang[j] << str.substr(ip) << endl;
        }
        else if (action[Sm][a] == 'A') {
            cout << "该文法识别该语言" << endl; return;
        }
        else {
            cout << errorstr << endl; return;
        }
    }
}

string intToString(int num) {
    int j = num;
    if (j == 0) return "0";
    string str = "";
    while (j) { str += char(j % 10 + '0'); j /= 10; }
    reverse(str.begin(), str.end());
    if (num > 9) str = "(" + str + ")";
    return str;
}