#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
using namespace std;

const int IMAGE_SIZE = 1440 * 1024;
const char * red = "\033[31m";
const char * white = "\033[0m";
const char * DefaultOutput = "<";
const char * CmdEmpty = "The command is Empty!\n";
const char * ExitMore = "EXIT command should have only one exit and no other input!\n";
const char * CatShort = "CAT command requires a filename!\n";
const char * CatLong = "CAT command requires only one filename!\n";
const char * Unkown = "It is Unknown command!\n";
const char * PathWrong = "It do not exist this directory or file\n";
const char * PathWrongCat = "It do not exist this file\n";
const char * CatDirWrong = "This is a Directory\n";
const char * LsOpWrong = "Invalid option\n";
const char * LsMore = "LS requires only one filename or directory name\n";
const char * PathNotInvalid = "This is an invalid path!\n";
vector< vector<string> > relPath;
bool isFile = false;
bool isFileFirst = false;
extern "C" {
    void myprint(const char *);
}

void setRed(){
    myprint(red);
}

void setWhite(){
    myprint(white);
}

void redPrint(const char *s) {
    setRed();
    myprint(s);
    setWhite();
//    cout << red;
//    cout << s;
//    cout << white;
}

void whitePrint(const char *s) {
    myprint(s);
    //cout<<s;
}

//小端存储转成数字
int charToInt (const char* a, int begin, int size) {
    int res = 0;
    for (int i = begin + size - 1; i >= begin; i--) {
        res = (res << 8) +  (unsigned char) a[i];
    }
    return res;
}

void splitPath(const string &path, vector<string>* paths){
    string fileName = "/";
    for (char c: path) {
        if (c == '/'){//可以解决多个/问题
            if(!fileName.empty()){//保证推进去的不为空
                if (fileName == "."){//.表示当前目录
                    fileName = "";
                    continue;
                }else if (fileName == ".." && !paths->empty()){
                    vector<string> temp;
                    for (const string& pathBeforeRel : *paths) {
                        temp.push_back(pathBeforeRel);
                    }
                    relPath.push_back(temp);
                    paths->pop_back();
                }else if (fileName != ".."&&fileName!="/"){// '/'是默认值
                    paths->push_back(fileName);
                }
                fileName = "";
            }
        } else{
            fileName += c;
        }
    }
}

string getFileName(const char *filename){
    string name;
    string type;
    for (int i = 0; i < 8; i++) {
        if (filename[i] == ' '){
            break;
        }
        name += filename[i];
    }
    if (filename[8] != ' ' ){
        for (int i = 8; i < 11; i++) {
            type += filename[i];
        }
    }
    if (type.empty()){
        return name;
    } else{
        return name + "." + type;
    }
}

bool compare(const char* name1, const string& name2){
    if (name2.size()>12){
        return false;
    }
    string name1Change = getFileName(name1);
    if(name1Change == name2){
        return true;
    } else{
        return false;
    }
}

bool IsNotDot(const char* filename){
    string name = getFileName(filename);
    if (name == "." || name == ".."){
        return false;
    }
    return true;
}

struct rootEntry {
    char filename[11];
    char classification[1];
    char reserves[10]; //保留位
    char wrtTime[2];
    char wrtDate[2];
    char firstCluster[2];
    char fileSize[4];
};

class Image{
private:
    char data[IMAGE_SIZE];
    int BytsPerSec; // 每个扇区的字节数
    int SecPerClus; // 每簇扇区数
    int RsvdSecCnt; // Boot记录占用的扇区数
    int NumFATs; // FAT表数 
    int RootEntCnt; // 根目录最大文件数
    int TotSec16; // 扇区数
    int FATSz16; // 一个FAT的扇区数
    int FATstartSector;
    int rootStartSector;
    int dataStartSector;
    vector<rootEntry> rootEntries;
public:
    Image(const char* path);
    void readData(char * target, int begin_sec, int begin, int size);
    void readBoot(char * target, int size);
    void readCluster(char * target, int clu);
    int readFat(int clus);
    bool readFile(vector<string> paths, string * content);
    bool getFileEntry(vector<string> paths, vector<rootEntry>* fileEntries);
} image("./lab2.img");


