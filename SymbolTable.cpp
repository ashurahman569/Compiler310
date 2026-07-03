#include<bits/stdc++.h>
using namespace std;

class SymbolInfo{ 
    string name;
    string type;
    string retType;
    string* args;
    string* fields;
    string* fieldTypes;
    int numFields;
    int numArgs;
    public:
    SymbolInfo* next;
    SymbolInfo(){
        this->name = "";
        this->type = "";
        this->retType = "";
        this->args = nullptr;
        this->fields = nullptr;
        this->fieldTypes = nullptr;
        this->numFields = 0;
        this->numArgs = 0;
        this->next = nullptr;
    }
    SymbolInfo(string name, string full){
        this->name = name;
        this->type = "";
        this->retType = "";
        this->args = nullptr;
        this->fields = nullptr;
        this->fieldTypes = nullptr;
        this->numFields = 0;
        this->numArgs = 0;
        this->next = nullptr;
        addType(full);        
    }
    SymbolInfo(const SymbolInfo &symbol){
        this->name = symbol.name;
        this->type = symbol.type;
        this->retType = symbol.retType;
        this->args = nullptr;
        this->fields = nullptr;
        this->fieldTypes = nullptr;
        this->numFields = 0;
        this->numArgs = 0;
        this->next = nullptr;

        for (int i = 0; i < symbol.numArgs; i++) {
            addArgs(symbol.args[i]);
        }
        for (int i = 0; i < symbol.numFields; i++) {
            addField(symbol.fieldTypes[i], symbol.fields[i]);
        }
    }
    ~SymbolInfo(){
        delete[] args;
        delete[] fields;
        delete[] fieldTypes;
    }
    string getName(){
        return name;
    }
    string getType(){
        return type;
    }
    void setName(string name){
        this->name = name;
    }
    void setType(string type){
        this->type = type;
    }
    void setRetType(string retType){
        this->retType = retType;
    }
    void addArgs(string arg){
        numArgs++;
        string* newArgs = new string[numArgs];
        for (int i = 0; i < numArgs - 1; i++) {
            newArgs[i] = args[i];
        }
        newArgs[numArgs - 1] = arg;
        delete[] args;
        args = newArgs;
    }
    void addType(string full){
        istringstream iss(full);
        string tok;
        string tok2;
        if (iss >> tok) {
            setType(tok);
            if (tok == "FUNCTION") {
                if (iss >> tok) {
                    setRetType(tok);
                }
                while (iss >> tok) {
                    addArgs(tok);
                }
            } else if (tok == "STRUCT" || tok == "UNION") {
                while (iss >> tok) {                    
                    if (iss >> tok2) addField(tok, tok2);
                }

            } else {
                setType(full);
            }
        }
    }
    void addField(string fieldType, string fieldName){
        numFields++;
        string* newFields = new string[numFields];
        string* newFieldTypes = new string[numFields];
        for (int i = 0; i < numFields - 1; i++) {
            newFields[i] = fields[i];
            newFieldTypes[i] = fieldTypes[i];
        }
        newFields[numFields - 1] = fieldName;
        newFieldTypes[numFields - 1] = fieldType;
        delete[] fields;
        delete[] fieldTypes;
        fields = newFields;
        fieldTypes = newFieldTypes;
    }
    string getString(){
        string result = name + "," + type;
        if (type == "FUNCTION") {
            result += "," + retType+"<==(";
            if (numArgs > 0) {
                result += args[0];
            }
            for (int i = 1; i < numArgs; i++) {
                result += "," + args[i];
            }
            result += ")";
        } else if (type == "STRUCT" || type == "UNION") {
            result+= ",{";
            if(numFields > 0){
                result += "(" + fieldTypes[0] + "," + fields[0] + ")";
            }
            for (int i = 1; i < numFields; i++) {
                result += ",(" + fieldTypes[i] + "," + fields[i] + ")";
            }
            result += "}";
        }
        return result;
    }
};

