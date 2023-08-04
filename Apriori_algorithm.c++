#include<iostream>
#include<omp.h>
#include<stdio.h>
#include <fstream>
#include <bits/stdc++.h>
#include <string>
#include <cmath>
#include <chrono>
#define totalThreads 2

using namespace std::chrono;
using namespace std;

bool isCheck(string a, string b){
	map<char, int> occur1;
	for(int i=0;i<a.length();i++)
		occur1[a[i]]++;
	map<char, int> occur2;
	for(int i=0;i<b.length();i++)
		occur2[b[i]]++;
	if(occur1 == occur2)
		return true;
	else
		return false;
}

class HashNode {
public:
    int intKey;
    string stringKey;
    bool approve;
    int count;
	  
  	HashNode(){
        this->stringKey = "-1";
        approve=0;
        count=0;
    }
};

class HashMap{
	public:
		int capacity;
		HashNode *table;
		HashMap(int totalItems){
			table=new HashNode[totalItems];
			this->capacity=totalItems;
		}
};

//finding all possible combinations
int totalCombinations=0;//ye dekhne keliyea ke humne kitne combinations banaye hain 
void Combi(string a[], int reqLen, int s, int currLen, bool check[], int l,string possibleCombinations[]){
   if(currLen > reqLen)
   	return;
   else if (currLen == reqLen) {
      for (int i = 0; i < l; i++) {
        if (check[i] == true) {
            possibleCombinations[totalCombinations].append(a[i]);
        }
      }
      totalCombinations++;
      return;
   }
   if (s == l) {
      return;
   }
   check[s] = true;
   Combi(a, reqLen, s + 1, currLen + 1, check,l,possibleCombinations);
   check[s] = false;
   Combi(a, reqLen, s + 1, currLen, check,l,possibleCombinations);
}


int main(){
	auto start = high_resolution_clock::now();
	string temp;
	string localUnique[totalThreads];//har thread apni local unique ko apne index par rakhe ga
	int totalLines=0,supportCount=1;
	ifstream readFile;
	readFile.open("input.txt");  
	
	while (getline (readFile, temp))
		totalLines++;
	
	string transactions[totalLines];
	ifstream readFileTwo;
	readFileTwo.open("input.txt");  
	for(int i=0;i<totalLines;i++){
		getline (readFileTwo, temp);
		transactions[i]=temp;
	}
	
	int partitionTransaction=totalLines/totalThreads;
	//har thread apni transactions main se unique item nikale ga aur apne local buffer main add kardega 
	#pragma omp parallel num_threads(totalThreads)
	{
		int tId=omp_get_thread_num();
		int startIndex=tId*partitionTransaction;
		int endIndex=startIndex+partitionTransaction-1;
		string word,temp;
		//printf("tId %d startIndex %d endIndex %d\n",tId,startIndex,endIndex);
		for(int i=startIndex;i<=endIndex;i++){
			temp=transactions[i];
			stringstream transactionTemp(temp);
			while (transactionTemp.good()) {
		        getline(transactionTemp, word, ',');
		        size_t found = localUnique[tId].find(word);
		        if (found == string::npos){
					localUnique[tId].append(word);
					localUnique[tId].append(",");
				}
			}
		}
		localUnique[tId] = localUnique[tId].substr(0, localUnique[tId].size()-1);
	}
	
	//ab master local results main se global result generate kare ga
	string word,uniqueItemsTemp;
	int uniqueItemsCount=0;
	
	for(int i=0;i<totalThreads;i++){
		stringstream transactionTemp(localUnique[i]);
		while (transactionTemp.good()) {
	        getline(transactionTemp, word, ',');
	        size_t found = uniqueItemsTemp.find(word);
	        if (found == string::npos){
				uniqueItemsTemp.append(word);
				uniqueItemsTemp.append(",");
				uniqueItemsCount++;
			}
		}
	}
	uniqueItemsTemp = uniqueItemsTemp.substr(0, uniqueItemsTemp.size()-1);
	string uniqueItems[uniqueItemsCount];
	stringstream items(uniqueItemsTemp);
	int index=0;
	while (items.good()) {
		getline(items, word, ',');
		uniqueItems[index]=word;
		uniqueItems[index++].append(",");
	}
	
	int memoryRequired= pow(2, uniqueItemsCount);
	string possibleCombinations[memoryRequired];//ye har possible combination ko store karne keliyea hai
	
	//filling all possible combinations array
	bool check[uniqueItemsCount]={false};
	for(int i = 1; i <= uniqueItemsCount; i++) {
		Combi(uniqueItems, i, 0, 0, check, uniqueItemsCount,possibleCombinations);
	}
	HashMap hash(totalCombinations);//jitne combinations hain utne ka table banado
	
	//insert data in parallel
	#pragma omp parallel for schedule(runtime) num_threads(totalThreads)
	for(int i=0;i<totalCombinations;i++){
		possibleCombinations[i] = possibleCombinations[i].substr(0, possibleCombinations[i].length() - 1);
		hash.table[i].stringKey=possibleCombinations[i];
	}
	
	//check each transaction in parallel
	#pragma omp parallel for schedule(runtime) num_threads(totalThreads)
	for(int i=0;i<totalLines;i++){
		string transactionEntry=transactions[i];
		//ab is transaction main check kareinge ke kon kon si hash map entry hain
		for(int j=0;j<hash.capacity;j++){
			string combination=hash.table[j].stringKey;
			size_t found = transactionEntry.find(combination);
	        if (found != string::npos)//humare transaction main ye hassh table entry yaani ye combination hai to increase count
	        	hash.table[j].count++;
	        else{//ab hum character by character check kareinge ke A,B == B,A wale combinations ko check kareinge
				int countItems=0,foundCount=0;//kitne items transaction main hain
				stringstream combination(hash.table[j].stringKey);
				string word;
				while (combination.good()){
					getline(combination, word, ',');
					countItems++;
					size_t found = transactionEntry.find(word);
	        		if(found != string::npos)
	        			foundCount++;
	        	}
	        	if(countItems == foundCount)
        			hash.table[j].count++;
        	}
			if(hash.table[j].count == supportCount)
				hash.table[j].approve=1;
		}	
	}
	
	
	#pragma omp parallel for schedule(runtime) num_threads(totalThreads)
	for(int i=0;i<totalCombinations;i++){
		if(hash.table[i].approve){
			#pragma omp critical
			{
				cout<<hash.table[i].stringKey<<" "<<hash.table[i].count<<endl;		
			}
		}
	}
	
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
 	cout << duration.count() << endl;
	return 0;
}