Image::Image(const char* path){
    //读取a.img
    FILE *fptr = fopen(path,"r");
    fread(data, IMAGE_SIZE, 1, fptr);
    fclose(fptr);

    //读取引导扇区
    char bootDate[512];
    readBoot(bootDate, 512);
    BytsPerSec = charToInt(bootDate,11,2);//每扇区字节数
    SecPerClus = charToInt(bootDate,13,1);//每簇扇区数
    RsvdSecCnt = charToInt(bootDate,14,2);//bootrecord占用扇区数
    NumFATs = charToInt(bootDate,16,1);//FAT数量
    RootEntCnt = charToInt(bootDate,17,2);//根目录最大文件数
    TotSec16 = charToInt(bootDate,19,2);//扇区数
    FATSz16 = charToInt(bootDate,22,2);//一个FAT扇区数
    FATstartSector = RsvdSecCnt;//(1)
    rootStartSector = FATstartSector + NumFATs*FATSz16;//(19)
    int rootSec =(int ) (sizeof(rootEntry) * RootEntCnt + BytsPerSec - 1) / BytsPerSec;//进行上取整(14)
    dataStartSector = rootStartSector + rootSec;//(33)

    //读取根目录区
    for(int i = 0; i < RootEntCnt; i++) {
        char rootEntryData[sizeof(rootEntry)];
        rootEntry tempRootEntry{};
        readData(rootEntryData,rootStartSector,i*sizeof(rootEntry),sizeof(rootEntry));
        memcpy(&tempRootEntry,rootEntryData,sizeof(rootEntry));
        int attr = charToInt(tempRootEntry.classification,0,sizeof tempRootEntry.classification);
        if(attr == 0x10 || attr == 0x20) {
            rootEntries.push_back(tempRootEntry);
        }
    }
}
//读取引导扇区
void Image::readBoot(char * target, int size){
    for(int i = 0; i < size; i++) {
        target[i] = data[i];
    }
}
//读取数据ALLKINDS
void Image::readData(char * target, int begin_sec, int begin, int size){
    for (int i = 0; i < size; i++) {
        target[i] = data[begin_sec * BytsPerSec + begin + i];
    }
}
//读取FAT
int Image::readFat(const int clus){
    char buffer[2];//12位等于一个半字节
    int offset = clus * 3 / 2;
    readData(buffer, FATstartSector, offset, 2);
    int newClu;
    //偶数：后字节低4位加前字节
    //奇数：后字节加前字节高4位
    if (clus % 2 == 1){
        newClu = (((unsigned char)buffer[1]) << 4) + ((unsigned char)buffer[0]) / 16;
        //奇数：后字节前移4位加前字节后移4位
    }
    else{
        newClu = ((((unsigned char)buffer[1]) % 16) << 8) + ((unsigned char)buffer[0]);
        //偶数：后字节取低4位前移8位加前字节
    }
    if (newClu >= 0xff7 || newClu <= 1){
        newClu = 0;
    }//终结簇也置成0
    return newClu;
}
//读取数据区
void Image::readCluster(char * target, int clu){
    readData(target, dataStartSector+clu-2, 0, SecPerClus*BytsPerSec);
}//注意簇数-2是数据区
//根据路径读取
bool Image::getFileEntry(vector<string> paths, vector<rootEntry>* fileEntries){
    fileEntries->clear();
    for (rootEntry rootEntry: rootEntries) {
        fileEntries->push_back(rootEntry);
    }//加载根目录
    if (paths.empty()){
        return true;//根目录
    }
    for (int i = 0; i < paths.size(); i++) {
        string fileName = paths[i];
        rootEntry *entry = nullptr;
        for (rootEntry fileEntry: *fileEntries) {
            if(compare(fileEntry.filename,fileName)){
                entry = &fileEntry;
                break;
            }
        }
        fileEntries->clear();//清空，为了读当前目录下的所有文件
        if(entry == nullptr){
            return false;
        }//没有此目录或者文件
        int attr = charToInt(entry->classification,0,sizeof (entry->classification));
        if (attr == 0x10) {
            int nextClu = charToInt(entry->firstCluster,0,sizeof (entry->firstCluster));
            char buf[BytsPerSec];
            while (nextClu > 0){
                readCluster(buf,nextClu);
                nextClu = readFat(nextClu);
                for (int j = 0; j < BytsPerSec/sizeof (rootEntry); j++) {
                    rootEntry temp{};
                    memcpy(&temp,buf+j*sizeof (rootEntry),sizeof (rootEntry));
                    int attr2 = charToInt(temp.classification,0,sizeof (temp.classification));
                    int nextClu2 = charToInt(temp.filename,0,1);
                    if (nextClu2 == 0xE5){
                        continue;
                    }
                    if (attr2 != 0x10 && attr2 != 0x20){
                        continue;
                    }
                    fileEntries->push_back(temp);
                }
            }
        } else if (attr == 0x20 && i+1 == paths.size()){//防止文件被当作目录
            fileEntries->push_back(*entry);
            isFile = true;
            return true;
        } else{
            return false;
        }
    }
    return true;
}