class ScopeTable{
    SymbolInfo** table;
    static int number;
    int scopeID;
    int numBuckets;
    string id;
    unsigned int SDBMHash(string str, unsigned int num_buckets) {
        unsigned int hash = 0;
        unsigned int len = str.length();
        for (unsigned int i = 0; i < len ; i ++){
            hash = ((str[i]) + (hash << 6) + (hash << 16) - hash) % num_buckets ;
        }
        return hash ;
    }
    public:
    ScopeTable *parentScope;
    ScopeTable(int numBuckets, ScopeTable *parentScope){
        this->numBuckets = numBuckets;
        number++;
        scopeID = number;
        this->parentScope = parentScope;
        table = new SymbolInfo*[numBuckets];
        for(int i=0; i<numBuckets; i++){
            table[i] = nullptr;
        }
        cout<<"\tScopeTable# "<<scopeID<<" created"<<endl;
    }
    ~ScopeTable (){
        for(int i=0; i<numBuckets; i++){
            SymbolInfo* entry = table[i];
            while(entry != nullptr){
                SymbolInfo* temp = entry;
                entry = entry->next;
                delete temp;
            }
        }
        delete[] table;
    }   
    int getScopeID(){
        return scopeID;
    }
    bool Insert(SymbolInfo symbol){
        unsigned int index = SDBMHash(symbol.getName(), numBuckets);
        int n = 1;
        SymbolInfo* ptr = table[index];
        if (ptr == nullptr){
            SymbolInfo* newSymbol = new SymbolInfo(symbol);
            table[index] = newSymbol;
            cout << "\tInserted in ScopeTable# "<<scopeID<<" at position "<<index+1<<", "<<n<<endl;
            return true;
        }
        else {
            while(ptr->next != nullptr){
                if(ptr->getName() == symbol.getName()){
                    cout << "\t'" << symbol.getName() << "' already exists in the current ScopeTable" << endl;
                    return false; // Symbol already exists
                }
                ptr = ptr->next;
                n++;
            }
            if(ptr->getName() == symbol.getName()){
                cout << "\t'" << symbol.getName() << "' already exists in the current ScopeTable" << endl;
                return false; // Symbol already exists
            }
            ptr->next = new SymbolInfo(symbol);
            cout << "\tInserted in ScopeTable# "<<scopeID<<" at position "<<index+1<<", "<<n+1<<endl;
        }

        return true;
    }

    SymbolInfo* Lookup(string name){
        unsigned int index = SDBMHash(name, numBuckets);
        int n = 1;
        SymbolInfo* ptr = table[index];
        while(ptr != nullptr){
            if(ptr->getName() == name){
                cout << "\t" << "'" << name << "'"<<" found in ScopeTable# "<<scopeID<<" at position "<<index+1<<", "<<n<<endl;
                return ptr; //found
            }
            ptr = ptr->next;
            n++;
        }

        if (scopeID == 1) {
            cout << "\t" << "'"<< name <<"'"<< " not found in any of the ScopeTables"<<endl;
            return nullptr; //not found
        } else{ 
            return parentScope->Lookup(name); 
        }
    }

    bool Delete(string name){
        unsigned int index = SDBMHash(name, numBuckets);
        SymbolInfo* ptr = table[index];
        int n = 1;
        SymbolInfo* prev = nullptr;
        while(ptr != nullptr){
            if(ptr->getName() == name){
                if(prev == nullptr){
                    table[index] = ptr->next;
                } else {
                    prev->next = ptr->next;
                }
                cout << "\tDeleted " << "'" << name << "'"<<" from ScopeTable# "<<scopeID<<" at position "<<index+1<<", "<<n<<endl;
                delete ptr;
                return true; //deleted
            }
            prev = ptr;
            ptr = ptr->next;
            n++;
        }
        cout << "\t" << "Not found in the current ScopeTable" << endl;
        return false; //not found
    }
    void Print(int indent){
        cout << string(indent, ' ') << "ScopeTable# " << scopeID << endl;
        for(int i=0; i<numBuckets; i++){
            cout << string(indent, ' ') << i+1 << "--> ";
            SymbolInfo* ptr = table[i];
            if(ptr != nullptr){                
                while(ptr != nullptr){
                    cout << "<" << ptr->getString() << "> ";
                    ptr = ptr->next;
                }                
            }
            cout << endl;
        }
    }

};
int ScopeTable::number = 0;

