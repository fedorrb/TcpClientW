// TCPClientW.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>
#include <exception>
#include "socketCl.h"

using namespace std;
using std::string;
using std::wstring;
using std::vector;

vector<string> sendLines; //повідомлення для відправки
vector<string> receivedLines; //прийняте повідомлення
vector<string> receivedArgs; //прийняті параметри
void LoadTxtFile (const char * filename, vector<string> & receive);
void SaveTxtFile (const char* filename, vector<string> & send);
string GetPathLogFile(string path, string insp);
void SplitStringLine(const std::string& str, vector<string> & cont, const std::string& delims = "\\");
int DisplayResourceNAMessageBox(wstring msg_w);
wstring GetMessageFromJson(vector<string> & receive);

int main(int argc, char *argv[])
{
	bool error = true;
	receivedArgs.clear();
	for(int i = 0; i < argc; i++) {
		receivedArgs.push_back(argv[i]);
	}
	if(argc == 5) {
		string srvrIP(receivedArgs[1]);
		unsigned short srvrPort = 1861;
		try {
			stringstream tmpStr(receivedArgs[2]);
			tmpStr >> srvrPort;
		}
		catch(...) {
			srvrPort = 1861;
		}
		string localRaj(receivedArgs[3]);
		string localInsp(receivedArgs[4]);
		
		sendLines.clear();
		receivedLines.clear();
		string sendStr = "rajon:" + localRaj + ",insp:" + localInsp + ",code:777|[{\"Req_Rajon\":\"" + localRaj + "\"}]\r\n\1";
		sendLines.push_back(sendStr);
		Socket * socket = new Socket(srvrIP, srvrPort, 1);
		socket->NewSocket(sendLines, receivedLines);
		if(!socket->IsError()) {
			//SaveTxtFile(fileNameOut, receivedLines);
		}
		delete socket;
		receivedArgs.insert( receivedArgs.end(), sendLines.begin(), sendLines.end() );
		receivedArgs.insert( receivedArgs.end(), receivedLines.begin(), receivedLines.end() );
		SaveTxtFile(GetPathLogFile(receivedArgs[0], receivedArgs[4]).c_str(), receivedArgs);
		DisplayResourceNAMessageBox(GetMessageFromJson(receivedLines));
	}
	else {
		DisplayResourceNAMessageBox(L"TCPClientw.exe Невірна кількість параметрів");
	}
	return 0;
}

void LoadTxtFile (const char * fileNameIn, vector<string> & send) {
	string str;
	std::ifstream in(fileNameIn);
	while (std::getline(in, str))
	{
		if(str.size() > 0)
			send.push_back(str);
	}
}

void SaveTxtFile (const char* filename, vector<string> & send) {
	std::ofstream output_file(filename);
    std::ostream_iterator<std::string> output_iterator(output_file, "\n");
    std::copy(send.begin(), send.end(), output_iterator);
}

int DisplayResourceNAMessageBox(wstring msg_w)
{
	if(!msg_w.length())
		return 0;
	const WCHAR *pwcsName = msg_w.c_str();
    int msgboxID = MessageBox(
        NULL,
        (LPCWSTR)pwcsName,
        (LPCWSTR)L"Увага!",
        MB_OK
    );

    switch (msgboxID)
    {
    case IDCANCEL:
        // TODO: add code
        break;
    case IDTRYAGAIN:
        // TODO: add code
        break;
    case IDCONTINUE:
        // TODO: add code
        break;
    }

    return msgboxID;
}

void SplitStringLine(const std::string& str, vector<string> & cont, const std::string& delims)
{
    std::size_t current, previous = 0;
    current = str.find_first_of(delims);
    while (current != std::string::npos) {
        cont.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find_first_of(delims, previous);
    }
    cont.push_back(str.substr(previous, current - previous));
}

string GetPathLogFile(string path, string insp) {
	vector<string> splitPath;
	splitPath.clear();
	SplitStringLine(path, splitPath);
	string pathIni;
	pathIni.empty();
	splitPath.pop_back();
	for (std::vector<string>::iterator it = splitPath.begin() ; it != splitPath.end(); ++it) {
		pathIni += *it;
		pathIni += "\\";
	}
	pathIni += "TCPClientW";
	pathIni += insp;
	pathIni += ".log";
	return pathIni;
}

wstring GetMessageFromJson(vector<string> & receive) {
	wstring wstr;
	vector<string> tempStr;
	tempStr.clear();
	wstring tempWstr;
	tempWstr.empty();
	size_t pos = 0;
	string strAmount = "0";
	wstring wstrAmount;
	int amount = 0;
	for (std::vector<string>::iterator it = receive.begin() ; it != receive.end(); ++it) { //прохід по всіх строках вектора
		std::transform((*it).begin(), (*it).end(), (*it).begin(), ::toupper);
		pos = (*it).find_last_of("}");
		if(pos != string::npos)
			(*it).erase ((*it).begin() + pos, (*it).end()); //якщо знайдено, то видалити всі символи від останнього } до кіня строки
		(*it).erase(std::remove((*it).begin(), (*it).end(), ' '), (*it).end());
		(*it).erase(std::remove((*it).begin(), (*it).end(), '{'), (*it).end());
		(*it).erase(std::remove((*it).begin(), (*it).end(), '}'), (*it).end());
		(*it).erase(std::remove((*it).begin(), (*it).end(), '[ '), (*it).end());
		(*it).erase(std::remove((*it).begin(), (*it).end(), ']'), (*it).end());
		(*it).erase(std::remove((*it).begin(), (*it).end(), '\"'), (*it).end());
		SplitStringLine(*it, tempStr, ",");
		tempWstr.clear();
		//прохід по парах - ім'я поля : значення
		for (std::vector<string>::iterator it2 = tempStr.begin() ; it2 != tempStr.end(); ++it2) {
			pos = (*it2).find("SYSTEM_NAME:EM");
			if(pos != string::npos)
				tempWstr = L"Необхідно отримати нові заяви на допомогу при народженні дитини.\r\n";
			pos = (*it2).find("CNT:");
			if(pos != string::npos) {
				strAmount = (*it2).substr(pos+4);
				try {
					stringstream tmpStr(strAmount);
					tmpStr >> amount;
				}
				catch(...) {
					amount = 0;
				}
				if(amount) {
					tempWstr.append(L"Є ");
					wstrAmount.assign(strAmount.begin(), strAmount.end());
					tempWstr.append(wstrAmount);
					tempWstr.append(L" нових заяв(и).");
					wstr.append(tempWstr);
				}
			}			
		}
	}
	return wstr;
}