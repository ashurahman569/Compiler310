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
        string result = name + " : " + type;
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
    string scopeID;
    int childNum;
    int numBuckets;
    
    string id;
    unsigned int SDBMHash(string str, unsigned int num_buckets = 7) {
        unsigned int hash = 0;
        unsigned int len = str.length();
        for (unsigned int i = 0; i < len ; i ++){
            hash = ((str[i]) + (hash << 6) + (hash << 16) - hash);
        }
        return hash % num_buckets ;
    }
    public:
    int entries;
    ScopeTable *parentScope;
    ScopeTable(int numBuckets, ScopeTable *parentScope){
        this->numBuckets = numBuckets;
        entries=0;
        if (parentScope==nullptr){
            scopeID = "1";
            childNum = 0;
        }
        else{
            parentScope->childNum++;            
            string num = to_string( parentScope->childNum);
            scopeID = parentScope->scopeID + "."+num ;
        }
        this->parentScope = parentScope;
        table = new SymbolInfo*[numBuckets];
        for(int i=0; i<numBuckets; i++){
            table[i] = nullptr;
        }
        //cout<<"\tScopeTable# "<<scopeID<<" created"<<endl;
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
    string getScopeID(){
        return scopeID;
    }
    bool Insert(SymbolInfo symbol){
        unsigned int index = SDBMHash(symbol.getName(), numBuckets);
        int n = 0;
        SymbolInfo* ptr = table[index];
        if (ptr == nullptr){
            SymbolInfo* newSymbol = new SymbolInfo(symbol);
            table[index] = newSymbol;
            //cout << "\tInserted in ScopeTable# "<<scopeID<<" at position "<<index<<", "<<n<<endl;
            entries++;
            return true;
            
        }
        else {
            while(ptr->next != nullptr){
                if(ptr->getName() == symbol.getName()){
                    cout <<"< "<< symbol.getString() << " >"<< " already exists in ScopeTable# "<< scopeID<<" at position "<<index<<", "<< n << endl<< endl;
                    return false; // Symbol already exists
                }
                ptr = ptr->next;
                n++;
            }
            if(ptr->getName() == symbol.getName()){
                cout <<"< "<< symbol.getString() << " >"<< " already exists in ScopeTable# "<< scopeID<<" at position "<<index<<", "<< n <<endl<< endl;
                return false; // Symbol already exists
            }
            ptr->next = new SymbolInfo(symbol);
            entries++;
            //cout << "\tInserted in ScopeTable# "<<scopeID<<" at position "<<index+1<<", "<<n+1<<endl;
        }

        return true;
    }

    SymbolInfo* Lookup(string name){
        unsigned int index = SDBMHash(name, numBuckets);
        int n = 1;
        SymbolInfo* ptr = table[index];
        while(ptr != nullptr){
            if(ptr->getName() == name){
                //cout << "\t" << "'" << name << "'"<<" found in ScopeTable# "<<scopeID<<" at position "<<index+1<<", "<<n<<endl;
                return ptr; //found
            }
            ptr = ptr->next;
            n++;
        }

        if (scopeID == "1") {
            //cout << "\t" << "'"<< name <<"'"<< " not found in any of the ScopeTables"<<endl;
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
                //cout << "\tDeleted " << "'" << name << "'"<<" from ScopeTable# "<<scopeID<<" at position "<<index+1<<", "<<n<<endl;
                delete ptr;
                entries--;
                return true; //deleted
            }
            prev = ptr;
            ptr = ptr->next;
            n++;
        }
        //cout << "\t" << "Not found in the current ScopeTable" << endl;
        return false; //not found
    }
    void Print(int indent=0){
        cout << string(indent, ' ') << "ScopeTable # " << scopeID << endl;
        for(int i=0; i<numBuckets; i++){            
            SymbolInfo* ptr = table[i];
            if(ptr != nullptr){ 
                cout << string(indent, ' ') << i << " --> ";               
                while(ptr != nullptr){
                    cout << "< " << ptr->getString() << " >";
                    ptr = ptr->next;
                }
                cout << endl;                
            }
            
        }
    }

    SymbolInfo* getItems(){
        SymbolInfo* array= new SymbolInfo [entries];
        int k=0;
        for (int i =0; i<numBuckets; i++){
            if (table[i]==nullptr) continue;
            else{
                SymbolInfo* ptr= table[i];
                while(ptr!=nullptr){
                    SymbolInfo newSymbol = SymbolInfo(*ptr);
                    array[k]=newSymbol;
                    k++;
                    ptr= ptr->next;
                }
            }
        }
        return array;
    }

};

class SymbolTable{
    ScopeTable *currentScope;
    public:
    SymbolTable(int numBuckets = 7){
        currentScope = new ScopeTable(numBuckets, nullptr);
    }
    ~SymbolTable(){
        while(currentScope != nullptr){
            ScopeTable* temp = currentScope;
            currentScope = currentScope->parentScope;
            delete temp;
        }
    }
    void EnterScope(int numBuckets=7){
        ScopeTable *newScope = new ScopeTable(numBuckets, currentScope);
        currentScope = newScope;
    }
    bool ExitScope(int& cmdcount){
        if(currentScope->parentScope != nullptr){ //can't exit root scope
            ScopeTable *temp = currentScope;
            currentScope = currentScope->parentScope;
            string ID = temp->getScopeID();
            //cout << "Cmd " << cmdcount++ << ": E" << endl;
            //cout << "\tScopeTable# "<<ID<<" removed" << endl;
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
            string ID = temp->getScopeID();
            //cout << "\tScopeTable# "<<ID<<" removed" << endl;
            delete temp;
            return true;
        }
        else{
            string ID = currentScope->getScopeID();
            //cout << "\tScopeTable# "<<ID<<" removed" << endl;
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
        currentScope->Print(0);
    }
    void PrintAll(){
        ScopeTable *temp = currentScope;
        int indent = 0;
        while(temp != nullptr){
            temp->Print(indent);
            indent = indent + 0;
            temp = temp->parentScope;
        }
        cout<<endl;
    }
};

    