class SymbolTable{
    ScopeTable *currentScope;
    public:
    SymbolTable(int numBuckets){
        currentScope = new ScopeTable(numBuckets, nullptr);
    }
    ~SymbolTable(){
        while(currentScope != nullptr){
            ScopeTable* temp = currentScope;
            currentScope = currentScope->parentScope;
            delete temp;
        }
    }
    void EnterScope(int numBuckets){
        ScopeTable *newScope = new ScopeTable(numBuckets, currentScope);
        currentScope = newScope;
    }
    bool ExitScope(int& cmdcount){
        if(currentScope->parentScope != nullptr){ //can't exit root scope
            ScopeTable *temp = currentScope;
            currentScope = currentScope->parentScope;
            int ID = temp->getScopeID();
            cout << "Cmd " << cmdcount++ << ": E" << endl;
            cout << "\tScopeTable# "<<ID<<" removed" << endl;
            delete temp;
            return true;
        }
        else{
            //cout << "\tCannot exit root scope" << endl;
            return false;
        }
    }
    bool ExitScope(){
        if(currentScope->parentScope != nullptr){
            ScopeTable *temp = currentScope;
            currentScope = currentScope->parentScope;
            int ID = temp->getScopeID();
            cout << "\tScopeTable# "<<ID<<" removed" << endl;
            delete temp;
            return true;
        }
        else{
            int ID = currentScope->getScopeID();
            cout << "\tScopeTable# "<<ID<<" removed" << endl;
            delete currentScope;
            currentScope = nullptr;
            return false;
        }
    }
    bool Insert(string name, string type){
        SymbolInfo symbol(name, type);
        return currentScope->Insert(symbol);
    }
    bool Remove(string name){
        return currentScope->Delete(name);
    }
    SymbolInfo* Lookup(string name){
        return currentScope->Lookup(name);
    }
    void PrintCurrent(){
        currentScope->Print(4);
    }
    void PrintAll(){
        ScopeTable *temp = currentScope;
        int indent = 4;
        while(temp != nullptr){
            temp->Print(indent);
            indent = indent + 4;
            temp = temp->parentScope;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) { 
        cerr << "Please give two files as arguments" << endl;
        return 1; 
    }
    ifstream in(argv[1]); 
    ofstream out(argv[2]);
    if (!in.is_open()) { 
        cerr << "Cannot open " << argv[1] << endl; 
        return 1; 
    }
    if (!out.is_open()) { 
        cerr << "Cannot open " << argv[2] << endl; 
        return 1; 
    }
 
    streambuf* buf = cout.rdbuf(); 
    cout.rdbuf(out.rdbuf());
 
    int n; 
    in >> n; 
    in.ignore();
    SymbolTable symbolTable(n);
 
    int cmdcount = 1; 
    string line;
    while (getline(in, line)) {
        while (!line.empty() && isspace((unsigned char)line.back())) line.pop_back();
        if (line.empty()) continue;        
 
        istringstream iss(line);
        char cmd; 
        iss >> cmd;
 
        if (cmd == 'Q') {
            cout << "Cmd " << cmdcount++ << ": " << line << endl;
            while (symbolTable.ExitScope());
            break;
        }
 
        if (cmd == 'E') {
            if (symbolTable.ExitScope(cmdcount));
            continue;
        }
 
        if (cmd == 'P') {
            char sub; iss >> sub;
            if (sub != 'A' && sub != 'C') continue;
            cout << "Cmd " << cmdcount++ << ": " << line << endl;
            if (sub == 'A') symbolTable.PrintAll(); 
            else symbolTable.PrintCurrent();
            continue;
        }
 
        if (cmd == 'I' || cmd == 'L' || cmd == 'D' || cmd == 'S') {
            cout << "Cmd " << cmdcount++ << ": " << line << endl;
        } else {
            //cout << "\tInvalid command: " << cmd << endl;
            continue;
        }
 
        if (cmd == 'I') {
            string name, type;
            if (!(iss >> name) || !(iss >> type)) {
                cout << "\tNumber of parameters mismatch for the command I" << endl; 
                continue;
            }
            string tok, full = type;
            while (iss >> tok) full += " " + tok;
            symbolTable.Insert(name, full);
        } else if (cmd == 'L') {
            string name, extra;
            if (!(iss >> name)) { 
                cout << "\tNumber of parameters mismatch for the command L" << endl; 
                continue; 
            }
            if (iss >> extra) { 
                cout << "\tNumber of parameters mismatch for the command L" << endl; 
                continue; 
            }
            symbolTable.Lookup(name);
        } else if (cmd == 'D') {
            string name, extra;
            if (!(iss >> name)) { 
                cout << "\tNumber of parameters mismatch for the command D" << endl; 
                continue; 
            }
            if (iss >> extra) { 
                cout << "\tNumber of parameters mismatch for the command D" << endl; 
                continue; 
            }
            symbolTable.Remove(name);
        } else if (cmd == 'S') {
            symbolTable.EnterScope(n);
        }
    }

    cout.rdbuf(buf); 
    in.close(); 
    out.close();
    return 0;
}

    