bool Image::readFile(vector<string> paths, string *content) {
    vector<rootEntry> fileEntries;
    bool pathError = getFileEntry(std::move(paths),&fileEntries);
    if (!pathError){
        whitePrint(PathWrongCat);//不存在此文件
        return false;
    }
    if (fileEntries.size() != 1){
        whitePrint(CatDirWrong);
        return false;
    }
    rootEntry temp = fileEntries[0];
    int attr = charToInt(temp.classification,0,sizeof temp.classification);
    if (attr != 0x20){
        whitePrint(CatDirWrong);
        return false;
    }
    *content = "";
    int fileSize = charToInt(temp.fileSize,0,sizeof (temp.fileSize));
    int nextClus = charToInt(temp.firstCluster,0,sizeof (temp.firstCluster));
    char buf[BytsPerSec];
    while (nextClus > 0){
        readCluster(buf,nextClus);
        nextClus = readFat(nextClus);
        for (int i = 0; i < BytsPerSec && fileSize > 0; i++,fileSize--) {
            *content += buf[i];
        }
    }
    return true;
}

string classical(const vector<string>& path){
    string outcome = "/";
    for (const auto & i : path) {
        outcome += i + "/";
    }
    return outcome;
}

string nameToPath(string s){
    while (s.find("//") != string::npos) {
        s = s.replace(s.find("//"), 2, "/");
    }
    if(s[0] != '/'){
        return '/' + s;
    }
    return s;
}

bool handleRelPathError(){
    for (const auto & i : relPath) {
        vector<rootEntry> fileEntries;
        if (!image.getFileEntry(i,&fileEntries)){
            return false;
        }
    }
    return true;
}

void countSub(vector<string> path){
    vector<rootEntry> fileEntries;
    image.getFileEntry(std::move(path),&fileEntries);
    int fileNum = 0;
    int dirNum = 0;
    for (auto & fileEntry : fileEntries) {
        int attr = charToInt(fileEntry.classification,0, sizeof fileEntry.classification);
        if (attr == 0x10){
            if (IsNotDot(fileEntry.filename)){
                dirNum++;
            }
        } else{
            fileNum++;
        }
    }
    string output = " "+ to_string(dirNum) + " " + to_string(fileNum);
    whitePrint(output.c_str());
}

int main() {
    string putIn;
    while (true){
        isFile = false;//初始化
        whitePrint(DefaultOutput);//打印 <
        if (!getline(cin,putIn)){
            break;
        }//结束读入任务
        vector<string> cmd;
        istringstream stream(putIn);
        string word;
        while (stream >> word){//遇到空格换行忽略
            cmd.push_back(word);
        }
        if (cmd.empty()){
            whitePrint(CmdEmpty);
            continue;
        }
        if (cmd[0] == "ls"){
            bool flagOpWrong = false;
            bool lsOp = false;
            vector<string> cmdSimplify;
            for (auto & i : cmd){
                if (i[0] == '-'){
                    lsOp = true;
                    for (int j = 1; j < i.size(); j++){
                        if (i[j] != 'l'){
                            whitePrint(LsOpWrong);
                            flagOpWrong = true;
                            break;
                        }//处理-lll、多个-l、-后错误
                    }
                    if (flagOpWrong){
                        break;
                    }
                }else{
                    cmdSimplify.push_back(i);
                }
            }
            if (flagOpWrong){
                    continue;
            }
            string name;
            if(cmdSimplify.size() == 2) {
                    name = cmdSimplify[1]+"/";
            }else if(cmdSimplify.size() == 1) {
                name = "/";
            }else {
                whitePrint(LsMore);
                continue;
            }
            string path = nameToPath(name);
            vector<rootEntry> fileEntries;
            vector<string> paths;
            relPath.clear();
            splitPath(path,&paths);
            if (!handleRelPathError()){
                whitePrint(PathNotInvalid);
                relPath.clear();
                continue;
            }//处理..问题
            bool isGetFileEntry = image.getFileEntry(paths,&fileEntries);
            if (!isGetFileEntry){
                whitePrint(PathWrong);
                continue;
            }
            vector<vector<string>> AllFile;
            AllFile.push_back(paths);
            vector<string> currentPath;
            isFileFirst = isFile;
            while (!AllFile.empty()){
                currentPath = AllFile.front();
                for (int i = 1; i < AllFile.size(); i++) {
                    AllFile[i - 1] = AllFile[i];
                }
                AllFile.pop_back();
                string PathFull = classical(currentPath);
                if (!isFileFirst) {
                    whitePrint(PathFull.c_str());
                    if (lsOp) {
                        countSub(currentPath);
                    }
                    whitePrint(":\n");
                }
                image.getFileEntry(currentPath,&fileEntries);
                for (rootEntry rootEntry:fileEntries) {
                    int attr = charToInt(rootEntry.classification,0,sizeof (rootEntry.classification));
                    if (attr == 0x10){
                        if (lsOp){
                            redPrint(getFileName(rootEntry.filename).c_str());//红
                            if (IsNotDot(rootEntry.filename)){
                                vector<string> subPath;
                                for (const auto & i : currentPath) {
                                    subPath.push_back(i);
                                }
                                subPath.push_back(getFileName(rootEntry.filename));
                                countSub(subPath);
                            }
                            whitePrint("\n");
                        } else{
                            redPrint(getFileName(rootEntry.filename).c_str());
                            whitePrint("  ");
                        }
                        if (IsNotDot(rootEntry.filename)){
                            vector<string> subPath;
                            for (const auto & i : currentPath) {
                                subPath.push_back(i);
                            }
                            subPath.push_back(getFileName(rootEntry.filename));
                            AllFile.push_back(subPath);
                        }
                    } else{
                        if(lsOp){
                            whitePrint(getFileName(rootEntry.filename).c_str());
                            whitePrint("  ");
                            whitePrint(to_string(charToInt(rootEntry.fileSize,0,sizeof (rootEntry.fileSize))).c_str());//输出文件字节
                            if (!isFileFirst){
                                whitePrint("\n");
                            }
                        } else{
                            whitePrint(getFileName(rootEntry.filename).c_str());
                            whitePrint("  ");
                        }
                    }
                }
                whitePrint("\n");
            }
        }else if (cmd[0] == "cat"){
            if (cmd.size() == 1){
                whitePrint(CatShort);//没有文件名
            }else if (cmd.size() > 2){
                whitePrint(CatLong);//输入多余option
            }else{
                string name = cmd[1] + "/";
                string path = nameToPath(name);
                vector<string> paths;
                relPath.clear();
                splitPath(path,&paths);
                if (!handleRelPathError()){
                    whitePrint(PathNotInvalid);
                    relPath.clear();
                    continue;
                }//处理..问题
                string data;
                bool readFileError = image.readFile(paths,&data);
                if (!readFileError){
                    continue;
                } else{
                    whitePrint(data.c_str());
                    whitePrint("\n");
                }
            }
        }else if (cmd[0] == "exit"){
            if (cmd.size() != 1){
                whitePrint(ExitMore);
                continue;
            }
            break;
        }else{
            whitePrint(Unkown);
        }
    }
    return 0;
}
